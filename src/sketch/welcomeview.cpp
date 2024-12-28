/*********************************************************************

Part of the Fritzing project - http://fritzing.org
Copyright (c) 2007-2019 Fritzing

Fritzing is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Fritzing is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Fritzing.  If not, see <http://www.gnu.org/licenses/>.

********************************************************************/

#include "welcomeview.h"
#include "debugdialog.h"
#include "help/tipsandtricks.h"
#include "utils/uploadpair.h"
#include "referencemodel/sqlitereferencemodel.h"

#include <QTextEdit>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QPixmap>
#include <QSpacerItem>
#include <QSettings>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QDesktopServices>
#include <QDomDocument>
#include <QDomNodeList>
#include <QDomElement>
#include <QBuffer>
#include <QScrollArea>
#include <QStyleOption>
#include <QStyle>
#include <QApplication>

////////////////////////////////////////////////////////////

constexpr auto TitleRole = Qt::UserRole;
constexpr auto IntroRole = Qt::UserRole + 1;
constexpr auto DateRole = Qt::UserRole + 2;
constexpr auto AuthorRole = Qt::UserRole + 3;
constexpr auto IconRole = Qt::UserRole + 4;
constexpr auto RefRole = Qt::UserRole + 5;
constexpr auto ImageSpace = 65;
constexpr auto TopSpace = 1;

QString WelcomeView::m_activeHeaderLabelColor = "#333";
QString WelcomeView::m_inactiveHeaderLabelColor = "#b1b1b1";

///////////////////////////////////////////////////////////////////////////////

void zeroMargin(QLayout * layout) {
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);
}

QString makeUrlText(const QString & url, const QString & urlText, const QString & color) {
	return QString("<a href='%1' style='font-family:Droid Sans; text-decoration:none; font-weight:bold; color:%3;'>%2</a>").arg(url).arg(urlText).arg(color);
}

QString hackColor(QString oldText, const QString & color) {
	QRegularExpression colorFinder("color:(#[^;]*);");
	QRegularExpressionMatch match;
	if (oldText.contains(colorFinder, &match)) {
		oldText.replace(match.captured(1), color);
	}
	return oldText;
}

int pixelSize(const QString & sizeString) {
	// assume all sizes are of the form Npx otherwise return -1
	QString temp = sizeString;
	temp.remove(" ");
	if (temp.contains("px")) {
		temp.remove("px");
		bool ok;
		int ps = temp.toInt(&ok);
		if (ok) return ps;
	}

	return -1;
}


QString cleanData(const QString & data) {
	static QRegularExpression ListItemMatcher("<li>.*</li>", QRegularExpression::InvertedGreedinessOption | QRegularExpression::MultilineOption | QRegularExpression::DotMatchesEverythingOption);

	QDomDocument doc;
	QStringList listItems;
	int pos = 0;
	QString errorMsg;
	int errorLine;
	int errorColumn;
	while (pos < data.size()) {
		QRegularExpressionMatch match;
		int ix = data.indexOf(ListItemMatcher, pos, &match);
		if (ix < 0) break;

		QString listItem = match.captured(0);
		//DebugDialog::debug("ListItem " + listItem);
		if (doc.setContent(listItem, &errorMsg, &errorLine, &errorColumn)) {
			listItems << listItem;
		} else {
			DebugDialog::debug(QString("Error reading data %1 %2 %3").arg(errorMsg).arg(errorLine).arg(errorColumn));
		}
		pos += listItem.size();
	}
	return listItems.join("");
}

////////////////////////////////////////////////////////////////////////////////

CustomListItem::CustomListItem(const QString &leftText, const QIcon &leftIcon, const QString &leftData,
			       const QString &rightText, const QIcon &rightIcon, const QString &rightData,
			       int listWidgetWidth, QWidget *parent)
	: QWidget(parent), leftData(leftData), rightData(rightData) {
	QHBoxLayout *layout = new QHBoxLayout(this);
	int padding = 3;
	layout->setContentsMargins(2, 2, 2, 2);
	layout->setSpacing(0);

	QList<QSize> availableIconSizes = leftIcon.availableSizes();
	m_iconSize = availableIconSizes.isEmpty() ? QSize(16, 16) : availableIconSizes.first();

	leftButton = new QPushButton(leftIcon, "", this);
	rightButton = new QPushButton(rightIcon, "", this);

	leftButton->setFlat(true);
	rightButton->setFlat(true);

	QString buttonStyle = QString("QPushButton { "
								  "text-align: left; "
								  "background-color: transparent; "
								  "border: none; "
								  "padding-left: %1px; "
								  "padding-right: %1px; "
								  "color: #333;"
								  "}"
								  "QPushButton:pressed { "
								  "color: #005; "
								  "}")
							  .arg(padding);
	leftButton->setStyleSheet(buttonStyle);
	rightButton->setStyleSheet(buttonStyle);

	QFont buttonFont("Droid Sans", 10, QFont::Normal);
	leftButton->setFont(buttonFont);
	rightButton->setFont(buttonFont);

	int scrollbarWidth = this->style()->pixelMetric(QStyle::PM_ScrollBarExtent);
	int leftButtonWidth = static_cast<int>((listWidgetWidth - scrollbarWidth) * 0.7);
	int rightButtonWidth = static_cast<int>((listWidgetWidth - scrollbarWidth) * 0.3);

	leftButton->setFixedWidth(leftButtonWidth);
	rightButton->setFixedWidth(rightButtonWidth);

	QFontMetrics metrics(leftButton->font());
	QString elidedLeftText = metrics.elidedText(leftText, Qt::ElideRight, leftButtonWidth - m_iconSize.width() - 4 * padding);
	QString elidedRightText = metrics.elidedText(rightText, Qt::ElideRight, rightButtonWidth - m_iconSize.width() - 4 * padding);

	leftButton->setText(elidedLeftText);
	leftButton->setToolTip(leftData);
	rightButton->setText(elidedRightText);
	rightButton->setToolTip(rightData);

	layout->addWidget(leftButton);
	layout->addWidget(rightButton);

	connect(leftButton, &QPushButton::clicked, this, &CustomListItem::onLeftButtonClicked);
	connect(rightButton, &QPushButton::clicked, this, &CustomListItem::onRightButtonClicked);

	setLayout(layout);
}

