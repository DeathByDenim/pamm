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

#include "modfilterwidget.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <qstyle.h>

ModFilterWidget::ModFilterWidget(QWidget* parent)
 : QWidget(parent)
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	
	QLabel *filterLabel = new QLabel(this);
	filterLabel->setText("FILTER:");
	layout->addWidget(filterLabel);
	
	QLineEdit *filterLineEdit = new QLineEdit(this);
	connect(filterLineEdit, SIGNAL(textChanged(QString)), SLOT(textChanged(QString)));
	layout->addWidget(filterLineEdit);

	QToolButton *filterCloseButton = new QToolButton(this);
	filterCloseButton->setIcon(style()->standardIcon(QStyle::QStyle::SP_DialogCloseButton));
	connect(filterCloseButton, SIGNAL(clicked(bool)), SLOT(clicked(bool)));
	layout->addWidget(filterCloseButton);
	
	setVisible(false);
}

ModFilterWidget::~ModFilterWidget()
{
}

void ModFilterWidget::clicked(bool checked)
{
	setVisible(false);
}

void ModFilterWidget::textChanged(const QString& text)
{
	emit filterTextChanged(text);
}

#include "modfilterwidget.moc"
