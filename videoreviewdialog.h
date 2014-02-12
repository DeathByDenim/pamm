/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2014  Jarno van der Kolk <jvanderk@uottawa.ca>
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

#ifndef VIDEOREVIEWDIALOG_H
#define VIDEOREVIEWDIALOG_H

#include <QDialog>

class VideoReviewDialog : public QDialog
{
	Q_OBJECT

public:
	VideoReviewDialog(QWidget* parent, const QString& imgdir);
	~VideoReviewDialog();

private:
};

#endif // VIDEOREVIEWDIALOG_H
