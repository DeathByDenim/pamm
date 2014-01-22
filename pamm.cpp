/*
    Mod manager for Planetary Annihilation. Based of raevn's Windows-only version.
    Copyright (C) 2013  DeathByDenim <jarno@jarno.ca>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "pamm.h"
#include "modmanager.h"
#include "availablemod.h"
#include "installedmod.h"

#include <QtGui/QLabel>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QAction>
#include <QtGui>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <qjson/parser.h>

const char *strNewsStyleSheet =
	".news_container {\n"
	"\tpadding: 4px;\n"
	"}\n"
	"\n"
	".news_item {\n"
	"\tborder-bottom: 1px solid #515151;\n"
	"\tpadding-left: 3px;\n"
	"\tpadding-right: 3px;\n"
	"\tpadding-top: 3px;\n"
	"\tpadding-bottom: 14px;\n"
	"\tmargin-bottom: 14px;\n"
	"}\n"
	"\n"
	".news_title {\n"
	"\tcolor: #008888;\n"
	"\tfont-family: \"Verdana\";\n"
	"\tfont-size: 1.0em;\n"
	"}\n"
	"\n"
	".news_date {\n"
	"\tfont-size: 0.7em;\n"
	"\tcolor: #888888;\n"
	"\tfont-style: italic;\n"
	"\tpadding-bottom: 3px;\n"
	"}\n"
	"\n"
	".news_body {\n"
	"\tfont-size: 0.8em;\n"
	"\tcolor: #F9F9F9;\n"
	"}\n"
	"\n"
	".brackets {\n"
	"\tcolor: #888888;\n"
	"}\n";


PAMM::PAMM(ModManager* manager, QString imgdir)
 : Manager(manager)
{
	QWidget *mainWidget = new QWidget(this);
	setCentralWidget(mainWidget);

	QVBoxLayout *layout = new QVBoxLayout(mainWidget);

	QLabel* logoLabel = new QLabel(this);

	QImageReader *reader = new QImageReader(imgdir + "img_pa_logo_alpha.png");
	QImage pa_logo = reader->read();
	reader->setFileName(imgdir + "img_start_bground_sample.jpg");
	QImage bground = reader->read();
	delete reader;

	QPalette* palette = new QPalette();
	palette->setBrush(QPalette::Background, *(new QBrush(bground)));
	setPalette(*palette);
	setBaseSize(520, 600);

	logoLabel->setPixmap(QPixmap::fromImage(pa_logo));
	logoLabel->adjustSize();

	QLabel *titleLabel = new QLabel(this);
	titleLabel->setText(tr("UI MOD MANAGER"));
	QFont font = titleLabel->font();
	font.setPixelSize(32);
	titleLabel->setFont(font);
	titleLabel->setStyleSheet("QLabel	{ font-size: 2em; font-weight: 700; margin-bottom: 0.67em; margin-top: 0.67em; color: #008888; font-family: \"Verdana\"; margin: 5px; padding: 0;}");

	QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
	effect->setColor(QColor(0, 0, 0));
	effect->setOffset(2, 2);
	titleLabel->setGraphicsEffect(effect);

	layout->addWidget(logoLabel);
	layout->addWidget(titleLabel);
	
	Tabs = new QTabWidget(this);
	Tabs->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	QFont tabfont = Tabs->font();
	tabfont.setBold(true);
	Tabs->setFont(tabfont);
	QPalette pallette = Tabs->palette();
	pallette.setColor(QPalette::WindowText, QColor("#008888"));
	Tabs->setPalette(pallette);
	layout->addWidget(Tabs);

	NewsBrowser = new QTextBrowser(this);
	{
		QFont font = NewsBrowser->font();
		font.setBold(false);
		NewsBrowser->setFont(font);
	}
	loadNews();
	QPalette newsPalette = NewsBrowser->palette();
	newsPalette.setColor(QPalette::Base, QColor(0, 0, 0, 0));
	newsPalette.setColor(QPalette::Text, QColor(200, 200, 200, 255));
	NewsBrowser->setPalette(newsPalette);
	NewsBrowser->document()->setIndentWidth(10);
	NewsBrowser->setOpenLinks(true);
	NewsBrowser->setOpenExternalLinks(true);
	Tabs->addTab(NewsBrowser, tr("NEWS"));


	QScrollArea *scrollAreaInstalled = new QScrollArea(this);
	InstalledModsWidget = new QWidget(scrollAreaInstalled);
	QVBoxLayout *modsLayout = new QVBoxLayout(InstalledModsWidget);

	if(Manager->installedMods.count() == 0)
	{
		QLabel *nomodsLabel = new QLabel(InstalledModsWidget);
		nomodsLabel->setText(tr("No mods found"));
		nomodsLabel->setAlignment(Qt::AlignHCenter);
		nomodsLabel->setStyleSheet("QLabel {color: #ffffff}");
		modsLayout->addWidget(nomodsLabel);
	}
	else
	{
		for(QList<InstalledMod *>::const_iterator m = Manager->installedMods.constBegin(); m != Manager->installedMods.constEnd(); ++m)
		{
			(*m)->setParent(InstalledModsWidget);
			modsLayout->addWidget(*m);
			connect(*m, SIGNAL(updateMe()), Manager, SLOT(downloadMod()));
			connect(*m, SIGNAL(uninstallMe()), Manager, SLOT(uninstallMod()));
		}
		modsLayout->addStretch();
	}
	scrollAreaInstalled->setWidget(InstalledModsWidget);
	scrollAreaInstalled->setWidgetResizable(true);
	Tabs->addTab(scrollAreaInstalled, tr("INSTALLED MODS"));

	QWidget *availableTabWidget = new QWidget(this);
	QVBoxLayout *availableTabWidgetLayout = new QVBoxLayout(availableTabWidget);

	QWidget *availableMenuWidget = new QWidget(availableTabWidget);
	QHBoxLayout *availableMenuWidgetLayout = new QHBoxLayout(availableMenuWidget);
	availableTabWidgetLayout->addWidget(availableMenuWidget);

	QLabel *filterLabel = new QLabel(availableMenuWidget);
	filterLabel->setText(tr("SHOW:"));
	availableMenuWidgetLayout->addWidget(filterLabel);

	FilterComboBox = new QComboBox(availableMenuWidget);
	FilterComboBox->addItem(tr("ALL"));
	FilterComboBox->addItem(tr("INSTALLED"));
	FilterComboBox->addItem(tr("REQUIRE UPDATE"));
	FilterComboBox->addItem(tr("NOT INSTALLED"));
	connect(FilterComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(filterIndexChanged(const QString &)));

	availableMenuWidgetLayout->addWidget(FilterComboBox);
	availableMenuWidgetLayout->addStretch();
	availableMenuWidgetLayout->addStrut(10);

	QLabel *sortLabel = new QLabel(availableMenuWidget);
	sortLabel->setText("SORT:");
	availableMenuWidgetLayout->addWidget(sortLabel);

	QComboBox *sortComboBox = new QComboBox(availableMenuWidget);
	sortComboBox->addItem(tr("RANDOM"));
	sortComboBox->addItem(tr("LAST UPDATED"));
	sortComboBox->addItem(tr("TITLE"));
	sortComboBox->addItem(tr("AUTHOR"));
	sortComboBox->addItem(tr("BUILD"));
	sortComboBox->addItem(tr("DOWNLOADS"));
	sortComboBox->addItem(tr("LIKES"));
	connect(sortComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(sortIndexChanged(const QString &)));

	availableMenuWidgetLayout->addWidget(sortComboBox);
	availableMenuWidgetLayout->addStretch();

	QScrollArea *scrollAreaAvailable = new QScrollArea(availableTabWidget);
	availableModsWidget = new QWidget(scrollAreaAvailable);
	QFont availableModsWidgetFont = availableModsWidget->font();
	availableModsWidgetFont.setBold(false);
	availableModsWidget->setFont(availableModsWidgetFont);
	modsLayout = new QVBoxLayout(availableModsWidget);
	scrollAreaAvailable->setWidget(availableModsWidget);
	scrollAreaAvailable->setWidgetResizable(true);
	availableTabWidgetLayout->addWidget(scrollAreaAvailable);

	Tabs->addTab(availableTabWidget, tr("AVAILABLE MODS"));
	connect(Tabs, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));


	UpdateAllButton = new QPushButton(this);
	UpdateAllButton->setText(tr("Update all mods") + " (0)");
	layout->addWidget(UpdateAllButton);
	connect(UpdateAllButton, SIGNAL(clicked()), this, SLOT(updateAllButtonClicked()));

	populateAvailableModsWidget(true);

	QWidget *buttonsWidget = new QWidget(this);
	QHBoxLayout *buttonsLayout = new QHBoxLayout(buttonsWidget);
	
	QPushButton *launchPAButton = new QPushButton(buttonsWidget);
	launchPAButton->setText(tr("Launch PA"));
	connect(launchPAButton, SIGNAL(clicked()), this, SLOT(launchPAButtonClicked()));

	RefreshButton = new QPushButton(buttonsWidget);
	RefreshButton->setText(tr("Refresh"));
	connect(RefreshButton, SIGNAL(clicked()), this, SLOT(refreshButtonClicked()));

	QPushButton *exitButton = new QPushButton(buttonsWidget);
	exitButton->setText(tr("Exit"));
	connect(exitButton, SIGNAL(clicked()), this, SLOT(exitButtonClicked()));

	QLabel *versionLabel = new QLabel(buttonsWidget);
	versionLabel->setText(tr("Version") + " " PAMM_VERSION);
	versionLabel->setStyleSheet("QLabel {color: #008888; font-style: italic; }");

	QLabel *creditLabel = new QLabel(buttonsWidget);
	creditLabel->setText(tr(", created by DeathByDenim (based on Raevn)"));
	creditLabel->setStyleSheet("QLabel {color: #ffffff; font-style: italic; }");

	buttonsLayout->addWidget(launchPAButton);
	buttonsLayout->addWidget(RefreshButton);
	buttonsLayout->addWidget(exitButton);
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(versionLabel);
	buttonsLayout->addWidget(creditLabel);

	layout->addWidget(buttonsWidget);
	
	centralWidget()->setLayout(layout);

	connect(Manager, SIGNAL(availableModsLoaded()), this, SLOT(availableModsLoaded()));
	connect(Manager, SIGNAL(newModInstalled(InstalledMod *)), this, SLOT(newModInstalled(InstalledMod *)));

	sortIndexChanged(tr("RANDOM"));

	Tabs->setCurrentIndex(QSettings("DeathByDenim", "PAMM").value("tabs/lastindex", 0).toInt());

	checkForUpdate();
}

PAMM::~PAMM()
{
}

void PAMM::exitButtonClicked()
{
	close();
}

void PAMM::launchPAButtonClicked()
{
	// This is very Linux specific. No idea if this works on Mac OS...
	int result = system(("nohup " + Manager->paPath() + "/PA &").toStdString().c_str());
}

void PAMM::refreshButtonClicked()
{
	if(Tabs)
	{
		if(Tabs->currentIndex() == 2)
			Manager->loadAvailableMods(true);
		else if(Tabs->currentIndex() == 0)
		{
			NewsBrowser->clear();
			QNetworkAccessManager *newsManager = new QNetworkAccessManager(this);
			connect(newsManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(newsReplyFinished(QNetworkReply*)));

			QNetworkRequest request(QUrl("http://pamods.github.io/news.html"));
			request.setRawHeader("User-Agent" , "Opera/9.80 (X11; Linux x86_64) Presto/2.12.388 Version/12.16");
			newsManager->get(request);
		}
	}
}

void PAMM::availableModsLoaded()
{
	populateAvailableModsWidget(true);
}

void PAMM::clearWidgets(QLayout *layout, bool deleteWidgets)
{
	while (QLayoutItem* item = layout->takeAt(0))
	{
		if(deleteWidgets)
		{
			if(QWidget* widget = item->widget())
				delete widget;

			if(QLayout* childLayout = item->layout())
				clearWidgets(childLayout, deleteWidgets);
		}
		else
		{
			AvailableMod *mod = dynamic_cast<AvailableMod *>(item->widget());
			if(mod)
			{
				mod->setParent(NULL);
				disconnect(mod, SIGNAL(installMe()), Manager, SLOT(downloadMod()));
			}
		}

		delete item;
	}
}

void PAMM::populateAvailableModsWidget(bool deleteWidgets, ModFilter filter)
{
	QVBoxLayout *modsLayout = dynamic_cast<QVBoxLayout *>(availableModsWidget->layout());
	if(!modsLayout)
		return;

	setUpdatesEnabled(false);
	// Delete anything that's there now.
	clearWidgets(modsLayout, deleteWidgets);

	if(Manager->availableMods.count() == 0)
	{
/*		QLabel *nomodsLabel = new QLabel(availableModsWidget);
		nomodsLabel->setText("No mods found");
		nomodsLabel->setAlignment(Qt::AlignHCenter);
		nomodsLabel->setStyleSheet("QLabel {color: #ffffff}");
		modsLayout->addWidget(nomodsLabel);
*/	}
	else
	{
		for(QList<AvailableMod *>::const_iterator m = Manager->availableMods.constBegin(); m != Manager->availableMods.constEnd(); ++m)
		{
			if(
				filter == All ||
				(filter == Installed && (*m)->state() == AvailableMod::installed) ||
				(filter == Require_update && (*m)->state() == AvailableMod::updateavailable) ||
				(filter == Not_installed && (*m)->state() == AvailableMod::notinstalled)
			)
			{
				(*m)->setParent(availableModsWidget);
				connect(*m, SIGNAL(installMe()), Manager, SLOT(downloadMod()));
				modsLayout->addWidget(*m);
			}
		}
		modsLayout->addStretch();
	}

	updateUpdateAllButton();

	setUpdatesEnabled(true);

}

