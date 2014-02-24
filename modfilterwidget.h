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

#ifndef MODFILTERWIDGET_H
#define MODFILTERWIDGET_H

#include <QWidget>

class ModFilterWidget : public QWidget
{
    Q_OBJECT

public:
    ModFilterWidget(QWidget *parent = 0);
    ~ModFilterWidget();

private:
	
private Q_SLOTS:
	void clicked(bool checked = false);
	void textChanged(const QString &text);

Q_SIGNALS:
	void filterTextChanged(const QString &text);
};

#endif // MODFILTERWIDGET_H
