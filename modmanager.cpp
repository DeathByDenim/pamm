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

#include <QDirIterator>
#include <QSettings>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QImageReader>
#include <QMessageBox>
#include <QMetaType>
#include <fstream>
#include <sstream>
#include <exception>
#include <zip.h>
#include <cerrno>
#include "modmanager.h"
#include "installedmod.h"

ModManager::ModManager(QString PAPath, QString ModPath)
 : PAPath(PAPath), ModPath(ModPath)
{
	ConfigPath = QDir::homePath();
#ifdef __APPLE__
	ConfigPath += "/Library/Application Support/Uber Entertainment/Planetary Annihilation/";
#else
	ConfigPath += "/.local/Uber Entertainment/Planetary Annihilation/";
#endif

	// Clean up zip files in cache
	QDirIterator it(ConfigPath + "modcache/", QStringList("*.zip"), QDir::Files);
	while(it.hasNext())
		QFile::remove(it.next());
	
	Internet = new QNetworkAccessManager(this);
	connect(Internet, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
}

ModManager::~ModManager()
{
	if(Internet)
		delete Internet;
}

void ModManager::findInstalledMods()
{
	QDirIterator it(ModPath, QDirIterator::Subdirectories);
	while (it.hasNext())
	{
		QString filename = it.next();
		if(filename.right(4).compare(".ini", Qt::CaseInsensitive) == 0 && it.fileInfo().isFile())
		{
			InstalledMod *mod = parseIni(filename);
			if(mod != NULL)
			{
				mod->setEnabled(isEnabled(mod->name()));
				connect(mod, SIGNAL(modStateChanged()), this, SLOT(modstateChanged()));
				installedMods.append(mod);
			}
		}
	}
}

InstalledMod *ModManager::parseIni(const QString filename)
{
	QSettings inifile(filename, QSettings::IniFormat);

	inifile.beginGroup("PAMM");
	if(!inifile.contains("Name"))
		return NULL;

	unsigned int Priority = 100;
	QStringList Authors;
	QVariant var = inifile.value("Author", QString(""));
	if(var.type() == QVariant::StringList)
		Authors = var.toStringList();
	else
		Authors = QStringList(var.toString().trimmed());

	InstalledMod *mod = new InstalledMod(
		QFileInfo(filename).baseName(),
		inifile.value("Name", QString("")).toString().trimmed(),
		Authors,
		QUrl(inifile.value("Link", QString("")).toString().trimmed()),
		inifile.value("Category", QString("")).toString().trimmed(),
		inifile.value("Version", QString("")).toString().trimmed(),
		inifile.value("Build", QString("")).toULongLong(),
		inifile.value("Priority", 100U).toUInt(),
		inifile.value("Requires").toStringList(),
		false
	);

	if(inifile.contains("global_mod_list"))
		mod->setScene(inifile.value("global_mod_list"), InstalledMod::global_mod_list);
	if(inifile.contains("connect_to_game"))
		mod->setScene(inifile.value("connect_to_game"), InstalledMod::connect_to_game);
	if(inifile.contains("game_over"))
		mod->setScene(inifile.value("game_over"), InstalledMod::game_over);
	if(inifile.contains("icon_atlas"))
		mod->setScene(inifile.value("icon_atlas"), InstalledMod::icon_atlas);
	if(inifile.contains("live_game"))
		mod->setScene(inifile.value("live_game"), InstalledMod::live_game);
	if(inifile.contains("load_planet"))
		mod->setScene(inifile.value("load_planet"), InstalledMod::load_planet);
	if(inifile.contains("lobby"))
		mod->setScene(inifile.value("lobby"), InstalledMod::lobby);
	if(inifile.contains("matchmaking"))
		mod->setScene(inifile.value("matchmaking"), InstalledMod::matchmaking);
	if(inifile.contains("new_game"))
		mod->setScene(inifile.value("new_game"), InstalledMod::new_game);
	if(inifile.contains("server_browser"))
		mod->setScene(inifile.value("server_browser"), InstalledMod::server_browser);
	if(inifile.contains("settings"))
		mod->setScene(inifile.value("settings"), InstalledMod::settings);
	if(inifile.contains("special_icon_atlas"))
		mod->setScene(inifile.value("special_icon_atlas"), InstalledMod::special_icon_atlas);
	if(inifile.contains("start"))
		mod->setScene(inifile.value("start"), InstalledMod::start);
	if(inifile.contains("system_editor"))
		mod->setScene(inifile.value("system_editor"), InstalledMod::system_editor);
	if(inifile.contains("transit"))
		mod->setScene(inifile.value("transit"), InstalledMod::transit);

	return mod;
}

bool ModManager::isEnabled(const QString Name)
{
	std::ifstream UiModListJS((ModPath + "/ui_mod_list.js").toStdString().c_str());
	if(!UiModListJS.good())
		return false;

	char buffer[100000];
	do
	{
		UiModListJS.getline(buffer, 100000);
		if(QString(buffer).contains("/* " + Name + " */"))
			return true;
	}
	while(!UiModListJS.eof());
	
	return false;
}

void ModManager::sceneToStream(std::ostream& os, const QList<InstalledMod *> modList, const InstalledMod::scene_t scene)
{
	std::stringstream str;
	bool firstpass = true;

	for(QList<InstalledMod *>::const_iterator m = modList.constBegin(); m != modList.constEnd(); ++m)
	{
		if(!(*m)->enabled())
			continue;

		const QStringList sceneList = (*m)->getSceneList(scene);

		if(sceneList.count() > 0)
		{
			if(firstpass)
			{
				str << "        /* PAMM BEGIN (do not edit below) */" << std::endl;
				firstpass = false;
			}
			else
				str << ',' << std::endl;

			str << "        /* " << (*m)->name().toStdString() << " */" << std::endl;
			for(QStringList::const_iterator s = sceneList.constBegin(); s != sceneList.constEnd(); ++s)
			{
				if(s != sceneList.constBegin())
				{
					str << "," << std::endl;
				}
				str << "        " << s->toStdString();
			}
		}
	}
	str << std::endl;
	if(!firstpass)
		str << "        /* PAMM END (do not edit above) */" << std::endl;


	os << str.str();
}

void ModManager::readUiModListJS()
{
	std::ifstream UiModListJS((ModPath + "/ui_mod_list.js").toStdString().c_str());
	if(!UiModListJS.good())
		return;

	char buffer[2048];
	while(!UiModListJS.eof())
	{
		UiModListJS.getline(buffer, 2048);
		QString line(buffer);

		if(line.contains("global_mod_list"))
		{
		}
	}
}

void ModManager::writeUiModListJS()
{
	std::ofstream UiModListJS((ModPath + "/ui_mod_list.js").toStdString().c_str());
	if(!UiModListJS.good())
		return;

	QList<InstalledMod *> prioritySorted(installedMods);
	qSort(prioritySorted.begin(), prioritySorted.end(), InstalledMod::sortPriority);

	UiModListJS <<
		"/* start ui_mod_list */\n"
		"var global_mod_list = [\n";

	sceneToStream(UiModListJS, prioritySorted, InstalledMod::global_mod_list);

	UiModListJS <<
		"];\n\n"
		"var scene_mod_list = {\n"
		"    'connect_to_game': [\n";

	sceneToStream(UiModListJS, prioritySorted, InstalledMod::connect_to_game);

	UiModListJS <<
		"    ],\n"
		"    'game_over': [\n";
		
	sceneToStream(UiModListJS, prioritySorted, InstalledMod::game_over);

	UiModListJS <<
		"    ],\n"
		"    'icon_atlas': [\n";
		
	sceneToStream(UiModListJS, prioritySorted, InstalledMod::icon_atlas);
	
	UiModListJS <<
		"    ],\n"
		"    'live_game': [\n";
		
	sceneToStream(UiModListJS, prioritySorted, InstalledMod::live_game);

	UiModListJS <<
		"    ],\n"
		"    'load_planet': [\n";
	
	sceneToStream(UiModListJS, prioritySorted, InstalledMod::load_planet);

	UiModListJS <<
		"    ],\n"
		"    'lobby': [\n";
	
	sceneToStream(UiModListJS, prioritySorted, InstalledMod::lobby);

	UiModListJS <<
		"    ],\n"
		"    'matchmaking': [\n";
	
	sceneToStream(UiModListJS, prioritySorted, InstalledMod::matchmaking);

	UiModListJS <<
		"    ],\n"
		"    'new_game': [\n";
	
	sceneToStream(UiModListJS, prioritySorted, InstalledMod::new_game);

	UiModListJS <<
		"    ],\n"
		"    'server_browser': [\n";
	
	sceneToStream(UiModListJS, prioritySorted, InstalledMod::server_browser);

	UiModListJS <<
		"    ],\n"
		"    'settings': [\n";
	
	sceneToStream(UiModListJS, prioritySorted, InstalledMod::settings);

	UiModListJS <<
		"    ],\n"
		"    'special_icon_atlas': [\n";
	
	sceneToStream(UiModListJS, prioritySorted, InstalledMod::special_icon_atlas);

	UiModListJS <<
		"    ],\n"
		"    'start': [\n";
	
	sceneToStream(UiModListJS, prioritySorted, InstalledMod::start);

	UiModListJS <<
		"    ],\n"
		"    'system_editor': [\n";
	
	sceneToStream(UiModListJS, prioritySorted, InstalledMod::system_editor);

	UiModListJS <<
		"    ],\n"
		"    'transit': [\n";
	
	sceneToStream(UiModListJS, prioritySorted, InstalledMod::transit);
	
	UiModListJS <<
		"    ]\n"
		"}\n"
		"/* end ui_mod_list */\n";
		
	UiModListJS <<
		"\n/* start rModsList */\n"
		"var rModsList = [\n";

	bool firstpass = true;
	for(QList<InstalledMod *>::const_iterator mod = installedMods.constBegin(); mod != installedMods.constEnd(); ++mod)
	{
		if(!(*mod)->enabled())
			continue;

		if(!firstpass)
			UiModListJS << ",\n";
		firstpass = false;

		UiModListJS << (*mod)->getModUiJsInfoString().toStdString();
	}

	UiModListJS << "\n];\n";
}

AvailableMod::installstate_t ModManager::isInstalled(QString modName, QString modVersion)
{
	for(QList<InstalledMod *>::const_iterator mod = installedMods.constBegin(); mod != installedMods.constEnd(); ++mod)
	{
		if((*mod)->name() == modName)
		{
			if((*mod)->isOlderThan(modVersion))
				return AvailableMod::updateavailable;
			else
				return AvailableMod::installed;
		}
	}

	return AvailableMod::notinstalled;
}

void ModManager::loadAvailableMods(bool refresh)
{
	QFileInfo inifile(ConfigPath + "modcache/modlist.ini");
	if(!inifile.exists() || !inifile.isFile() || inifile.lastModified() > QDateTime::currentDateTime().addDays(2) || refresh)
	{

		QNetworkRequest request(QUrl("http://pamods.github.io/modlist.ini"));
		request.setAttribute(QNetworkRequest::User, QVariant("modlist"));
		Internet->get(request);
	}
	else
	{
		availableMods.clear();
		readAvailableModListIni(ConfigPath + "modcache/modlist.ini");
	}
	
	// Get the mod count.
	QNetworkRequest request(QUrl("http://pa.raevn.com/modcount.php"));

	// Raevn's website doesn't like Qt4 downloading its modcount! Darn you, mod_security!
	// Pretend to be Opera! Everybody loves Opera, right?
	request.setRawHeader("User-Agent" , "Opera/9.80 (X11; Linux x86_64) Presto/2.12.388 Version/12.16");
	request.setAttribute(QNetworkRequest::User, QVariant("modcount"));
	Internet->get(request);
}

void ModManager::replyFinished(QNetworkReply* reply)
{
	if(reply)
	{
		QString type = reply->request().attribute(QNetworkRequest::User).toString();
		if(type == "icon")
		{
			int index = reply->request().attribute((QNetworkRequest::Attribute)(QNetworkRequest::User+1)).toInt();
			if(index >= 0 && index < availableMods.size())
			{
				// Load pixmap
				QImage modIcon = QImage::fromData(reply->readAll());
				availableMods[index]->setPixmap(QPixmap::fromImage(modIcon));
			}
		}
		else if(type == "modlist" || type == "modcount")
		{
			QDir(ConfigPath).mkdir("modcache");
			QString inifilename(ConfigPath + "modcache/");
			if(type == "modlist")
				inifilename += "modlist.ini";
			else if(type == "modcount")
				inifilename += "modcount.ini";
			else
				return;

			QFile inifile(inifilename);
			if(inifile.open(QIODevice::WriteOnly | QIODevice::Text))
			{
				QByteArray arr = reply->readAll();
				inifile.write(arr);
				inifile.flush();
				inifile.close();

				if(type == "modlist")
				{
					availableMods.clear();
					readAvailableModListIni(inifilename);
				}
				else if(type == "modcount")
				{
					updateModCount();
				}
			}
			else
			{
				QMessageBox msgBox;
				msgBox.setText("Couldn't write to \"" + inifilename + "\"");
				msgBox.setIcon(QMessageBox::Warning);
				msgBox.exec();
			}
		}
		else if(type == "moddownload")
		{
			AvailableMod *mod = dynamic_cast<AvailableMod *>(reply->request().attribute((QNetworkRequest::Attribute)(QNetworkRequest::User+1)).value<QWidget *>());
			if(mod)
			{
				QStringList urlpath = reply->url().path().split('/');
				QDir(ConfigPath).mkdir("modcache");
				QString modfilename(ConfigPath + "/modcache/" + urlpath[urlpath.count()-1]);
				QFile modfile(modfilename);
				if(modfile.open(QIODevice::WriteOnly | QIODevice::Text))
				{
					QByteArray arr = reply->readAll();
					modfile.write(arr);
					modfile.flush();
					modfile.close();
					installMod(mod, modfilename);
				}
			}
		}
	}
}

void ModManager::readAvailableModListIni(QString filename)
{
	QSettings modcacheini(filename, QSettings::IniFormat);
	modcacheini.beginGroup("PAMODS");
	QStringList modssections = modcacheini.allKeys();
	modcacheini.endGroup();
	for(QStringList::const_iterator modentry = modssections.constBegin(); modentry != modssections.constEnd(); ++modentry)
	{
		if(modentry->compare("nonpamm") == 0)
			continue;

		modcacheini.beginGroup(*modentry);

		QStringList authors;
		QVariant var = modcacheini.value("Author", QString(""));
		if(var.type() == QVariant::StringList)
			authors = var.toStringList();
		else
			authors = QStringList(var.toString().trimmed());

		QString description;
		var = modcacheini.value("Description", QString(""));
		if(var.type() == QVariant::StringList)
			// Comma's confuse QSettings.
			description = var.toStringList().join(", ");
		else
			description = var.toString().trimmed();

		QString datestring = modcacheini.value("Date").toString();

		/* Hack, because QSettings doesn't care about ordering and raevn bases
		 * the compatibility of a mod on the fact if it is listed before or
		 * after the fake mod, "nonpamm". For now, TOUM is the only noncompatible
		 * mod out there.
		 */ 
		bool compatible = (modentry->compare("TOUM") != 0);
		
		QString name = modcacheini.value("Name", QString("")).toString().trimmed();
		QString version = modcacheini.value("Version", QString("")).toString().trimmed();

		AvailableMod *mod = new AvailableMod(
			*modentry,
			name,
			authors,
			QUrl(modcacheini.value("Link", QString("")).toString().trimmed()),
			modcacheini.value("Category", QString("")).toString().trimmed(),
			version,
			modcacheini.value("Build", QString("")).toUInt(),
			compatible,
			QUrl(modcacheini.value("Download", QString("")).toString().trimmed()),
			description,
			QDate::fromString(datestring, "yyyy/MM/dd"),
			isInstalled(name, version)
		);
		availableMods.append(mod);

		if(modcacheini.contains("Icon"))
		{
			// If it is the default icon. Don't bother downloading.
			if(modcacheini.value("Icon").toString() != "http://pamods.github.io/icons/default.png")
			{
				QNetworkRequest request(QUrl(modcacheini.value("Icon").toUrl()));
				request.setAttribute(QNetworkRequest::User, "icon");
				request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User+1), availableMods.size()-1);
				Internet->get(request);
			}
		}

		modcacheini.endGroup();
	}

	emit availableModsLoaded();
}

