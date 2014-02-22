/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Jarno van der Kolk <jarno@jarno.ca>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QTextBrowser>
#include "helpdialog.h"

HelpDialog::HelpDialog(QWidget* parent, const QString &imgPath)
 : QDialog(parent)
{
	setWindowTitle("Help");
	
	resize(600, 600);

	QPalette mypalette(palette());
	mypalette.setBrush(backgroundRole(), QBrush(QImage(imgPath + "/img_start_bground_sample.jpg")));
	setPalette(mypalette);


	QVBoxLayout *layout = new QVBoxLayout(this);
	setLayout(layout);

	QTextBrowser *helpTextEdit = new QTextBrowser(this);
	helpTextEdit->setReadOnly(true);
	helpTextEdit->setHtml(getHelp());
	helpTextEdit->setOpenLinks(true);
	helpTextEdit->setOpenExternalLinks(true);
	QPalette helpPalette = palette();
	helpPalette.setColor(QPalette::Base, QColor(0, 0, 0, 0));
	helpPalette.setColor(QPalette::Text, QColor(200, 200, 200, 255));
	helpTextEdit->setPalette(helpPalette);

	layout->addWidget(helpTextEdit);

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	layout->addWidget(buttonBox);

	buttonBox->setFocus();
}

HelpDialog::~HelpDialog()
{
}

QString HelpDialog::getHelp()
{
	return
		tr(
		"<html>"
		"<body>"
		"<h1>Usage</h1>"
		"<p>Mods can greatly increase you enjoyment of Planetary Annihilation. So feel free to install as many as you like.</p>"
		"<h2>Tabs</h2>"
		"<h3>NEWS</h3>"
		"<p>The NEWS tab show you recent developments in the modding community. Information about updated of new mods are shown there.</p>"
		"<h3>INSTALLED MODS</h3>"
		"<p>The INSTALLED MODS tab shows you the mods you currently have installed. The checkbox in front of it indicates if that mod is enabled or not. You can also uninstall mods from here. Clicking on the title will take you to the website of the mod, which is usually a thread in the forum. You can use that for questions or comments about the mod. Note that the PAMM mod cannot be disabled or uninstalled. It is required for the operation of PAMM itself.</li>"
		"<h3>AVAILABLE MODS</h3>"
		"<p>Mods that are available for download are listed here. The SHOW dropdown menu allows you to filter the content, while the SORT dropdown menu lets you sort the mods. The mod entries themselves tell you if the mod is installed or not and if it requires an update. It also shows you how many times the mod has already been downloaded by fellow Planetary Annihilation players and how often the are liked on the forum.</p>"
		"<h2>Buttons</h2>"
		"<h3>Update all mods</h3>"
		"<p>This will tell you how many mods can be update. A click on this button will then update them all. Alternatively, you can go to the AVAILABLE MODS tabs and update only the mods that you want.</p>"
		"<h3>Start PA</h3>"
		"<p>This is purely a convenience button to start PA directly. It's not necessary for mods to work or anything.</p>"
		"<h3>Refresh</h3>"
		"<p>This refreshes the <b>current</b> tab. Normally, information is cached and only automatically refreshed once per day.</p>"
		"<h3>Quit</h3>"
		"<p>This quits PAMM</p>"
		"<h2>Menu</h2>"
		"<h3>File</h3>"
		"<p>This has two menu items, &quot;Show Mod folder&quot; and &quot;Quit&quot;. The first will open the directory where all of the mods are automatically installed by PAMM. This is for advanced use only. Normally, you have no business here. The menu item &quot;Quit&quot; quits PAMM.</p>"
		"<h3>Help</h3>"
		"<p>The first menu item, &quot;Help&quot; is what you are looking at now. The second menu item, &quot;About...&quot; gives you some info about PAMM.</p>"
		"<hr><h1>More info</h1>"
		"<p>For more information, see the <a href=\"https://forums.uberent.com/threads/rel-raevns-pa-mod-manager-for-linux-and-mac-os-x-version-3-1-2.50958/\">PAMM thread</a> in the Planetary Annihilation forums. Furthermore, the source code can be found on <a href=\"https://github.com/DeathByDenim/pamm/\">GitHub</a>.</p>"
		"<p>If you want to know how to get your own mod listed in PAMM, look <a href=\"https://forums.uberent.com/threads/guide-getting-your-mod-on-pamm.55189/\">here</a>.</p>"
		"<p></p>"
		"</body>"
		"</html>"
		);
}

#include "helpdialog.moc"