QSize CustomListItem::sizeHint() const {
	QFontMetrics metrics(font());
	int textHeight = metrics.height();

	int verticalPadding = 10;
	int totalHeight = qMax(textHeight, m_iconSize.height()) + verticalPadding;

	// Width is based on the list widget's width
	int listWidgetWidth = parentWidget() ? parentWidget()->width() : 100;
	int totalWidth = listWidgetWidth - layout()->contentsMargins().left() - layout()->contentsMargins().right();

	return QSize(totalWidth, totalHeight);
}

void CustomListItem::onLeftButtonClicked() {
    emit leftItemClicked(leftData);
}

void CustomListItem::onRightButtonClicked() {
    emit rightItemClicked(rightData);
}

////////////////////////////////////////////////////////////////////////////////

BlogListWidget::BlogListWidget(QWidget * parent) : QListWidget(parent)
{
	connect(this, SIGNAL(itemEntered(QListWidgetItem *)), this, SLOT(itemEnteredSlot(QListWidgetItem *)));
}

BlogListWidget::~BlogListWidget()
{
}

QStringList & BlogListWidget::imageRequestList() {
	return m_imageRequestList;
}

/* blogEntry Title text properties color, fontfamily, fontsize*/

QColor BlogListWidget::titleTextColor() const {
	return m_titleTextColor;
}

void BlogListWidget::setTitleTextColor(QColor color) {
	m_titleTextColor = color;
}

QString BlogListWidget::titleTextFontFamily() const {
	return m_titleTextFontFamily;
}

void BlogListWidget::setTitleTextFontFamily(QString family) {
	m_titleTextFontFamily = family;
}

QString BlogListWidget::titleTextFontSize() const {
	return m_titleTextFontSize;
}

void BlogListWidget::setTitleTextFontSize(QString size) {
	m_titleTextFontSize = size;
}

QString BlogListWidget::titleTextExtraLeading() const {
	return m_titleTextExtraLeading;
}

void BlogListWidget::setTitleTextExtraLeading(QString leading) {
	m_titleTextExtraLeading = leading;
}

/* blogEntry intro text properties color, fontfamily, fontsize*/
QColor BlogListWidget::introTextColor() const {
	return m_introTextColor;
}

void BlogListWidget::setIntroTextColor(QColor color) {
	m_introTextColor = color;
}

QString BlogListWidget::introTextFontFamily() const {
	return m_introTextFontFamily;
}

void BlogListWidget::setIntroTextFontFamily(QString family) {
	m_introTextFontFamily = family;
}

QString BlogListWidget::introTextFontSize() const {
	return m_introTextFontSize;
}

void BlogListWidget::setIntroTextFontSize(QString size) {
	m_introTextFontSize = size;
}

QString BlogListWidget::introTextExtraLeading() const {
	return m_introTextExtraLeading;
}

void BlogListWidget::setIntroTextExtraLeading(QString leading) {
	m_introTextExtraLeading = leading;
}

/* blogEntry Date text properties color, fontfamily, fontsize*/

QColor BlogListWidget::dateTextColor() const {
	return m_dateTextColor;
}

void BlogListWidget::setDateTextColor(QColor color) {
	m_dateTextColor = color;
}

QString BlogListWidget::dateTextFontFamily() const {
	return m_dateTextFontFamily;
}

void BlogListWidget::setDateTextFontFamily(QString family) {
	m_dateTextFontFamily = family;
}

QString BlogListWidget::dateTextFontSize() const {
	return m_dateTextFontSize;
}

void BlogListWidget::setDateTextFontSize(QString size) {
	m_dateTextFontSize = size;
}

void BlogListWidget::itemEnteredSlot(QListWidgetItem * item) {
	QString url = item->data(RefRole).toString();
	bool arrow = (url.isEmpty()) || (url == "nop");

	setCursor(arrow ? Qt::ArrowCursor : Qt::PointingHandCursor);
}

////////////////////////////////////////////////////////////////////////////////

BlogListDelegate::BlogListDelegate(QObject *parent) : QAbstractItemDelegate(parent)
{
}

BlogListDelegate::~BlogListDelegate()
{
}

