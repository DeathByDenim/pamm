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
//#include "videoreviewdialog.h"

AvailableMod::AvailableMod(const QString& Key, const QString& DisplayName, const QString& Description, const QString& Author, const QString& Version, const QString& Build, const QDate& Date, const QUrl& Forum, const QUrl& Url, const QStringList& Category, const QStringList& Requires, const AvailableMod::installstate_t State, const QString imgPath)
 : Mod(Key, DisplayName, Description, Author, Forum, Category, Version, Requires, Date, Build), Download(Url), State(State), ModIconLabel(NULL), ModButtonsWidget(NULL), ModStatus(NULL), InstallProgressBar(NULL), NumDownloaded(-1), Likes(-1), ModDownloadCountLabel(NULL), ModLikesLabel(NULL), ImgPath(imgPath)
{
	QGridLayout *modLayout = new QGridLayout(this);
#ifdef __APPLE__
	modLayout->setMargin(0);
	modLayout->setSpacing(-1);
#endif
	this->setLayout(modLayout);

	QLabel *modNameLabel = new QLabel(this);
	modNameLabel->setText("<a href=\"" + Forum.toString() + "\" style=\"text-decoration:none;\">" + DisplayName + "</a>");
	modNameLabel->setOpenExternalLinks(true);
	modNameLabel->setStyleSheet("QLabel {color: #008888; font-family: \"Verdana\"; font-size: 0.95em; text-decoration: none; }");
	modNameLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

	QLabel *modAuthorLabel = new QLabel(this);
	modAuthorLabel->setText(tr("by") + " " + Author);
	modAuthorLabel->setStyleSheet("QLabel {color: #F9F9F9; margin-left: 5px; font-style: italic; font-size: 0.7em;}");

	ModStatus = new QLabel(this);

	if(State == installed)
	{
		ModStatus->setText(tr("INSTALLED"));
		ModStatus->setStyleSheet("QLabel {font-size: 0.8em; color: #448844;}");
	}
	else if(State == updateavailable)
	{
		ModStatus->setText(tr("UPDATE AVAILABLE"));
		ModStatus->setStyleSheet("QLabel {font-size: 0.8em; color: #ff8844;}");
	}
	else if(State == notinstalled)
	{
		ModStatus->setText(tr("NOT INSTALLED"));
		ModStatus->setStyleSheet("QLabel {font-size: 0.8em; color: #888844;}");
	}

	QLabel *modInfoLabel = new QLabel(this);
	QString modInfoText = QString("Version : ") + Version + ", build " + Build + " (" + Date.toString("yyyy/MM/dd") + ")";
	if(!Requires.isEmpty())
	modInfoText += "\n" + tr("REQUIRES") + ": " + Requires.join(", ");
	modInfoLabel->setText(modInfoText);
	modInfoLabel->setWordWrap(true);
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
		modDescription->setText("-- " + tr("No description available") + " --");
	else
		modDescription->setText(Description);
	modDescription->setStyleSheet("QLabel {font-size: 0.8em; color: #ffffff; }");

	ModButtonsWidget = new QWidget(this);
	QHBoxLayout *modButtonsLayout = new QHBoxLayout(ModButtonsWidget);
	ModButtonsWidget->setLayout(modButtonsLayout);

	if(State != installed)
	{
		QPushButton *installButton = new QPushButton(ModButtonsWidget);
		installButton->setText(tr("Install"));
		modButtonsLayout->addWidget(installButton);
		
/*		QPushButton *videoReviewButton = new QPushButton(ModButtonsWidget);
		videoReviewButton->setText(tr("Video review"));
		modButtonsLayout->addWidget(videoReviewButton);
*/		
		modButtonsLayout->addStretch();
		connect(installButton, SIGNAL(clicked()), this, SLOT(installButtonClicked()));
//		connect(videoReviewButton, SIGNAL(clicked()), this, SLOT(videoReviewButtonClicked()));
	}
	modLayout->addWidget(ModButtonsWidget, 6, 2, 1, -1);
	
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

void AvailableMod::setIcon(const QImage& icon)
{
	if(icon.width() == 100 && icon.height() == 100)
		Icon = icon.convertToFormat(QImage::Format_ARGB32);
	else
		Icon = icon.scaled(100, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation).convertToFormat(QImage::Format_ARGB32);
	if(ModIconLabel)
	{
		if(Date >= QDate::currentDate().addDays(-7))
		{
			QPainter p(&Icon);
			QImageReader *reader = new QImageReader(ImgPath + "new.png");
			QImage modNewIcon = reader->read();
			delete reader;
			p.setCompositionMode(QPainter::CompositionMode_SourceOver);
			QRect size(QPoint(0,0), modNewIcon.size());
			p.drawImage(size, modNewIcon);
		}
		ModIconLabel->setPixmap(QPixmap::fromImage(Icon));
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
			ModStatus->setText(tr("INSTALLED"));
			ModStatus->setStyleSheet("QLabel {font-size: 0.8em; color: #448844;}");
		}
	}
}

