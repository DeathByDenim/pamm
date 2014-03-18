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
		"<html>"
		"<body>"
		"<h1>" + tr("Usage") + "</h1>"
		"<p>" + tr("Mods can greatly increase you enjoyment of Planetary Annihilation. So feel free to install as many as you like.") + "</p>"
		"<h2>" + tr("Tabs") + "</h2>"
		"<h3>" + tr("NEWS") + "</h3>"
		"<p>" + tr("The NEWS tab show you recent developments in the modding community. Information about updated of new mods are shown there.") + "</p>"
		"<h3>" + tr("INSTALLED MODS") + "</h3>"
		"<p>" + tr("The INSTALLED MODS tab shows you the mods you currently have installed. The checkbox in front of it indicates if that mod is enabled or not. You can also uninstall mods from here. Clicking on the title will take you to the website of the mod, which is usually a thread in the forum. You can use that for questions or comments about the mod. Note that the PAMM mod cannot be disabled or uninstalled. It is required for the operation of PAMM itself.") + "</p>"
		"<h3>" + tr("AVAILABLE MODS") + "</h3>"
		"<p>" + tr("Mods that are available for download are listed here. The SHOW dropdown menu allows you to filter the content, while the SORT dropdown menu lets you sort the mods. The mod entries themselves tell you if the mod is installed or not and if it requires an update. It also shows you how many times the mod has already been downloaded by fellow Planetary Annihilation players and how often the are liked on the forum.") + "</p>"
		"<h2>" + tr("Buttons") + "</h2>"
		"<h3>" + tr("Update all mods") + "</h3>"
		"<p>" + tr("This will tell you how many mods can be update. A click on this button will then update them all. Alternatively, you can go to the AVAILABLE MODS tabs and update only the mods that you want.") + "</p>"
		"<h3>" + tr("Launch PA") + "</h3>"
		"<p>" + tr("This is purely a convenience button to start PA directly. It's not necessary for mods to work or anything.") + "</p>"
		"<h3>" + tr("Refresh") + "</h3>"
		"<p>" + tr("This refreshes the <b>current</b> tab. Normally, information is cached and only automatically refreshed once per day.") + "</p>"
		"<h3>" + tr("Quit") + "</h3>"
		"<p>" + tr("This quits PAMM") + "</p>"
		"<h2>" + tr("Menu") + "</h2>"
		"<h3>" + tr("File") + "</h3>"
		"<p>" + tr("This has four menu items.") +
		"<p>" + tr("&quot;Show Mod folder&quot; will open the directory where all of the mods are automatically installed by PAMM. This is for advanced use only. Normally, you have no business here.") +
		"<p>" + tr("The menu item &quot;Disable all mods&quot; will deactivate all of the mods. This is useful to see if mods are interfering with the client. The state of the mods is saved however, and you can easily restore it using the &quot;Reenable previously disabled mods&quot; menu item.") + "</p>"
		"<p>" + tr("The menu item &quot;Quit&quot; quits PAMM.") + "</p>"
		"<h3>" + tr("View") + "</h3>"
		"<p>" + tr("The menu item &quot;Show mod filter&quot; will open text field at the bottom of the tab where you can type a filter text. Only mods that have that text will be displayed.") + "</p>"
		"<p>" + tr("The menu item &quot;Compact view&quot; will show you a summarized version of the list so you can get a better overview.") + "</p>"
		"<h3>" + tr("Help") + "</h3>"
		"<p>" + tr("The first menu item, &quot;Help&quot; is what you are looking at now. The second menu item, &quot;About...&quot; gives you some info about PAMM.") + "</p>"
		"<hr><h1>" + tr("More info") + "</h1>"
		"<p>" + tr("For more information, see the <a href=\"https://forums.uberent.com/threads/rel-raevns-pa-mod-manager-for-linux-and-mac-os-x-version-3-1-2.50958/\">PAMM thread</a> in the Planetary Annihilation forums. Furthermore, the source code can be found on <a href=\"https://github.com/DeathByDenim/pamm/\">GitHub</a>.") + "</p>"
		"<p>" + tr("If you want to know how to get your own mod listed in PAMM, look <a href=\"https://forums.uberent.com/threads/guide-getting-your-mod-on-pamm.55189/\">here</a>.") + "</p>"
		"<p></p>"
		"</body>"
		"</html>";
}

#include "helpdialog.moc"
