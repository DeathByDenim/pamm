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

#include "modlistwidget.h"
#include "modmanager.h"
#include "modfilterwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QScrollArea>
#include <QAction>
#include <QSettings>

ModListWidget::ModListWidget(QWidget* parent, QAction* modfilteraction, ModManager* manager, ModListWidget::mode_t mode)
 : QWidget(parent), Manager(manager), Mode(mode), ListWidget(NULL), FilterWidget(NULL), CurrentStateFilter(StateInvalid), CurrentSort(SortInvalid)
{
#ifdef __APPLE__
	QFont currentfont = font();
	currentfont.setPointSize(currentfont.pointSize() - 2);
	setFont(currentfont);
#endif
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
#ifdef __APPLE__
	mainLayout->setMargin(0);
	mainLayout->setSpacing(-1);
#endif

	QWidget *sortFilterWidget = createSortFilterWidget(this);
	mainLayout->addWidget(sortFilterWidget);

	QScrollArea *modListScrollArea = createModListScrollArea(this);
	mainLayout->addWidget(modListScrollArea);

	FilterWidget = new ModFilterWidget(this);
	connect(FilterWidget, SIGNAL(filterTextChanged(QString)), SLOT(filterTextChanged(QString)));
	connect(FilterWidget, SIGNAL(visibilityChanged(bool)), modfilteraction, SLOT(setChecked(bool)));
	connect(modfilteraction, SIGNAL(triggered(bool)), SLOT(showModFilter(bool)));
	mainLayout->addWidget(FilterWidget);

	QSettings settings("DeathByDenim", "PAMM");
	switch(Mode)
	{
		case ModListWidget::ModeAvailable:
			modfilteraction->setChecked(settings.value("view/modfilteravailable", false).toBool());
			break;
		case ModListWidget::ModeInstalled:
			modfilteraction->setChecked(settings.value("view/modfilterinstalled", false).toBool());
			break;
	}
	showModFilter(modfilteraction->isChecked());

	
	connect(Manager, SIGNAL(newModInstalled()), SLOT(updateList()));
	connect(Manager, SIGNAL(likeCountUpdated()), SLOT(maybeUpdateList()));
	if(Mode == ModListWidget::ModeAvailable)
		connect(Manager, SIGNAL(availableModsLoaded()), SLOT(updateList()));
}

ModListWidget::~ModListWidget()
{
}

QWidget* ModListWidget::createSortFilterWidget(QWidget* parent)
{
	QSettings settings("DeathByDenim", "PAMM");

	QWidget *sort_filter_widget = new QWidget(parent);
	QHBoxLayout *mylayout = new QHBoxLayout(sort_filter_widget);

	if(Mode == ModListWidget::ModeAvailable)
	{
		CurrentStateFilter = StateAll;

		QLabel *filterLabel = new QLabel(sort_filter_widget);
		filterLabel->setText(tr("SHOW:"));
		filterLabel->setStyleSheet("QLabel	{ color: #008888 }");

		mylayout->addWidget(filterLabel);

		QComboBox *filterComboBox = new QComboBox(sort_filter_widget);
		filterComboBox->addItem(tr("ALL"));
		filterComboBox->addItem(tr("INSTALLED"));
		filterComboBox->addItem(tr("REQUIRE UPDATE"));
		filterComboBox->addItem(tr("NOT INSTALLED"));

		connect(filterComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(filterIndexChanged(const QString &)));
		filterComboBox->setCurrentIndex(settings.value(QString("filter/mode%1_filter").arg(Mode), 0).toInt());

		mylayout->addWidget(filterComboBox);
	}
	mylayout->addStretch();
	mylayout->addStrut(10);

	QLabel *sortLabel = new QLabel(sort_filter_widget);
	sortLabel->setText(tr("SORT:"));
	sortLabel->setStyleSheet("QLabel	{ color: #008888 }");
	mylayout->addWidget(sortLabel);

	QComboBox *sortComboBox = new QComboBox(sort_filter_widget);
	sortComboBox->addItem(tr("RANDOM"));
	sortComboBox->addItem(tr("LAST UPDATED"));
	sortComboBox->addItem(tr("TITLE"));
	sortComboBox->addItem(tr("AUTHOR"));
	sortComboBox->addItem(tr("BUILD"));
	if(Mode == ModListWidget::ModeAvailable)
	{
		sortComboBox->addItem(tr("DOWNLOADS"));
		sortComboBox->addItem(tr("LIKES"));
	}
	else if(Mode == ModListWidget::ModeInstalled)
	{
		sortComboBox->setCurrentIndex(2);
	}
	connect(sortComboBox, SIGNAL(currentIndexChanged(const QString &)), this, SLOT(sortIndexChanged(const QString &)));
	CurrentSort = SortRandom;
	sortComboBox->setCurrentIndex(settings.value(QString("filter/mode%1_sort").arg(Mode), 0).toInt());

	mylayout->addWidget(sortComboBox);
	mylayout->addStretch();

	
	return sort_filter_widget;
}

