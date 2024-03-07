
/*!
	\file

	\author Igor Mironchik (igor.mironchik at gmail dot com).

	Copyright (c) 2019-2024 Igor Mironchik

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef MD_PDF_MAIN_WINDOW_HPP_INCLUDED
#define MD_PDF_MAIN_WINDOW_HPP_INCLUDED


// md-pdf include.
#include "ui_main_window.h"

// Qt include.
#include <QWidget>
#include <QScopedPointer>
#include <QThread>
#include <QMainWindow>

// C++ include.
#include <memory>

// md-pdf include.
#include "syntax.hpp"


//
// MainWidget
//

//! Main window.
class MainWidget final
	:	public QWidget
{
	Q_OBJECT

public:
	explicit MainWidget( QWidget * parent );
	~MainWidget() override;

	void setMarkdownFile( const QString & fileName );

private slots:
	void changeLinkColor();
	void changeBorderColor();
	void selectMarkdown();
	void process();
	void codeFontSizeChanged( int i );
	void textFontSizeChanged( int i );
	void mmButtonToggled( bool on );
	void textFontChanged( const QFont & f );
	void codeFontChanged( const QFont & f );

private:
	void changeStateOfStartButton();
	QString configFileName( bool inPlace ) const;
	void readCfg();
	void saveCfg();

private:
	QScopedPointer< Ui::MainWindow > m_ui;
	QThread * m_thread;
	bool m_textFontOk;
	bool m_codeFontOk;
	std::shared_ptr< Syntax > m_syntax;

	Q_DISABLE_COPY( MainWidget )
}; // class MainWindow


//
// MainWindow
//

class MainWindow
	:	public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	~MainWindow() override = default;

	void setMarkdownFile( const QString & fileName );

private slots:
	//! About.
	void about();
	//! About Qt.
	void aboutQt();
	//! Licenses.
	void licenses();
	//! Quit.
	void quit();

private:
	MainWidget * ui = nullptr;
}; // class MainWindow

#endif // MD_PDF_MAIN_WINDOW_HPP_INCLUDED
