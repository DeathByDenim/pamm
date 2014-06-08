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
    InstalledMod(const QString& Key, const context_t Context, const QString& Identifier, const QString& DisplayName, const QString& Description, const QString& Author, const QString& Version, const QString& Signature, const unsigned int& Priority, const bool& Enabled, const QString& Id, const QUrl& Forum, const QStringList& Category, const QStringList& Requires, const QDate& Date, const QString& Build);
    ~InstalledMod();

	static const QStringList KnownScenes;

	QStringList getSceneList(QString scene);
	QString getModUiJsInfoString();
	int compareVersion(const QString& version_in);
	void setScene(QVariant files, QString scene);
	void setEnabled(bool enabled);
	bool enabled() {return Enabled;};
	void enable(bool enabled = true);
	QString identifier() {return Identifier;};
	static bool sortPriority(const Mod* m1, const Mod* m2);
	QVariantMap completeJson() {return CompleteJson;};
	void setCompleteJson(QVariantMap &json) {CompleteJson = json;};
	void setUpdateAvailable(bool updateavailable);
	void addReverseRequirement(InstalledMod *mod);
	void clearReverseRequirement();
	void disableReverseRequirements();
	QString json();
	virtual void setCompactView(bool compact);
	const QMap<QString,QStringList> &scenesFiles() { return SceneFiles; }

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
	QMap<QString,QStringList> SceneFiles;

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
