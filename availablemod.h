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

#ifndef AVAILABLEMOD_H
#define AVAILABLEMOD_H

#include <QDate>
#include "mod.h"

class QLabel;
class QHBoxLayout;
class QProgressBar;

class AvailableMod : public Mod
{
	Q_OBJECT

public:
	enum installstate_t{installed, notinstalled, updateavailable};

	AvailableMod(const QString& Key, const QString& DisplayName, const QString& Description, const QString& Author, const QString& Version, const QString& Build, const QDate& Date, const QUrl& Forum, const QUrl& Url, const QStringList& Category, const QStringList& Requires, const AvailableMod::installstate_t State, const QString imgPath, context_t Context);
	~AvailableMod();

	void setIcon(const QImage &icon);
	void setCount(int count);
	QUrl downloadLink() {return Download;};
	installstate_t state() {return State;};
	void setState(installstate_t state);
	void parseForumPostForLikes(const QByteArray &data);
	virtual void setCompactView(bool compact);

	static bool sortDownloads(const Mod *m1, const Mod *m2);
	static bool sortLikes(const Mod *m1, const Mod *m2);

private:
	const QUrl Download;
	QImage Icon;
	installstate_t State;
	int NumDownloaded;
	const QString ImgPath;
	int Likes;

	QLabel *ModIconLabel;
	QWidget *ModButtonsWidget;
	QLabel *ModStatus;
	QProgressBar *InstallProgressBar;
	QLabel *ModDownloadCountLabel;
	QLabel *ModLikesLabel;
	QWidget *CompactView;
	QWidget* NormalView;

public Q_SLOTS:
	void installButtonClicked();
	void videoReviewButtonClicked();
	void progress(int percentage);
	void downloadProgress( qint64 bytesReceived, qint64 bytesTotal);

Q_SIGNALS:
	void installMe();
};

#endif // AVAILABLEMOD_H
