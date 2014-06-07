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
#include <QFile>
#include <QTextStream>
#include <QLocale>
#include <fstream>
#include <sstream>
#include <iostream>
#include <exception>
#include <zip.h>
#include <qjson/parser.h>
#include <qjson/serializer.h>
#include <cerrno>
#include "modmanager.h"
#include "installedmod.h"

ModManager::ModManager(QString ConfigPath, QString PAPath, QString ModPath, QString ImgPath, QLocale Locale)
 : ConfigPath(ConfigPath), PAPath(PAPath), ModPath(ModPath), ImgPath(ImgPath), Locale(Locale)
{
	// Clean up zip files in cache
	QDirIterator it(ConfigPath + "pamm_cache/", QStringList("*.zip"), QDir::Files);
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
	QDirIterator it(ModPath, QDirIterator::Subdirectories /*| QDirIterator::FollowSymlinks*/); // No symlinks, since PA doesn't support it anyway.
	while (it.hasNext())
	{
		QString filename = it.next();
		if(QFileInfo(filename).fileName().compare("modinfo.json", Qt::CaseInsensitive) == 0)
		{
			InstalledMod *mod = parseJson(filename);
			if(mod != NULL)
			{
				connect(mod, SIGNAL(modStateChanged()), SLOT(modstateChanged()));
				connect(mod, SIGNAL(updateMe()), SLOT(downloadMod()));
				connect(mod, SIGNAL(uninstallMe()), SLOT(uninstallMod()));
				installedMods.append(mod);
			}
		}
	}

	qSort(installedMods.begin(), installedMods.end(), Mod::sortTitle);

	refreshReverseRequirements();
}

void ModManager::parseScenes(const QVariantMap& result, InstalledMod* mod, bool allowunknown)
{
	QList<QString> keys = result.keys();
	for(QList<QString>::const_iterator key = keys.constBegin(); key != keys.constEnd(); ++key)
	{
		if(allowunknown || InstalledMod::KnownScenes.contains(*key))
		{
			mod->setScene(result[*key], *key);
		}
	}
}

InstalledMod *ModManager::parseJson(const QString filename)
{
	QJson::Parser parser;

	QFile jsonFile(filename);
	if(!jsonFile.open(QIODevice::ReadOnly | QIODevice::Text))
		return NULL;

	bool ok;
	QVariantMap result = parser.parse(&jsonFile, &ok).toMap();
	if(!ok)
		return NULL;

	unsigned int priority = result["priority"].toUInt(&ok);
	if(!ok || priority == 0)
		priority = 100;

	InstalledMod *mod = new InstalledMod(
		QFileInfo(filename).absoluteDir().dirName(),
		result["context"].toString(),
		result["identifier"].toString(),
		readLocaleField(result, "display_name").toString(),
		readLocaleField(result, "description").toString(),
		result["author"].toString(),
		result["version"].toString(),
		result["signature"].toString(),
		priority,
		result["enabled"].toBool(),
		result["id"].toString(),
		result["forum"].toUrl(),
		result["category"].toStringList(),
		result["requires"].toStringList(),
		QDate::fromString(result["date"].toString(), "yyyy/MM/dd"),
		result["build"].toString()
	);
	
	parseScenes(result, mod, false);

	if(result.contains("scenes"))
		parseScenes(result["scenes"].toMap(), mod, true);

	mod->setCompleteJson(result);

	return mod;
}

