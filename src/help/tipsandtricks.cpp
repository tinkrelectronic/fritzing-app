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

#include <QVBoxLayout>
#include <QPixmap>
#include <QIcon>
#include <QTime>
#include <QRandomGenerator>

#include "tipsandtricks.h"



TipsAndTricks* TipsAndTricks::Singleton = nullptr;
QList<TipSet *>  TipsAndTricks::TipSets;

TipsAndTricks::TipsAndTricks(QWidget *parent)
	: QDialog(parent)
{
	initTipSets();

	// Let's set the icon
	this->setWindowIcon(QIcon(QPixmap(":resources/images/fritzing_icon.png")));
	QString html("<html><body>");
	html += QString("<h3>%1</h3>").arg(tr("Tips and Tricks"));
	html += "<ul>";
	Q_FOREACH (TipSet * tipSet, TipSets) {
		html += QString("<li><h4>%1</h4><ul>").arg(tipSet->heading);
		Q_FOREACH (QString tip, tipSet->tips) {
			html += QString("<li>%1</li>").arg(tip);
		}
		html += "</ul></li>";
	}
	html += "</ul>";
	html += "</body></html>";

	Singleton = this;
	setWindowTitle(tr("Tips and Tricks"));
	resize(600, 500);
	m_textEdit = new QTextEdit();
	m_textEdit->setReadOnly(true);
	m_textEdit->setHtml(html);

	auto * vLayout = new QVBoxLayout(this);
	vLayout->addWidget(m_textEdit);

}

