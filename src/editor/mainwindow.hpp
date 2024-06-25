/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QMainWindow>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE
class QTreeWidgetItem;
QT_END_NAMESPACE


namespace MdEditor {

//
// MainWindow
//

struct MainWindowPrivate;

//! Main window.
class MainWindow
	:	public QMainWindow
{
	Q_OBJECT

public:
	MainWindow();
	~MainWindow() override;

public slots:
	void openFile( const QString & path );
	void openInPreviewMode( bool loadAllLinked );
	void loadAllLinkedFiles();

protected:
	void resizeEvent( QResizeEvent * e ) override;
    void closeEvent( QCloseEvent * e ) override;
	bool event( QEvent * event ) override;

private slots:
    void onFileNew();
    void onFileOpen();
    void onFileSave();
    void onFileSaveAs();
	void onTextChanged();
	void onAbout();
	void onAboutQt();
	void onLineHovered( int lineNumber, const QPoint & pos );
	void onFind( bool on );
	void onFindWeb( bool on );
	void onGoToLine( bool on );
	void onChooseFont();
	void onLessFontSize();
	void onMoreFontSize();
	void onToolHide();
	void onCursorPositionChanged();
	void onEditMenuActionTriggered( QAction * action );
	void onNavigationDoubleClicked( QTreeWidgetItem * item, int column );
	void onTogglePreviewAction( bool checked );
	void onShowLicenses();
	void onConvertToPdf();
	void onAddTOC();
	void onChangeColors();
	void onTocClicked( const QModelIndex & index );
	void onTabClicked( int index );
	void onSettings();

private:
    bool isModified() const;
	QString htmlContent() const;
	void saveCfg() const;
	void readCfg();
	void readAllLinked();
	void updateWindowTitle();
	void updateLoadAllLinkedFilesMenuText();
	void closeAllLinkedFiles();
	QString configFileName( bool inPlace ) const;
	void showOrHideTabs();

private:
	Q_DISABLE_COPY( MainWindow )

	friend struct MainWindowPrivate;
	friend class Find;
	friend class FindWeb;
	friend class GoToLine;

	QScopedPointer< MainWindowPrivate > d;
}; // class MainWindow

} /* namespace MdEditor */