void ModManager::sceneToStream(std::ostream& os, const QList< Mod* > modList, const QString scene)
{
	std::stringstream str;
	bool firstpass = true;

	for(QList<Mod *>::const_iterator m = modList.constBegin(); m != modList.constEnd(); ++m)
	{
		InstalledMod *im = dynamic_cast<InstalledMod *>(*m);
		Q_ASSERT(im != NULL);

		if(!im->enabled())
			continue;

		const QStringList sceneList = im->getSceneList(scene);

		if(sceneList.count() > 0)
		{
			if(firstpass)
			{
				firstpass = false;
			}
			else
				str << ',' << std::endl;

			str << "        /* " << (*m)->displayName().toStdString() << " */" << std::endl;
			for(QStringList::const_iterator s = sceneList.constBegin(); s != sceneList.constEnd(); ++s)
			{
				if(s != sceneList.constBegin())
				{
					str << "," << std::endl;
				}
				str << "        '" << s->toStdString() << "'";
			}
		}
	}
	str << std::endl;

	os << str.str();
}

void ModManager::writeUiModListJS()
{
	std::ofstream UiModListJS((ModPath + "/PAMM/ui/client_mods/ui_mod_list.js").toStdString().c_str());
	if(!UiModListJS.good())
		return;

	QList<Mod *> prioritySorted(installedMods);
	qSort(prioritySorted.begin(), prioritySorted.end(), InstalledMod::sortPriority);

	UiModListJS << "/* DO NOT EDIT. This file is overwritten by pamm. */" << std::endl << std::endl;

	UiModListJS <<
		"/* start ui_mod_list */\n"
		"var global_mod_list = [\n";

	sceneToStream(UiModListJS, prioritySorted, "global_mod_list");


	UiModListJS <<
		"];\n\n"
		"var scene_mod_list = {\n";
	QMap<QString,QStringList> scenes;
	for(QList<Mod *>::const_iterator m = prioritySorted.constBegin(); m != prioritySorted.constEnd(); ++m)
	{
		InstalledMod *instmod = dynamic_cast<InstalledMod *>(*m);
		if(instmod)
		{
			QMap<QString,QStringList> instscenes(instmod->scenesFiles());
			for(QMap<QString,QStringList>::const_iterator s = instscenes.constBegin(); s != instscenes.constEnd(); ++s)
			{
				QStringList files = scenes[s.key()];
				files += s.value();
				scenes[s.key()] = files;
			}
		}
	}

	bool firstpass_s = false;
	for(QMap<QString,QStringList>::const_iterator s = scenes.constBegin(); s != scenes.constEnd(); ++s)
	{
		if(!firstpass_s)
			firstpass_s = true;
		else
			UiModListJS << ',';
		UiModListJS << std::endl;
		UiModListJS << "    '" << s.key().toStdString() << "': [";
		bool firstpass_file = false;
		for(QStringList::const_iterator file = s.value().constBegin(); file != s.value().constEnd(); ++file)
		{
			if(!firstpass_file)
				firstpass_file = true;
			else
				UiModListJS << ',';
			UiModListJS << std::endl;
			UiModListJS << "        '" << file->toStdString() << "'";
		}
		UiModListJS << std::endl << "    ]";
	}
	UiModListJS << std::endl;

	UiModListJS <<
		"}\n"
		"/* end ui_mod_list */\n";
		
	UiModListJS <<
		"\n/* start rModsList */\n"
		"var rModsList = [\n";

	bool firstpass = true;
	for(QList<Mod *>::const_iterator mod = installedMods.constBegin(); mod != installedMods.constEnd(); ++mod)
	{
		InstalledMod *imod = dynamic_cast<InstalledMod *>(*mod);
		Q_ASSERT(imod != NULL);

		if(!imod->enabled())
			continue;

		if(!firstpass)
			UiModListJS << ",\n";
		firstpass = false;

		UiModListJS << imod->getModUiJsInfoString().toStdString();
	}

	UiModListJS << "\n];\n";
}

AvailableMod::installstate_t ModManager::isInstalled(QString modKey, QString modVersion)
{
	for(QList<Mod *>::const_iterator mod = installedMods.constBegin(); mod != installedMods.constEnd(); ++mod)
	{
		InstalledMod *imod = dynamic_cast<InstalledMod *>(*mod);
		Q_ASSERT(imod != NULL);

		if(imod->key() == modKey)
		{
			if(imod->compareVersion(modVersion) < 0)
				return AvailableMod::updateavailable;
			else
				return AvailableMod::installed;
		}
	}

	return AvailableMod::notinstalled;
}