void PAMM::updateUpdateAllButton()
{
	if(UpdateAllButton)
	{
		QList<AvailableMod *> updatableMods;
		for(QList<AvailableMod *>::const_iterator m = Manager->availableMods.constBegin(); m != Manager->availableMods.constEnd(); ++m)
		{
			if((*m)->state() == AvailableMod::updateavailable)
			{
				updatableMods.push_back(*m);
			}
		}

		UpdateAllButton->setText(tr("Update all mods") + " (" + QString("%1").arg(updatableMods.length()) + ")");
		UpdateAllButton->setEnabled(!updatableMods.empty());
	}
}

void PAMM::loadNews()
{
	if(NewsBrowser)
	{
		QFileInfo newsFileInfo(Manager->configPath() + "/pamm_cache/news.html");
		if(newsFileInfo.exists() && newsFileInfo.isFile() && newsFileInfo.lastModified() > QDateTime::currentDateTime().addDays(-1))
		{
			QTextDocument *newsDocument = new QTextDocument(this);
			newsDocument->addResource(QTextDocument::StyleSheetResource, QUrl("format.css"), strNewsStyleSheet);
			QFile newsFile(Manager->configPath() + "/pamm_cache/news.html");
			if(newsFile.open(QIODevice::ReadOnly | QIODevice::Text))
			{
				QString html(newsFile.readAll());
				newsDocument->setHtml(html);
				NewsBrowser->setDocument(newsDocument);
			}
		}
		else
		{
			NewsBrowser->clear();
			QNetworkAccessManager *newsManager = new QNetworkAccessManager(this);
			connect(newsManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(newsReplyFinished(QNetworkReply*)));

			QNetworkRequest request(QUrl("http://pamods.github.io/news.html"));
			request.setRawHeader("User-Agent" , "Opera/9.80 (X11; Linux x86_64) Presto/2.12.388 Version/12.16");
			newsManager->get(request);
		}
	}
}

