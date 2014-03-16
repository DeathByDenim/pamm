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

#ifndef MOD_H
#define MOD_H

#include <QWidget>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QDate>

class QLabel;
class Mod : public QWidget
{
    Q_OBJECT

public:
	Mod(const QString& Key, const QString& DisplayName, const QString& Description, const QString& Author, const QUrl& Forum, const QStringList& Category, const QString& Version, const QStringList& Requires, const QDate& Date, const QString& Build);
	~Mod();

	QString displayName() {return DisplayName;}
	QString key() {return Key;}
	QStringList requires() {return Requires;}
	QStringList category() {return Category;}
	bool textContains(QString filtertext);
	virtual void setCompactView(bool compact) = 0;

	static bool sortLastUpdated(const Mod *m1, const Mod *m2);
	static bool sortTitle(const Mod *m1, const Mod *m2);
	static bool sortAuthor(const Mod *m1, const Mod *m2);
	static bool sortBuild(const Mod *m1, const Mod *m2);
	static bool sortRandom(const Mod *m1, const Mod *m2);


protected:
	const QString Key;
	const QString DisplayName; // Name of Mod (avoid special characters, and be mindful of the length)
	const QString Description; // Name of Mod (avoid special characters, and be mindful of the length)
	const QString Author; // Your Name
	const QUrl Forum; // Forum Link to Mod
	const QStringList Category; // Category
	const QString Version; // Version Number
	QStringList Requires;
	const QDate Date;
	const QString Build; // Build Number

	void setRelativeFontSizeForLabel(QLabel *label, qreal size);
};

#endif // MOD_H