void AvailableMod::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	progress((75. * bytesReceived) / bytesTotal);
}

bool AvailableMod::sortDownloads(const Mod* m1, const Mod* m2)
{
	const AvailableMod *im1 = dynamic_cast<const AvailableMod *>(m1);
	const AvailableMod *im2 = dynamic_cast<const AvailableMod *>(m2);
	Q_ASSERT(im1 != NULL && im2 != NULL);

	return (im1->NumDownloaded > im2->NumDownloaded);
}

bool AvailableMod::sortLikes(const Mod* m1, const Mod* m2)
{
	const AvailableMod *im1 = dynamic_cast<const AvailableMod *>(m1);
	const AvailableMod *im2 = dynamic_cast<const AvailableMod *>(m2);
	Q_ASSERT(im1 != NULL && im2 != NULL);

	return (im1->Likes > im2->Likes);
}

void AvailableMod::setCount(int count)
{
	NumDownloaded = count;

	if(count < 0)
		return;

	if(ModDownloadCountLabel == NULL)
	{
		ModDownloadCountLabel = new QLabel(this);
		ModDownloadCountLabel->setStyleSheet("QLabel {font-size: 0.8em; color: #888888; }");
		QGridLayout *gridlayout = dynamic_cast<QGridLayout *>(layout());
		if(gridlayout)
			gridlayout->addWidget(ModDownloadCountLabel, 5, 3, 1, -1, Qt::AlignRight);
	}

	ModDownloadCountLabel->setText(QString(tr("Downloaded %1 times")).arg(count));
}

void AvailableMod::parseForumPostForLikes(const QByteArray& data)
{
	int begin_of_first_post = data.indexOf("<li id=\"post-");
	if(begin_of_first_post < 0)
		return;

	int end_of_first_post = data.indexOf("<li id=\"post-", begin_of_first_post + 1);
	
	int begin_of_likes = data.indexOf("<span class=\"LikeText\">", begin_of_first_post);
	if(begin_of_likes > end_of_first_post)
	{
		Likes = 0;
		return;
	}

	int end_of_likes = data.indexOf("</span>", begin_of_likes);
	if(end_of_likes < 0)
		return;
	
	QByteArray likespan = data.mid(begin_of_likes, end_of_likes - begin_of_likes);
	
	Likes = likespan.count("class=\"username\"");

	int begin_of_others = likespan.indexOf("class=\"OverlayTrigger\">");
	if(begin_of_others >= 0)
	{
		begin_of_others += QString("class=\"OverlayTrigger\">").length();
		int end_of_others = likespan.indexOf(" ", begin_of_others);

		bool ok;
		int others = likespan.mid(begin_of_others, end_of_others - begin_of_others).toInt(&ok);
		if(ok)
			Likes += others;
	}

	if(ModLikesLabel == NULL)
	{
		ModLikesLabel = new QLabel(this);
		ModLikesLabel->setStyleSheet("QLabel {font-size: 0.8em; color: #888888; }");
		QGridLayout *gridlayout = dynamic_cast<QGridLayout *>(layout());
		if(gridlayout)
			gridlayout->addWidget(ModLikesLabel, 4, 3, 1, -1, Qt::AlignRight);
	}

	ModLikesLabel->setText(QString(tr("Liked %1 times")).arg(Likes));
}

void AvailableMod::setState(AvailableMod::installstate_t state)
{
	State = state;
	
	Q_ASSERT(ModButtonsWidget != NULL);

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

	switch(State)
	{
		case installed:
		{
			ModStatus->setText(tr("INSTALLED"));
			ModStatus->setStyleSheet("QLabel {font-size: 0.8em; color: #448844;}");
		}
		break;
		case updateavailable:
		{
			ModStatus->setText(tr("UPDATE AVAILABLE"));
			ModStatus->setStyleSheet("QLabel {font-size: 0.8em; color: #ff8844;}");
		}
		break;
		case notinstalled:
		{
			ModStatus->setText(tr("NOT INSTALLED"));
			ModStatus->setStyleSheet("QLabel {font-size: 0.8em; color: #888844;}");
		}
		break;
		default:
			Q_ASSERT(false);
	}

	if(State != installed && ModButtonsWidget)
	{
		QPushButton *installButton = new QPushButton(ModButtonsWidget);
		installButton->setText(tr("Install"));
		ModButtonsWidget->layout()->addWidget(installButton);
		static_cast<QHBoxLayout *>(ModButtonsWidget->layout())->addStretch();

		connect(installButton, SIGNAL(clicked()), this, SLOT(installButtonClicked()));
	}

}

void AvailableMod::videoReviewButtonClicked()
{
//	VideoReviewDialog *diag = new VideoReviewDialog(this, ImgPath);
//	diag->exec();
}

#include "availablemod.moc"