void TipsAndTricks::initTipSets() {
	if (TipSets.count() > 0) return;


	QString localStorage = tr("The local storage folder is used for storing data that is specific to Fritzing, "
							  "such as custom parts and settings, as well as temporary and long-term files "
							  "that are created during the use of the software. On Linux systems, the default "
							  "location for the Fritzing storage folder is usually ~/.config/Fritzing/. "
							  "On Windows 11, the default location is typically C:\\Users[user name]\\AppData\\Roaming\\Fritzing, "
							  "and on macOS Mojave or later, the default location is usually ~/Library/Application Support/Fritzing/.");

	auto * ts = new TipSet;
	ts->heading = tr("Money Tip");
	ts->tips << tr("Don't take wooden nickels.");
	TipSets.append(ts);

	ts = new TipSet;
	ts->heading = tr("Use the Right Tool");
	ts->tips << tr("FreeCAD is great for making parametric parts https://freecad.org");
	ts->tips << tr("KiCad is great for making PCB's https://kicad.org");
	ts->tips << tr("Fritzing is great for instructional information https://fritzing.org");
	
	TipSets.append(ts);

	ts = new TipSet;
	ts->heading = tr("moving and selection");
	ts->tips << tr("To constrain the motion of a part to horizontal or vertical, hold down the shift key as you drag it.");
	ts->tips << tr("If you're having trouble selecting a part or a wire (segment), try selecting the part that's in the way and send it to the back: use the Raise and Lower functions on the Part menu or the context menu (right-click menu).");
	ts->tips << tr("To more precisely move a selection of parts, use the arrow keys.  Shift-arrow moves by 10 units.");
	ts->tips << tr("Click on the mouse position indicator in the status bar (below) to switch the units between inch, mm and px.");
	TipSets.append(ts);

	ts = new TipSet;
	ts->heading = tr("curves and bendable legs");
	ts->tips << tr("In Breadboard view, to drag a part with bendable legs while keeping it connected to the breadboard, hold the Alt (Linux: Meta) key down when you start dragging.");
	ts->tips << tr("In Breadboard view, to drag out a wire from the end of a bendable leg, drag with the Alt (Linux: Meta) key down.");
	ts->tips << tr("In Breadboard view, to add a curve to a wire or bendable leg, drag with the Control (Mac: Command) key down.  You can set whether curvy wires are the default in Preferences.");
	TipSets.append(ts);

	ts = new TipSet;
	ts->heading = tr("layers and views");
	ts->tips << tr("To drag the canvas, hold down the space bar and drag with the mouse.");
	ts->tips << tr("To toggle the visibility of layer in a view, go to the view menu and choose one of the view layer items.  Or open up the <b>Layers</b> palette from the <b>Window</b> menu.");
	ts->tips << tr("When you export images from Fritzing, you can choose which layers are exported. Before you choose 'Export...', go into the 'View' menu and hide the layers you don't want to be visible.");
	TipSets.append(ts);

	ts = new TipSet;
	ts->heading = tr("part labels");
	ts->tips << tr("To edit a part label, double-click it, or use the text input widget in the inspector window.");
	ts->tips << tr("To display different properties in a part label, rotate it, or change the font, right-click the label.");
	ts->tips << tr("To move a part label independently from its part, select the part first--both the part and the label will be highlighted. Once the label is selected you can drag it.");
	TipSets.append(ts);

	ts = new TipSet;
	ts->heading = tr("wires and bendpoints");
	ts->tips << tr("To add a bendpoint to a wire, double-click where you want the bendpoint.");
	ts->tips << tr("To delete a bendpoint from a wire, double-click it.");
	ts->tips << tr("In Schematic view, if you drag from a bendpoint with the Alt (Linux: Meta) key down, you will drag out a new wire from that bendpoint.");
	ts->tips << tr("To drag a wire segment (a section of a wire between two bendpoints), drag it with the Alt (Linux: Meta) key down.  If you also hold down the shift key, the wire segment will be constrained to horizontal or vertical motion.");
	ts->tips << tr("Use shift-drag on a wire end or bendpoint to constrain its wire segment to an angle of 45 degrees (or some multiple of 45 degrees).  If the wire segment is connected to other wire segments, the segment you're dragging will snap to make 90 degree angles with the neighboring wire segment.");
	TipSets.append(ts);

	ts = new TipSet;
	ts->heading = tr("connections");
	ts->tips << tr("To see all the connectors connected to a given connector, hold the mouse down on the connector--all the connections will be highlighted.");
	ts->tips << tr("A ratsnest line (very thin 'wire') between connections in one view means that those connections are somehow connected in another view.");
	TipSets.append(ts);

	ts = new TipSet;
	ts->heading = tr("parts editor");
	ts->tips << tr("Check out Parts Editor Help under the <b>Help</b> Menu.");
	ts->tips << tr("Before using the Parts Editor, see whether a Generic IC, Mystery Part, or Pin Header will do the job. Once you drop one of these into your sketch, you can change the number of pins, pin spacing, and other properties.");
	ts->tips << tr("In the Parts Editor, to select a graphic underneath another graphic, use the mouse wheel while holding down the shift key.");
	ts->tips << tr("You can use the Parts Editor to find the SVG file for a part's image for a given view. In the Parts Editor, click on the tab for that view, then use <b>Show in Folder</b> under the <b>File</b> Menu");
	ts->tips << localStorage;
	ts->tips << tr("Do not store your custom part files in the Fritzing installation folder. If you upgrade Fritzing, these files will probably be deleted. Also, files in the Fritzing installation folder will not be saved in sketch (.fzz) files, so you won't be able to share them.");
	TipSets.append(ts);

	ts = new TipSet;
	ts->heading = tr("notes");
	ts->tips << tr("To insert a clickable URL into a note, select some text in the note and type ctrl-l (Mac: command-l). (That's 'l' as in the first letter of 'link'.) To modify an existing link select the linked text and use ctrl-l (command-l) again.");
	TipSets.append(ts);

	ts = new TipSet;
	ts->heading = tr("local file storage");
	ts->tips << localStorage;
	TipSets.append(ts);

}

TipsAndTricks::~TipsAndTricks()
{
}

void TipsAndTricks::hideTipsAndTricks() {
	if (Singleton != nullptr) {
		Singleton->hide();
	}
}

void TipsAndTricks::showTipsAndTricks() {
	if (Singleton == nullptr) {
		new TipsAndTricks();
	}

	Singleton->show();
}

void TipsAndTricks::cleanup() {
	if (Singleton != nullptr) {
		delete Singleton;
		Singleton = nullptr;
	}
}

const QString & TipsAndTricks::randomTip() {
	int tipCount = 0;
	Q_FOREACH (TipSet * tipSet, TipSets) {
		tipCount += tipSet->tips.count();
	}
	if (tipCount == 0) return ___emptyString___;

	int ix = QRandomGenerator::global()->generate() % tipCount;
	tipCount = 0;
	Q_FOREACH (TipSet * tipSet, TipSets) {
		int count = tipSet->tips.count();
		if (tipCount + count > ix) {
			return tipSet->tips.at(ix - tipCount);
		}
		tipCount += count;
	}

	// should not happen
	return ___emptyString___;
}