void BlogListDelegate::paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	auto *listWidget = qobject_cast<BlogListWidget *>(this->parent());
	if (!listWidget) return;

	auto *style = listWidget->style();
	if (!style) return;

	painter->save();

	QFont itemFont(painter->font());

	style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter, listWidget);

	auto pixmap = qvariant_cast<QPixmap>(index.data(IconRole));
	QString title = index.data(TitleRole).toString();
	QString date = index.data(DateRole).toString();
	QString author = index.data(AuthorRole).toString();
	QString intro = index.data(IntroRole).toString();

	//  QRect rect;
	int imageSpace = ImageSpace + 10;

	// TITLE
	painter->setPen(listWidget->titleTextColor());
	QFont titleFont(listWidget->titleTextFontFamily());
	titleFont.setPixelSize(pixelSize(listWidget->titleTextFontSize()));
	painter->setFont(titleFont);
	QRect rect = option.rect.adjusted(imageSpace, TopSpace, 0, 0);
	style->drawItemText(painter, rect, Qt::AlignLeft, option.palette, true, title);
	QFontMetrics titleFontMetrics(titleFont);

	// INTRO
	painter->setPen(listWidget->introTextColor());
	QFont introFont(listWidget->introTextFontFamily());
	introFont.setPixelSize(pixelSize(listWidget->introTextFontSize()));
	painter->setFont(introFont);
	rect = option.rect.adjusted(imageSpace, TopSpace + titleFontMetrics.lineSpacing() + pixelSize(listWidget->titleTextExtraLeading()), 0, 0);
	style->drawItemText(painter, rect, Qt::AlignLeft, option.palette, true, intro);
	QFontMetrics introFontMetrics(introFont);

	// DATE
	painter->setPen(listWidget->dateTextColor());
	QFont font(listWidget->dateTextFontFamily());
	font.setPixelSize(pixelSize(listWidget->dateTextFontSize()));
	painter->setFont(font);
	rect = option.rect.adjusted(imageSpace, TopSpace + titleFontMetrics.lineSpacing() + introFontMetrics.lineSpacing() + pixelSize(listWidget->introTextExtraLeading()), 0, 0);
	style->drawItemText(painter, rect, Qt::AlignLeft, option.palette, true, date);
	QFontMetrics dateTextFontMetrics(font);

	// AUTHOR
	QRect textRect = style->itemTextRect(dateTextFontMetrics, option.rect, Qt::AlignLeft, true, date);
	rect = option.rect.adjusted(imageSpace + textRect.width() + 7, TopSpace + titleFontMetrics.lineSpacing() + introFontMetrics.lineSpacing() + pixelSize(listWidget->introTextExtraLeading()), 0, 0);
	style->drawItemText(painter, rect, Qt::AlignLeft, option.palette, true, author);

	if (!pixmap.isNull()) {
		//ic.paint(painter, option.rect, Qt::AlignVCenter|Qt::AlignLeft);
		style->drawItemPixmap(painter, option.rect.adjusted(0, TopSpace, 0, -TopSpace), Qt::AlignLeft, pixmap);
	}

	painter->restore();
}

QSize BlogListDelegate::sizeHint (const QStyleOptionViewItem &, const QModelIndex &) const
{
	return QSize(100, ImageSpace); // very dumb value
}

//////////////////////////////////////

WelcomeView::WelcomeView(QWidget * parent) : QFrame(parent)
{
	this->setObjectName("welcomeView");

	setAcceptDrops(true);
	initLayout();

	connect(this, SIGNAL(newSketch()), this->window(), SLOT(createNewSketch()));
	connect(this, SIGNAL(openSketch()), this->window(), SLOT(mainLoad()));
	connect(this, SIGNAL(recentSketch(const QString &, const QString &)), this->window(), SLOT(openRecentOrExampleFile(const QString &, const QString &)));

	QString protocol = QSslSocket::supportsSsl() ? "https" : "http";
	// TODO: blog network calls should only happen once, not for each window?
	auto * manager = new QNetworkAccessManager(this);
	connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(gotBlogSnippet(QNetworkReply *)));
	manager->get(QNetworkRequest(QUrl(QString("%1://blog.fritzing.org/recent-posts-app/").arg(protocol))));

	manager = new QNetworkAccessManager(this);

	connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(gotBlogSnippet(QNetworkReply *)));
	manager->get(QNetworkRequest(QUrl(QString("%1://fritzing.org/projects/snippet/").arg(protocol))));

	TipsAndTricks::initTipSets();
	nextTip();
}

void WelcomeView::initLayout()
{
	auto * mainLayout = new QGridLayout();

	//mainLayout->setSpacing (0);
	//mainLayout->setContentsMargins (0, 0, 0, 0);
	mainLayout->setSizeConstraint (QLayout::SetMaximumSize);

	QWidget * recent = initRecent();
	mainLayout->addWidget(recent, 0, 0);

	QWidget * widget = initBlog();
	mainLayout->addWidget(widget, 0, 1);

	//widget = initShop();
	//mainLayout->addWidget(widget, 1, 1);

	widget = initTip();
	mainLayout->addWidget(widget, 1, 0);


	this->setLayout(mainLayout);
}


