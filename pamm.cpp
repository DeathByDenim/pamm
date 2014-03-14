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
#include "helpdialog.h"
#include "modfilterwidget.h"
#include "modlistwidget.h"

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


PAMM::PAMM(ModManager* manager, const QString& imgPath)
 : Manager(manager), ImgPath(imgPath)
{
	QSettings settings("DeathByDenim", "PAMM");
	restoreGeometry(settings.value("geometry").toByteArray());

	QWidget *mainWidget = new QWidget(this);
	QImageReader *reader = new QImageReader(ImgPath + "img_pa_logo_start_rest.png");
	QImage pa_logo = reader->read();
	reader->setFileName(ImgPath + "img_start_bground_sample.jpg");
	QImage bground = reader->read();
	delete reader;

	QPalette mypalette;
	mypalette.setBrush(mainWidget->backgroundRole(), QBrush(bground));
	mainWidget->setAutoFillBackground(true);
	mainWidget->setPalette(mypalette);

	setCentralWidget(mainWidget);
	
	
	// MENU
	QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
	QAction* quitAction = new QAction(this);
	quitAction->setText(tr("&Quit"));
	quitAction->setIcon(style()->standardIcon(QStyle::SP_DialogCloseButton));
	quitAction->setShortcut(QKeySequence("Ctrl+q"));
	quitAction->setMenuRole(QAction::QuitRole);
	connect(quitAction, SIGNAL(triggered()), SLOT(close()));
	QAction* showModFolderAction = new QAction(this);
	showModFolderAction->setText(tr("&Show Mod folder"));
	showModFolderAction->setIcon(style()->standardIcon(QStyle::SP_DirIcon));
	connect(showModFolderAction, SIGNAL(triggered()), SLOT(showModFolder()));
	fileMenu->addAction(showModFolderAction);
	fileMenu->addAction(quitAction);

	QMenu *viewMenu = menuBar()->addMenu(tr("&View"));
	AvailableModFilterAction = new QAction(this);
	AvailableModFilterAction->setText(tr("Mod filter"));
	AvailableModFilterAction->setCheckable(true);
	AvailableModFilterAction->setShortcut(QKeySequence("Ctrl+m"));
	AvailableModFilterAction->setEnabled(false);
	viewMenu->addAction(AvailableModFilterAction);
	InstalledModFilterAction = new QAction(this);
	InstalledModFilterAction->setText(tr("Mod filter"));
	InstalledModFilterAction->setCheckable(true);
	InstalledModFilterAction->setShortcut(QKeySequence("Ctrl+m"));
	InstalledModFilterAction->setVisible(false);
	viewMenu->addAction(InstalledModFilterAction);
	AvailableCompactViewAction = new QAction(this);
	AvailableCompactViewAction->setText(tr("&Compact view"));
	AvailableCompactViewAction->setCheckable(true);
	AvailableCompactViewAction->setShortcut(QKeySequence("Alt+."));
	AvailableCompactViewAction->setEnabled(false);
	viewMenu->addAction(AvailableCompactViewAction);
	InstalledCompactViewAction = new QAction(this);
	InstalledCompactViewAction->setText(tr("&Compact view"));
	InstalledCompactViewAction->setCheckable(true);
	InstalledCompactViewAction->setShortcut(QKeySequence("Alt+."));
	InstalledCompactViewAction->setVisible(false);
	viewMenu->addAction(InstalledCompactViewAction);
	connect(InstalledCompactViewAction, SIGNAL(triggered(bool)), SLOT(installedCompactViewActionClicked(bool)));
	connect(AvailableCompactViewAction, SIGNAL(triggered(bool)), SLOT(availableCompactViewActionClicked(bool)));


	QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));
	QAction* helpAction = new QAction(this);
	helpAction->setText(tr("&Help"));
	helpAction->setShortcut(QKeySequence("f1"));
	helpAction->setIcon(style()->standardIcon(QStyle::SP_DialogHelpButton));
	connect(helpAction, SIGNAL(triggered()), SLOT(showHelpDialog()));
	QAction* aboutAction = new QAction(this);
	aboutAction->setText(tr("&About..."));
	aboutAction->setIcon(style()->standardIcon(QStyle::SP_MessageBoxInformation));
	aboutAction->setMenuRole(QAction::AboutRole);
	connect(aboutAction, SIGNAL(triggered()), SLOT(showAboutDialog()));
	helpMenu->addAction(helpAction);
	helpMenu->addAction(aboutAction);



	QVBoxLayout *layout = new QVBoxLayout(mainWidget);

	QLabel* logoLabel = new QLabel(this);
	logoLabel->setPixmap(QPixmap::fromImage(pa_logo));
	logoLabel->adjustSize();
	logoLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
	QPalette tt;
	tt.setBrush(logoLabel->backgroundRole(), QBrush(bground));
	logoLabel->setPalette(tt);
	layout->addWidget(logoLabel);

	QLabel *titleLabel = new QLabel(this);
	titleLabel->setText(tr("PA MOD MANAGER"));
	QFont font = titleLabel->font();
	font.setPixelSize(18);
	titleLabel->setFont(font);
	titleLabel->setStyleSheet("QLabel	{ font-size: 2em; font-weight: 700; margin-bottom: 0.67em; margin-top: 0.67em; color: #008888; font-family: \"Verdana\"; margin: 5px; padding: 0;}");

	QGraphicsDropShadowEffect* effect = new QGraphicsDropShadowEffect();
	effect->setColor(QColor(0, 0, 0));
	effect->setOffset(2, 2);
	titleLabel->setGraphicsEffect(effect);
	titleLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

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


	// NEWS TAB
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


	// INSTALLED TAB
	ModListWidget *installedTabWidget = new ModListWidget(this, InstalledModFilterAction, Manager, ModListWidget::ModeInstalled);
	Tabs->addTab(installedTabWidget, tr("INSTALLED MODS"));


	// AVAILABLE TAB
	ModListWidget *availableTabWidget = new ModListWidget(this, AvailableModFilterAction, Manager, ModListWidget::ModeAvailable);
	Tabs->addTab(availableTabWidget, tr("AVAILABLE MODS"));

	connect(Tabs, SIGNAL(currentChanged(int)), SLOT(tabChanged(int)));


	UpdateAllButton = new QPushButton(this);
	UpdateAllButton->setText(tr("Update all mods") + " (0)");
	layout->addWidget(UpdateAllButton);
	connect(UpdateAllButton, SIGNAL(clicked()), SLOT(updateAllButtonClicked()));
	connect(Manager, SIGNAL(availableModsLoaded()), SLOT(updateUpdateAllButton()));
	connect(Manager, SIGNAL(newModInstalled()), SLOT(updateUpdateAllButton()));
	updateUpdateAllButton();


	QWidget *buttonsWidget = new QWidget(this);
	QHBoxLayout *buttonsLayout = new QHBoxLayout(buttonsWidget);
	
	QPushButton *launchPAButton = new QPushButton(buttonsWidget);
	launchPAButton->setText(tr("Launch PA"));
	connect(launchPAButton, SIGNAL(clicked()), SLOT(launchPAButtonClicked()));

	RefreshButton = new QPushButton(buttonsWidget);
	RefreshButton->setText(tr("Refresh"));
	connect(RefreshButton, SIGNAL(clicked()), SLOT(refreshButtonClicked()));

	QPushButton *exitButton = new QPushButton(buttonsWidget);
	exitButton->setText(tr("Exit"));
	connect(exitButton, SIGNAL(clicked()), SLOT(exitButtonClicked()));

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

	Tabs->setCurrentIndex(settings.value("tabs/lastindex", 0).toInt());
	AvailableCompactViewAction->setChecked(settings.value("view/compactavailable", false).toBool());
	InstalledCompactViewAction->setChecked(settings.value("view/compactinstalled", false).toBool());
	AvailableModFilterAction->setChecked(settings.value("view/modfilteravailable", false).toBool());
	InstalledModFilterAction->setChecked(settings.value("view/modfilterinstalled", false).toBool());
	
	availableCompactViewActionClicked(AvailableCompactViewAction->isChecked());
	installedCompactViewActionClicked(InstalledCompactViewAction->isChecked());
	qDebug() << settings.value("view/modfilteravailable");
	qDebug() << settings.value("view/modfilterinstalled");

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