void ModManager::downloadMod(AvailableMod* mod)
{
	if(mod)
	{
		QNetworkRequest request(mod->downloadLink());
		request.setAttribute(QNetworkRequest::User, "moddownload");
		request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User+1), QVariant::fromValue((QWidget *)mod));
		QNetworkReply *reply = Internet->get(request);
		connect(reply, SIGNAL(downloadProgress(qint64, qint64)), mod, SLOT(downloadProgress(qint64,qint64)));

	}
}


void ModManager::installMod(AvailableMod* mod, const QString& filename)
{
	connect(this, SIGNAL(progress(int)), mod, SLOT(progress(int)));

	QString inifilename;

	int error;
	struct zip *zippedMod = zip_open(filename.toStdString().c_str(), 0, &error);
	if(zippedMod)
	{
		zip_uint64_t num_files = zip_get_num_entries(zippedMod, 0);
		for(zip_uint64_t i = 0; i < num_files; ++i)
		{
			QString filename(zip_get_name(zippedMod, i, 0));
			if(filename.right(1) == "/")
			{	// This is just a directory. I don't care about those!
				emit progress(75 + (25*(i+1)) / num_files);
				continue;
			}

			if(filename.right(4).compare(".ini", Qt::CaseInsensitive) == 0)
			{
				inifilename = filename;
			}
			
			struct zip_file *zippedFile = zip_fopen_index(zippedMod, i, 0);
			QByteArray data;
			char buffer[1024];
			int len = 0;
			while((len = zip_fread(zippedFile, buffer, 1024)) > 0)
				data.append(buffer, len);

			zip_fclose(zippedFile);

			QFileInfo outFileInfo(ModPath + filename);
			if(QDir::root().mkpath(outFileInfo.absolutePath()))
			{
				QFile outFile(ModPath + filename);
				if(outFile.open(QIODevice::WriteOnly | QIODevice::Text))
				{
					outFile.write(data);
					outFile.close();

					emit progress(75 + (25*(i+1)) / num_files);
				}
				else
				{
					QMessageBox message;
					message.setText(QString("Couldn't write to ") + filename);
					message.setIcon(QMessageBox::Critical);
					message.exec();
					break;
				}
			}
			else
			{
				QMessageBox message;
				message.setText("Couldn't create directory " + outFileInfo.absolutePath());
				message.setIcon(QMessageBox::Critical);
				message.exec();
				break;
			}
		}

		zip_close(zippedMod);
	}
	else
	{
		QMessageBox message;
		char ziperror[256];
		zip_error_to_str(ziperror, 256, error, errno);
		message.setText(QString("Couldn't open ZIP file!\nReason: ") + ziperror);
		message.setIcon(QMessageBox::Critical);
		message.exec();
	}

	disconnect(this, SIGNAL(progress(int)), mod, SLOT(progress(int)));

	InstalledMod *newmod = parseIni(ModPath + inifilename);
	if(newmod)
	{
		// Remove the old outdated mod from the list
		for(size_t i = 0; i < installedMods.count(); i++)
		{
			if(installedMods[i]->name() == newmod->name())
			{
				delete installedMods[i];
				installedMods.removeAt(i);
				break;
			}
		}

		installedMods.append(newmod);

		QNetworkRequest request(QUrl("http://pa.raevn.com/manage.php?download=" + mod->key()));
		request.setRawHeader("User-Agent" , "Opera/9.80 (X11; Linux x86_64) Presto/2.12.388 Version/12.16");
		request.setAttribute(QNetworkRequest::User, QVariant("ignore me"));
		Internet->get(request);

		connect(newmod, SIGNAL(modStateChanged()), this, SLOT(modstateChanged()));
		emit newModInstalled(newmod);

		QStringList requires = newmod->requires();
		for(QList<InstalledMod *>::const_iterator inmod = installedMods.begin(); inmod != installedMods.end(); ++inmod)
		{
			requires.removeAll((*inmod)->key());
		}

		if(!requires.isEmpty())
		{
			QMessageBox installDependenciesMessageBox;
			installDependenciesMessageBox.setText(newmod->name() + " depends on other mods.");
			installDependenciesMessageBox.setInformativeText("Do you want to install those?");
			installDependenciesMessageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			installDependenciesMessageBox.setDefaultButton(QMessageBox::Yes);
			int ret = installDependenciesMessageBox.exec();
			if(ret == QMessageBox::Yes)
			{
				for(QStringList::const_iterator req = requires.begin(); req != requires.end(); ++req)
				{
					for(QList<AvailableMod *>::const_iterator avmod = availableMods.begin(); avmod != availableMods.end(); ++avmod)
					{
						if((*req) == (*avmod)->key())
						{
							(*avmod)->installButtonClicked();
							break;
						}
					}
				}
			}
		}
	}
}

void ModManager::modstateChanged()
{
	writeUiModListJS();
}

void ModManager::updateModCount()
{
	QSettings modCountSettings(ConfigPath + "modcache/modcount.ini", QSettings::IniFormat);
	modCountSettings.beginGroup("PAMM");
	for(QList<AvailableMod *>::iterator mod = availableMods.begin(); mod != availableMods.end(); ++mod)
	{
		if(modCountSettings.contains((*mod)->key()))
			(*mod)->setCount(modCountSettings.value((*mod)->name()).toInt());
	}
	modCountSettings.endGroup();
}


#include "modmanager.moc"
