/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2013  Jarno van der Kolk <jarno@jarno.ca>
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

#ifndef INSTALLEDMOD_H
#define INSTALLEDMOD_H

#include "mod.h"
#include <QVariantMap>

class QPushButton;
class QCheckBox;
class QVariant;

class InstalledMod : public Mod
{
    Q_OBJECT

public:
    InstalledMod(const QString& Key, const QString &Context, const QString &Identifier, const QString& DisplayName, const QString& Description, const QString& Author, const QString &Version, const QString &Signature, const unsigned int& Priority, const bool& Enabled, const QString &Id, const QUrl &Forum, const QStringList &Category, const QStringList &Requires, const QDate &Date, const QString &Build);
    ~InstalledMod();
	
	enum scene_t
	{
		unknown,
		global_mod_list,
		connect_to_game,
		game_over,
		icon_atlas,
		live_game,
		live_game_econ,
		live_game_hover,
		load_planet,
		lobby,
		matchmaking,
		new_game,
		replay_browser,
		server_browser,
		settings,
		social,
		special_icon_atlas,
		start,
		system_editor,
		transit
	};

	QStringList getSceneList(scene_t scene);
	QString getModUiJsInfoString();
	   int compareVersion(const QString& version_in);
	void setScene(QVariant files, scene_t scene);
	void setEnabled(bool enabled);
	bool enabled() {return Enabled;};
	void enable();
	QString identifier() {return Identifier;};
	static bool sortPriority(const InstalledMod *m1, const InstalledMod *m2);
	QVariantMap completeJson() {return CompleteJson;};
	void setCompleteJson(QVariantMap &json) {CompleteJson = json;};
	void setUpdateAvailable(bool updateavailable);
	void addReverseRequirement(InstalledMod *mod);
	void clearReverseRequirement();
	void disableReverseRequirements();
	QString json();

public Q_SLOTS:
	void checkBoxStateChanged(int state);
	void uninstallButtonClicked();
	void updateButtonClicked();

Q_SIGNALS:
	void modStateChanged();
	void uninstallMe();
	void updateMe();

private:
	QCheckBox *ModCheckbox;
	QPushButton *ModUpdateButton;
	QStringList Scene_global_mod_list; // files (comma separated)
	QStringList Scene_connect_to_game; // files (comma separated)
	QStringList Scene_game_over; // files (comma separated)
	QStringList Scene_icon_atlas; // files (comma separated)
	QStringList Scene_live_game; // files (comma separated)
	QStringList Scene_live_game_econ; // files (comma separated)
	QStringList Scene_live_game_hover; // files (comma separated)
	QStringList Scene_load_planet; // files (comma separated)
	QStringList Scene_lobby; // files (comma separated)
	QStringList Scene_matchmaking; // files (comma separated)
	QStringList Scene_new_game; // files (comma separated)
	QStringList Scene_replay_browser; // files (comma separated)
	QStringList Scene_server_browser; // files (comma separated)
	QStringList Scene_settings; // files (comma separated)
	QStringList Scene_social; // files (comma separated)
	QStringList Scene_special_icon_atlas; // files (comma separated)
	QStringList Scene_start; // files (comma separated)
	QStringList Scene_system_editor; // files (comma separated)
	QStringList Scene_transit; // files (comma separated)
	const unsigned int Priority; // Lower numbers are included in ui_mod_list.js first. Default is 100 if not specified
	bool Enabled;

	const QString Context;
	const QString Identifier;
	const QString Signature;
	const QString Id;
	QList<InstalledMod *> ReverseRequirements;

	QVariantMap CompleteJson;
};

#endif // INSTALLEDMOD_H