QWidget * WelcomeView::initRecent() {
	auto * frame = new QFrame;
	frame->setObjectName("recentFrame");
	auto * frameLayout = new QVBoxLayout;
	zeroMargin(frameLayout);

	auto * titleFrame = new QFrame;
	titleFrame-> setObjectName("recentTitleFrame");
	auto * titleFrameLayout = new QHBoxLayout;
	zeroMargin(titleFrameLayout);
	auto * label = new QLabel(tr("Recent Sketches"));

	label->setObjectName("recentTitle");
	titleFrameLayout->addWidget(label);
	titleFrame ->setLayout(titleFrameLayout);

	frameLayout->addWidget(titleFrame);

	m_recentListWidget = new QListWidget();
	m_recentListWidget->setObjectName("recentList");
	m_recentListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	auto * listsFrame = new QFrame;
	auto * listsFrameLayout = new QHBoxLayout;
	zeroMargin(listsFrameLayout);
	listsFrameLayout->addWidget(m_recentListWidget);
	listsFrame ->setLayout(listsFrameLayout);
	frameLayout->addWidget(listsFrame);

	QStringList names;
	names << "recentSpace" << "recentNewSketch" << "recentOpenSketch";

	Q_FOREACH (QString name, names) {
		QWidget * widget = nullptr;
		QLayout * whichLayout = frameLayout;
		QLabel * icon = nullptr;
		QLabel * text = nullptr;
		if (name == "recentSpace") {
			widget = new QLabel();
		}
		else if (name == "recentTitleSpace") {
			widget = new QLabel();
		}
		else if (name == "recentNewSketch") {
			widget = makeRecentItem(name,
			                        QString("<a href='new' style='text-decoration:none; color:#666; margin-right:5px;'><img src=':/resources/images/icons/WS-new-icon.png' /></a>"),
			                        QString("<a href='new' style='text-decoration:none; color:#666;'>%1</a>").arg(tr("New Sketch")),
			                        icon,
			                        text);

		}
		else if (name == "recentOpenSketch") {
			widget = makeRecentItem(name,
			                        QString("<a href='open' style='text-decoration:none; color:#666; margin-right:5px;'><img src=':/resources/images/icons/WS-open-icon.png' /></a>"),
			                        QString("<a href='open' style='text-decoration:none; color:#666;'>%1</a>").arg(tr("Open Sketch")),
			                        icon,
			                        text);
		}

		if (widget) {
			widget->setObjectName(name);
			whichLayout->addWidget(widget);
		}
	}

	//  frameLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

	frame->setLayout(frameLayout);
	return frame;
}

QWidget * WelcomeView::makeRecentItem(const QString & objectName, const QString & iconText, const QString & textText, QLabel * & icon, QLabel * & text) {
	auto * rFrame = new QFrame;
	auto * rFrameLayout = new QHBoxLayout;

	zeroMargin(rFrameLayout);

	icon = new QLabel(iconText);
	icon->setObjectName("recentIcon");
	connect(icon, SIGNAL(linkActivated(const QString &)), this, SLOT(clickRecent(const QString &)));
	rFrameLayout->addWidget(icon);

	text = new QLabel(textText);
	text->setObjectName(objectName);
	text->setObjectName("recentText");
	rFrameLayout->addWidget(text);
	connect(text, SIGNAL(linkActivated(const QString &)), this, SLOT(clickRecent(const QString &)));

	rFrame->setLayout(rFrameLayout);
	return rFrame;
}
/*
QWidget * WelcomeView::initShop() {

	auto * frame = new QFrame();
	frame->setObjectName("shopFrame");
	auto * frameLayout = new QVBoxLayout;
	zeroMargin(frameLayout);

	QWidget * headerFrame = createHeaderFrame( "Fab", tr("Fab"), "", "",  m_activeHeaderLabelColor, m_inactiveHeaderLabelColor, m_fabLabel, m_fabLabel);
	frameLayout->addWidget(headerFrame);

	m_fabUberFrame = createShopContentFrame(":/resources/images/pcbs_2013.png",
	                                        tr("Fritzing Fab"),
	                                        tr("Fritzing Fab is an easy and affordable service for producing professional PCBs from your Fritzing sketches."),
											"https://fab.fritzing.org/",
	                                        tr("produce your first pcb now >>"),
	                                        tr("Order your PCB now."),
	                                        ":/resources/images/icons/WS-fabLogo.png",
	                                        "#5f4d4a"
	                                       );
	frameLayout->addWidget(m_fabUberFrame);

	frame->setLayout(frameLayout);

	return frame;
}

QWidget * WelcomeView::createShopContentFrame(const QString & imagePath, const QString & headline, const QString & description,
        const QString & url, const QString & urlText, const QString & urlText2, const QString & logoPath, const QString & footerLabelColor )
{
	auto * uberFrame = new QFrame();
	uberFrame->setObjectName("shopUberFrame");
	auto * shopUberFrameLayout = new QVBoxLayout;
	zeroMargin(shopUberFrameLayout);

	auto* shopContentFrame = new QFrame();
	shopContentFrame->setObjectName("shopContentFrame");

	auto * contentFrameLayout = new QHBoxLayout;
	zeroMargin(contentFrameLayout);

	auto * label = new QLabel(QString("<img src='%1' />").arg(imagePath));
	label->setObjectName("shopContentImage");
	contentFrameLayout->addWidget(label);

	auto * contentTextFrame = new QFrame();
	contentTextFrame->setObjectName("shopContentTextFrame");

	auto * contentTextFrameLayout = new QVBoxLayout;
	zeroMargin(contentTextFrameLayout);

	contentTextFrameLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));

	label = new QLabel(headline);
	label->setTextFormat(Qt::RichText);
	label->setObjectName("shopContentTextHeadline");
	//label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	contentTextFrameLayout->addWidget(label);

	label = new QLabel(description);
	label->setObjectName("shopContentTextDescription");
	//label->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	contentTextFrameLayout->addWidget(label);

	label = new QLabel(QString("<a href='%1' style='text-decoration:none; color:#802742;'>%2</a>").arg(url).arg(urlText));
	label->setObjectName("shopContentTextCaption");
	contentTextFrameLayout->addWidget(label);
	//label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	connect(label, &QLabel::linkActivated, this, &WelcomeView::clickBlog);

	contentTextFrameLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Expanding));

	contentTextFrame->setLayout(contentTextFrameLayout);
	contentFrameLayout->addWidget(contentTextFrame);

	shopContentFrame->setLayout(contentFrameLayout);

	shopUberFrameLayout->addWidget(shopContentFrame);
	shopUberFrameLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Minimum, QSizePolicy::Expanding));

	auto * shopFooterFrame = new QFrame();
	shopFooterFrame->setObjectName("shopFooterFrame");

	auto * footerFrameLayout = new QHBoxLayout;
	zeroMargin(footerFrameLayout);
	footerFrameLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));

	auto * footerLabel = new QLabel(QString("<a href='%1' style='text-decoration:none; color:%3;'>%2</a>").arg(url).arg(urlText2).arg(footerLabelColor));
	footerLabel->setObjectName("shopLogoText");
	footerFrameLayout->addWidget(footerLabel);
    connect(footerLabel, &QLabel::linkActivated, this, &WelcomeView::clickBlog);

	auto * footerLogoLabel = new QLabel(tr("<a href='%1'><img src='%2'/></a>").arg(url).arg(logoPath));
	footerLogoLabel->setObjectName("shopLogo");
	footerFrameLayout->addWidget(footerLogoLabel);
    connect(footerLogoLabel, &QLabel::linkActivated, this, &WelcomeView::clickBlog);

	shopFooterFrame->setLayout(footerFrameLayout);

	shopUberFrameLayout->addWidget(shopFooterFrame);
	uberFrame->setLayout(shopUberFrameLayout);
	return uberFrame;
}
*/
QWidget * WelcomeView::initBlog() {

	auto * frame = new QFrame();
	frame->setObjectName("blogFrame");
	auto * frameLayout = new QVBoxLayout;
	zeroMargin(frameLayout);

	QWidget * headerFrame = createHeaderFrame("Projects", tr("Projects"), "Blog", tr("Blog"), m_inactiveHeaderLabelColor,  m_activeHeaderLabelColor, m_projectsLabel, m_blogLabel);
	frameLayout->addWidget(headerFrame);

	m_blogListWidget = createBlogContentFrame("https://blog.fritzing.org", tr("Fritzing News."), ":/resources/images/icons/WS-blogLogo.png", "#802742");
	m_blogUberFrame = m_blogListWidget;
	while (m_blogUberFrame->parentWidget()) m_blogUberFrame = m_blogUberFrame->parentWidget();
	frameLayout->addWidget(m_blogUberFrame);

	m_projectListWidget = createBlogContentFrame("https://fritzing.org/projects/", tr("Fritzing Projects."), ":/resources/images/icons/WS-galleryLogo.png", "#00a55b");
	m_projectsUberFrame = m_projectListWidget;
	while (m_projectsUberFrame->parentWidget()) m_projectsUberFrame = m_projectsUberFrame->parentWidget();
	frameLayout->addWidget(m_projectsUberFrame);

	frame->setLayout(frameLayout);

	//DebugDialog::debug("first click blog");

	clickBlog("Blog");

	return frame;
}

