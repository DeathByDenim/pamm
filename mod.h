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

class Mod : public QWidget
{
    Q_OBJECT

public:
	Mod(const QString &Key, const QString &Name, const QStringList &Authors, const QUrl &Link, const QString &Category, const QString &Version, const unsigned int &Build);
	~Mod();

	QString name() {return Name;};
	QString key() {return Key;};

protected:
	const QString Key;
	const QString Name; // Name of Mod (avoid special characters, and be mindful of the length)
	const QStringList Authors; // Your Name
	const QUrl Link; // Forum Link to Mod
	const QString Category; // Category
	const QString Version; // Version Number
	const unsigned int Build; // Build Number

private:

};

#endif // MOD_H
