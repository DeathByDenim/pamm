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

#ifndef MODMANAGER_H
#define MODMANAGER_H

#include <QObject>
#include <QList>
#include <QStringList>
#include <QString>
#include <QLocale>
#include <sstream>
#include "installedmod.h"
#include "availablemod.h"

class QNetworkAccessManager;
class QNetworkReply;
class QDir;

class ModManager : public QObject
{
	Q_OBJECT

public:
	ModManager(QString ConfigPath, QString PAPath, QString ModPath, QString ImgPath, QLocale Locale);
	virtual ~ModManager();

	void findInstalledMods();
	void writeUiModListJS();
	QString paPath() { return PAPath; }
	QString configPath() { return ConfigPath; }
	AvailableMod::installstate_t isInstalled(QString modName, QString modVersion);
	void loadAvailableMods(bool refresh);
	void installAllUpdates();
/*
	struct mod_t
	{
		QString FileName;
		QString ID; // Unique Mod ID (avoid spaces or special characters)
		QString Name; // Name of Mod (avoid special characters, and be mindful of the length)
		QString Author; // Your Name
		QString Link; // Forum Link to Mod
		QString Category; // Category
		QString Folder; // Folder to detect (If this folder is found, the mod is presumed installed)
		QString File; // File to detect (If this file is found, the mod is presumed installed)
		QString Version; // Version Number
		unsigned long Build; // Build Number
		unsigned int Priority; // Lower numbers are included in ui_mod_list.js first. Default is 100 if not specified
		QStringList Requires; // mod ids, comma separated
		bool Enabled;
	};
*/
	QList<Mod *> installedMods;
	QList<Mod *> availableMods;

private:
	QString PAPath;
	QString ModPath;
	QString ConfigPath;
	QString ImgPath;
	QLocale Locale;
	QNetworkAccessManager *Internet;

	InstalledMod *parseJson(const QString filename);
	void sceneToStream(std::ostream& os, const QList< Mod* > modList, const InstalledMod::scene_t scene);
	void readAvailableModListJson(QString filename);
	void installMod(AvailableMod *mod, const QString &filename);
	void updateModCount();
	void writeModsJson();
	void writeModsListJson();
	void recursiveRemove(const QDir& dir);
	void refreshReverseRequirements();
	QVariant readLocaleField(const QVariantMap& map, const QString& field);
	void parseScenes(const QVariantMap &result, InstalledMod* mod);

public Q_SLOTS:
	void replyFinished(QNetworkReply* reply = NULL);
	void downloadMod();
	void modstateChanged();
	void uninstallMod();

Q_SIGNALS:
	void availableModsLoaded();
	void progress(int);
	void newModInstalled();
	void modUpdated();
};

#endif // MODMANAGER_H
