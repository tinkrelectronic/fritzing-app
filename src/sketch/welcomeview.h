/*******************************************************************

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



#ifndef WELCOMEVIEW_H
#define WELCOMEVIEW_H

#include <QFrame>
#include <QLabel>
#include <QList>
#include <QWidget>
#include <QNetworkReply>
#include <QDomDocument>
#include <QDragEnterEvent>
#include <QListWidget>
#include <QPainter>
#include <QAbstractItemDelegate>
#include <QPushButton>
#include <QHBoxLayout>

class CustomListItem : public QWidget {
	Q_OBJECT

public:
	explicit CustomListItem(const QString &leftText, const QIcon &leftIcon, const QString &leftData,
				const QString &rightText, const QIcon &rightIcon, const QString &rightData,
				int listWidgetWidth, QWidget *parent = nullptr);
	QSize sizeHint() const;

signals:
	void leftItemClicked(const QString &data);
	void rightItemClicked(const QString &data);

private slots:
	void onLeftButtonClicked();
	void onRightButtonClicked();

private:
	QPushButton *leftButton;
	QPushButton *rightButton;
	QString leftData;
	QString rightData;
	QSize m_iconSize;
};

class BlogListWidget : public QListWidget
{
	Q_OBJECT
	Q_PROPERTY(QColor titleTextColor READ titleTextColor WRITE setTitleTextColor DESIGNABLE true)
	Q_PROPERTY(QString titleTextFontFamily READ titleTextFontFamily WRITE setTitleTextFontFamily DESIGNABLE true)
	Q_PROPERTY(QString titleTextFontSize READ titleTextFontSize WRITE setTitleTextFontSize DESIGNABLE true)
	Q_PROPERTY(QString titleTextExtraLeading READ titleTextExtraLeading WRITE setTitleTextExtraLeading DESIGNABLE true)

	Q_PROPERTY(QColor introTextColor READ introTextColor WRITE setIntroTextColor DESIGNABLE true)
	Q_PROPERTY(QString introTextFontFamily READ introTextFontFamily WRITE setIntroTextFontFamily DESIGNABLE true)
	Q_PROPERTY(QString introTextFontSize READ introTextFontSize WRITE setIntroTextFontSize DESIGNABLE true)
	Q_PROPERTY(QString introTextExtraLeading READ introTextExtraLeading WRITE setIntroTextExtraLeading DESIGNABLE true)

	Q_PROPERTY(QColor dateTextColor READ dateTextColor WRITE setDateTextColor DESIGNABLE true)
	Q_PROPERTY(QString dateTextFontFamily READ dateTextFontFamily WRITE setDateTextFontFamily DESIGNABLE true)
	Q_PROPERTY(QString dateTextFontSize READ dateTextFontSize WRITE setDateTextFontSize DESIGNABLE true)

public:
	BlogListWidget(QWidget * parent = 0);
	~BlogListWidget();

	QColor titleTextColor() const;
	void setTitleTextColor(QColor);
	QString titleTextFontFamily() const;
	void setTitleTextFontFamily(QString);
	QString titleTextFontSize() const;
	void setTitleTextFontSize(QString);
	QString titleTextExtraLeading() const;
	void setTitleTextExtraLeading(QString);

	QColor introTextColor() const;
	void setIntroTextColor(QColor);
	QString introTextFontFamily() const;
	void setIntroTextFontFamily(QString);
	QString introTextFontSize() const;
	void setIntroTextFontSize(QString);
	QString introTextExtraLeading() const;
	void setIntroTextExtraLeading(QString);

	QColor dateTextColor() const;
	void setDateTextColor(QColor);
	QString dateTextFontFamily() const;
	void setDateTextFontFamily(QString);
	QString dateTextFontSize() const;
	void setDateTextFontSize(QString);

	QStringList & imageRequestList();

public Q_SLOTS:
	void itemEnteredSlot(QListWidgetItem *);

protected:
	QColor m_titleTextColor;
	QString m_titleTextFontFamily;
	QString m_titleTextFontSize;
	QString m_titleTextExtraLeading;

	QColor m_introTextColor;
	QString m_introTextFontFamily;
	QString m_introTextFontSize;
	QString m_introTextExtraLeading;

	QColor m_dateTextColor;
	QString m_dateTextFontFamily;
	QString m_dateTextFontSize;

	QStringList m_imageRequestList;
};

class WelcomeView : public QFrame
{
	Q_OBJECT

public:
	WelcomeView(QWidget * parent = 0);
	~WelcomeView() = default;

	void showEvent(QShowEvent * event);
	void dragEnterEvent(QDragEnterEvent *event);
	void updateRecent();

protected:
	void initLayout();
	QWidget * initRecent();
	QWidget * initBlog();
	//QWidget * initShop();
	QWidget * initTip();
	void readBlog(const QDomDocument &, bool doEmit, bool blog, const QString & prefix);
	QWidget * makeRecentItem(const QString & objectName, const QString & iconText, const QString & textText, QLabel * & icon, QLabel * & text);
	void getNextBlogImage(int ix, bool blog);
	void setBlogItemImage(QPixmap &, int index, bool blog) ;
	//QWidget * createShopContentFrame(const QString & imagePath, const QString & headline, const QString & description,
	//                                 const QString & url, const QString & urlText, const QString & urlText2, const QString & logoPath, const QString & footerLabelColor);
	BlogListWidget * createBlogContentFrame(const QString & url, const QString & urlText, const QString & logoPath, const QString & footerLabelColor);

    QFrame * createHeaderFrame(const QString & url1, const QString & urlText1, const QString & url2, const QString & urlText2, const QString & inactiveColor, const QString & activeColor, QLabel * & label1, QLabel * & label2);


Q_SIGNALS:
	void newSketch();
	void openSketch();
	void recentSketch(const QString & filename, const QString & actionText);

protected Q_SLOTS:
	void clickRecent(const QString &);
	void gotBlogSnippet(QNetworkReply *);
	void gotBlogImage(QNetworkReply *);
	void clickBlog(const QString &);
	void recentSketchClicked(const QString &data);
	void uploadLinkClicked(const QString &data);
	void blogItemClicked(QListWidgetItem *);
	void nextTip();

protected:
	BlogListWidget * m_blogListWidget = nullptr;
	BlogListWidget * m_projectListWidget = nullptr;
	QWidget * m_blogUberFrame = nullptr;
	QWidget * m_projectsUberFrame = nullptr;
	QLabel * m_tip = nullptr;
	QListWidget * m_recentListWidget = nullptr;
	QWidget * m_fabUberFrame = nullptr;
	QWidget * m_shopUberFrame = nullptr;
	QLabel * m_projectsLabel = nullptr;
	QLabel * m_blogLabel = nullptr;
	QLabel * m_fabLabel = nullptr;
	QLabel * m_shopLabel = nullptr;

	static QString m_activeHeaderLabelColor;
	static QString m_inactiveHeaderLabelColor;

};

class BlogListDelegate : public QAbstractItemDelegate
{
	Q_OBJECT
public:
	BlogListDelegate(QObject *parent = 0);
	virtual ~BlogListDelegate();

	void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
	QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;
};

#endif
