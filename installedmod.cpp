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
#include <QPushButton>
#include <QMessageBox>
#include <QVariant>
#include <QStringList>
#include <QString>

InstalledMod::InstalledMod(const QString& Key, const QString& Context, const QString& Identifier, const QString& DisplayName, const QString& Description, const QString& Author, const QString& Version, const QString& Signature, const unsigned int& Priority, const bool& Enabled, const QString& Id, const QUrl& Forum, const QStringList& Category, const QStringList& Requires, const QDate& Date, const QString& Build)
 : Mod(Key, DisplayName, Description, Author, Forum, Category, Version, Requires, Date, Build), Context(Context), Identifier(Identifier), Signature(Signature), Priority(Priority), Enabled(Enabled), Id(Id)
{
	QGridLayout *modLayout = new QGridLayout(this);
	setLayout(modLayout);

	ModCheckbox = new QCheckBox(this);
	ModCheckbox->setFixedWidth(30);
	ModCheckbox->setCheckState(Enabled ? Qt::Checked : Qt::Unchecked);
	connect(ModCheckbox, SIGNAL(stateChanged(int)), this, SLOT(checkBoxStateChanged(int)));

	QLabel *modNameLabel = new QLabel(this);
	modNameLabel->setText("<a href=\"" + Forum.toString() + "\" style=\"text-decoration:none;\">" + DisplayName + "</a>");
	modNameLabel->setOpenExternalLinks(true);
	modNameLabel->setStyleSheet("QLabel {color: #008888; font-family: \"Verdana\"; font-size: 0.95em; text-decoration: none; }");
	modNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

	QLabel *modAuthorLabel = new QLabel(this);
	modAuthorLabel->setText(tr("by") + " " + Author);
	modAuthorLabel->setStyleSheet("QLabel {color: #F9F9F9; margin-left: 5px; font-style: italic; font-size: 0.7em;}");

	QLabel *modInfoLabel = new QLabel(this);
	modInfoLabel->setText(QString(tr("Version") + ": ") + Version + ", " + tr("build") + " " + Build + " (" + Date.toString("yyyy/MM/dd") + ")");
	modInfoLabel->setStyleSheet("QLabel {font-size: 0.8em; color: #888888; }");

	if(Identifier == "com.pa.deathbydenim.dpamm")
		ModCheckbox->setDisabled(true);

	modLayout->addWidget(ModCheckbox, 1, 1, 2, 1);
	modLayout->addWidget(modNameLabel, 1, 2);
	modLayout->addWidget(modAuthorLabel, 1, 3);
	modLayout->addWidget(modInfoLabel, 2, 2, 1, -1);

	if(!Requires.isEmpty())
	{
		QLabel *modRequires = new QLabel(this);
		modRequires->setText(tr("REQUIRES") + ": " + Requires.join(", "));
		modRequires->setStyleSheet("QLabel {font-size: 0.8em; color: #888888; }");
		setRelativeFontSizeForLabel(modRequires, .8);
		modLayout->addWidget(modRequires, 3, 2, 1, -1);
	}

	QWidget *buttonWidget = new QWidget(this);
	QFont font = buttonWidget->font();
	font.setBold(false);
	font.setPointSize(font.pointSize()-1);
	buttonWidget->setFont(font);
	QHBoxLayout *buttonLayout = new QHBoxLayout(buttonWidget);
	ModUpdateButton = new QPushButton(buttonWidget);
	ModUpdateButton->setText(tr("Update"));
	ModUpdateButton->setEnabled(false);
	buttonLayout->addWidget(ModUpdateButton);
	QPushButton *modUninstallButton = new QPushButton(buttonWidget);
	modUninstallButton->setText(tr("Uninstall"));
	if(Identifier == "com.pa.deathbydenim.dpamm")
		modUninstallButton->setDisabled(true);
	buttonLayout->addWidget(modUninstallButton);
	buttonLayout->addStretch();
	modLayout->addWidget(buttonWidget, 4, 2, 1, -1);
	
	connect(ModUpdateButton, SIGNAL(clicked()), this, SLOT(updateButtonClicked()));
	connect(modUninstallButton, SIGNAL(clicked()), this, SLOT(uninstallButtonClicked()));
}