void ModManager::loadAvailableMods(bool refresh)
{
	QFileInfo jsonfile(ConfigPath + "/pamm_cache/modlist.json");
	if(!jsonfile.exists() || jsonfile.lastModified() <= QDateTime::currentDateTime().addDays(-1) || refresh)
	{

		QNetworkRequest request(QUrl("http://pamods.github.io/modlist.json"));
//		request.setRawHeader("User-Agent" , "Opera/9.80 (X11; Linux x86_64) Presto/2.12.388 Version/12.16");
		request.setAttribute(QNetworkRequest::User, QVariant("modlist"));
		Internet->get(request);
	}
	else
	{
		while(!availableMods.isEmpty())
			delete availableMods.takeLast();

		readAvailableModListJson(ConfigPath + "/pamm_cache/modlist.json");
	}
	
	// Get the mod count.
	QNetworkRequest request(QUrl("http://pa.raevn.com/modcount_json.php"));

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
			AvailableMod *mod = static_cast<AvailableMod *>(reply->request().attribute((QNetworkRequest::Attribute)(QNetworkRequest::User+1)).value<QWidget *>());
			if(mod && availableMods.contains(mod))
			{
				// Load pixmap
				QImage modIcon = QImage::fromData(reply->readAll());
				mod->setIcon(modIcon);
			}
		}
		else if(type == "modlist" || type == "modcount")
		{
			QDir(ConfigPath).mkdir("pamm_cache");
			QString jsonfilename(ConfigPath + "/pamm_cache/");
			if(type == "modlist")
				jsonfilename += "modlist.json";
			else if(type == "modcount")
				jsonfilename += "modcount.json";
			else
				return;

			QFile jsonfile(jsonfilename);
			if(jsonfile.open(QIODevice::WriteOnly | QIODevice::Text))
			{
				QByteArray arr = reply->readAll();
				jsonfile.write(arr);
				jsonfile.flush();
				jsonfile.close();

				if(type == "modlist")
				{
					while(!availableMods.isEmpty())
						delete availableMods.takeLast();
					readAvailableModListJson(jsonfilename);
				}
				else if(type == "modcount")
				{
					updateModCount();
				}
			}
			else
			{
				QMessageBox msgBox;
				msgBox.setText("Couldn't write to \"" + jsonfilename + "\"");
				msgBox.setIcon(QMessageBox::Warning);
				msgBox.exec();
			}
		}
		else if(type == "moddownload")
		{
			AvailableMod *mod = static_cast<AvailableMod *>(reply->request().attribute((QNetworkRequest::Attribute)(QNetworkRequest::User+1)).value<QWidget *>());
			if(mod && availableMods.contains(mod))
			{
				QStringList urlpath = reply->url().path().split('/');
				QDir(ConfigPath).mkdir("pamm_cache");
				QString modfilename(ConfigPath + "/pamm_cache/" + urlpath[urlpath.count()-1]);
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
		else if(type == "likes")
		{
			AvailableMod *mod = static_cast<AvailableMod *>(reply->request().attribute((QNetworkRequest::Attribute)(QNetworkRequest::User+1)).value<QWidget *>());
			if(mod && availableMods.contains(mod)) // It might be deleted by clicking refresh before all the "likes" have been loaded.
			{
				int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
				if(statusCode == 301)
				{
					QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
					QNetworkRequest request(redirectUrl);

					request.setAttribute(QNetworkRequest::User, QVariant("likes"));
					request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User+1), QVariant::fromValue((QWidget *)mod));
					Internet->get(request);
				}
				else
				{
					mod->parseForumPostForLikes(reply->readAll());
					emit likeCountUpdated();
				}
			}
		}
	}
}

