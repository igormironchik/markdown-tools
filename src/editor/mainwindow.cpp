/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "mainwindow.hpp"
#include "editor.hpp"
#include "webview.hpp"
#include "previewpage.hpp"
#include "htmldocument.hpp"
#include "find.hpp"
#include "findweb.hpp"
#include "gotoline.hpp"
#include "fontdlg.hpp"
#include "cfg.hpp"
#include "version.hpp"
#include "colors.hpp"

// Qt include.
#include <QSplitter>
#include <QWidget>
#include <QHBoxLayout>
#include <QResizeEvent>
#include <QWebChannel>
#include <QMessageBox>
#include <QFileDialog>
#include <QCloseEvent>
#include <QMenu>
#include <QAction>
#include <QMenuBar>
#include <QFileInfo>
#include <QDir>
#include <QToolTip>
#include <QStandardPaths>
#include <QVBoxLayout>
#include <QTextDocumentFragment>
#include <QStatusBar>
#include <QApplication>
#include <QDockWidget>
#include <QTreeWidget>
#include <QProcess>
#include <QLineEdit>
#include <QLabel>
#include <QTextBlock>

// md4qt include.
#define MD4QT_QT_SUPPORT
#include <md4qt/traits.hpp>
#include <md4qt/parser.hpp>
#include <md4qt/html.hpp>

// cfgfile include.
#include <cfgfile/all.hpp>

// shared include.
#include "license_dialog.hpp"


namespace MdEditor {

//
// MainWindowPrivate
//

struct MainWindowPrivate {
	MainWindowPrivate( MainWindow * parent )
		:	q( parent )
	{
	}

	void initUi()
	{
		auto w = new QWidget( q );
		auto l = new QHBoxLayout( w );

		splitter = new QSplitter( Qt::Horizontal, w );

		auto ew = new QWidget( w );
		auto v = new QVBoxLayout( ew );
		v->setContentsMargins( 0, 0, 0, 0 );
		v->setSpacing( 0 );
		editor = new Editor( ew );
		find = new Find( q, editor, ew );
		gotoline = new GoToLine( q, editor, ew );
		v->addWidget( editor );
		v->addWidget( gotoline );
		v->addWidget( find );

		auto pw = new QWidget( w );
		auto v1 = new QVBoxLayout( pw );
		v1->setContentsMargins( 0, 0, 0, 0 );
		v1->setSpacing( 0 );
		preview = new WebView( w );
		findWeb = new FindWeb( q, preview, pw );
		v1->addWidget( preview );
		v1->addWidget( findWeb );

		find->hide();
		gotoline->hide();
		findWeb->hide();

		splitter->addWidget( ew );
		splitter->addWidget( pw );

		l->addWidget( splitter );

		q->setCentralWidget( w );

		page = new PreviewPage( preview );
		preview->setPage( page );

		html = new HtmlDocument( q );

		auto channel = new QWebChannel( q );
		channel->registerObject( QStringLiteral( "content" ), html );
		page->setWebChannel( channel );

		baseUrl = QString( "file:%1/" ).arg(
			QString( QUrl::toPercentEncoding(
				QStandardPaths::standardLocations( QStandardPaths::HomeLocation ).first(),
					"/\\:", {} ) ) );
		editor->setDocName( QStringLiteral( "default.md" ) );
		page->setHtml( q->htmlContent(), baseUrl );

		q->updateWindowTitle();

#ifdef Q_OS_WIN
		mdPdfExe = QStringLiteral( "md-pdf-gui.exe" );
		launcherExe = QStringLiteral( "md-launcher.exe" );
#else
		mdPdfExe = QStringLiteral( "md-pdf-gui" );
		launcherExe = QStringLiteral( "md-launcher" );
#endif

		QDir workingDir( QApplication::applicationDirPath() );
		const auto mdPdfExeFiles = workingDir.entryInfoList( { mdPdfExe },
			QDir::Executable | QDir::Files );
		const auto starterExeFiles = workingDir.entryInfoList( { launcherExe },
			QDir::Executable | QDir::Files );

		auto fileMenu = q->menuBar()->addMenu( MainWindow::tr( "&File" ) );
		newAction = fileMenu->addAction( QIcon( QStringLiteral( ":/res/img/document-new.png" ) ),
				MainWindow::tr( "New" ), MainWindow::tr( "Ctrl+N" ), q, &MainWindow::onFileNew );
		openAction = fileMenu->addAction( QIcon( QStringLiteral( ":/res/img/document-open.png" ) ),
				MainWindow::tr( "Open" ), MainWindow::tr( "Ctrl+O" ), q, &MainWindow::onFileOpen );
		fileMenu->addSeparator();
		saveAction = fileMenu->addAction( QIcon( QStringLiteral( ":/res/img/document-save.png" ) ),
			MainWindow::tr( "Save" ), MainWindow::tr( "Ctrl+S" ), q, &MainWindow::onFileSave );
		saveAsAction = fileMenu->addAction( QIcon( QStringLiteral( ":/res/img/document-save-as.png" ) ),
			MainWindow::tr( "Save As" ), q, &MainWindow::onFileSaveAs );
		fileMenu->addSeparator();
		loadAllAction = fileMenu->addAction( MainWindow::tr( "Load All Linked Files..." ),
			MainWindow::tr( "Ctrl+R" ), q, &MainWindow::loadAllLinkedFiles );
		loadAllAction->setEnabled( false );

		if( !mdPdfExeFiles.isEmpty() && ! starterExeFiles.isEmpty() )
		{
			fileMenu->addSeparator();

			convertToPdfAction = fileMenu->addAction( MainWindow::tr( "Convert To PDF..." ),
				q, &MainWindow::onConvertToPdf );

			convertToPdfAction->setEnabled( false );
		}

		fileMenu->addSeparator();
		fileMenu->addAction( QIcon( QStringLiteral( ":/res/img/application-exit.png" ) ),
			MainWindow::tr( "Quit" ), MainWindow::tr( "Ctrl+Q" ), q, &QWidget::close );

		editMenuAction = q->menuBar()->addAction( MainWindow::tr( "&Edit" ) );
		toggleFindAction = new QAction(
			QIcon( QStringLiteral( ":/res/img/edit-find.png" ) ),
			MainWindow::tr( "Find/Replace" ), q );
		toggleFindAction->setShortcut( MainWindow::tr( "Ctrl+F" ) );
		q->addAction( toggleFindAction );

		toggleFindWebAction = new QAction(
			QIcon( QStringLiteral( ":/res/img/edit-find.png" ) ),
			MainWindow::tr( "Find In Preview" ), q );
		toggleFindWebAction->setShortcut( MainWindow::tr( "Ctrl+W" ) );
		q->addAction( toggleFindWebAction );

		toggleGoToLineAction = new QAction(
			QIcon( QStringLiteral( ":/res/img/go-next-use.png" ) ),
			MainWindow::tr( "Go to Line" ), q );
		toggleGoToLineAction->setShortcut( MainWindow::tr( "Ctrl+L" ) );
		q->addAction( toggleGoToLineAction );

		addTOCAction = new QAction( MainWindow::tr( "Add TOC" ), q );
		
		auto formatMenu = q->menuBar()->addMenu( MainWindow::tr( "F&ormat" ) );
		
		tabAction = new QAction( MainWindow::tr( "Indent" ), formatMenu );
		tabAction->setShortcut( MainWindow::tr( "Tab" ) );
		tabAction->setShortcutContext( Qt::WidgetShortcut );
		QObject::connect( tabAction, &QAction::triggered,
			[this](){
				qApp->postEvent( this->editor,
					new QKeyEvent( QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier ) );
			} );
		formatMenu->addAction( tabAction );
		
		backtabAction = new QAction( MainWindow::tr( "Unindent" ), formatMenu );
		backtabAction->setShortcut( MainWindow::tr( "Shift+Tab" ) );
		backtabAction->setShortcutContext( Qt::WidgetShortcut );
		QObject::connect( backtabAction, &QAction::triggered,
			[this](){
				qApp->postEvent( this->editor,
					new QKeyEvent( QEvent::KeyPress, Qt::Key_Backtab, Qt::NoModifier ) );
			} );
		formatMenu->addAction( backtabAction );

		auto viewMenu = q->menuBar()->addMenu( MainWindow::tr( "&View" ) );
		viewAction = new QAction( QIcon( QStringLiteral( ":/res/img/view-preview.png" ) ),
			MainWindow::tr( "Toggle Preview Mode" ) );
		viewAction->setShortcut( MainWindow::tr( "Ctrl+P" ) );
		viewAction->setCheckable( true );
		viewAction->setChecked( false );
		viewMenu->addAction( viewAction );

		settingsMenu = q->menuBar()->addMenu( MainWindow::tr( "&Settings" ) );
		auto toggleLineNumbersAction = new QAction(
			QIcon( QStringLiteral( ":/res/img/view-table-of-contents-ltr.png" ) ),
			MainWindow::tr( "Show Line Numbers" ), q );
		toggleLineNumbersAction->setCheckable( true );
		toggleLineNumbersAction->setShortcut( MainWindow::tr( "Alt+L" ) );
		toggleLineNumbersAction->setChecked( true );
		settingsMenu->addAction( toggleLineNumbersAction );

		auto toggleUnprintableCharacters = new QAction(
			QIcon( QStringLiteral( ":/res/img/character-set.png" ) ),
			MainWindow::tr( "Show Tabs/Spaces" ), q );
		toggleUnprintableCharacters->setCheckable( true );
		toggleUnprintableCharacters->setShortcut( MainWindow::tr( "Alt+T" ) );
		toggleUnprintableCharacters->setChecked( true );
		settingsMenu->addAction( toggleUnprintableCharacters );

		settingsMenu->addSeparator();

		settingsMenu->addAction( QIcon( QStringLiteral( ":/res/img/format-font-size-less.png" ) ),
			MainWindow::tr( "Decrease Font Size" ), MainWindow::tr( "Ctrl+-" ),
			q, &MainWindow::onLessFontSize );
		settingsMenu->addAction( QIcon( QStringLiteral( ":/res/img/format-font-size-more.png" ) ),
			MainWindow::tr( "Increase Font Size" ), MainWindow::tr( "Ctrl+=" ),
			q, &MainWindow::onMoreFontSize );

		settingsMenu->addSeparator();

		settingsMenu->addAction( QIcon( QStringLiteral( ":/res/img/preferences-desktop-font.png" ) ),
			MainWindow::tr( "Font..." ),
			q, &MainWindow::onChooseFont );

		settingsMenu->addAction( QIcon( QStringLiteral( ":/res/img/fill-color.png" ) ),
			MainWindow::tr( "Colors..." ),
			q, &MainWindow::onChangeColors );


		auto helpMenu = q->menuBar()->addMenu( MainWindow::tr( "&Help" ) );
		helpMenu->addAction( QIcon( QStringLiteral( ":/icon/icon_24x24.png" ) ),
			MainWindow::tr( "About" ), q, &MainWindow::onAbout );
		helpMenu->addAction( QIcon( QStringLiteral( ":/img/Qt-logo-neon-transparent.png" ) ),
			MainWindow::tr( "About Qt" ), q, &MainWindow::onAboutQt );
		helpMenu->addAction( QIcon( QStringLiteral( ":/res/img/bookmarks-organize.png" ) ),
				MainWindow::tr( "Licenses" ), q, &MainWindow::onShowLicenses );

		cursorPosLabel = new QLabel( q );
		q->statusBar()->addPermanentWidget( cursorPosLabel );

		QObject::connect( editor->document(), &QTextDocument::modificationChanged,
			saveAction, &QAction::setEnabled );
		QObject::connect( editor->document(), &QTextDocument::modificationChanged,
			q, &MainWindow::setWindowModified );
		QObject::connect( editor, &QPlainTextEdit::textChanged, q, &MainWindow::onTextChanged );
		QObject::connect( editor, &Editor::lineHovered, q, &MainWindow::onLineHovered );
		QObject::connect( toggleLineNumbersAction, &QAction::toggled,
			editor, &Editor::showLineNumbers );
		QObject::connect( toggleUnprintableCharacters, &QAction::toggled,
			editor, &Editor::showUnprintableCharacters );
		QObject::connect( toggleFindAction, &QAction::triggered,
			q, &MainWindow::onFind );
		QObject::connect( toggleFindWebAction, &QAction::triggered,
			q, &MainWindow::onFindWeb );
		QObject::connect( toggleGoToLineAction, &QAction::triggered,
			q, &MainWindow::onGoToLine );
		QObject::connect( page, &QWebEnginePage::linkHovered,
			[this]( const QString & url )
			{
				if( !url.isEmpty() )
					this->q->statusBar()->showMessage( url );
				else
					this->q->statusBar()->clearMessage();
			} );
		QObject::connect( editor, &QPlainTextEdit::cursorPositionChanged,
			q, &MainWindow::onCursorPositionChanged );
		QObject::connect( viewAction, &QAction::toggled,
			q, &MainWindow::onTogglePreviewAction );
		QObject::connect( addTOCAction, &QAction::triggered,
			q, &MainWindow::onAddTOC );

		q->readCfg();

		q->onCursorPositionChanged();

		editor->setFocus();

		preview->setFocusPolicy( Qt::ClickFocus );

		editor->applyColors( mdColors );

		q->setTabOrder( gotoline->line(), find->editLine() );
		q->setTabOrder( find->editLine(), find->replaceLine() );
		q->setTabOrder( find->replaceLine(), findWeb->line() );
	}