InstalledMod::~InstalledMod()
{
}

void InstalledMod::checkBoxStateChanged(int state)
{
	Enabled = (state == Qt::Checked);
	CompleteJson["enabled"] = Enabled;
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
		case replay_browser:
			return Scene_replay_browser;
			break;
		case settings:
			return Scene_settings;
			break;
		case social:
			return Scene_social;
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
			"name: '" + DisplayName + "', "
			"author: '" + Author + "', "
			"version: '" + Version + "', "
			"build: '" + QString("%1").arg(Build) + "', "
			"category: '" + Category.join(", ") + "'"
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
	{
		filesStringList = files.toStringList();
		filesStringList.removeAll("");
	}
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
		case replay_browser:
			Scene_replay_browser = filesStringList;
			break;
		case server_browser:
			Scene_server_browser = filesStringList;
			break;
		case settings:
			Scene_settings = filesStringList;
			break;
		case social:
			Scene_social = filesStringList;
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
	CompleteJson["enabled"] = enabled;
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

void InstalledMod::uninstallButtonClicked()
{
	if(!ReverseRequirements.isEmpty())
	{
		QString revreq;
		for(int i = 0; i < ReverseRequirements.length(); i++)
		{
			if(i > 0)
			{
				if(i == ReverseRequirements.length() - 1)
				{
					revreq += " and ";
				}
				else
				{
					revreq += ", ";
				}
			}
			revreq += "\"" + ReverseRequirements[i]->displayName() + "\"";
		}

		QMessageBox msgBox;
		msgBox.setText(revreq + " " + tr("depend on this mod."));
		msgBox.setInformativeText(tr("Are you sure you want to uninstall?"));
		msgBox.setIcon(QMessageBox::Warning);
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
		if(msgBox.exec() != QMessageBox::Yes)
			return;
	}

	QPushButton *button = dynamic_cast<QPushButton *>( sender() );
	if(button)
		button->setEnabled(false);
	
	emit uninstallMe();
}

void InstalledMod::updateButtonClicked()
{
	QPushButton *button = dynamic_cast<QPushButton *>( sender() );
	if(button)
		button->setEnabled(false);
	
	emit updateMe();
}

void InstalledMod::setUpdateAvailable(bool updateavailable)
{
	if(ModUpdateButton)
		ModUpdateButton->setEnabled(updateavailable);
}

void InstalledMod::addReverseRequirement(InstalledMod* mod)
{
	if(mod)
		ReverseRequirements.push_back(mod);
}

void InstalledMod::disableReverseRequirements()
{
	for(QList<InstalledMod *>::iterator mod = ReverseRequirements.begin(); mod != ReverseRequirements.end(); ++mod)
	{
		(*mod)->ModCheckbox->setCheckState(Qt::Unchecked);
	}
}

void InstalledMod::enable()
{
	if(ModCheckbox)
		ModCheckbox->setCheckState(Qt::Checked);
}

QString InstalledMod::json()
{
	QString jsonString;
	jsonString = "\t\"" + Key + "\" : {\n";
	jsonString += "\t\t\"display_name\": \"" + DisplayName + "\",\n";
	jsonString += "\t\t\"author\": \"" + Author + "\",\n";
	jsonString += "\t\t\"enabled\": " + QString(Enabled ? "true" : "false") + ",\n";
	jsonString += "\t\t\"version\": \"" + Version + "\",\n";
	jsonString += "\t\t\"build\": \"" + Build + "\"\n";
	jsonString += "\t}";

	return jsonString;
}

void InstalledMod::clearReverseRequirement()
{
	ReverseRequirements.clear();
}


#include "installedmod.moc"
