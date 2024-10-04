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

#include "fmessagebox.h"
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QCloseEvent>
#include "../debugdialog.h"


bool FMessageBox::BlockMessages = false;

QList<QPair<QString, QString>> FMessageBox::messageLog;

FMessageBox::FMessageBox(QWidget* parent) : QMessageBox(parent), m_clipboardButtonEnabled(false) {
}

void FMessageBox::enableClipboardButton(bool enable) {
	m_clipboardButtonEnabled = enable;
	if (m_copyButton) {
		m_copyButton->setVisible(enable);
		if (m_buttonBox) {
			if (enable) {
				if (m_buttonBox->buttons().indexOf(m_copyButton) == -1) {
					m_buttonBox->addButton(m_copyButton, QDialogButtonBox::ActionRole);
				}
			} else {
				m_buttonBox->removeButton(m_copyButton);
			}
		}
	}
}

int FMessageBox::exec() {
	if (BlockMessages) return QMessageBox::Cancel;

	return QMessageBox::exec();
}


void FMessageBox::logMessage(const QString &title, const QString &text) {
    messageLog.append(qMakePair(title, text));
}

QList<QPair<QString, QString>> FMessageBox::getLoggedMessages() {
    return messageLog;
}

FMessageBox* FMessageBox::createCustom(QWidget* parent, Icon icon, const QString& title, const QString& text,
									   StandardButtons buttons, StandardButton defaultButton) {
	FMessageBox* msgBox = new FMessageBox(parent);
	msgBox->setIcon(icon);
	msgBox->setWindowTitle(title);
	msgBox->setText(text);
	msgBox->setStandardButtons(buttons);
	msgBox->setDefaultButton(defaultButton);
	msgBox->setupUI();
	return msgBox;
}

void FMessageBox::setDetailedText(const QString& text) {
	QMessageBox::setDetailedText(text);

	QTextEdit* textEdit = findChild<QTextEdit*>();
	if (textEdit) {
		textEdit->setFixedSize(400, 250);
	}
}

void FMessageBox::setupUI() {
	m_copyButton = new QPushButton(tr("Copy to Clipboard"), this);
	m_copyButton->setVisible(m_clipboardButtonEnabled);
	connect(m_copyButton, &QPushButton::clicked, [this]() {
		QString fullText = text() + "\n\n" + detailedText();
		QApplication::clipboard()->setText(fullText);
	});

	m_buttonBox = findChild<QDialogButtonBox*>();

	enableClipboardButton(m_clipboardButtonEnabled);
}

void FMessageBox::closeEvent(QCloseEvent* event)
{
	event->accept();
	this->done(QDialog::Rejected);
}

QMessageBox::StandardButton FMessageBox::critical(QWidget* parent, const QString& title, const QString& text, StandardButtons buttons, StandardButton defaultButton) {
	logMessage("critical: " + title, text);
	if (BlockMessages) {
		DebugDialog::debug("critical " + title);
		DebugDialog::debug(text);
		return QMessageBox::Cancel;
	}

	FMessageBox* msgBox = createCustom(parent, QMessageBox::Critical, title, text, buttons, defaultButton);
	return (QMessageBox::StandardButton)msgBox->exec();
}

QMessageBox::StandardButton FMessageBox::information( QWidget * parent, const QString & title, const QString & text, StandardButtons buttons, StandardButton defaultButton) {
	logMessage("information: " + title, text);
	if (BlockMessages) {
		DebugDialog::debug("information " + title);
		DebugDialog::debug(text);
		return QMessageBox::Cancel;
	}

	return QMessageBox::information(parent, title, text, buttons, defaultButton);
}

QMessageBox::StandardButton FMessageBox::question( QWidget * parent, const QString & title, const QString & text, StandardButtons buttons, StandardButton defaultButton) {
	logMessage("question: " + title, text);
	if (BlockMessages) {
		DebugDialog::debug("question " + title);
		DebugDialog::debug(text);
		return QMessageBox::Cancel;
	}

	return QMessageBox::question(parent, title, text, buttons, defaultButton);
}

QMessageBox::StandardButton FMessageBox::warning(QWidget* parent, const QString& title, const QString& text, StandardButtons buttons, StandardButton defaultButton) {
	logMessage("warning: " + title, text);
	if (BlockMessages) {
		DebugDialog::debug("warning " + title);
		DebugDialog::debug(text);
		return QMessageBox::Cancel;
	}

	FMessageBox* msgBox = createCustom(parent, QMessageBox::Warning, title, text, buttons, defaultButton);
	return (QMessageBox::StandardButton)msgBox->exec();
}