QScrollArea* ModListWidget::createModListScrollArea(QWidget* parent)
{
	QScrollArea *scrollAreaInstalled = new QScrollArea(parent);
	ListWidget = new QWidget(scrollAreaInstalled);
	QVBoxLayout *modListLayout = new QVBoxLayout(ListWidget);

	populateModList(true);

	QFont listWidgetFont = ListWidget->font();
	listWidgetFont.setBold(false);
	ListWidget->setFont(listWidgetFont);
#ifdef __APPLE__
	listWidgetFont.setPointSize(listWidgetFont.pointSize() - 2);
#endif
	scrollAreaInstalled->setWidget(ListWidget);
	scrollAreaInstalled->setWidgetResizable(true);
	
	return scrollAreaInstalled;
}


void ModListWidget::filterIndexChanged(const QString& text)
{
	if(text == tr("ALL"))
	{
		CurrentStateFilter = StateAll;
	}
	else if(text == tr("INSTALLED"))
	{
		CurrentStateFilter = StateInstalled;
	}
	else if(text == tr("REQUIRE UPDATE"))
	{
		CurrentStateFilter = StateRequireUpdate;
	}
	else if(text == tr("NOT INSTALLED"))
	{
		CurrentStateFilter = StateNotInstalled;
	}
	else
		CurrentStateFilter = StateInvalid;

	QSettings settings("DeathByDenim", "PAMM");
	QComboBox *cb = dynamic_cast<QComboBox *>(sender());
	if(cb)
		settings.setValue(QString("filter/mode%1_filter").arg(Mode), cb->currentIndex());

	populateModList(true);
}

void ModListWidget::sortIndexChanged(const QString& text)
{
	if(text == tr("RANDOM"))
	{
		CurrentSort = SortRandom;
	}
	else if(text == tr("LAST UPDATED"))
	{
		CurrentSort = SortLastUpdated;
	}
	else if(text == tr("TITLE"))
	{
		CurrentSort = SortTitle;
	}
	else if(text == tr("AUTHOR"))
	{
		CurrentSort = SortAuthor;
	}
	else if(text == tr("BUILD"))
	{
		CurrentSort = SortBuild;
	}
	else if(text == tr("DOWNLOADS"))
	{
		CurrentSort = SortDownloads;
	}
	else if(text == tr("LIKES"))
	{
		CurrentSort = SortLikes;
	}
	else
		CurrentSort = SortInvalid;

	QSettings settings("DeathByDenim", "PAMM");
	QComboBox *cb = dynamic_cast<QComboBox *>(sender());
	if(cb)
		settings.setValue(QString("filter/mode%1_sort").arg(Mode), cb->currentIndex());

	populateModList(true);
}

