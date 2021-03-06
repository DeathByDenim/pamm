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

class QTimer;
class QLineEdit;

class ModFilterWidget : public QWidget
{
    Q_OBJECT
    QLineEdit* FilterLineEdit;

public:
	ModFilterWidget(QWidget *parent = 0);
	~ModFilterWidget();

protected:
	virtual void keyPressEvent(QKeyEvent *event);
	virtual void focusInEvent(QFocusEvent *event);
	virtual void showEvent(QShowEvent *event);
	virtual void hideEvent(QHideEvent *event);

private:
	QTimer *TextTimer;

private Q_SLOTS:
	void clicked(bool checked = false);
	void textChanged(const QString &text);
	void filterTextReallyChanged();

Q_SIGNALS:
	void filterTextChanged(const QString &text);
	void visibilityChanged(bool visible);
};

#endif // MODFILTERWIDGET_H