QFrame * WelcomeView::createHeaderFrame (const QString & url1, const QString & urlText1, const QString & url2, const QString & urlText2, const QString & inactiveColor, const QString & activeColor,
        QLabel * & label1, QLabel * & label2) {
	auto * titleFrame = new QFrame();
	titleFrame->setObjectName("wsSwitchableFrameHeader");

	auto * titleFrameLayout = new QHBoxLayout;
	zeroMargin(titleFrameLayout);

	label1 = new QLabel(makeUrlText(url1, urlText1, inactiveColor));
	label1->setObjectName("headerTitle1");
	titleFrameLayout->addWidget(label1);
	connect(label1, SIGNAL(linkActivated(const QString &)), this, SLOT(clickBlog(const QString &)));

	if (!urlText2.isEmpty()) {
		auto * titleSpace = new QLabel("|");
		titleSpace->setObjectName("headerTitleSpace");
		titleFrameLayout->addWidget(titleSpace);

		label2 = new QLabel(makeUrlText(url2, urlText2, activeColor));
		label2->setObjectName("headerTitle2");
		titleFrameLayout->addWidget(label2);
		connect(label2, SIGNAL(linkActivated(const QString &)), this, SLOT(clickBlog(const QString &)));
	}
	titleFrameLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));
	titleFrame->setLayout(titleFrameLayout);

	return titleFrame;
}

BlogListWidget * WelcomeView::createBlogContentFrame(const QString & url, const QString & urlText, const QString & logoPath, const QString & footerLabelColor) {
	auto * uberFrame = new QFrame;
	auto * uberFrameLayout = new QVBoxLayout;
	zeroMargin(uberFrameLayout);

	auto * listWidget = new BlogListWidget;
	listWidget->setObjectName("blogList");
	listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	listWidget->setItemDelegate(new BlogListDelegate(listWidget));
	connect(listWidget, SIGNAL(itemClicked (QListWidgetItem *)), this, SLOT(blogItemClicked(QListWidgetItem *)));

	uberFrameLayout->addWidget(listWidget);

	auto * footerFrame = new QFrame();
	footerFrame->setObjectName("blogFooterFrame");

	auto * footerFrameLayout = new QHBoxLayout;
	zeroMargin(footerFrameLayout);
	footerFrameLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));

	auto * footerLabel = new QLabel(QString("<a href='%1'  style='font-family:Droid Sans; text-decoration:none; color:%3;'>%2</a>").arg(url).arg(urlText).arg(footerLabelColor));
	footerLabel->setObjectName("blogLogoText");
	footerFrameLayout->addWidget(footerLabel);
	connect(footerLabel, SIGNAL(linkActivated(const QString &)), this, SLOT(clickBlog(const QString &)));
	footerLabel = new QLabel(tr("<a href='%1'><img src='%2' /></a>").arg(url).arg(logoPath));
	footerLabel->setObjectName("blogLogo");

	footerFrameLayout->addWidget(footerLabel);
	connect(footerLabel, SIGNAL(linkActivated(const QString &)), this, SLOT(clickBlog(const QString &)));

	footerFrame->setLayout(footerFrameLayout);

	uberFrameLayout->addWidget(footerFrame);
	uberFrame->setLayout(uberFrameLayout);

	return listWidget;
}