void ModListWidget::populateModList(bool do_sort)
{
	if(!ListWidget)
		return;

	QList<Mod *> *mods;
	switch(Mode)
	{
		case ModeInstalled:
			mods = &Manager->installedMods;
			break;
		case ModeAvailable:
			mods = &Manager->availableMods;
			break;
		default:
			return;
	}

	QVBoxLayout *modListLayout = dynamic_cast<QVBoxLayout *>(ListWidget->layout());
	if(modListLayout == NULL)
		return;

	setUpdatesEnabled(false);

	// Clear the list first.
	while (QLayoutItem* item = modListLayout->takeAt(0))
	{
		Mod *mod = dynamic_cast<Mod *>(item->widget());
		if(mod)
			mod->setParent(NULL);
		else
			delete item->widget();

		delete item;
	}
	
	if(do_sort)
	{
		switch(CurrentSort)
		{
			case SortRandom:
				qSort(mods->begin(), mods->end(), Mod::sortRandom);
				break;
			case SortLastUpdated:
				qSort(mods->begin(), mods->end(), Mod::sortLastUpdated);
				break;
			case SortTitle:
				qSort(mods->begin(), mods->end(), Mod::sortTitle);
				break;
			case SortAuthor:
				qSort(mods->begin(), mods->end(), Mod::sortAuthor);
				break;
			case SortBuild:
				qSort(mods->begin(), mods->end(), Mod::sortBuild);
				break;
			case SortDownloads:
				qSort(mods->begin(), mods->end(), AvailableMod::sortDownloads);
				break;
			case SortLikes:
				qSort(mods->begin(), mods->end(), AvailableMod::sortLikes);
				break;
			default:
				break;
		}
	}
	
	Q_ASSERT(CurrentSort != SortInvalid);
	Q_ASSERT(CurrentStateFilter != StateInvalid || Mode != ModListWidget::ModeAvailable);

	QList<Mod *> filteredmods;
	if(Mode == ModListWidget::ModeInstalled || (Mode == ModListWidget::ModeAvailable && CurrentStateFilter == StateAll))
	{
		if(CurrentFilterText == "")
		{
			filteredmods = *mods;
		}
		else
		{
			for(QList<Mod *>::const_iterator m = mods->constBegin(); m != mods->constEnd(); ++m)
			{
				if((*m)->textContains(CurrentFilterText))
					filteredmods.append(*m);
			}
		}
	}
	else if(Mode == ModListWidget::ModeAvailable)
	{
		for(QList<Mod *>::const_iterator m = mods->constBegin(); m != mods->constEnd(); ++m)
		{
			AvailableMod *am = dynamic_cast<AvailableMod *>(*m);
			Q_ASSERT(am != NULL);

			switch(CurrentStateFilter)
			{
				case StateInstalled:
					if(am->state() != AvailableMod::installed)
						continue;
					break;
				case StateNotInstalled:
					if(am->state() != AvailableMod::notinstalled)
						continue;
					break;
				case StateRequireUpdate:
					if(am->state() != AvailableMod::updateavailable)
						continue;
					break;
				default:
					Q_ASSERT(false);
			}

			if(CurrentFilterText == "" || (*m)->textContains(CurrentFilterText))
				filteredmods.append(*m);
		}
	}

	// Add only the required ones.
	if(filteredmods.count() == 0)
	{
		QLabel *nomodsLabel = new QLabel(ListWidget);
		nomodsLabel->setText(tr("No mods found"));
		nomodsLabel->setAlignment(Qt::AlignHCenter);
		nomodsLabel->setStyleSheet("QLabel {color: #ffffff}");
		modListLayout->addWidget(nomodsLabel);
	}
	else
	{
		for(QList<Mod *>::const_iterator m = filteredmods.constBegin(); m != filteredmods.constEnd(); ++m)
		{
			(*m)->setParent(ListWidget);
			modListLayout->layout()->addWidget(*m);
		}
	}
	modListLayout->addStretch();

	setUpdatesEnabled(true);
}

void ModListWidget::updateList()
{
	populateModList(true);
}

void ModListWidget::maybeUpdateList()
{
	if(CurrentSort == SortLikes)
		populateModList(true);
}

void ModListWidget::filterTextChanged(const QString& text)
{
	CurrentFilterText = text.toLower();

	populateModList(false);
}

void ModListWidget::showModFilter(bool checked)
{
	FilterWidget->setVisible(checked);
	FilterWidget->setFocus();

	QAction *ac = dynamic_cast<QAction *>(sender());
	if(ac == NULL)
		return;

	QSettings settings("DeathByDenim", "PAMM");
	switch(Mode)
	{
		case ModListWidget::ModeAvailable:
			settings.setValue("view/modfilteravailable", ac->isChecked());
			break;
		case ModListWidget::ModeInstalled:
			settings.setValue("view/modfilterinstalled", ac->isChecked());
			break;
	}
}

#include "modlistwidget.moc"
