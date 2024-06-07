#include "fprobefocuswidget.h"
#include <QStringList>

FProbeFocusWidget::FProbeFocusWidget()
	: FProbe("FocusWidget")
{
}

QVariant FProbeFocusWidget::read()
{
	return QVariant();
}

void FProbeFocusWidget::write(QVariant data)
{
	QString input = data.toString();
	QStringList parts = input.split(";");
	if (parts.size() == 2) {
		QString objectName = parts[0];
		int index = parts[1].toInt();
		emit focusWidget(objectName, index);
	}
}