void PAMM::newsReplyFinished(QNetworkReply* reply)
{
	if(reply)
	{
		QDir(Manager->configPath()).mkpath("pamm_cache");
		QFile newsFile(Manager->configPath() + "/pamm_cache/news.html");
		if(newsFile.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			newsFile.write(
				"<html>"
				"<head>"
				"<link rel=\"stylesheet\" type=\"text/css\" href=\"format.css\">"
				"</head>"
				"<body>"
			);
			QByteArray arr = reply->readAll();
			newsFile.write(arr);
			newsFile.write(
				"</body>"
				"</html>"
			);
			newsFile.flush();
			newsFile.close();

			QTextDocument *newsDocument = new QTextDocument(this);
			newsDocument->addResource(QTextDocument::StyleSheetResource, QUrl("format.css"), strNewsStyleSheet);
			QFile newsFile(Manager->configPath() + "/pamm_cache/news.html");
			if(newsFile.open(QIODevice::ReadOnly | QIODevice::Text))
			{
				QString html(newsFile.readAll());
				newsDocument->setHtml(html);
				NewsBrowser->setDocument(newsDocument);
			}
		}
		else
		{
			QMessageBox msgBox;
			msgBox.setText(tr("Couldn't write to") + " \"" + newsFile.fileName() + "\"");
			msgBox.setIcon(QMessageBox::Warning);
			msgBox.exec();
		}
		reply->manager()->deleteLater();
	}
}