void PAMM::updateUpdateAllButton()
{
	if(UpdateAllButton)
	{
		int count = 0;
		for(QList<Mod *>::const_iterator m = Manager->availableMods.constBegin(); m != Manager->availableMods.constEnd(); ++m)
		{
			AvailableMod *am = dynamic_cast<AvailableMod *>(*m);
			Q_ASSERT(am != NULL);

			if(am->state() == AvailableMod::updateavailable)
				count++;
		}

		UpdateAllButton->setText(tr("Update all mods") + " (" + QString("%1").arg(count) + ")");
		UpdateAllButton->setEnabled(count > 0);
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
	
	InstalledModFilterAction->setEnabled(index != 0);
	InstalledModFilterAction->setVisible(index != 2);
	InstalledCompactViewAction->setEnabled(index != 0);
	InstalledCompactViewAction->setVisible(index != 2);
	AvailableModFilterAction->setEnabled(index != 0);
	AvailableModFilterAction->setVisible(index == 2);
	AvailableCompactViewAction->setEnabled(index != 0);
	AvailableCompactViewAction->setVisible(index == 2);
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
		if(ok && result.count() > 0)
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
/* TODO		if(Tabs)
			Tabs->setCurrentIndex(2);
		if(FilterComboBox)
		{
			int index = FilterComboBox->findText(tr("REQUIRE UPDATE"));
			if(index >= 0)
				FilterComboBox->setCurrentIndex(index);
		}
*/		if(Manager)
			Manager->installAllUpdates();
	}
}

void PAMM::closeEvent(QCloseEvent *event)
{
	QSettings settings("DeathByDenim", "PAMM");
	settings.setValue("geometry", saveGeometry());

	QMainWindow::closeEvent(event);
}

void PAMM::showModFolder()
{
	QDesktopServices::openUrl(QUrl("file://" + Manager->configPath() + "/mods"));
}

void PAMM::showAboutDialog()
{
	QMessageBox::about(this, tr("About"), tr("Version") + " " + PAMM_VERSION + "<br>" + tr("Created by DeathByDenim.") + "<br>" + tr("Based on raevn's Windows-only version.") + "<br><br>" + tr("Source code is available on <a href='https://github.com/DeathByDenim/pamm/'>GitHub</a>"));
}

void PAMM::showHelpDialog()
{
	HelpDialog help(this, ImgPath);
	help.exec();
}

void PAMM::availableCompactViewActionClicked(bool checked)
{
	QSettings settings("DeathByDenim", "PAMM");
	settings.setValue("view/compactavailable", AvailableCompactViewAction->isChecked());
	for(QList<Mod *>::iterator m = Manager->availableMods.begin(); m != Manager->availableMods.end(); ++m)
	{
		(*m)->setCompactView(checked);
	}
}

void PAMM::installedCompactViewActionClicked(bool checked)
{
	QSettings settings("DeathByDenim", "PAMM");
	settings.setValue("view/compactinstalled", InstalledCompactViewAction->isChecked());
	for(QList<Mod *>::iterator m = Manager->installedMods.begin(); m != Manager->installedMods.end(); ++m)
	{
		(*m)->setCompactView(checked);
	}
}

#include "pamm.moc"
