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

#include "mod.h"
#include <QLabel>

Mod::Mod(const QString &Key, const QString &DisplayName, const QString &Description, const QString &Author, const QUrl &Forum, const QStringList &Category, const QString &Version, const QStringList &Requires, const QDate &Date, const QString &Build)
 : QWidget(), Key(Key), DisplayName(DisplayName), Description(Description), Author(Author), Forum(Forum), Category(Category), Version(Version), Requires(Requires), Date(Date), Build(Build)
{
}

Mod::~Mod()
{
}

void Mod::setRelativeFontSizeForLabel(QLabel* label, qreal size)
{
	QFont font = label->font();
	font.setPointSizeF(size * font.pointSizeF());
	label->setFont(font);
}

#include "mod.moc"
