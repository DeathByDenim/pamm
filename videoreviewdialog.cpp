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

#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QWebView>
#include <QImageReader>
#include "videoreviewdialog.h"

VideoReviewDialog::VideoReviewDialog(QWidget* parent, const QString& imgdir)
 : QDialog(parent)
{
	setWindowTitle("Video review for Hotbuild2");

	QImageReader *reader = new QImageReader(imgdir + "img_start_bground_sample.jpg");
	QImage bground = reader->read();
	delete reader;

	QPalette* palette = new QPalette();
	palette->setBrush(QPalette::Background, *(new QBrush(bground)));
	setPalette(*palette);

	QVBoxLayout *layout = new QVBoxLayout(this);

	QWebView *webview = new QWebView(this);
	webview->setGeometry(0, 0, 640, 480);
	webview->resize(640, 480);
	QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
	webview->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
	webview->load(QUrl("http://www.youtube.com/embed/NNXV01cttMY?html5=1"));
	webview->show();

	QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, Qt::Horizontal, this);
//	connect(buttonBox, SIGNAL(accepted()), SLOT(accept()));
	
	layout->addWidget(webview);
	layout->addWidget(buttonBox);
}

VideoReviewDialog::~VideoReviewDialog()
{
}

#include "videoreviewdialog.moc"