void PAMM::tabChanged(int index)
{
	QSettings("DeathByDenim", "PAMM").setValue("tabs/lastindex", index);
	RefreshButton->setEnabled(index != 1);
}

void PAMM::newModInstalled(InstalledMod* newmod)
{
	newmod->setParent(InstalledModsWidget);
	QVBoxLayout *modlayout = dynamic_cast<QVBoxLayout *>(InstalledModsWidget->layout());
	if(modlayout)
	{
		if(modlayout->count() <= 1)
		{	// No mods are installed
			QLayoutItem *item = modlayout->takeAt(0);
			if(QWidget* widget = item->widget())
				delete widget;

			modlayout->addWidget(newmod);
			modlayout->addStretch();
		}
		else
			modlayout->insertWidget(modlayout->count() - 1, newmod);

		newmod->setEnabled(true);
	}

	connect(newmod, SIGNAL(updateMe()), Manager, SLOT(downloadMod()));
	connect(newmod, SIGNAL(uninstallMe()), Manager, SLOT(uninstallMod()));

	updateUpdateAllButton();
}

void PAMM::filterIndexChanged(const QString& text)
{
	ModFilter filter;
	if(text == tr("ALL"))
	{
		filter = All;
	}
	else if(text == tr("INSTALLED"))
	{
		filter = Installed;
	}
	else if(text == tr("REQUIRE UPDATE"))
	{
		filter = Require_update;
	}
	else if(text == tr("NOT INSTALLED"))
	{
		filter = Not_installed;
	}
	else
		filter = All;

	populateAvailableModsWidget(false, filter);
}