void ModManager::readAvailableModListJson(QString filename)
{
	QJson::Parser parser;
	bool ok;

	QFile jsonFile(filename);
	if(!jsonFile.open(QIODevice::ReadOnly | QIODevice::Text))
		return;

	QVariantMap result = parser.parse(&jsonFile, &ok).toMap();
	if(!ok)
		return;

	QMapIterator<QString, QVariant> modentry(result);
	while(modentry.hasNext())
	{
		modentry.next();
		QVariantMap moddata = modentry.value().toMap();

		QString display_name = readLocaleField(moddata, "display_name").toString();
		QString version = moddata["version"].toString();
		QUrl forumlink = moddata["forum"].toUrl();
		AvailableMod::installstate_t state = isInstalled(modentry.key(), version);

		AvailableMod *mod = new AvailableMod(
			modentry.key(),
			display_name,
			readLocaleField(moddata, "description").toString(),
			moddata["author"].toString(),
			version,
			moddata["build"].toString(),
			QDate::fromString(moddata["date"].toString(), "yyyy/MM/dd"),
			forumlink,
			moddata["url"].toUrl(),
			moddata["category"].toStringList(),
			moddata["requires"].toStringList(),
			state,
			ImgPath
		);
		availableMods.append(mod);
		connect(mod, SIGNAL(installMe()), SLOT(downloadMod()));

		if(moddata.contains("icon") && moddata["icon"].toString() != "")
		{
			// Get the mod count.
			QNetworkRequest request(moddata["icon"].toUrl());

			// Raevn's website doesn't like Qt4 downloading its modcount! Darn you, mod_security!
			// Pretend to be Opera! Everybody loves Opera, right?
			request.setRawHeader("User-Agent" , "Opera/9.80 (X11; Linux x86_64) Presto/2.12.388 Version/12.16");
			request.setAttribute(QNetworkRequest::User, QVariant("icon"));
			request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User+1), QVariant::fromValue((QWidget *)mod));
			Internet->get(request);
		}

		if(!forumlink.isEmpty() && forumlink.isValid())
		{
			// Scrape the number of likes from the forums.
			QNetworkRequest request(forumlink);

			request.setAttribute(QNetworkRequest::User, QVariant("likes"));
			request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User+1), QVariant::fromValue((QWidget *)mod));
			Internet->get(request);
		}

		for(QList<Mod *>::iterator m = installedMods.begin(); m != installedMods.end(); ++m)
		{
			InstalledMod *im = dynamic_cast<InstalledMod *>(*m);
			if(im->key() == mod->key())
			{
				if(state == AvailableMod::updateavailable)
					im->setUpdateAvailable(true);
			}
		}
	}

	emit availableModsLoaded();
}

void ModManager::purgeFiles(AvailableMod* mod)
{
	recursiveRemove(QDir(ModPath + '/' + mod->key()));
	if(!QDir(ModPath).rmdir(mod->key()))
	{
		QMessageBox msgBox;
		msgBox.setText(tr("Couldn't delete all of the old files."));
		msgBox.setInformativeText(tr("See") + " \"" + ModPath + '/' + mod->key() + "\"");
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.exec();
	}
}

void ModManager::downloadMod()
{
	AvailableMod *mod = dynamic_cast<AvailableMod *>( sender() );
	if(mod == NULL)
	{
		InstalledMod *instmod = dynamic_cast<InstalledMod *>( sender() );
		if(instmod)
		{
			for(QList<Mod *>::const_iterator avmod = availableMods.constBegin(); avmod != availableMods.constEnd(); ++avmod)
			{
				if(instmod->key() == (*avmod)->key())
				{
					mod = dynamic_cast<AvailableMod *>(*avmod);
					break;
				}
			}
		}
	}

	if(mod)
	{
		QNetworkRequest request(mod->downloadLink());
		request.setAttribute(QNetworkRequest::User, "moddownload");
		request.setRawHeader("User-Agent" , "Opera/9.80 (X11; Linux x86_64) Presto/2.12.388 Version/12.16");
		request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User+1), QVariant::fromValue((QWidget *)mod));
		QNetworkReply *reply = Internet->get(request);
		connect(reply, SIGNAL(downloadProgress(qint64, qint64)), mod, SLOT(downloadProgress(qint64,qint64)));

	}
}


