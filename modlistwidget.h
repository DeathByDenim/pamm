/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Jarno van der Kolk <jarno@jarno.ca>
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

#ifndef MODLISTWIDGET_H
#define MODLISTWIDGET_H

#include <QWidget>

class ModFilterWidget;
class QScrollArea;
class QAction;
class QComboBox;
class ModManager;
class InstalledMod;

class ModListWidget : public QWidget
{
	Q_OBJECT

public:
	enum mode_t {ModeInvalid, ModeInstalled, ModeAvailable};
	ModListWidget(QWidget* parent, QAction* modfilteraction, ModManager* manager, ModListWidget::mode_t mode);
	~ModListWidget();

private:
	enum
	{
		StateInvalid,
		StateAll,
		StateInstalled,
		StateRequireUpdate,
		StateNotInstalled
	} CurrentStateFilter;
	enum
	{
		SortInvalid,
		SortRandom,
		SortLastUpdated,
		SortTitle,
		SortAuthor,
		SortBuild,
		SortDownloads,
		SortLikes
	} CurrentSort;
	QString CurrentFilterText;
	QString CurrentCategoryFilter;
	mode_t Mode;
    QWidget* ListWidget;
    ModFilterWidget* FilterWidget;
    QComboBox* FilterCategoryComboBox;
	ModManager *Manager;

	QWidget* createSortFilterWidget(QWidget *parent = 0);
    QScrollArea* createModListScrollArea(QWidget* parent = 0);
    void populateModList(bool do_sort = false);
	void populateCategoryFilter();

public Q_SLOTS:
	void sortIndexChanged(const QString & text);
	void filterIndexChanged(const QString & text);
	void filterCategoryIndexChanged(const QString & text);
	void updateList();
	void maybeUpdateList();
	void filterTextChanged(const QString &text);
	void showModFilter(bool checked);
};

#endif // MODLISTWIDGET_H