void PAMM::sortIndexChanged(const QString& text)
{
	if(text == tr("RANDOM"))
	{
		qSort(Manager->availableMods.begin(), Manager->availableMods.end(), AvailableMod::sortRandom);
	}
	else if(text == tr("LAST UPDATED"))
	{
		qSort(Manager->availableMods.begin(), Manager->availableMods.end(), AvailableMod::sortLastUpdated);
	}
	else if(text == tr("TITLE"))
	{
		qSort(Manager->availableMods.begin(), Manager->availableMods.end(), AvailableMod::sortTitle);
	}
	else if(text == tr("AUTHOR"))
	{
		qSort(Manager->availableMods.begin(), Manager->availableMods.end(), AvailableMod::sortAuthor);
	}
	else if(text == tr("BUILD"))
	{
		qSort(Manager->availableMods.begin(), Manager->availableMods.end(), AvailableMod::sortBuild);
	}
	else if(text == tr("DOWNLOADS"))
	{
		qSort(Manager->availableMods.begin(), Manager->availableMods.end(), AvailableMod::sortDownloads);
	}
	else if(text == tr("LIKES"))
	{
		qSort(Manager->availableMods.begin(), Manager->availableMods.end(), AvailableMod::sortLikes);
	}

	populateAvailableModsWidget(false);
}

void PAMM::checkForUpdate()
{
	QNetworkAccessManager *updateNetworkAccessManager = new QNetworkAccessManager(this);
	connect(updateNetworkAccessManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(updateReplyFinished(QNetworkReply*)));

	QNetworkRequest request(QUrl("https://api.github.com/repos/DeathByDenim/pamm/releases"));
	updateNetworkAccessManager->get(request);
}

void PAMM::updateReplyFinished(QNetworkReply* reply)
{
	if(reply)
	{
		QJson::Parser parser;
		bool ok;
		QVariantList result = parser.parse(reply->readAll(), &ok).toList();
		if(ok)
		{
			QString latest_version = result[0].toMap()["tag_name"].toString();
			if(latest_version.length() > 0)
			{
				if(latest_version.mid(1) != PAMM_VERSION)
				{	// Update available
					QMessageBox msgBox;
					msgBox.setText(tr("New update available for the PA Mod Manager."));
					msgBox.setInformativeText(tr("Download now?"));
					msgBox.setIcon(QMessageBox::Information);
					msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
					msgBox.setDefaultButton(QMessageBox::Yes);
					if(msgBox.exec() == QMessageBox::Yes)
					{
						QDesktopServices::openUrl(QUrl("https://forums.uberent.com/threads/rel-raevns-pa-mod-manager-for-linux-and-mac-os-x-version-3-0-2.50958/"));
					}
				}
			}
		}
	}
}

void PAMM::updateAllButtonClicked()
{
	if(UpdateAllButton)
	{
		if(Tabs)
			Tabs->setCurrentIndex(2);
		if(FilterComboBox)
		{
			int index = FilterComboBox->findText(tr("REQUIRE UPDATE"));
			if(index >= 0)
				FilterComboBox->setCurrentIndex(index);
		}
		if(Manager)
			Manager->installAllUpdates();
	}
}

#include "pamm.moc"
