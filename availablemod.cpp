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

#include "availablemod.h"
#include <QLabel>
#include <QGridLayout>
#include <QImageReader>
#include <QHBoxLayout>
#include <QPushButton>
#include <QProgressBar>

AvailableMod::AvailableMod(QString Key, QString Name, QStringList Authors, QUrl Link, QString Category, QString Version, unsigned int Build, bool CompatibleWithPAMM, QUrl Download, QString Description, QDate Date, AvailableMod::installstate_t State)
 : Mod(Key, Name, Authors, Link, Category, Version, Build), CompatibleWithPAMM(CompatibleWithPAMM), Download(Download), Description(Description), Date(Date), State(State), ModIconLabel(NULL), ModButtonsWidget(NULL), ModStatus(NULL), InstallProgressBar(NULL), NumDownloaded(-1), ModDownloadCount(NULL)
{
	QGridLayout *modLayout = new QGridLayout(this);
	this->setLayout(modLayout);

	QLabel *modNameLabel = new QLabel(this);
	modNameLabel->setText("<a href=\"" + Link.toString() + "\" style=\"text-decoration:none;\">" + Key + "</a>");
	modNameLabel->setOpenExternalLinks(true);
	modNameLabel->setStyleSheet("QLabel {color: #008888; font-family: \"Verdana\"; font-size: 0.95em; text-decoration: none; }");
	modNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

	QLabel *modAuthorLabel = new QLabel(this);
	modAuthorLabel->setText("by " + Authors.join(", "));
	modAuthorLabel->setStyleSheet("QLabel {color: #F9F9F9; margin-left: 5px; font-style: italic; font-size: 0.7em;}");

	ModStatus = new QLabel(this);

	if(CompatibleWithPAMM)
	{
		if(State == installed)
		{
			ModStatus->setText("INSTALLED");
			ModStatus->setStyleSheet("QLabel {font-size: 0.8em; color: #448844;}");
		}
		else if(State == updateavailable)
		{
			ModStatus->setText("UPDATE AVAILABLE");
			ModStatus->setStyleSheet("QLabel {font-size: 0.8em; color: #ff8844;}");
		}
		else if(State == notinstalled)
		{
			ModStatus->setText("NOT INSTALLED");
			ModStatus->setStyleSheet("QLabel {font-size: 0.8em; color: #888844;}");
		}
	}
	else
	{
		ModStatus->setText("NOT COMPATIBLE WITH PAMM");
		ModStatus->setStyleSheet("QLabel {font-size: 0.8em; color: #448844;}");
	}

	QLabel *modInfoLabel = new QLabel(this);
	modInfoLabel->setText(QString("Version ") + Version + " (" + QString("%1").arg(Build) + "), Category: " + Category);
	modInfoLabel->setStyleSheet("QLabel {font-size: 0.8em; color: #888888; }");

	ModIconLabel = new QLabel(this);
	QImageReader *reader = new QImageReader("img/generic.png");
	QImage modIcon = reader->read();
	delete reader;
	ModIconLabel->setPixmap(QPixmap::fromImage(modIcon));

	QLabel *modDescription = new QLabel(this);
	modDescription->setWordWrap(true);
	modDescription->setText(Description);
	modDescription->setStyleSheet("QLabel {font-size: 0.8em; color: #ffffff; }");
	
	if(CompatibleWithPAMM && State != installed)
	{
		ModButtonsWidget = new QWidget(this);
		QHBoxLayout *modButtonsLayout = new QHBoxLayout(ModButtonsWidget);
		ModButtonsWidget->setLayout(modButtonsLayout);

		QPushButton *installButton = new QPushButton(ModButtonsWidget);
		installButton->setText("Install");
		modButtonsLayout->addWidget(installButton);
		modButtonsLayout->addStretch();
		modLayout->addWidget(ModButtonsWidget, 5, 2, 1, -1);
		connect(installButton, SIGNAL(clicked()), this, SLOT(installButtonClicked()));
	}

	modLayout->addWidget(ModIconLabel, 1, 1, -1, 1);
	modLayout->addWidget(modNameLabel, 1, 2);
	modLayout->addWidget(modAuthorLabel, 1, 3, Qt::AlignRight);
	modLayout->addWidget(modDescription, 2, 2, 1, -1);
	modLayout->addWidget(modInfoLabel, 3, 2, 1, -1);
	modLayout->addWidget(ModStatus, 4, 2, 1, 1);
}

