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
	ModManager(QString ConfigPath, QString PAPath, QString ModPath);
	virtual ~ModManager();

	void findInstalledMods();
	void writeUiModListJS();
	QString paPath() { return PAPath; }
	QString configPath() { return ConfigPath; }
	AvailableMod::installstate_t isInstalled(QString modName, QString modVersion);
	void loadAvailableMods(bool refresh);
	void installAllUpdates();

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
		QStringList Scene_global_mod_list; // files (comma separated)
		QStringList Scene_connect_to_game; // files (comma separated)
		QStringList Scene_game_over; // files (comma separated)
		QStringList Scene_icon_atlas; // files (comma separated)
		QStringList Scene_live_game; // files (comma separated)
		QStringList Scene_load_planet; // files (comma separated)
		QStringList Scene_lobby; // files (comma separated)
		QStringList Scene_matchmaking; // files (comma separated)
		QStringList Scene_new_game; // files (comma separated)
		QStringList Scene_server_browser; // files (comma separated)
		QStringList Scene_settings; // files (comma separated)
		QStringList Scene_special_icon_atlas; // files (comma separated)
		QStringList Scene_start; // files (comma separated)
		QStringList Scene_system_editor; // files (comma separated)
		QStringList Scene_transit; // files (comma separated)
		QString Version; // Version Number
		unsigned long Build; // Build Number
		unsigned int Priority; // Lower numbers are included in ui_mod_list.js first. Default is 100 if not specified
		QStringList Requires; // mod ids, comma separated
		bool Enabled;
	};

	QList<InstalledMod *> installedMods;
	QList<AvailableMod *> availableMods;

private:
	QString PAPath;
	QString ModPath;
	QString ConfigPath;
	QNetworkAccessManager *Internet;

	InstalledMod *parseJson(const QString filename);
	void sceneToStream(std::ostream& os, const QList< InstalledMod* > modList, const InstalledMod::scene_t scene);
	void readAvailableModListJson(QString filename);
	void installMod(AvailableMod *mod, const QString &filename);
    void updateModCount();
    void writeModsJson();
    bool recursiveRemove(const QDir &dir);
	void refreshReverseRequirements();

public Q_SLOTS:
	void replyFinished(QNetworkReply* reply = NULL);
	void downloadMod();
	void modstateChanged();
	void uninstallMod();

Q_SIGNALS:
	void availableModsLoaded();
	void progress(int);
	void newModInstalled(InstalledMod *);
	void updatedMod();
};

#endif // MODMANAGER_H
