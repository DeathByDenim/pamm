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
#include <QStyle>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QShowEvent>
#include <QHideEvent>

ModFilterWidget::ModFilterWidget(QWidget* parent)
 : QWidget(parent)
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	
	QLabel *filterLabel = new QLabel(this);
	filterLabel->setText(tr("FILTER:"));
	layout->addWidget(filterLabel);
	
	FilterLineEdit = new QLineEdit(this);
	connect(FilterLineEdit, SIGNAL(textChanged(QString)), SLOT(textChanged(QString)));
	layout->addWidget(FilterLineEdit);

	QToolButton *filterCloseButton = new QToolButton(this);
	filterCloseButton->setIcon(style()->standardIcon(QStyle::QStyle::SP_DialogCloseButton));
	connect(filterCloseButton, SIGNAL(clicked(bool)), SLOT(clicked(bool)));
	layout->addWidget(filterCloseButton);
}

ModFilterWidget::~ModFilterWidget()
{
}

void ModFilterWidget::clicked(bool checked)
{
	setVisible(false);
	emit visibilityChanged(false);
}

void ModFilterWidget::textChanged(const QString& text)
{
	emit filterTextChanged(text);
}

void ModFilterWidget::keyPressEvent(QKeyEvent* event)
{
	if(event->key() == Qt::Key_Escape)
	{
		setVisible(false);
		emit visibilityChanged(false);
		event->accept();
	}
	else
		QWidget::keyPressEvent(event);
}

void ModFilterWidget::focusInEvent(QFocusEvent* event)
{
	QWidget::focusInEvent(event);
	
	if(event->gotFocus())
		FilterLineEdit->setFocus();
}

void ModFilterWidget::showEvent(QShowEvent* event)
{
	QWidget::showEvent(event);

	emit filterTextChanged(FilterLineEdit->text());
}

void ModFilterWidget::hideEvent(QHideEvent* event)
{
	QWidget::hideEvent(event);

	emit filterTextChanged("");
}

#include "modfilterwidget.moc"