AvailableMod::~AvailableMod()
{
}

void AvailableMod::setPixmap(const QPixmap& pixmap)
{
	if(ModIconLabel)
	{
		ModIconLabel->setPixmap(pixmap);
		ModIconLabel->setScaledContents(true);
		ModIconLabel->setFixedSize(100, 100);
	}
}

bool AvailableMod::sortCompatibility(const AvailableMod *m1, const AvailableMod *m2)
{
	return (m1->CompatibleWithPAMM && !m2->CompatibleWithPAMM);
}

void AvailableMod::installButtonClicked()
{
	if(ModButtonsWidget && ModButtonsWidget->layout())
	{
		while (QLayoutItem* item = ModButtonsWidget->layout()->takeAt(0))
		{
			if(QWidget* widget = item->widget())
				delete widget;

			delete item;
		}
		InstallProgressBar = NULL;
	}

	InstallProgressBar = new QProgressBar(ModButtonsWidget);
	if(ModButtonsWidget)
		ModButtonsWidget->layout()->addWidget(InstallProgressBar);
	InstallProgressBar->setRange(0, 100);
	InstallProgressBar->setValue(0);

	emit installMe(this);
}

void AvailableMod::progress(int percentage)
{
	if(InstallProgressBar)
		InstallProgressBar->setValue(percentage);

	if(percentage == 100)
	{
		if(ModButtonsWidget && ModButtonsWidget->layout())
		{
			while (QLayoutItem* item = ModButtonsWidget->layout()->takeAt(0))
			{
				if(QWidget* widget = item->widget())
					delete widget;

				delete item;
			}
			InstallProgressBar = NULL;
		}
		
		if(ModStatus)
		{
			ModStatus->setText("INSTALLED");
			ModStatus->setStyleSheet("QLabel {font-size: 0.8em; color: #448844;}");
		}
	}
}

void AvailableMod::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	progress((75. * bytesReceived) / bytesTotal);
}


bool AvailableMod::sortLastUpdated(const AvailableMod* m1, const AvailableMod* m2)
{
	return (m1->Date < m2->Date);
}

bool AvailableMod::sortAuthor(const AvailableMod* m1, const AvailableMod* m2)
{
	return (m1->Authors[0].compare(m2->Authors[0], Qt::CaseInsensitive) < 0);
}

bool AvailableMod::sortBuild(const AvailableMod* m1, const AvailableMod* m2)
{
	return (m1->Build < m2->Build);
}

bool AvailableMod::sortRandom(const AvailableMod* m1, const AvailableMod* m2)
{
	return (rand() < RAND_MAX / 2);
}

bool AvailableMod::sortTitle(const AvailableMod* m1, const AvailableMod* m2)
{
	return (m1->Name.compare(m2->Name, Qt::CaseInsensitive) < 0);
}

bool AvailableMod::sortDownloads(const AvailableMod* m1, const AvailableMod* m2)
{
	return (m1->NumDownloaded > m2->NumDownloaded);
}

void AvailableMod::setCount(int count)
{
	NumDownloaded = count;

	if(count < 0)
		return;

	if(ModDownloadCount == NULL)
	{
		ModDownloadCount = new QLabel(this);
		ModDownloadCount->setStyleSheet("QLabel {font-size: 0.8em; color: #888888; }");
		QGridLayout *gridlayout = dynamic_cast<QGridLayout *>(layout());
		if(gridlayout)
			gridlayout->addWidget(ModDownloadCount, 4, 3, 1, -1, Qt::AlignRight);
	}

	ModDownloadCount->setText(QString("Downloaded %1 times").arg(count));
}


#include "availablemod.moc"