void ModManager::installMod(AvailableMod* mod, const QString& filename)
{
	connect(this, SIGNAL(progress(int)), mod, SLOT(progress(int)));

	// This is an update, so delete the old stuff first.
	if(mod->state() == AvailableMod::updateavailable)
		purgeFiles(mod);

	QString jsonfilename;

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

			if(filename.right(13).compare("/modinfo.json", Qt::CaseInsensitive) == 0)
			{
				jsonfilename = filename;
			}
			
			struct zip_file *zippedFile = zip_fopen_index(zippedMod, i, 0);
			QByteArray data;
			char buffer[1024];
			int len = 0;
			while((len = zip_fread(zippedFile, buffer, 1024)) > 0)
				data.append(buffer, len);

			zip_fclose(zippedFile);

			QFileInfo outFileInfo(ModPath + '/' + filename);
			if(QDir::root().mkpath(outFileInfo.absolutePath()))
			{
				QFile outFile(ModPath + '/' + filename);
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
		message.setText(tr("Couldn't open ZIP file!"));
		message.setInformativeText(tr("Reason") + ": " + ziperror);
		message.setIcon(QMessageBox::Critical);
		message.exec();
	}

	disconnect(this, SIGNAL(progress(int)), mod, SLOT(progress(int)));

	InstalledMod *newmod = parseJson(ModPath + '/' + jsonfilename);
	if(newmod)
	{
		// Remove the old outdated mod from the list
		for(size_t i = 0; i < installedMods.count(); i++)
		{
			if(installedMods[i]->displayName() == newmod->displayName())
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

		connect(newmod, SIGNAL(modStateChanged()), SLOT(modstateChanged()));
		connect(newmod, SIGNAL(updateMe()), SLOT(downloadMod()));
		connect(newmod, SIGNAL(uninstallMe()), SLOT(uninstallMod()));
		emit newModInstalled();

		QStringList requires = newmod->requires();
		for(QList<Mod *>::const_iterator m = installedMods.begin(); m != installedMods.end(); ++m)
		{
			requires.removeAll((*m)->key());
		}

		if(!requires.isEmpty())
		{
			QString reqmodnames;
			for(QStringList::const_iterator req = requires.begin(); req != requires.end(); ++req)
			{
				reqmodnames += "<li>" + (*req) + "</li>";
			}

			QMessageBox installDependenciesMessageBox;
			installDependenciesMessageBox.setText(QString(tr("%1 depends on other mods.")).arg(newmod->displayName()));
			installDependenciesMessageBox.setInformativeText("<p>" + tr("Do you want to install the following?") + "</p><ul>" + reqmodnames + "</ul>");
			installDependenciesMessageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
			installDependenciesMessageBox.setDefaultButton(QMessageBox::Yes);
			int ret = installDependenciesMessageBox.exec();
			if(ret == QMessageBox::Yes)
			{
				for(QStringList::const_iterator req = requires.begin(); req != requires.end(); ++req)
				{
					for(QList<Mod *>::const_iterator avmod = availableMods.begin(); avmod != availableMods.end(); ++avmod)
					{
						if((*req) == (*avmod)->key())
						{
							AvailableMod *am = dynamic_cast<AvailableMod *>(*avmod);
							Q_ASSERT(am != NULL);

							am->installButtonClicked();
							break;
						}
					}
				}
			}
		}
	}
	
	refreshReverseRequirements();
	writeModsJson();
	writeModsListJson();
	writeUiModListJS();
}

void ModManager::modstateChanged()
{
	InstalledMod *mod = dynamic_cast<InstalledMod *>( sender() );
	if(mod)
	{
		QFile modinfoJsonFile(ModPath + "/" + mod->key() + "/modinfo.json");
		if(modinfoJsonFile.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			QJson::Serializer serializer;
			bool ok;
			modinfoJsonFile.write(serializer.serialize(mod->completeJson(), &ok));
			modinfoJsonFile.close();
		}
	}
	writeModsJson();
	writeModsListJson();
	writeUiModListJS();
	
	if(mod)
	{
		if(mod->enabled())
		{
			QStringList requires = mod->requires();
			if(!requires.isEmpty())
			{
				for(QStringList::const_iterator reqstr = requires.constBegin(); reqstr != requires.constEnd(); ++reqstr)
				{
					bool found = false;
					for(QList<Mod *>::iterator reqmod = installedMods.begin(); reqmod != installedMods.end(); ++reqmod)
					{
						InstalledMod *reqimod = dynamic_cast<InstalledMod *>(*reqmod);
						Q_ASSERT(reqimod != NULL);

						if(reqimod->key() == *reqstr)
						{
							found = true;
							reqimod->enable();
						}
					}
					if(!found)
					{
						QMessageBox msgBox;
						msgBox.setText(tr("Unmet requirements!"));
						msgBox.setInformativeText(tr("%1 is not installed, but is needed for this mod. Mod will not work.").arg((*reqstr)));
						msgBox.setIcon(QMessageBox::Critical);
						msgBox.exec();
					}
				}
			}
		}
		else
			mod->disableReverseRequirements();
	}
}

void ModManager::updateModCount()
{
	QFile modCountFile(ConfigPath + "/pamm_cache/modcount.json");
	if(!modCountFile.open(QIODevice::ReadOnly | QIODevice::Text))
		return;
	
	QJson::Parser parser;
	bool ok;

	QVariantMap result = parser.parse(&modCountFile, &ok).toMap();
	if(!ok)
		return;

	for(QList<Mod *>::iterator mod = availableMods.begin(); mod != availableMods.end(); ++mod)
	{
		AvailableMod *amod = dynamic_cast<AvailableMod *>(*mod);
		Q_ASSERT(amod != NULL);

		amod->setCount(result[amod->key()].toInt());
	}
}

void ModManager::writeModsJson()
{
	QFile modsJsonFile(ModPath + "/mods.json");
	if(!modsJsonFile.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		return;
	}
	
	QTextStream stream(&modsJsonFile);
	
	QList<Mod *> prioritySorted(installedMods);
	qSort(prioritySorted.begin(), prioritySorted.end(), InstalledMod::sortPriority);
	
	stream << "{" << endl << "\t\"mount_order\":" << endl << "\t[" << endl;
	bool firstpass = true;
	for(QList<Mod *>::const_iterator mod = prioritySorted.begin(); mod != prioritySorted.end(); ++mod)
	{
		InstalledMod *imod = dynamic_cast<InstalledMod *>(*mod);
		Q_ASSERT(imod != NULL);

		if(!imod->enabled())
			continue;

		if(firstpass)
			firstpass = false;
		else
			stream << ',' << endl;
		
		stream << "\t\t\"" << imod->identifier() << "\"";
	}
	stream << endl << "\t]" << endl << "}" << endl;
	modsJsonFile.close();
}

void ModManager::writeModsListJson()
{
	QFile modsJsonFile(ModPath + "/PAMM/ui/client_mods/mods_list.json");
	if(!modsJsonFile.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		return;
	}
	
	QTextStream stream(&modsJsonFile);

	stream << "{" << endl;
	bool firstpass = true;
	for(QList<Mod *>::const_iterator mod = installedMods.constBegin(); mod != installedMods.constEnd(); ++mod)
	{
		InstalledMod *imod = dynamic_cast<InstalledMod *>(*mod);
		Q_ASSERT(imod != NULL);

		if(!imod->enabled() || imod->key() == "PAMM")
			continue;

		if(firstpass)
			firstpass = false;
		else
			stream << ',' << endl;
		
		stream << imod->json();
	}
	stream << endl << "}" << endl;
	modsJsonFile.close();
}

void ModManager::recursiveRemove(const QDir &dir)
{
	QFileInfoList list = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden);

	for(QFileInfoList::const_iterator item = list.constBegin(); item != list.constEnd(); ++item)
	{
		if(item->isDir())
		{
			recursiveRemove(QDir(item->filePath()));
			dir.rmdir(item->fileName());
		}
		else
			QDir(dir).remove(item->fileName());
	}
}