	MainWindow * q = nullptr;
	Editor * editor = nullptr;
	WebView * preview = nullptr;
	PreviewPage * page = nullptr;
	QSplitter * splitter = nullptr;
	HtmlDocument * html = nullptr;
	Find * find = nullptr;
	FindWeb * findWeb = nullptr;
	GoToLine * gotoline = nullptr;
	QAction * newAction = nullptr;
	QAction * openAction = nullptr;
	QAction * saveAction = nullptr;
	QAction * saveAsAction = nullptr;
	QAction * toggleFindAction = nullptr;
	QAction * toggleFindWebAction = nullptr;
	QAction * toggleGoToLineAction = nullptr;
	QAction * editMenuAction = nullptr;
	QAction * loadAllAction = nullptr;
	QAction * viewAction = nullptr;
	QAction * convertToPdfAction = nullptr;
	QAction * addTOCAction = nullptr;
	QAction * tabAction = nullptr;
	QAction * backtabAction = nullptr;
	QMenu * standardEditMenu = nullptr;
	QMenu * settingsMenu = nullptr;
	QDockWidget * fileTreeDock = nullptr;
	QTreeWidget * fileTree = nullptr;
	QLabel * cursorPosLabel = nullptr;
	bool init = false;
	bool loadAllFlag = false;
	bool previewMode = false;
	std::shared_ptr< MD::Document< MD::QStringTrait > > mdDoc;
	QString baseUrl;
	QString rootFilePath;
	QString mdPdfExe;
	QString launcherExe;
	Colors mdColors;
}; // struct MainWindowPrivate


//
// MainWindow
//

MainWindow::MainWindow()
	:	d( new MainWindowPrivate( this ) )
{
	d->initUi();
}

MainWindow::~MainWindow()
{
	if( d->standardEditMenu )
		d->standardEditMenu->deleteLater();

	d->standardEditMenu = nullptr;
}

void
MainWindow::onConvertToPdf()
{
	QDir workingDir( QApplication::applicationDirPath() );
	const auto mdPdfExeFiles = workingDir.entryInfoList( { d->mdPdfExe },
		QDir::Executable | QDir::Files );
	const auto starterExeFiles = workingDir.entryInfoList( { d->launcherExe },
		QDir::Executable | QDir::Files );

	if( !mdPdfExeFiles.isEmpty() && !starterExeFiles.isEmpty() )
	{
		QProcess::startDetached( starterExeFiles.at( 0 ).absoluteFilePath(),
			{ QStringLiteral( "--exe" ), mdPdfExeFiles.at( 0 ).fileName(),
			  QStringLiteral( "--mode" ), QStringLiteral( "detached" ),
			  QStringLiteral( "--arg" ), d->rootFilePath },
			workingDir.absolutePath() );
	}
}

void
MainWindow::resizeEvent( QResizeEvent * e )
{
	if( !d->init )
	{
		d->init = true;

		auto w = centralWidget()->width() / 2;

		if( !d->previewMode )
			d->splitter->setSizes( { w, w } );
		else
			d->splitter->setSizes( { 0, centralWidget()->width() } );
	}

	e->accept();
}

void
MainWindow::openFile( const QString & path )
{
	QFile f( path );
	if( !f.open( QIODevice::ReadOnly ) )
	{
		QMessageBox::warning( this, windowTitle(),
			tr( "Could not open file %1: %2" ).arg(
				QDir::toNativeSeparators( path ), f.errorString() ) );
		return;
	}

	d->editor->setDocName( path );
	d->baseUrl = QString( "file:%1/" ).arg(
		QString( QUrl::toPercentEncoding(
			QFileInfo( path ).absoluteDir().absolutePath(), "/\\:", {} ) ) );
	d->page->setHtml( htmlContent(), d->baseUrl );

	d->editor->setText( f.readAll() );
	f.close();
	updateWindowTitle();
	d->editor->setFocus();
	d->editor->document()->clearUndoRedoStacks();
	onCursorPositionChanged();
	d->loadAllAction->setEnabled( true );
	d->rootFilePath = path;

	if( d->convertToPdfAction )
		d->convertToPdfAction->setEnabled( true );

	closeAllLinkedFiles();
	updateLoadAllLinkedFilesMenuText();
}

void
MainWindow::openInPreviewMode( bool loadAllLinked )
{
	d->loadAllFlag = loadAllLinked;

	d->viewAction->setChecked( true );
}

bool
MainWindow::isModified() const
{
	return d->editor->document()->isModified();
}

void
MainWindow::onFileNew()
{
	if( isModified() )
	{
		QMessageBox::StandardButton button = QMessageBox::question( this, windowTitle(),
			tr( "You have unsaved changes. Do you want to create a new document anyway?" ),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No );

		if( button != QMessageBox::Yes )
			return;
	}

	d->editor->setDocName( QStringLiteral( "default.md" ) );
	d->editor->setText( "" );
	d->editor->document()->setModified( false );
	d->editor->document()->clearUndoRedoStacks();
	updateWindowTitle();

	d->baseUrl = QString( "file:%1/" ).arg(
		QString( QUrl::toPercentEncoding(
			QStandardPaths::standardLocations( QStandardPaths::HomeLocation ).first(),
				"/\\:", {} ) ) );
	d->page->setHtml( htmlContent(), d->baseUrl );
	onCursorPositionChanged();
	d->loadAllAction->setEnabled( false );
	d->rootFilePath.clear();

	if( d->convertToPdfAction )
		d->convertToPdfAction->setEnabled( false );

	closeAllLinkedFiles();
	updateLoadAllLinkedFilesMenuText();
}

void
MainWindow::onFileOpen()
{
	if( isModified() )
	{
		QMessageBox::StandardButton button = QMessageBox::question( this, windowTitle(),
			tr( "You have unsaved changes. Do you want to open a new document anyway?" ),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No );

		if( button != QMessageBox::Yes )
			return;
	}

	const auto folder = d->editor->docName() == QStringLiteral( "default.md" ) ?
		QStandardPaths::standardLocations( QStandardPaths::HomeLocation ).first() :
		QFileInfo( d->editor->docName() ).absolutePath();

	QFileDialog dialog( this, tr( "Open Markdown File" ), folder );
	dialog.setMimeTypeFilters( { "text/markdown" } );
	dialog.setAcceptMode( QFileDialog::AcceptOpen );

	if( dialog.exec() == QDialog::Accepted )
		openFile( dialog.selectedFiles().constFirst() );
}

void
MainWindow::onFileSave()
{
	if( d->editor->docName() == QStringLiteral( "default.md" ) )
	{
		onFileSaveAs();
		return;
	}

	QFile f( d->editor->docName() );
	if( !f.open( QIODevice::WriteOnly | QIODevice::Text ) )
	{
		QMessageBox::warning( this, windowTitle(),
			tr( "Could not write to file %1: %2" ).arg(
				QDir::toNativeSeparators( d->editor->docName() ), f.errorString() ) );

		return;
	}

	QTextStream str( &f );
	str << d->editor->toPlainText();
	f.close();

	d->editor->document()->setModified( false );

	updateWindowTitle();

	readAllLinked();
}

void
MainWindow::onFileSaveAs()
{
	QFileDialog dialog( this, tr( "Save Markdown File" ),
		QStandardPaths::standardLocations( QStandardPaths::HomeLocation ).first() );
	dialog.setMimeTypeFilters( { "text/markdown" } );
	dialog.setAcceptMode( QFileDialog::AcceptSave );
	dialog.setDefaultSuffix( "md" );

	if( dialog.exec() != QDialog::Accepted )
		return;

	d->editor->setDocName( dialog.selectedFiles().constFirst() );
	d->baseUrl = QString( "file:%1/" ).arg(
		QString( QUrl::toPercentEncoding(
			QFileInfo( d->editor->docName() ).absoluteDir().absolutePath(), "/\\:", {} ) ) );
	d->rootFilePath = d->editor->docName();

	if( d->convertToPdfAction )
		d->convertToPdfAction->setEnabled( true );

	onFileSave();

	d->page->setHtml( htmlContent(), d->baseUrl );

	closeAllLinkedFiles();

	updateLoadAllLinkedFilesMenuText();
}

void
MainWindow::closeEvent( QCloseEvent * e )
{
	if( isModified() )
	{
		QMessageBox::StandardButton button = QMessageBox::question( this, windowTitle(),
			tr( "You have unsaved changes. Do you want to exit anyway?" ),
			QMessageBox::Yes | QMessageBox::No, QMessageBox::No );

		if( button != QMessageBox::Yes )
			e->ignore();
	}
}

bool
MainWindow::event( QEvent * event )
{
	switch( event->type() )
	{
		case QEvent::ShortcutOverride :
		{
			if( static_cast< QKeyEvent* >( event )->key() == Qt::Key_Escape )
			{
				event->accept();

				if( d->findWeb->isVisible() )
					d->findWeb->hide();
				else if( d->gotoline->isVisible() )
					d->gotoline->hide();
				else if( d->find->isVisible() )
					d->find->hide();

				onToolHide();

				return true;
			}
		}
			break;

		default :
			break;
	}

	return QMainWindow::event( event );
}

void
MainWindow::onToolHide()
{
	if( !d->find->isVisible() && !d->gotoline->isVisible() )
	{
		d->editor->setFocus();
		d->editor->clearHighlighting();
	}
	else if( d->find->isVisible() && !d->gotoline->isVisible() )
		d->find->setFocusOnFind();
	else if( d->gotoline->isVisible() && !d->find->isVisible() )
	{
		d->gotoline->setFocusOnLine();
		d->editor->clearHighlighting();
	}
}

QString
MainWindow::htmlContent() const
{
	return QStringLiteral( "<!doctype html>\n"
		"<meta charset=\"utf-8\">\n"
		"<head>\n"
		"  <script src=\"qrc:/qtwebchannel/qwebchannel.js\"></script>\n"
		"  <link rel=\"stylesheet\" href=\"qrc:/res/css/github.css\">\n"
		"  <link rel=\"stylesheet\" href=\"qrc:/res/css/github-markdown.css\">\n"
		"  <script src=\"qrc:/res/highlight.js\"></script>\n"
		"  <script>hljs.configure({languages:[]});</script>\n"
		"  <link rel=\"stylesheet\" href=\"qrc:/res/katex/katex.min.css\">\n"
		"  <script src=\"qrc:/res/katex/katex.min.js\"></script>\n"
		"  <script src=\"qrc:/res/katex/contrib/auto-render.min.js\"></script>\n"
		"  <link href=\"qrc:/res/css/emoji.css\" rel=\"stylesheet\" type=\"text/css\" />\n"
		"  <script src=\"qrc:/res/emoji.js\" type=\"text/javascript\"></script>\n"
		"</head>\n"
		"<body>\n"
		"  <div id=\"placeholder\"></div>\n"
		"  <script type=\"module\">\n"
		"  import { replaceBadges } from \"qrc:/res/hightlight-blockquote.js\";\n"
		"  (() => {\n"
		"  'use strict';\n"
		"\n"
		"  let placeholder = document.getElementById('placeholder');\n"
		"  let emoji = new EmojiConvertor();\n"
		"  emoji.replace_mode = 'unified';\n"
		"  emoji.allow_native = true;\n"
		"  emoji.addAliases({\n"
		"    'robot' : '1f916',\n"
		"    'metal' : '1f918',\n"
		"    'fu' : '1f595'\n"
		"  });\n"
		"\n"
		"  function replace_emoji() {\n"
		"    let elementsToReplace = document.querySelectorAll('p, h1, h2, h3, h4, h5, h6, li, table');\n"
		"    let elementsToIgnore = document.querySelectorAll('code');\n"
		"    for (let i = 0; i < elementsToReplace.length; i++) {\n"
		"      replace_emoji_in_node(elementsToReplace[i]);\n"
		"    }\n"
		"\n"
		"    function replace_emoji_in_node(node) {\n"
		"      if (node.constructor === Text) {\n"
		"	    node.data = emoji.replace_colons(node.data);\n"
		"      } else if (node.constructor === HTMLScriptElement || [].indexOf.call(elementsToIgnore, node) >= 0) {\n"
		"        return;\n"
		"      } else {\n"
		"        let childNodes = node.childNodes;\n"
		"        for (let i = 0; i < childNodes.length; i++) {\n"
		"          replace_emoji_in_node(childNodes[i]);\n"
		"        }\n"
		"      }\n"
		"    }\n"
		"  }\n"
		"\n"
		"  let updateText = function(text) {\n"
		"     placeholder.innerHTML = text;\n"
		"     hljs.highlightAll();\n"
		"     renderMathInElement(document.body, {\n"
		"         delimiters: [\n"
		"             {left: '$$', right: '$$', display: true},\n"
		"             {left: '$', right: '$', display: false},\n"
		"         ],\n"
		"         throwOnError : false,\n"
		"         strict : false,\n"
		"         trust : true\n"
		"     });\n"
		"     replaceBadges(placeholder);\n"
		"     replace_emoji();\n"
		"  }\n"
		"\n"
		"  new QWebChannel(qt.webChannelTransport,\n"
		"	function(channel) {\n"
		"	  let content = channel.objects.content;\n"
		"	  updateText(content.text);\n"
		"	  content.textChanged.connect(updateText);\n"
		"	}\n"
		"  );\n"
		"  })();</script>\n"
		"</body>\n"
		"</html>" );
}

void
MainWindow::onTextChanged()
{
	if( !d->loadAllFlag )
	{
		d->mdDoc = d->editor->currentDoc();

		d->html->setText( MD::toHtml( d->mdDoc, false,
			QStringLiteral( "qrc:/res/img/go-jump.png" ) ) );
	}
}

void
MainWindow::onAbout()
{
	QMessageBox::about( this, tr( "About Markdown Editor" ),
		tr( "Markdown Editor.\n\n"
			"Version %1\n\n"
			"md4qt version %2\n\n"
			"Author - Igor Mironchik (igor.mironchik at gmail dot com).\n\n"
			"Copyright (c) 2023-2024 Igor Mironchik.\n\n"
			"Licensed under GNU GPL 3.0." )
				.arg( c_version )
				.arg( c_md4qtVersion ) );
}

void
MainWindow::onAboutQt()
{
	QMessageBox::aboutQt( this );
}

namespace /* anonymous */ {

inline QString
itemType( MD::ItemType t )
{
	switch( t )
	{
		case MD::ItemType::Heading :
			return MainWindow::tr( "Heading" );
		case MD::ItemType::Text :
			return MainWindow::tr( "Text" );
		case MD::ItemType::Paragraph :
			return MainWindow::tr( "Paragraph" );
		case MD::ItemType::LineBreak :
			return MainWindow::tr( "Line Break" );
		case MD::ItemType::Blockquote :
			return MainWindow::tr( "Blockquote" );
		case MD::ItemType::ListItem :
			return MainWindow::tr( "List Item" );
		case MD::ItemType::List :
			return MainWindow::tr( "List" );
		case MD::ItemType::Link :
			return MainWindow::tr( "Link" );
		case MD::ItemType::Image :
			return MainWindow::tr( "Image" );
		case MD::ItemType::Code :
			return MainWindow::tr( "Code" );
		case MD::ItemType::TableCell :
			return MainWindow::tr( "Table Cell" );
		case MD::ItemType::TableRow :
			return MainWindow::tr( "Table Row" );
		case MD::ItemType::Table :
			return MainWindow::tr( "Table" );
		case MD::ItemType::FootnoteRef :
			return MainWindow::tr( "Footnote Reference" );
		case MD::ItemType::Footnote :
			return MainWindow::tr( "Footnote" );
		case MD::ItemType::Document :
			return MainWindow::tr( "Document" );
		case MD::ItemType::PageBreak :
			return MainWindow::tr( "Page Break" );
		case MD::ItemType::Anchor :
			return MainWindow::tr( "Anchor" );
		case MD::ItemType::HorizontalLine :
			return MainWindow::tr( "Horizontal Line" );
		case MD::ItemType::RawHtml :
			return MainWindow::tr( "Raw HTML" );
		case MD::ItemType::Math :
			return MainWindow::tr( "LaTeX Math Expression" );
	}

	return QString();
}

inline bool
inRange( long long int start, long long int end, int pos )
{
	return ( pos >= start && pos <= end );
}

} /* namespace anonymous */

void
MainWindow::onLineHovered( int lineNumber, const QPoint & pos )
{
	auto isItemInRange = [&]( std::shared_ptr< MD::Item< MD::QStringTrait > > item ) -> bool
	{
		const auto code = ( item->type() == MD::ItemType::Code ?
			static_cast< MD::Code< MD::QStringTrait >* > ( item.get() ) : nullptr );
		const auto startLine = ( code && code->isFensedCode() ? code->startDelim().startLine() :
			item->startLine() );
		const auto endLine = ( code && code->isFensedCode() && code->endDelim().startLine() != -1 ?
			code->endDelim().startLine() : item->endLine() );
		
		return inRange( startLine, endLine, lineNumber );
	};
	
	auto checkNested = [&]( std::shared_ptr< MD::Item< MD::QStringTrait > > item,
		std::shared_ptr< MD::Block< MD::QStringTrait > > parent ) -> bool
	{
		if( isItemInRange( item ) )
		{
			QToolTip::showText( pos, tr( "%1 in %2" )
				.arg( itemType( item->type() ), itemType( parent->type() ) ) );

			return true;
		}
		
		return false;
	};
	
	auto checkListOrFootnote = [&]( std::shared_ptr< MD::Block< MD::QStringTrait > > it ) -> bool
	{
		auto block = static_cast< MD::Block< MD::QStringTrait > * > ( it.get() );

		for( auto lit = block->items().cbegin(), llast = block->items().cend();
			lit != llast; ++lit )
		{
			if( (*lit)->startLine() == lineNumber )
			{
				QToolTip::showText( pos, itemType( it->type() ) );

				return true;
			}
			else
			{
				auto item = static_cast< MD::Block< MD::QStringTrait > * > ( lit->get() );

				if( block->type() == MD::ItemType::List )
				{
					for( auto iit = item->items().cbegin(), ilast = item->items().cend();
						iit != ilast; ++iit )
					{
						if( checkNested( *iit, it ) )
							return true;
					}
				}
				else if( checkNested( *lit, it ) )
					return true;
			}
		}
		
		return false;
	};
	
	auto checkItem = [&]( std::shared_ptr< MD::Item< MD::QStringTrait > > item ) -> bool
	{
		if( item->type() == MD::ItemType::List || item->type() == MD::ItemType::Footnote )
		{
			if( checkListOrFootnote(
				std::static_pointer_cast< MD::Block< MD::QStringTrait > > ( item ) ) )
					return true;
		}
		else
		{
			if( isItemInRange( item ) )
			{
				QToolTip::showText( pos, itemType( item->type() ) );

				return true;
			}
		}
		
		return false;
	};
	
	if( d->mdDoc.get() )
	{
		QString fileName;
		
		auto it = d->mdDoc->items().cbegin(), last = d->mdDoc->items().cend();

		for( ; it != last; ++it )
		{
			if( (*it)->type() == MD::ItemType::Anchor )
				fileName = static_cast< MD::Anchor< MD::QStringTrait >* > ( it->get() )->label();
			else if( d->editor->docName() == fileName )
			{
				if( checkItem( *it ) )
					break;
			}
		}
		
		if( it == last )
		{
			for( auto it = d->mdDoc->footnotesMap().cbegin(), last = d->mdDoc->footnotesMap().cend();
				it != last; ++it )
			{
				if( it->first.endsWith( d->editor->docName() ) && checkItem( it->second ) )
					break;
			}
		}
	}
}

void
MainWindow::onFind( bool )
{
	if( !d->find->isVisible() )
		d->find->show();

	if( !d->editor->textCursor().selection().isEmpty() )
		d->find->setFindText( d->editor->textCursor().selection().toPlainText() );
	else
		d->find->setFocusOnFind();
}

void
MainWindow::onFindWeb( bool )
{
	if( !d->findWeb->isVisible() )
		d->findWeb->show();

	d->findWeb->setFocusOnFindWeb();
}

void
MainWindow::onGoToLine( bool )
{
	if( !d->gotoline->isVisible() )
		d->gotoline->show();

	d->gotoline->setFocusOnLine();
}

void
MainWindow::onChooseFont()
{
	FontDlg dlg( d->editor->font(), this );

	if( dlg.exec() == QDialog::Accepted )
	{
		d->editor->applyFont( dlg.font() );

		saveCfg();
	}
}

static const QString c_appCfgFileName = QStringLiteral( "md-editor.cfg" );
static const QString c_appCfgFolderName = QStringLiteral( "Markdown" );

QString
MainWindow::configFileName( bool inPlace ) const
{
	const auto folders = QStandardPaths::standardLocations( QStandardPaths::ConfigLocation );
	
	if( !folders.isEmpty() && !inPlace )
		return folders.front() + QDir::separator() + c_appCfgFolderName + QDir::separator() +
			c_appCfgFileName;
	else
		return QApplication::applicationDirPath() + QDir::separator() + c_appCfgFileName;
}

void
MainWindow::saveCfg() const
{
	auto fileName = configFileName( false );
	
	const QDir dir( "./" );
	
	if( !dir.mkpath( QFileInfo( fileName ).absolutePath() ) )
		fileName = configFileName( true );
	
	QFile file( fileName );

	if( file.open( QIODevice::WriteOnly ) )
	{
		try {
			const auto f = d->editor->font();

			Cfg cfg;
			cfg.set_font( f.family() );
			cfg.set_fontSize( f.pointSize() );
			cfg.set_useColors( d->mdColors.enabled );
			cfg.set_linkColor( d->mdColors.linkColor.name( QColor::HexRgb ) );
			cfg.set_listColor( d->mdColors.listColor.name( QColor::HexRgb ) );
			cfg.set_textColor( d->mdColors.textColor.name( QColor::HexRgb ) );
			cfg.set_inlineColor( d->mdColors.inlineColor.name( QColor::HexRgb ) );
			cfg.set_htmlColor( d->mdColors.htmlColor.name( QColor::HexRgb ) );
			cfg.set_tableColor( d->mdColors.tableColor.name( QColor::HexRgb ) );
			cfg.set_blockquoteColor( d->mdColors.blockquoteColor.name( QColor::HexRgb ) );
			cfg.set_codeColor( d->mdColors.codeColor.name( QColor::HexRgb ) );
			cfg.set_headingColor( d->mdColors.headingColor.name( QColor::HexRgb ) );
			cfg.set_mathColor( d->mdColors.mathColor.name( QColor::HexRgb ) );
			cfg.set_footnoteColor( d->mdColors.footnoteColor.name( QColor::HexRgb ) );
			cfg.set_specialColor( d->mdColors.specialColor.name( QColor::HexRgb ) );

			tag_Cfg< cfgfile::qstring_trait_t > tag( cfg );

			QTextStream stream( &file );

			cfgfile::write_cfgfile( tag, stream );

			file.close();
		}
		catch( const cfgfile::exception_t< cfgfile::qstring_trait_t > & )
		{
			file.close();
		}
	}
}

void
MainWindow::readCfg()
{
	auto fileName = configFileName( false );
	
	if( !QFileInfo::exists( fileName ) )
		fileName = configFileName( true );
	
	QFile file( fileName );

	if( file.open( QIODevice::ReadOnly ) )
	{
		try {
			tag_Cfg< cfgfile::qstring_trait_t > tag;

			QTextStream stream( &file );

			cfgfile::read_cfgfile( tag, stream, c_appCfgFileName );

			file.close();

			const auto cfg = tag.get_cfg();

			if( !cfg.font().isEmpty() && cfg.fontSize() != -1 )
			{
				const QFont f( cfg.font(), cfg.fontSize() );

				d->editor->applyFont( f );
			}

			if( !cfg.linkColor().isEmpty() )
				d->mdColors.linkColor = QColor( cfg.linkColor() );

			if( !cfg.listColor().isEmpty() )
				d->mdColors.listColor = QColor( cfg.listColor() );

			if( !cfg.textColor().isEmpty() )
				d->mdColors.textColor = QColor( cfg.textColor() );

			if( !cfg.inlineColor().isEmpty() )
				d->mdColors.inlineColor = QColor( cfg.inlineColor() );

			if( !cfg.htmlColor().isEmpty() )
				d->mdColors.htmlColor = QColor( cfg.htmlColor() );

			if( !cfg.tableColor().isEmpty() )
				d->mdColors.tableColor = QColor( cfg.tableColor() );

			if( !cfg.blockquoteColor().isEmpty() )
				d->mdColors.blockquoteColor = QColor( cfg.blockquoteColor() );

			if( !cfg.codeColor().isEmpty() )
				d->mdColors.codeColor = QColor( cfg.codeColor() );

			if( !cfg.headingColor().isEmpty() )
				d->mdColors.headingColor = QColor( cfg.headingColor() );

			if( !cfg.mathColor().isEmpty() )
				d->mdColors.mathColor = QColor( cfg.mathColor() );

			if( !cfg.footnoteColor().isEmpty() )
				d->mdColors.footnoteColor = QColor( cfg.footnoteColor() );
			
			if( !cfg.specialColor().isEmpty() )
				d->mdColors.specialColor = QColor( cfg.specialColor() );

			d->mdColors.enabled = cfg.useColors();
		}
		catch( const cfgfile::exception_t< cfgfile::qstring_trait_t > & )
		{
			file.close();
		}
	}
}

void
MainWindow::onLessFontSize()
{
	auto f = d->editor->font();

	if( f.pointSize() > 5 )
	{
		f.setPointSize( f.pointSize() - 1 );

		d->editor->applyFont( f );

		saveCfg();
	}
}

void
MainWindow::onMoreFontSize()
{
	auto f = d->editor->font();

	if( f.pointSize() < 66 )
	{
		f.setPointSize( f.pointSize() + 1 );

		d->editor->applyFont( f );

		saveCfg();
	}
}

void
MainWindow::onCursorPositionChanged()
{
	if( d->standardEditMenu )
	{
		d->standardEditMenu->deleteLater();
		d->standardEditMenu = nullptr;
	}

	d->standardEditMenu = d->editor->createStandardContextMenu(
		d->editor->cursorRect().center() );

	d->standardEditMenu->addSeparator();

	d->standardEditMenu->addAction( d->toggleFindAction );
	d->standardEditMenu->addAction( d->toggleGoToLineAction );
	d->standardEditMenu->addSeparator();
	d->standardEditMenu->addAction( d->toggleFindWebAction );
	d->standardEditMenu->addSeparator();
	d->standardEditMenu->addAction( d->addTOCAction );

	d->editMenuAction->setMenu( d->standardEditMenu );

	connect( d->standardEditMenu, &QMenu::triggered,
		this, &MainWindow::onEditMenuActionTriggered );

	const auto c = d->editor->textCursor();
	
	d->tabAction->setEnabled( c.hasSelection() );
	d->backtabAction->setEnabled( c.hasSelection() );

	d->cursorPosLabel->setText( tr( "Line: %1, Col: %2" )
		.arg( c.block().blockNumber() + 1 ).arg( c.columnNumber() + 1 ) );
}

void
MainWindow::onEditMenuActionTriggered( QAction * action )
{
	if( action != d->toggleFindAction && action != d->toggleGoToLineAction &&
		action != d->toggleFindWebAction )
			d->editor->setFocus();
}

namespace {

struct Node {
	QVector< QString > keys;
	QVector< QPair< QSharedPointer< Node >, QTreeWidgetItem* > > children;
	QTreeWidgetItem * self = nullptr;
};

}

void
MainWindow::loadAllLinkedFiles( bool doNotCloseDock )
{
	if( isModified() && !d->previewMode )
	{
		QMessageBox::information( this, windowTitle(),
			tr( "You have unsaved changes. Please save document first." ) );

		d->editor->setFocus();

		return;
	}

	if( !doNotCloseDock && d->fileTreeDock )
	{
		closeAllLinkedFiles();

		updateLoadAllLinkedFilesMenuText();

		onTextChanged();

		return;
	}

	d->loadAllFlag = true;

	readAllLinked();

	bool init = false;

	if( !d->fileTreeDock )
	{
		init = true;

		d->fileTreeDock = new QDockWidget( tr( "Navigation" ), this );
		d->fileTreeDock->setFeatures( QDockWidget::NoDockWidgetFeatures );
	}

	if( !d->fileTree )
	{
		d->fileTree = new QTreeWidget( d->fileTreeDock );
		d->fileTreeDock->setWidget( d->fileTree );
		d->fileTree->setHeaderHidden( true );
	}

	if( init )
	{
		const auto rootFolder = QFileInfo( d->rootFilePath ).absolutePath() + QStringLiteral( "/" );

		Node root;

		for( auto it = d->mdDoc->items().cbegin(), last = d->mdDoc->items().cend(); it != last; ++it )
		{
			if( (*it)->type() == MD::ItemType::Anchor )
			{
				const auto fullFileName =
					static_cast< MD::Anchor< MD::QStringTrait >* > ( it->get() )->label();

				const auto fileName = fullFileName.startsWith( rootFolder ) ?
					fullFileName.sliced( rootFolder.size() ) : fullFileName;

				const auto parts = fileName.split( QStringLiteral( "/" ) );

				Node * current = &root;

				for( qsizetype i = 0; i < parts.size(); ++i )
				{
					const QString f = parts.at( i ).isEmpty() ? QStringLiteral( "/" ) : parts.at( i );

					if( i == parts.size() - 1 )
					{
						if( !current->keys.contains( f ) )
						{
							auto tmp = QSharedPointer< Node >::create();
							auto item = new QTreeWidgetItem( current->self );
							item->setIcon( 0, QIcon( ":/res/img/icon_16x16.png" ) );
							item->setData( 0, Qt::UserRole, fullFileName );
							tmp->self = item;
							item->setText( 0, f );
							current->children.push_back( { tmp, item } );
							current->keys.push_back( f );
							current = tmp.get();
						}
					}
					else
					{
						if( !current->keys.contains( f ) )
						{
							auto tmp = QSharedPointer< Node >::create();
							auto item = new QTreeWidgetItem( current->self );
							item->setIcon( 0, QIcon( ":/res/img/folder-yellow.png" ) );
							tmp->self = item;
							item->setText( 0, f );
							current->children.push_back( { tmp, item } );
							current->keys.push_back( f );
							current = tmp.get();
						}
						else
							current = current->children.at( current->keys.indexOf( f ) ).first.get();
					}
				}
			}
		}

		if( root.children.size() > 1 )
		{
			for( auto it = root.children.cbegin(), last = root.children.cend(); it != last; ++it )
				d->fileTree->addTopLevelItem( it->second );

			connect( d->fileTree, &QTreeWidget::itemDoubleClicked,
				this, &MainWindow::onNavigationDoubleClicked );

			if( !d->previewMode )
			{
				addDockWidget( Qt::LeftDockWidgetArea, d->fileTreeDock );

				QMessageBox::information( this, windowTitle(),
					tr( "HTML preview is ready. Modifications in files will not update "
						"HTML preview till you save changes." ) );
			}
		}
		else
		{
			closeAllLinkedFiles();

			QMessageBox::information( this, windowTitle(),
				tr( "This document doesn't have linked documents." ) );
		}
	}

	if( !d->previewMode )
		d->editor->setFocus();

	updateLoadAllLinkedFilesMenuText();
}

void
MainWindow::closeAllLinkedFiles()
{
	d->loadAllFlag = false;

	if( d->fileTreeDock )
	{
		removeDockWidget( d->fileTreeDock );
		d->fileTreeDock->deleteLater();
	}

	d->fileTree = nullptr;
	d->fileTreeDock = nullptr;

	d->editor->setFocus();

	onTextChanged();
}

void
MainWindow::readAllLinked()
{
	if( d->loadAllFlag )
	{
		MD::Parser< MD::QStringTrait > parser;

		d->mdDoc = parser.parse( d->rootFilePath, true,
			{ QStringLiteral( "md" ), QStringLiteral( "mkd" ), QStringLiteral( "markdown" ) } );

		d->html->setText( MD::toHtml( d->mdDoc, false,
			QStringLiteral( "qrc:/res/img/go-jump.png" ) ) );
	}
}

void
MainWindow::onNavigationDoubleClicked( QTreeWidgetItem * item, int )
{
	const auto path = item->data( 0, Qt::UserRole ).toString();

	if( !path.isEmpty() )
	{
		if( isModified() )
		{
			QMessageBox::information( this, windowTitle(),
				tr( "You have unsaved changes. Please save document first." ) );

			d->editor->setFocus();

			return;
		}

		QFile f( path );
		if( !f.open( QIODevice::ReadOnly ) )
		{
			QMessageBox::warning( this, windowTitle(),
				tr( "Could not open file %1: %2" ).arg(
					QDir::toNativeSeparators( path ), f.errorString() ) );
			return;
		}

		d->editor->setDocName( path );
		d->editor->setText( f.readAll() );
		f.close();

		updateWindowTitle();

		d->editor->document()->clearUndoRedoStacks();
		d->editor->setFocus();

		onCursorPositionChanged();
	}
}

void
MainWindow::updateWindowTitle()
{
	setWindowTitle( tr( "%1[*] - Markdown Editor%2" )
		.arg( QFileInfo( d->editor->docName() ).fileName(),
			d->previewMode ? tr( " [Preview Mode]" ) : QString() ) );
}

void
MainWindow::updateLoadAllLinkedFilesMenuText()
{
	if( d->loadAllFlag )
	{
		d->loadAllAction->setText( tr( "Show Only Current File..." ) );
		d->addTOCAction->setEnabled( false );
	}
	else
	{
		d->loadAllAction->setText( tr( "Load All Linked Files..." ) );
		d->addTOCAction->setEnabled( true );
	}
}

void
MainWindow::onTogglePreviewAction( bool checked )
{
	d->previewMode = checked;

	if( d->loadAllFlag )
		loadAllLinkedFiles( true );
	else
		onTextChanged();

	if( checked )
	{
		d->settingsMenu->menuAction()->setVisible( false );
		d->editMenuAction->setVisible( false );
		d->saveAction->setVisible( false );
		d->saveAction->setEnabled( false );
		d->saveAsAction->setVisible( false );
		d->saveAsAction->setEnabled( false );
		d->toggleFindAction->setEnabled( false );
		d->toggleGoToLineAction->setEnabled( false );
		d->newAction->setVisible( false );
		d->newAction->setEnabled( false );
		d->editor->setVisible( false );
		d->splitter->handle( 1 )->setEnabled( false );
		d->splitter->handle( 1 )->setVisible( false );
		d->splitter->setSizes( { 0, centralWidget()->width() } );

		if( d->fileTreeDock )
			removeDockWidget( d->fileTreeDock );
	}
	else
	{
		d->settingsMenu->menuAction()->setVisible( true );
		d->editMenuAction->setVisible( true );
		d->saveAction->setVisible( true );
		d->saveAction->setEnabled( true );
		d->saveAsAction->setVisible( true );
		d->saveAsAction->setEnabled( true );
		d->toggleFindAction->setEnabled( true );
		d->toggleGoToLineAction->setEnabled( true );
		d->newAction->setVisible( true );
		d->newAction->setEnabled( true );
		d->editor->setVisible( true );
		d->splitter->handle( 1 )->setEnabled( true );
		d->splitter->handle( 1 )->setVisible( true );
		const auto w = centralWidget()->width() / 2;
		d->splitter->setSizes( { w, w } );

		if( d->fileTreeDock )
		{
			addDockWidget( Qt::LeftDockWidgetArea, d->fileTreeDock );
			d->fileTreeDock->show();
		}

		d->editor->setFocus();
	}

	updateLoadAllLinkedFilesMenuText();

	updateWindowTitle();
}

namespace /* anonymous */ {

inline QString
paragraphToMD( MD::Paragraph< MD::QStringTrait > * p, QPlainTextEdit * editor )
{
	QTextCursor c = editor->textCursor();

	c.movePosition( QTextCursor::Start );

	for( long long int i = 0; i < p->startLine(); ++i )
		c.movePosition( QTextCursor::NextBlock );

	for( long long int i = 0; i < p->startColumn(); ++i )
		c.movePosition( QTextCursor::Right );

	for( long long int i = p->startLine(); i < p->endLine(); ++i )
		c.movePosition( QTextCursor::NextBlock, QTextCursor::KeepAnchor );

	for( long long int i = ( p->startLine() == p->endLine() ? p->startColumn() : 0 );
		i <= p->endColumn(); ++i )
			c.movePosition( QTextCursor::Right, QTextCursor::KeepAnchor );

	auto res = c.selectedText();
	res.replace( QChar( '\n' ), QChar( ' ' ) );
	res.replace( QChar( 0x2029 ), QChar( ' ' ) );

	return res;
}

inline QString
simplifyLabel( const QString & label, const QString & fileName )
{
	return label.sliced( 0, label.lastIndexOf( fileName ) - 1 );
}

} /* namespace anonymous */

void
MainWindow::onAddTOC()
{
	QString toc;
	int lvl = 0;
	int offset = 0;
	QString fileName;

	for( auto it = d->mdDoc->items().cbegin(), last = d->mdDoc->items().cend();
		it != last; ++it )
	{
		if( (*it)->type() == MD::ItemType::Anchor )
		{
			auto a = static_cast< MD::Anchor< MD::QStringTrait > * > ( it->get() );
			fileName = a->label();
		}
		else if( (*it)->type() == MD::ItemType::Heading )
		{
			auto h = static_cast< MD::Heading< MD::QStringTrait > * > ( it->get() );

			if( lvl )
			{
				if( lvl < h->level() )
					offset += 2;
				else if( lvl > h->level() )
				{
					while( lvl > h->level() && offset >= 2 )
					{
						--lvl;
						offset -= 2;
					}
				}
			}

			lvl = h->level();

			toc.append( QString( offset, QChar( ' ' ) ) );
			toc.append( QStringLiteral( "* [" ) );
			toc.append( paragraphToMD( h->text().get(), d->editor ) );
			toc.append( QStringLiteral( "](" ) );
			toc.append( simplifyLabel( h->label(), fileName ) );
			toc.append( QStringLiteral( ")\n" ) );
		}
	}

	d->editor->insertPlainText( toc );
}

void
MainWindow::onShowLicenses()
{
	MdShared::LicenseDialog msg( this );
	msg.addLicense( QStringLiteral( "The Oxygen Icon Theme" ),
		QStringLiteral( "<p><b>The Oxygen Icon Theme</b>\n\n</p>"
		"<p>Copyright (C) 2007 Nuno Pinheiro &lt;nuno@oxygen-icons.org&gt;\n</p>"
		"<p>Copyright (C) 2007 David Vignoni &lt;david@icon-king.com&gt;\n</p>"
		"<p>Copyright (C) 2007 David Miller &lt;miller@oxygen-icons.org&gt;\n</p>"
		"<p>Copyright (C) 2007 Johann Ollivier Lapeyre &lt;johann@oxygen-icons.org&gt;\n</p>"
		"<p>Copyright (C) 2007 Kenneth Wimer &lt;kwwii@bootsplash.org&gt;\n</p>"
		"<p>Copyright (C) 2007 Riccardo Iaconelli &lt;riccardo@oxygen-icons.org&gt;\n</p>"
		"<p>\nand others\n</p>"
		"\n"
		"<p>This library is free software; you can redistribute it and/or "
		"modify it under the terms of the GNU Lesser General Public "
		"License as published by the Free Software Foundation; either "
		"version 3 of the License, or (at your option) any later version.\n</p>"
		"\n"
		"<p>This library is distributed in the hope that it will be useful, "
		"but WITHOUT ANY WARRANTY; without even the implied warranty of "
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU "
		"Lesser General Public License for more details.\n</p>"
		"\n"
		"<p>You should have received a copy of the GNU Lesser General Public "
		"License along with this library. If not, see "
		"<a href=\"http://www.gnu.org/licenses/\">&lt;http://www.gnu.org/licenses/&gt;</a>.\n</p>"
		"\n"
		"<p>Clarification:\n</p>"
		"\n"
		"<p>The GNU Lesser General Public License or LGPL is written for "
		"software libraries in the first place. We expressly want the LGPL to "
		"be valid for this artwork library too.\n</p>"
		"\n"
		"<p>KDE Oxygen theme icons is a special kind of software library, it is an "
		"artwork library, it's elements can be used in a Graphical User Interface, or "
		"GUI.\n</p>"
		"\n"
		"<p>Source code, for this library means:\n</p>"
		"<p><ul> <li>where they exist, SVG;\n</li>"
		" <li>otherwise, if applicable, the multi-layered formats xcf or psd, or "
		"otherwise png.\n</li></ul></p>"
		"\n"
		"<p>The LGPL in some sections obliges you to make the files carry "
		"notices. With images this is in some cases impossible or hardly useful.\n</p>"
		"\n"
		"<p>With this library a notice is placed at a prominent place in the directory "
		"containing the elements. You may follow this practice.\n</p>"
		"\n"
		"<p>The exception in section 5 of the GNU Lesser General Public License covers "
		"the use of elements of this art library in a GUI.\n</p>"
		"\n"
		"<p>kde-artists [at] kde.org\n</p>"
		"\n"
		"<p><b>GNU LESSER GENERAL PUBLIC LICENSE</b>\n</p>"
		"<p>Version 3, 29 June 2007\n</p>"
		"\n"
		"<p>Copyright (C) 2007 Free Software Foundation, Inc. <a href=\"http://fsf.org/\">&lt;http://fsf.org/&gt;</a> "
		"Everyone is permitted to copy and distribute verbatim copies "
		"of this license document, but changing it is not allowed.\n</p>"
		"\n"
		"\n"
		"<p>This version of the GNU Lesser General Public License incorporates "
		"the terms and conditions of version 3 of the GNU General Public "
		"License, supplemented by the additional permissions listed below.\n</p>"
		"\n"
		"<p><b>0.</b> Additional Definitions.\n</p>"
		"\n"
		"<p>As used herein, \"this License\" refers to version 3 of the GNU Lesser "
		"General Public License, and the \"GNU GPL\" refers to version 3 of the GNU "
		"General Public License.\n</p>"
		"\n"
		"<p>\"The Library\" refers to a covered work governed by this License, "
		"other than an Application or a Combined Work as defined below.\n</p>"
		"\n"
		"<p>An \"Application\" is any work that makes use of an interface provided "
		"by the Library, but which is not otherwise based on the Library. "
		"Defining a subclass of a class defined by the Library is deemed a mode "
		"of using an interface provided by the Library.\n</p>"
		"\n"
		"<p>A \"Combined Work\" is a work produced by combining or linking an "
		"Application with the Library.  The particular version of the Library "
		"with which the Combined Work was made is also called the \"Linked "
		"Version\".\n</p>"
		"\n"
		"<p>The \"Minimal Corresponding Source\" for a Combined Work means the "
		"Corresponding Source for the Combined Work, excluding any source code "
		"for portions of the Combined Work that, considered in isolation, are "
		"based on the Application, and not on the Linked Version.\n</p>"
		"\n"
		"<p>The \"Corresponding Application Code\" for a Combined Work means the "
		"object code and/or source code for the Application, including any data "
		"and utility programs needed for reproducing the Combined Work from the "
		"Application, but excluding the System Libraries of the Combined Work.\n</p>"
		"\n"
		"<p><b>1.</b> Exception to Section 3 of the GNU GPL.\n</p>"
		"\n"
		"<p>You may convey a covered work under sections 3 and 4 of this License "
		"without being bound by section 3 of the GNU GPL.\n</p>"
		"\n"
		"<p><b>2.</b> Conveying Modified Versions.\n</p>"
		"\n"
		"<p>If you modify a copy of the Library, and, in your modifications, a "
		"facility refers to a function or data to be supplied by an Application "
		"that uses the facility (other than as an argument passed when the "
		"facility is invoked), then you may convey a copy of the modified "
		"version:\n</p>"
		"\n"
		"<p><b>a)</b> under this License, provided that you make a good faith effort to "
		"ensure that, in the event an Application does not supply the "
		"function or data, the facility still operates, and performs "
		"whatever part of its purpose remains meaningful, or\n</p>"
		"\n"
		"<p><b>b)</b> under the GNU GPL, with none of the additional permissions of "
		"this License applicable to that copy.\n</p>"
		"\n"
		"<p><b>3.</b> Object Code Incorporating Material from Library Header Files.\n</p>"
		"\n"
		"<p>The object code form of an Application may incorporate material from "
		"a header file that is part of the Library.  You may convey such object "
		"code under terms of your choice, provided that, if the incorporated "
		"material is not limited to numerical parameters, data structure "
		"layouts and accessors, or small macros, inline functions and templates "
		"(ten or fewer lines in length), you do both of the following:\n</p>"
		"\n"
		"<p><b>a)</b> Give prominent notice with each copy of the object code that the "
		"Library is used in it and that the Library and its use are "
		"covered by this License.\n</p>"
		"\n"
		"<p><b>b)</b> Accompany the object code with a copy of the GNU GPL and this license "
		"document.\n</p>"
		"\n"
		"<p><b>4.</b> Combined Works.\n</p>"
		"\n"
		"<p>You may convey a Combined Work under terms of your choice that, "
		"taken together, effectively do not restrict modification of the "
		"portions of the Library contained in the Combined Work and reverse "
		"engineering for debugging such modifications, if you also do each of "
		"the following:\n</p>"
		"\n"
		"<p><b>a)</b> Give prominent notice with each copy of the Combined Work that "
		"the Library is used in it and that the Library and its use are "
		"covered by this License.\n</p>"
		"\n"
		"<p><b>b)</b> Accompany the Combined Work with a copy of the GNU GPL and this license "
		"document.\n</p>"
		"\n"
		"<p><b>c)</b> For a Combined Work that displays copyright notices during "
		"execution, include the copyright notice for the Library among "
		"these notices, as well as a reference directing the user to the "
		"copies of the GNU GPL and this license document.\n</p>"
		"\n"
		"<p><b>d)</b> Do one of the following:\n</p>"
		"\n"
		"<p>    <b>0)</b> Convey the Minimal Corresponding Source under the terms of this "
		"License, and the Corresponding Application Code in a form "
		"suitable for, and under terms that permit, the user to "
		"recombine or relink the Application with a modified version of "
		"the Linked Version to produce a modified Combined Work, in the "
		"manner specified by section 6 of the GNU GPL for conveying "
		"Corresponding Source.\n</p>"
		"\n"
		"<p>    <b>1)</b> Use a suitable shared library mechanism for linking with the "
		"Library.  A suitable mechanism is one that (a) uses at run time "
		"a copy of the Library already present on the user's computer "
		"system, and (b) will operate properly with a modified version "
		"of the Library that is interface-compatible with the Linked "
		"Version.\n</p>"
		"\n"
		"<p><b>e)</b> Provide Installation Information, but only if you would otherwise "
		"be required to provide such information under section 6 of the "
		"GNU GPL, and only to the extent that such information is "
		"necessary to install and execute a modified version of the "
		"Combined Work produced by recombining or relinking the "
		"Application with a modified version of the Linked Version. (If "
		"you use option 4d0, the Installation Information must accompany "
		"the Minimal Corresponding Source and Corresponding Application "
		"Code. If you use option 4d1, you must provide the Installation "
		"Information in the manner specified by section 6 of the GNU GPL "
		"for conveying Corresponding Source.)\n</p>"
		"\n"
		"<p><b>5.</b> Combined Libraries.\n</p>"
		"\n"
		"<p>You may place library facilities that are a work based on the "
		"Library side by side in a single library together with other library "
		"facilities that are not Applications and are not covered by this "
		"License, and convey such a combined library under terms of your "
		"choice, if you do both of the following:\n</p>"
		"\n"
		"<p><b>a)</b> Accompany the combined library with a copy of the same work based "
		"on the Library, uncombined with any other library facilities, "
		"conveyed under the terms of this License.\n</p>"
		"\n"
		"<p><b>b)</b> Give prominent notice with the combined library that part of it "
		"is a work based on the Library, and explaining where to find the "
		"accompanying uncombined form of the same work.\n</p>"
		"\n"
		"<p><b>6.</b> Revised Versions of the GNU Lesser General Public License.\n</p>"
		"\n"
		"<p>The Free Software Foundation may publish revised and/or new versions "
		"of the GNU Lesser General Public License from time to time. Such new "
		"versions will be similar in spirit to the present version, but may "
		"differ in detail to address new problems or concerns.\n</p>"
		"\n"
		"<p>Each version is given a distinguishing version number. If the "
		"Library as you received it specifies that a certain numbered version "
		"of the GNU Lesser General Public License \"or any later version\" "
		"applies to it, you have the option of following the terms and "
		"conditions either of that published version or of any later version "
		"published by the Free Software Foundation. If the Library as you "
		"received it does not specify a version number of the GNU Lesser "
		"General Public License, you may choose any version of the GNU Lesser "
		"General Public License ever published by the Free Software Foundation.\n</p>"
		"\n"
		"<p>If the Library as you received it specifies that a proxy can decide "
		"whether future versions of the GNU Lesser General Public License shall "
		"apply, that proxy's public statement of acceptance of any version is "
		"permanent authorization for you to choose that version for the "
		"Library.</p>" ) );

	msg.addLicense( QStringLiteral( "KaTeX" ),
		QStringLiteral( "<p><b>KaTeX</b></p>\n\n"
			"<p>The MIT License (MIT)</p>\n"
			"\n"
			"<p>Copyright (c) 2013-2020 Khan Academy and other contributors</p>\n"
			"\n"
			"<p>Permission is hereby granted, free of charge, to any person obtaining a copy "
			"of this software and associated documentation files (the \"Software\"), to deal "
			"in the Software without restriction, including without limitation the rights "
			"to use, copy, modify, merge, publish, distribute, sublicense, and/or sell "
			"copies of the Software, and to permit persons to whom the Software is "
			"furnished to do so, subject to the following conditions:</p>\n"
			"\n"
			"<p>The above copyright notice and this permission notice shall be included in all "
			"copies or substantial portions of the Software.</p>\n"
			"\n"
			"<p>THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR "
			"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, "
			"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE "
			"AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER "
			"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, "
			"OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE "
			"SOFTWARE.</p>" ) );

	msg.addLicense( QStringLiteral( "github-markdown-css" ),
		QStringLiteral( "<p><b>github-markdown-css</b></p>\n\n"
			"<p>The MIT License (MIT)</p>\n"
			"\n"
			"<p>Copyright (c) Sindre Sorhus "
			"<a href=\"mailto:sindresorhus@gmail.com\">&lt;sindresorhus@gmail.com&gt;</a> "
			"<a href=\"https://sindresorhus.com\">(https://sindresorhus.com)</a></p>\n"
			"\n"
			"<p>Permission is hereby granted, free of charge, to any person obtaining a copy "
			"of this software and associated documentation files (the \"Software\"), to deal "
			"in the Software without restriction, including without limitation the rights "
			"to use, copy, modify, merge, publish, distribute, sublicense, and/or sell "
			"copies of the Software, and to permit persons to whom the Software is "
			"furnished to do so, subject to the following conditions:</p>\n"
			"\n"
			"<p>The above copyright notice and this permission notice shall be included in all "
			"copies or substantial portions of the Software.</p>\n"
			"\n"
			"<p>THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR "
			"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, "
			"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE "
			"AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER "
			"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, "
			"OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE "
			"SOFTWARE.</p>" ) );

	msg.addLicense( QStringLiteral( "Highlight.js" ),
		QStringLiteral( "<p><b>Highlight.js</b>\n\n</p>"
			"<p>BSD 3-Clause License</p>\n"
			"\n"
			"<p>Copyright (c) 2006, Ivan Sagalaev.\n"
			"All rights reserved.</p>\n"
			"\n"
			"<p>Redistribution and use in source and binary forms, with or without "
			"modification, are permitted provided that the following conditions are met:</p>\n"
			"\n"
			"<ul><li>Redistributions of source code must retain the above copyright notice, this "
			"list of conditions and the following disclaimer.</li>\n"
			"\n"
			"<li>Redistributions in binary form must reproduce the above copyright notice, "
			"this list of conditions and the following disclaimer in the documentation "
			"and/or other materials provided with the distribution.</li>\n"
			"\n"
			"<li>Neither the name of the copyright holder nor the names of its "
			"contributors may be used to endorse or promote products derived from "
			"this software without specific prior written permission.</li></ul>\n"
			"\n"
			"<p>THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\" "
			"AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE "
			"IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE "
			"DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE "
			"FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL "
			"DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR "
			"SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER "
			"CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, "
			"OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE "
			"OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.</p>" ) );

	msg.addLicense( QStringLiteral( "js-emoji" ),
		QStringLiteral( "<p><b>js-emoji</b>\n\n</p>"
			"<p>The MIT License (MIT)</p>\n"
			"\n"
			"<p>Copyright (c) 2015 Cal Henderson</p>\n"
			"\n"
			"<p>Permission is hereby granted, free of charge, to any person obtaining a copy "
			"of this software and associated documentation files (the \"Software\"), to deal "
			"in the Software without restriction, including without limitation the rights "
			"to use, copy, modify, merge, publish, distribute, sublicense, and/or sell "
			"copies of the Software, and to permit persons to whom the Software is "
			"furnished to do so, subject to the following conditions:</p>\n"
			"\n"
			"<p>The above copyright notice and this permission notice shall be included in all "
			"copies or substantial portions of the Software.</p>\n"
			"\n"
			"<p>THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR "
			"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, "
			"FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE "
			"AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER "
			"LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, "
			"OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE "
			"SOFTWARE.</p>" ) );
	
	msg.addLicense( QStringLiteral( "hightlight-blockquote" ),
		QStringLiteral( "<p><b>hightlight-blockquote</b>\n\n</p>"
			"<p>The MIT License (MIT)</p>\n"
			"\n"
			"<p>Copyright © 2024 Ivan Stanevich</p>\n"
			"<p>Copyright © 2024 Igor Mironchik</p>\n"
			"\n"
			"<p>Permission is hereby granted, free of charge, to any person obtaining a copy of this "
			"software and associated documentation files (the \"Software\"), to deal in the Software "
			"without restriction, including without limitation the rights to use, copy, modify, merge, "
			"publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons "
			"to whom the Software is furnished to do so, subject to the following conditions:</p>\n"
			"\n"
			"<p>The above copyright notice and this permission notice shall be included in all copies or "
			"substantial portions of the Software.</p>\n"
			"\n"
			"<p>THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, "
			"INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR "
			"PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE "
			"FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, "
			"ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE "
			"SOFTWARE.</p>" ) );
	

	msg.exec();
}

void
MainWindow::onChangeColors()
{
	ColorsDialog dlg( d->mdColors, this );

	if( dlg.exec() == QDialog::Accepted )
	{
		if( d->mdColors != dlg.colors() )
		{
			d->mdColors = dlg.colors();

			d->editor->applyColors( d->mdColors );

			saveCfg();
		}
	}
}

} /* namespace MdEditor */