void WelcomeView::showEvent(QShowEvent * event) {
	QFrame::showEvent(event);
	updateRecent();
}

void WelcomeView::updateRecent() {
	if (!m_recentListWidget) return;

	QSettings settings;
	auto files = settings.value("recentFileList").toStringList();
	m_recentListWidget->clear();
	int listWidgetWidth = m_recentListWidget->width();

	auto gotOne = false;

	QIcon defaultIcon(":/resources/images/icons/WS-fzz-icon.png");
	QIcon aislerIcon(":/resources/images/icons/aisler_donut-cloud_logo_icon.png");

	for (int i = 0; i < files.size(); ++i) {
		QFileInfo finfo(files[i]);
		if (!finfo.exists()) continue;

		gotOne = true;
		QString leftText = finfo.fileName();
		QString leftData = finfo.absoluteFilePath();

		settings.beginGroup("sketches");
		QVariant settingValue = settings.value(finfo.absoluteFilePath());
		settings.endGroup();

		QString rightText;
		QIcon rightIcon;
		QString rightData;

		if (settingValue.isValid() && !settingValue.isNull()) {
			auto [fabName, link] = settingValue.value<UploadPair>();
			if (link.endsWith(QChar('/'))) {
				link.chop(1);  // Remove the last character
			}
			rightText = QString("%1").arg(fabName);
			rightData = link; // Data for the right button click
			QPixmap pixmap = SqliteReferenceModel().retrieveIcon(fabName);
			if (!pixmap.isNull()) {
				rightIcon = QIcon(pixmap);
			} else {
				if (fabName.compare("Aisler", Qt::CaseInsensitive) == 0) {
					rightIcon = aislerIcon;
				}
			}
		}

		CustomListItem *customItem = new CustomListItem(leftText, defaultIcon, leftData,
								rightText, rightIcon, rightData, listWidgetWidth);
		connect(customItem, &CustomListItem::leftItemClicked, this, &WelcomeView::recentSketchClicked);
		connect(customItem, &CustomListItem::rightItemClicked, this, &WelcomeView::uploadLinkClicked);

		QListWidgetItem *item = new QListWidgetItem(m_recentListWidget);
		item->setSizeHint(customItem->sizeHint());
		m_recentListWidget->setItemWidget(item, customItem);
	}

	if (!gotOne) {
		// put in a placeholder if there are no recent files
		CustomListItem *emptyItem = new CustomListItem(tr("No recent sketches found"), defaultIcon, QString(),
								       QString(), QIcon(), QString(), listWidgetWidth);
		QListWidgetItem *item = new QListWidgetItem(m_recentListWidget);
		item->setSizeHint(emptyItem->sizeHint());
		m_recentListWidget->setItemWidget(item, emptyItem);
	}
}

void WelcomeView::clickRecent(const QString & url) {
	if (url == "open") {
		Q_EMIT openSketch();
		return;
	}
	if (url == "new") {
		Q_EMIT newSketch();
		return;
	}
}

void WelcomeView::gotBlogSnippet(QNetworkReply * networkReply) {
	bool blog = networkReply->url().toString().contains("recent");
	QString prefix = networkReply->url().scheme() + "://" + networkReply->url().authority();
	QNetworkAccessManager * manager = networkReply->manager();
	int responseCode = networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

	auto goodBlog = false;
	QDomDocument doc;
	QString errorStr;
	auto errorLine = 0;
	auto errorColumn = 0;
	if (responseCode == 200) {
		QString data(networkReply->readAll());
		//DebugDialog::debug("response data " + data);
		data = "<thing>" + cleanData(data) + "</thing>";		// make it one tree for xml parsing
		if (doc.setContent(data, &errorStr, &errorLine, &errorColumn)) {
			readBlog(doc, true, blog, prefix);
			goodBlog = true;
		}
	}

	if (!goodBlog) {
		QString message = (blog) ? tr("Unable to reach blog.fritzing.org") : tr("Unable to reach fritzing.org/projects") ;
		QString placeHolder = QString("<li><a class='title' href='nop' title='%1'></a></li>").arg(message);
		if (doc.setContent(placeHolder, &errorStr, &errorLine, &errorColumn)) {
			readBlog(doc, true, blog, "");
		}
	}

	manager->deleteLater();
	networkReply->deleteLater();
}

