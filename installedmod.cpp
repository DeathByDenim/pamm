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

#include "installedmod.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QCheckBox>
#include <QWidget>
#include <QVariant>
#include <QStringList>
#include <QString>

InstalledMod::InstalledMod(const QString& Key, const QString& Name, const QStringList& Authors, const QUrl& Link, const QString& Category, const QString& Version, const unsigned int& Build, const unsigned int& Priority, const QStringList& Requires, const bool& Enabled)
 : Mod(Key, Name, Authors, Link, Category, Version, Build), Priority(Priority), Requires(Requires), Enabled(Enabled)
{
	QGridLayout *modLayout = new QGridLayout(this);
	setLayout(modLayout);

	ModCheckbox = new QCheckBox(this);
	ModCheckbox->setFixedWidth(30);
	ModCheckbox->setCheckState(Enabled ? Qt::Checked : Qt::Unchecked);
	connect(ModCheckbox, SIGNAL(stateChanged(int)), this, SLOT(checkBoxStateChanged(int)));

	QLabel *modNameLabel = new QLabel(this);
	modNameLabel->setText("<a href=\"" + Link.toString() + "\" style=\"text-decoration:none;\">" + Name + "</a>");
	modNameLabel->setOpenExternalLinks(true);
	modNameLabel->setStyleSheet("QLabel {color: #008888; font-family: \"Verdana\"; font-size: 0.95em; text-decoration: none; }");
	modNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

	QLabel *modAuthorLabel = new QLabel(this);
	modAuthorLabel->setText("by " + Authors.join(", "));
	modAuthorLabel->setStyleSheet("QLabel {color: #F9F9F9; margin-left: 5px; font-style: italic; font-size: 0.7em;}");

	QLabel *modInfoLabel = new QLabel(this);
	modInfoLabel->setText(QString("Version ") + Version + " (" + QString("%1").arg(Build) + "), Category: " + Category);
	modInfoLabel->setStyleSheet("QLabel {font-size: 0.8em; color: #888888; }");

	modLayout->addWidget(ModCheckbox, 1, 1, 2, 1);
	modLayout->addWidget(modNameLabel, 1, 2);
	modLayout->addWidget(modAuthorLabel, 1, 3);
	modLayout->addWidget(modInfoLabel, 2, 2, 1, -1);
}

InstalledMod::~InstalledMod()
{
}

void InstalledMod::checkBoxStateChanged(int state)
{
	Enabled = (state == Qt::Checked);
	emit modStateChanged();
}

QStringList InstalledMod::getSceneList(InstalledMod::scene_t scene)
{
	switch(scene)
	{
		case global_mod_list:
			return Scene_global_mod_list;
			break;
		case connect_to_game:
			return Scene_connect_to_game;
			break;
		case game_over:
			return Scene_game_over;
			break;
		case icon_atlas:
			return Scene_icon_atlas;
			break;
		case live_game:
			return Scene_live_game;
			break;
		case load_planet:
			return Scene_load_planet;
			break;
		case lobby:
			return Scene_lobby;
			break;
		case matchmaking:
			return Scene_matchmaking;
			break;
		case new_game:
			return Scene_new_game;
			break;
		case server_browser:
			return Scene_server_browser;
			break;
		case settings:
			return Scene_settings;
			break;
		case special_icon_atlas:
			return Scene_special_icon_atlas;
			break;
		case start:
			return Scene_start;
			break;
		case system_editor:
			return Scene_system_editor;
			break;
		case transit:
			return Scene_transit;
			break;
		default:
			return QStringList();
	}

}

QString InstalledMod::getModUiJsInfoString()
{
	return "    {"
			"name: '" + Name + "', "
			"author: '" + Authors.join(", ") + "', "
			"version: '" + Version + "', "
			"build: '" + QString("%1").arg(Build) + "', "
			"category: '" + Category + "'"
			"}";
}

bool InstalledMod::isOlderThan(const QString& version_in)
{
	QStringList v1 = Version.split(QRegExp("[.-]"));
	QStringList v2 = version_in.split(QRegExp("[.-]"));
	
	for(int i = 0; i < std::min(v1.count(), v2.count()); ++i)
	{
		bool ok1, ok2;
		uint numeric1, numeric2;

		numeric1 = v1[i].toUInt(&ok1);
		numeric2 = v2[i].toUInt(&ok2);
		
		if(ok1 && ok2)
		{
			if(numeric1 < numeric2)
				return -1;
			else if(numeric1 == numeric2)
				continue;
			else
				return 1;
		}
		else
		{
			int result = v1[i].compare(v2[i]);
			if(result != 0)
				return result;
			else
				continue;
		}
	}
	
	if(v1.count() < v2.count())
		return -1;
	else if(v1.count() == v2.count())
		return 0;
	else
		return 1;
}

void InstalledMod::setScene(QVariant files, InstalledMod::scene_t scene)
{
	QStringList filesStringList;
	if(files.canConvert(QVariant::StringList))
		filesStringList = files.toStringList();
	else if(files.canConvert(QVariant::String))
		filesStringList = QStringList(files.toString());
	else return;

	switch(scene)
	{
		case global_mod_list:
			Scene_global_mod_list = filesStringList;
			break;
		case connect_to_game:
			Scene_connect_to_game = filesStringList;
			break;
		case game_over:
			Scene_game_over = filesStringList;
			break;
		case icon_atlas:
			Scene_icon_atlas = filesStringList;
			break;
		case live_game:
			Scene_live_game = filesStringList;
			break;
		case load_planet:
			Scene_load_planet = filesStringList;
			break;
		case lobby:
			Scene_lobby = filesStringList;
			break;
		case matchmaking:
			Scene_matchmaking = filesStringList;
			break;
		case new_game:
			Scene_new_game = filesStringList;
			break;
		case server_browser:
			Scene_server_browser = filesStringList;
			break;
		case settings:
			Scene_settings = filesStringList;
			break;
		case special_icon_atlas:
			Scene_special_icon_atlas = filesStringList;
			break;
		case start:
			Scene_start = filesStringList;
			break;
		case system_editor:
			Scene_system_editor = filesStringList;
			break;
		case transit:
			Scene_transit = filesStringList;
			break;
		default:
			break;
	}
}

void InstalledMod::setEnabled(bool enabled)
{
	if(ModCheckbox)
	{
		ModCheckbox->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
		Enabled = enabled;
	}
}

bool InstalledMod::sortPriority(const InstalledMod* m1, const InstalledMod* m2)
{
	return (m1->Priority < m2->Priority);
}

#include "installedmod.moc"
