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

#ifndef PAMM_H
#define PAMM_H

#define PAMM_VERSION "3.0.3"

#include <QtGui/QMainWindow>
#include "modmanager.h"

class QLayout;
class QTextBrowser;
class QNetworkReply;
class QPushButton;
class QComboBox;

class PAMM : public QMainWindow
{
Q_OBJECT
public:
	PAMM(ModManager* manager, QString progdir);
	virtual ~PAMM();
	
private:
	enum ModFilter
	{
		All,
		Installed,
		Require_update,
		Not_installed
	};
	ModManager *Manager;
	QWidget *availableModsWidget;
	QWidget *InstalledModsWidget;
	QTextBrowser *NewsBrowser;
	QPushButton *RefreshButton;
	QTabWidget *Tabs;
	QPushButton *UpdateAllButton;
	QComboBox *FilterComboBox;
	void populateAvailableModsWidget(bool deleteWidgets, ModFilter filter = All);
	void clearWidgets(QLayout* layout, bool deleteWidgets);
    void loadNews();
	void checkForUpdate();
	void updateUpdateAllButton();

public Q_SLOTS:
	void exitButtonClicked();
	void refreshButtonClicked();
	void launchPAButtonClicked();
	void availableModsLoaded();
	void replyFinished(QNetworkReply* reply = NULL);
	void tabChanged(int index);
	void newModInstalled(InstalledMod *newmod);
	void sortIndexChanged(const QString & text);
	void filterIndexChanged(const QString & text);
	void updateAllButtonClicked();
};

#endif // PAMM_H
