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
#include <QPainter>

AvailableMod::AvailableMod(const QString& Key, const QString& DisplayName, const QString& Description, const QString& Author, const QString& Version, const QString& Build, const QDate& Date, const QUrl& Forum, const QUrl& Url, const QStringList& Category, const QStringList& Requires, const AvailableMod::installstate_t State, const QString imgPath)
 : Mod(Key, DisplayName, Description, Author, Forum, Category, Version, Requires, Date, Build), Download(Url), State(State), ModIconLabel(NULL), ModButtonsWidget(NULL), ModStatus(NULL), InstallProgressBar(NULL), NumDownloaded(-1), Likes(-1), ModDownloadCount(NULL)
{
	QGridLayout *modLayout = new QGridLayout(this);
	this->setLayout(modLayout);

	QLabel *modNameLabel = new QLabel(this);
	modNameLabel->setText("<a href=\"" + Forum.toString() + "\" style=\"text-decoration:none;\">" + DisplayName + "</a>");
	modNameLabel->setOpenExternalLinks(true);
	modNameLabel->setStyleSheet("QLabel {color: #008888; font-family: \"Verdana\"; font-size: 0.95em; text-decoration: none; }");
	modNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

	QLabel *modAuthorLabel = new QLabel(this);
	modAuthorLabel->setText("by " + Author);
	modAuthorLabel->setStyleSheet("QLabel {color: #F9F9F9; margin-left: 5px; font-style: italic; font-size: 0.7em;}");

	ModStatus = new QLabel(this);

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

	QLabel *modInfoLabel = new QLabel(this);
	QString modInfoText = QString("Version : ") + Version + ", build " + Build + " (" + Date.toString("yyyy/MM/dd") + ")";
	if(!Requires.isEmpty())
		modInfoText += "\nREQUIRES: " + Requires.join(", ");
	modInfoLabel->setText(modInfoText);
	modInfoLabel->setStyleSheet("QLabel {font-size: 0.8em; color: #888888; }");
	setRelativeFontSizeForLabel(modInfoLabel, .8);

	ModIconLabel = new QLabel(this);
	QImageReader *reader = new QImageReader(imgPath + "generic.png");
	QImage modIcon = reader->read();
	delete reader;
	
	if(Date >= QDate::currentDate().addDays(-7))
	{
		QPainter p(&modIcon);
		QImageReader *reader = new QImageReader(imgPath + "new.png");
		QImage modNewIcon = reader->read();
		delete reader;
		p.setCompositionMode(QPainter::CompositionMode_SourceOver);
		QRect size(QPoint(0,0), modNewIcon.size());
		p.drawImage(size, modNewIcon);
	}
	ModIconLabel->setPixmap(QPixmap::fromImage(modIcon));

	QLabel *modDescription = new QLabel(this);
	modDescription->setWordWrap(true);
	if(Description.isEmpty())
		modDescription->setText("-- No description available --");
	else
		modDescription->setText(Description);
	modDescription->setStyleSheet("QLabel {font-size: 0.8em; color: #ffffff; }");

	if(State != installed)
	{
		ModButtonsWidget = new QWidget(this);
		QHBoxLayout *modButtonsLayout = new QHBoxLayout(ModButtonsWidget);
		ModButtonsWidget->setLayout(modButtonsLayout);

		QPushButton *installButton = new QPushButton(ModButtonsWidget);
		installButton->setText("Install");
		modButtonsLayout->addWidget(installButton);
		modButtonsLayout->addStretch();
		modLayout->addWidget(ModButtonsWidget, 6, 2, 1, -1);
		connect(installButton, SIGNAL(clicked()), this, SLOT(installButtonClicked()));
	}
	
	if(!Category.isEmpty())
	{
		QLabel *modCategoriesLabel = new QLabel(this);
		modCategoriesLabel->setText(Category.join(", ").toUpper());
		modCategoriesLabel->setStyleSheet("QLabel {font-size: 0.8em; color: #888888; }");
		setRelativeFontSizeForLabel(modCategoriesLabel, .8);
		modLayout->addWidget(modCategoriesLabel, 4, 2, 1, -1);
	}

	modLayout->addWidget(ModIconLabel, 1, 1, -1, 1);
	modLayout->addWidget(modNameLabel, 1, 2);
	modLayout->addWidget(modAuthorLabel, 1, 3, Qt::AlignRight);
	modLayout->addWidget(modInfoLabel, 2, 2, 1, -1);
	modLayout->addWidget(modDescription, 3, 2, 1, -1);
	modLayout->addWidget(ModStatus, 5, 2, 1, 1);
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

	emit installMe();
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
			State = installed;
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
	return (m1->Date > m2->Date);
}

bool AvailableMod::sortAuthor(const AvailableMod* m1, const AvailableMod* m2)
{
	return (m1->Author.compare(m2->Author, Qt::CaseInsensitive) <= 0);
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
	return (m1->DisplayName.compare(m2->DisplayName, Qt::CaseInsensitive) < 0);
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
			gridlayout->addWidget(ModDownloadCount, 5, 3, 1, -1, Qt::AlignRight);
	}

	ModDownloadCount->setText(QString("Downloaded %1 times").arg(count));
}


void AvailableMod::parseForumPostForLikes(const QByteArray& data)
{
	int begin_of_first_post = data.indexOf("<li id=\"post-");
	if(begin_of_first_post < 0)
		return;

	int end_of_first_post = data.indexOf("<li id=\"post-", begin_of_first_post + 1);
	
	int begin_of_likes = data.indexOf("<span class=\"LikeText\">");
	if(begin_of_likes > end_of_first_post)
	{
		Likes = 0;
		return;
	}
}

#include "availablemod.moc"