void WelcomeView::clickBlog(const QString & url) {
	if (url.toLower() == "fab") {
//		m_shopUberFrame->setVisible(false);
		m_fabUberFrame->setVisible(true);
		m_fabLabel->setText(hackColor(m_fabLabel->text(), m_activeHeaderLabelColor));
//		m_shopLabel->setText(hackColor(m_shopLabel->text(), m_inactiveHeaderLabelColor));
		return;
	}

	if (url.toLower() == "shop") {
//		m_shopUberFrame->setVisible(true);
		m_fabUberFrame->setVisible(false);
		m_fabLabel->setText(hackColor(m_fabLabel->text(), m_inactiveHeaderLabelColor));
//		m_shopLabel->setText(hackColor(m_shopLabel->text(), m_activeHeaderLabelColor));
		return;
	}

    if (url.toLower() == "donate") {
//        m_shopUberFrame->setVisible(false);
        m_fabUberFrame->setVisible(false);
        m_fabLabel->setText(hackColor(m_fabLabel->text(), m_inactiveHeaderLabelColor));
//        m_shopLabel->setText(hackColor(m_shopLabel->text(), m_inactiveHeaderLabelColor));
        return;
    }

	if (url.toLower() == "nexttip") {
		nextTip();
		return;
	}

	if (url.toLower() == "projects") {
		m_projectsUberFrame->setVisible(true);
		m_blogUberFrame->setVisible(false);
		m_projectsLabel->setText(hackColor(m_projectsLabel->text(), m_activeHeaderLabelColor));
		m_blogLabel->setText(hackColor(m_blogLabel->text(), m_inactiveHeaderLabelColor));
		return;
	}

	if (url.toLower() == "blog") {
		m_projectsUberFrame->setVisible(false);
		m_blogUberFrame->setVisible(true);
		m_projectsLabel->setText(hackColor(m_projectsLabel->text(), m_inactiveHeaderLabelColor));
		m_blogLabel->setText(hackColor(m_blogLabel->text(), m_activeHeaderLabelColor));
		return;
	}

	QDesktopServices::openUrl(url);
}

/*

// sample output from http://blog.fritzing.org/recent-posts-app/
<ul>
    <li>
        <img src="http://blog.fritzing.org/wp-content/uploads/charles1.jpg"/>
        <a class="title" href="http://blog.fritzing.org/2013/11/15/light-up-your-flat-with-charles-planetary-gear-system/" title="Light up your flat with Charles&#039; planetary gear system">Light up your flat with Charles' planetary gear system</a>
        <p class="date">Nov. 15, 2013</p>
        <p class="author">Nushin Isabelle</p>
        <p class="intro">Today, we got a visitor in the Fritzing Lab: Our neighbour, Charles Oleg, came by to show us his new...</p>
    </li>
</ul>

// sample output from http://fritzing.org/projects/snippet/
<ul>
    <li>
        <a class="image" href="/projects/sensor-infrarrojos-para-nuestro-robot">
            <img src="" alt="Sensor Infrarrojos para nuestro Robot" />
        </a>
        <a class="title" href="/projects/sensor-infrarrojos-para-nuestro-robot" title="Sensor Infrarrojos para nuestro Robot">Sensor Infrarrojos para nuestro Robot</a>
        <p class="date">1 week ago</p>
        <p class="author">robotarduedu</p>
        <p class="difficulty">for kids</p>
        <p class="tags">in sensror infrarrojo, led infrarrojo, i.r., i.r., </p>
    </li>
</ul>
*/


void WelcomeView::readBlog(const QDomDocument & doc, bool doEmit, bool blog, const QString & prefix) {
	auto *listWidget = (blog) ? m_blogListWidget : m_projectListWidget;
	listWidget->clear();
	listWidget->imageRequestList().clear();

	QDomNodeList nodeList = doc.elementsByTagName("li");
	for (int i = 0; i < nodeList.count(); i++) {
		QDomElement element = nodeList.at(i).toElement();
		QDomElement child = element.firstChildElement();
		QHash<QString, QString> stuff;
		while (!child.isNull()) {
			if (child.tagName() == "img") {
				stuff.insert("img", child.attribute("src"));
			}
			else {
				QString clss = child.attribute("class");
				if (clss == "title") {
					QString title = child.attribute("title");
					QString href = child.attribute("href");
					if (!blog) {
						href.insert(0, prefix);
					}
					stuff.insert("title", title);
					stuff.insert("href", href);
				}
				else if (clss == "image") {
					QDomElement img = child.firstChildElement("img");
					QString src = img.attribute("src");
					if (!src.isEmpty()) src.insert(0, prefix);
					stuff.insert("img", src);
				}
				else {
					stuff.insert(clss, child.text());
				}
			}
			child = child.nextSiblingElement();
		}
		if (stuff.value("title", "").isEmpty()) continue;
		if (stuff.value("href", "").isEmpty()) continue;

		auto *item = new QListWidgetItem();
		item->setData(TitleRole, stuff.value("title"));
		item->setData(RefRole, stuff.value("href"));
		QString text = stuff.value("intro", "");
		text.replace("\r", " ");
		text.replace("\n", " ");
		text.replace("\t", " ");
		item->setData(IntroRole, text);
		listWidget->addItem(item);

		listWidget->imageRequestList() << stuff.value("img", "");

		if (!stuff.value("date", "").isEmpty()) {
			item->setData(DateRole, stuff.value("date"));
		}
		if (!stuff.value("author", "").isEmpty()) {
			item->setData(AuthorRole, stuff.value("author"));
		}
	}

	if (listWidget->count() > 0) {
		listWidget->itemEnteredSlot(listWidget->item(0));
	}

	if (doEmit) {
		getNextBlogImage(0, blog);
		Q_FOREACH (QWidget *widget, QApplication::topLevelWidgets()) {
			auto * other = widget->findChild<WelcomeView *>();
			if (!other) continue;
			if (other == this) continue;

			other->readBlog(doc, false, blog, prefix);
		}
	}
}