void ModManager::uninstallMod()
{
	InstalledMod *mod = dynamic_cast<InstalledMod *>( sender() );
	if(mod)
	{
		installedMods.removeOne(mod);
		recursiveRemove(QDir(ModPath + '/' + mod->key()));
		if(!QDir(ModPath).rmdir(mod->key()))
		{
			QMessageBox msgBox;
			msgBox.setText(tr("Couldn't delete all of the files."));
			msgBox.setInformativeText(tr("See") + " \"" + ModPath + '/' + mod->key() + "\"");
			msgBox.setIcon(QMessageBox::Warning);
			msgBox.exec();
		}

		for(QList<Mod *>::iterator avmod = availableMods.begin(); avmod != availableMods.end(); ++avmod)
		{
			if((*avmod)->key() == mod->key())
			{
				AvailableMod *am = dynamic_cast<AvailableMod *>(*avmod);
				Q_ASSERT(am != NULL);

				am->setState(AvailableMod::notinstalled);
				break;
			}
		}

		refreshReverseRequirements();
		writeModsJson();
		writeModsListJson();
		writeUiModListJS();

		delete mod;
	}
}

void ModManager::refreshReverseRequirements()
{
	// Clear everything
	for(QList<Mod *>::iterator mod = installedMods.begin(); mod != installedMods.end(); ++mod)
	{
		InstalledMod *imod = dynamic_cast<InstalledMod *>(*mod);
		Q_ASSERT(imod != NULL);
		imod->clearReverseRequirement();
	}

	// Find requirements
	for(QList<Mod *>::const_iterator mod = installedMods.constBegin(); mod != installedMods.constEnd(); ++mod)
	{
		InstalledMod *imod = dynamic_cast<InstalledMod *>(*mod);
		Q_ASSERT(imod != NULL);

		QStringList requirements = imod->requires();
		for(QStringList::const_iterator requirement = requirements.constBegin(); requirement != requirements.constEnd(); ++requirement)
		{
			for(QList<Mod *>::iterator reqmod = installedMods.begin(); reqmod != installedMods.end(); ++reqmod)
			{
				InstalledMod *reqimod = dynamic_cast<InstalledMod *>(*reqmod);
				Q_ASSERT(reqimod != NULL);
				if(reqimod->key() == *requirement)
					reqimod->addReverseRequirement(imod);
			}
		}
	}
}

void ModManager::installAllUpdates()
{
	for(QList<Mod *>::iterator m = availableMods.begin(); m != availableMods.end(); ++m)
	{
		AvailableMod *am = dynamic_cast<AvailableMod *>(*m);
		Q_ASSERT(am != NULL);

		if(am->state() == AvailableMod::updateavailable)
			am->installButtonClicked();
	}
}

QVariant ModManager::readLocaleField(const QVariantMap &map, const QString &field)
{
	QString lang = Locale.name();
	if(map.contains(field + '_' + lang))
		return map[field + '_' + lang];
	else if(lang.split('_')[0].length() > 0 && map.contains(field + '_' + lang.split('_')[0]))
		return map[field + '_' + lang.split('_')[0]];
	else
		return map[field];
}


#include "modmanager.moc"
