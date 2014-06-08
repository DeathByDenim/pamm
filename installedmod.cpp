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

const QStringList InstalledMod::KnownScenes = (QStringList() <<
	"global_mod_list" <<
	"armory" <<
	"blank" <<
	"building_planets" <<
	"connect_to_game" <<
	"game_over" <<
	"gw_war_over" << 
	"gw_start" <<
	"gw_game_over" <<
	"gw_play" <<
	"gw_lobby" <<
	"icon_atlas" <<
	"live_game" <<
	"live_game_econ" <<
	"live_game_hover" <<
	"load_planet" <<
	"lobby" <<
	"matchmaking" <<
	"main" <<
	"new_game" <<
	"replay_browser" <<
	"replay_loading" <<
	"server_browser" <<
	"settings" <<
	"social" <<
	"special_icon_atlas" <<
	"start" <<
	"system_editor" <<
	"transit" <<
	"uberbar"
	);

InstalledMod::InstalledMod(const QString& Key, const Mod::context_t Context, const QString& Identifier, const QString& DisplayName, const QString& Description, const QString& Author, const QString& Version, const QString& Signature, const unsigned int& Priority, const bool& Enabled, const QString& Id, const QUrl& Forum, const QStringList& Category, const QStringList& Requires, const QDate& Date, const QString& Build)
 : Mod(Key, DisplayName, Description, Author, Forum, Category, Version, Requires, Date, Build, Context), Context(Context), Identifier(Identifier), Signature(Signature), Priority(Priority), Enabled(Enabled), Id(Id)
{
	QGridLayout *modLayout = new QGridLayout(this);
#ifdef __APPLE__
	modLayout->setMargin(0);
	modLayout->setSpacing(-1);
#endif

	setLayout(modLayout);

	ModCheckbox = new QCheckBox(this);
	ModCheckbox->setFixedWidth(30);
	ModCheckbox->setCheckState(Enabled ? Qt::Checked : Qt::Unchecked);
	connect(ModCheckbox, SIGNAL(stateChanged(int)), this, SLOT(checkBoxStateChanged(int)));

	QString colourtext("white");
	if(Context == server)
		colourtext = "yellow";

	QLabel *modNameLabel = new QLabel(this);
	if(Forum.isEmpty())
		modNameLabel->setText("<span style=\"text-decoration:none; font-weight: normal; color: " + colourtext + "\">" + DisplayName + "</span>");
	else
		modNameLabel->setText("<a href=\"" + Forum.toString() + "\" style=\"text-decoration:none; font-weight: bold; color: " + colourtext + "\">" + DisplayName + "</a>");
	modNameLabel->setOpenExternalLinks(true);
	modNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

	QLabel *modAuthorLabel = new QLabel(this);
	modAuthorLabel->setText(tr("by") + " " + Author);
	modAuthorLabel->setStyleSheet("QLabel {color: #F9F9F9; margin-left: 5px; font-style: italic; font-size: 0.7em;}");

	QLabel *modInfoLabel = new QLabel(this);
	modInfoLabel->setText(QString(tr("Version") + ": ") + Version + ", " + tr("build") + " " + Build + " (" + Date.toString("yyyy/MM/dd") + ")");
	modInfoLabel->setStyleSheet("QLabel {font-size: 0.8em; color: #888888; }");

	if(Identifier == "com.pa.deathbydenim.dpamm")
		ModCheckbox->setDisabled(true);

	modLayout->addWidget(ModCheckbox, 0, 0, -1, 1);
	modLayout->addWidget(modNameLabel, 0, 1);
	modLayout->addWidget(modAuthorLabel, 0, 2);
	modLayout->addWidget(modInfoLabel, 1, 1, 1, -1);

	if(!Requires.isEmpty())
	{
		QLabel *modRequires = new QLabel(this);
		modRequires->setText(tr("REQUIRES") + ": " + Requires.join(", "));
		modRequires->setStyleSheet("QLabel {font-size: 0.8em; color: #888888; }");
		setRelativeFontSizeForLabel(modRequires, .8);
		modLayout->addWidget(modRequires, 2, 1, 1, -1);
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
	modLayout->addWidget(buttonWidget, 3, 1, 1, -1);
	
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

QStringList InstalledMod::getSceneList(QString scene)
{
	if(SceneFiles.contains(scene))
		return SceneFiles[scene];
	else
		return QStringList();
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

int InstalledMod::compareVersion(const QString& version_in)
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

void InstalledMod::setScene(QVariant files, QString scene)
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

	//TODO Does this work?
	SceneFiles[scene] = filesStringList;
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

bool InstalledMod::sortPriority(const Mod* m1, const Mod* m2)
{
	const InstalledMod *im1 = dynamic_cast<const InstalledMod *>(m1);
	const InstalledMod *im2 = dynamic_cast<const InstalledMod *>(m2);

	Q_ASSERT(im1 != NULL && im2 != NULL);

	return (im1->Priority < im2->Priority);
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

void InstalledMod::enable(bool enabled)
{
	if(ModCheckbox)
	{
		if(Key == "PAMM" && enabled == false)
			return;

		ModCheckbox->setCheckState(enabled ? Qt::Checked : Qt::Unchecked);
	}
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

void InstalledMod::setCompactView(bool compact)
{
	QGridLayout *gridlayout = dynamic_cast<QGridLayout *>(layout());
	if(gridlayout)
	{
		for(int i = 1; i < gridlayout->rowCount(); i++)
		{
			for(int j = 1; j < gridlayout->columnCount(); j++)
			{
				QLayoutItem *item = gridlayout->itemAtPosition(i, j);
				if(item)
				{
					QWidget *widget = item->widget();
					if(widget)
					{
						widget->setVisible(!compact);
					}
				}
			}
		}
	}
}

#include "installedmod.moc"