void WelcomeView::getNextBlogImage(int ix, bool blog) {
	BlogListWidget * listWidget = (blog) ? m_blogListWidget : m_projectListWidget;
	for (int i = ix; i < listWidget->imageRequestList().count(); i++) {
		QString image = listWidget->imageRequestList().at(i);
		if (image.isEmpty()) continue;

		auto * manager = new QNetworkAccessManager(this);
		manager->setProperty("index", i);
		manager->setProperty("blog", blog);
		connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(gotBlogImage(QNetworkReply *)));
		manager->get(QNetworkRequest(QUrl(image)));
	}
}

void WelcomeView::gotBlogImage(QNetworkReply * networkReply) {
	auto *manager = networkReply->manager();
	if (!manager) return;

	auto index = manager->property("index").toInt();
	auto blog = manager->property("blog").toBool();

	auto responseCode = networkReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
	if (responseCode == 200) {
		QByteArray data(networkReply->readAll());
		QPixmap pixmap;
		if (pixmap.loadFromData(data)) {
			QPixmap scaled = pixmap.scaled(QSize(ImageSpace, ImageSpace), Qt::KeepAspectRatio);
			setBlogItemImage(scaled, index, blog);
			Q_FOREACH (QWidget *widget, QApplication::topLevelWidgets()) {
				auto *other = widget->findChild<WelcomeView *>();
				if (!other) continue;
				if (other == this) continue;

				other->setBlogItemImage(scaled, index, blog);
			}
		}
	}

	manager->deleteLater();
	networkReply->deleteLater();
}

QWidget * WelcomeView::initTip() {
	auto *tipFrame = new QFrame();
	tipFrame->setObjectName("tipFrame");
	auto *tipLayout = new QVBoxLayout();
	zeroMargin(tipLayout);

	auto *tipTitle = new QLabel(tr("Tip of the Day:"));
	tipTitle->setObjectName("tipTitle");
	tipLayout->addWidget(tipTitle);

	auto *scrollArea = new QScrollArea;
	scrollArea->setObjectName("tipScrollArea");
	scrollArea->setWidgetResizable(true);
	// scrollArea->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	// scrollArea->setSizePolicy(QSizePolicy::MinimumExpanding,QSizePolicy::MinimumExpanding);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	m_tip = new QLabel();
	m_tip->setObjectName("tip");
	m_tip->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	//connect(m_tip, SIGNAL(linkActivated(const QString &)), this->window(), SLOT(tipsAndTricks()));

	scrollArea->setWidget(m_tip);
	tipLayout->addWidget(scrollArea);

	tipLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Fixed, QSizePolicy::Fixed));
	auto * footerFrame = new QFrame();
	footerFrame->setObjectName("tipFooterFrame");

	auto *footerFrameLayout = new QHBoxLayout;
	zeroMargin(footerFrameLayout);


	auto *footerLabel = new QLabel(QString("<a href='http://blog.fritzing.org'  style='font-family:Droid Sans; text-decoration:none; color:#2e94af;'>%1</a>").arg(tr("All Tips")));
	footerLabel->setObjectName("allTips");
	footerFrameLayout->addWidget(footerLabel);
	connect(footerLabel, SIGNAL(linkActivated(const QString &)), this->window(), SLOT(tipsAndTricks()));

	footerFrameLayout->addSpacerItem(new QSpacerItem(1, 1, QSizePolicy::Expanding));


	footerLabel = new QLabel(QString("<a href='http://blog.fritzing.org'  style='text-decoration:none; font-family:Droid Sans; color:#2e94af;'>%1</a>").arg(tr("Next Tip")));
	footerLabel->setObjectName("nextTip");
	footerFrameLayout->addWidget(footerLabel);
	connect(footerLabel, SIGNAL(linkActivated(const QString &)), this, SLOT(nextTip()));


	footerFrame->setLayout(footerFrameLayout);

	tipLayout->addWidget(footerFrame);

	tipFrame->setLayout(tipLayout);

	return tipFrame;
}

void WelcomeView::dragEnterEvent(QDragEnterEvent *event)
{
	DebugDialog::debug("ignoring drag enter");
	event->ignore();
}

void WelcomeView::nextTip() {
	if (!m_tip) return;

	m_tip->setText(QString("<a href='tip' style='text-decoration:none; color:#2e94af;'>%1</a>").arg(TipsAndTricks::randomTip()));
}

void WelcomeView::recentSketchClicked(const QString &data) {
    if (!data.isEmpty()) {
	Q_EMIT recentSketch(data, data);
    }
}

void WelcomeView::uploadLinkClicked(const QString &data) {
    if (!data.isEmpty()) {
	QDesktopServices::openUrl(QUrl(data));
    }
}

void WelcomeView::blogItemClicked(QListWidgetItem * item) {
	QString url = item->data(RefRole).toString();
	if (url.isEmpty()) return;
	if (url == "nop") return;

	QDesktopServices::openUrl(url);
}

void WelcomeView::setBlogItemImage(QPixmap & pixmap, int index, bool blog) {
	// TODO: this is not totally thread-safe if there are multiple sketch widgets opened within a very short time
	auto *listWidget = (blog) ? m_blogListWidget : m_projectListWidget;
	auto *item = listWidget->item(index);
	if (item) {
		item->setData(IconRole, pixmap);
	}
}
