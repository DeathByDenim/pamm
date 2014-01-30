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

#include <QtGui/QApplication>
#include <QFile>
#include <QDir>
#include <QString>
#include <QStringList>
#include <QMessageBox>
#include <QTranslator>
#include <QLocale>
#include <iostream>
#include <cstdlib>
#include "pamm.h"
#include "modmanager.h"


const char *strModInfoJson =
	"{\n"
	"    \"context\": \"client\",\n"
	"    \"identifier\": \"com.pa.deathbydenim.dpamm\",\n"
	"    \"display_name\": \"PA Mod Manager UI Mods List\",\n"
	"    \"description\": \"Install and manage mods with ease. Based heavily on raevn's mod manager\",\n"
	"    \"forum\": \"https://forums.uberent.com/threads/rel-raevns-ui-mod-manager-for-linux-and-mac-os-x-version-1-8-1.50958/\",\n"
	"    \"author\": \"DeathByDenim\",\n"
	"    \"version\": \"" PAMM_VERSION "\",\n"
	"    \"date\": \"2014/01/21\",\n"
	"    \"build\": \"59607\",\n"
	"    \"signature\": \"not yet implemented\",\n"
	"    \"priority\": 1,\n"
	"    \"enabled\": true,\n"
	"    \"id\": \"PAMM\"\n"
	"}\n";

QString findPAPath(QString configPath)
{
	QString papath;
	QString logdir = configPath + "/log";

	QDir dir(logdir, "PA*.txt", QDir::NoSort, QDir::Files);
	QStringList filelist = dir.entryList();
	for(QStringList::const_iterator filename = filelist.begin(); filename != filelist.constEnd(); ++filename)
	{
		QFile logfile(logdir + "/" + *filename);
		if(!logfile.open(QIODevice::ReadOnly | QIODevice::Text))
			continue;

		int pos;
		while(!logfile.atEnd())
		{
			QString line = logfile.readLine();
			pos = line.indexOf("INFO Coherent host dir");
			if(pos != -1)
			{  // What the?? Pos equals 1? Hmm, better use the C++ version instead...
				papath = line.mid(line.toStdString().find_first_of("INFO Coherent host dir") + 26);
				papath = papath.left(papath.size() - 6);
				break;
			}
		}
	}

	return papath;
}

int main(int argc, char** argv)
{
	QApplication app(argc, argv);

	QString configPath = QDir::homePath();
#ifdef __APPLE__
	configPath += "/Library/Application Support/Uber Entertainment/Planetary Annihilation/";
#else
	configPath += "/.local/Uber Entertainment/Planetary Annihilation/";
#endif

	// Where is this program installed?
	QString progdir(argv[0]);
	int pos = progdir.lastIndexOf(QRegExp("[/\\\\]"));
	if(pos > 0)
	{
		progdir = progdir.left(pos);
	}
	else
		progdir = ".";

	// Where is PA installed? Check the log files first
	QString paPath = findPAPath(configPath);
	if(paPath.isEmpty())
	{
		// Not found in the log files, eh? Then assume the same dir as pamm.
		paPath = progdir;
	}

	for(int i = 0; i < argc; i++)
	{
		if(QString(argv[i]).compare("--papath", Qt::CaseInsensitive) == 0 && i+1 < argc)
		{
			paPath = QString(argv[i+1]);
		}
	}

	QString modPath = configPath + "/mods";
	bool custommoddir = false;
	for(int i = 0; i < argc; i++)
	{
		if(QString(argv[i]).compare("--modpath", Qt::CaseInsensitive) == 0 && i+1 < argc)
		{
			custommoddir = true;
			modPath = QString(argv[i+1]);
			break;
		}
	}

	if(!custommoddir)
		QDir(configPath).mkdir("mods");
	
	QString imgPath = progdir + "/img/";
#ifdef __APPLE__
	if(!QFileInfo(imgPath).exists())
	{
		QDir imgDir(progdir);
		imgDir.cd("../Resources");
		imgPath = imgDir.canonicalPath() + '/';
	}
#endif

	QString i18nPath = progdir + "/i18n/";
#ifdef __APPLE__
	if(!QFileInfo(i18nPath).exists())
	{
		QDir i18nDir(progdir);
		i18nDir.cd("../Resources");
		i18nPath = i18nDir.canonicalPath() + '/';
	}
#endif

	QLocale locale;
	QString language = getenv("LANG");
	if(language.length() > 0)
		locale = QLocale(language);
	else
	{
		language = QString(getenv("LANGUAGE")).split(':')[0];
		if(language.length() > 0)
			locale = QLocale(language);
	}

	std::cout << "Expecting configdir in: \"" << configPath.toStdString() << "\"." << std::endl;
	std::cout << "Expecting executable in: \"" << paPath.toStdString() << "\"." << std::endl;
	std::cout << "Expecting moddir at: \"" << modPath.toStdString() << "\"." << std::endl;
	
	std::cout << "Locale: " << QLocale::languageToString(locale.language()).toStdString() << std::endl;

	QTranslator translator_specific;
	if(translator_specific.load(locale.name(), i18nPath))
		QCoreApplication::installTranslator(&translator_specific);

	// Install the pamm mod.
	QDir modDir(modPath);
	modDir.mkdir("PAMM");
	modDir.mkdir("PAMM/ui");
	modDir.mkdir("PAMM/ui/mods");

	QFile modinfojson(modPath + "/PAMM/modinfo.json");
	if(modinfojson.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		modinfojson.write(strModInfoJson);
		modinfojson.close();
	}
	else
	{
		QMessageBox msgBox;
		msgBox.setText("Couldn't write to \"" + modPath + "\"");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}

	ModManager manager(configPath, paPath, modPath, imgPath, locale);
	manager.findInstalledMods();
	manager.loadAvailableMods(false);

	PAMM pamm(&manager, imgPath);
	QIcon icon(imgPath + "/icon.png");
	pamm.setWindowIcon(icon);
	pamm.setWindowTitle("PA Mod Manager");
	pamm.show();
	return app.exec();
}
