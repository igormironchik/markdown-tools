/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

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


namespace MdPdf {

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

protected:
	void showEvent( QShowEvent * event ) override;

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

} /* namespace MdPdf */
