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
#include <iostream>
#include "pamm.h"
#include "modmanager.h"

QString findPAPath()
{
	QString papath;
	QString logdir(QDir::homePath());
#ifdef __APPLE__
	logdir += "/Library/Application Support/Uber Entertainment/Planetary Annihilation/log";
#else
	logdir += "/.local/Uber Entertainment/Planetary Annihilation/log";
#endif

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
			if(pos = line.indexOf("INFO Coherent host dir") != -1)
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
	QString papath = findPAPath();
	if(papath.isEmpty())
	{
		// Not found in the log files, eh? Then assume the same dir as pamm.
		papath = progdir;
	}

	for(int i = 0; i < argc; i++)
	{
		if(QString(argv[i]).compare("--papath", Qt::CaseInsensitive) == 0 && i+1 < argc)
		{
			papath = QString(argv[i+1]);
		}
	}

#ifdef __APPLE__
	QString modpath = papath + "/../Resources/ui/mods/";
#else
	QString modpath = papath + "/media/ui/mods/";
#endif
	for(int i = 0; i < argc; i++)
	{
		if(QString(argv[i]).compare("--modpath", Qt::CaseInsensitive) == 0 && i+1 < argc)
		{
			modpath = QString(argv[i+1]);
		}
	}

	std::cout << "Expecting executable in: \"" << papath.toStdString() << "\"." << std::endl;
	std::cout << "Expecting moddir at: \"" << modpath.toStdString() << "\"." << std::endl;
	ModManager manager(papath, modpath);
	manager.readUiModListJS();
	manager.findInstalledMods();
	manager.loadAvailableMods(false);

	PAMM pamm(&manager, progdir);
	pamm.show();
	return app.exec();
}
