
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

// md4qt include.
#define MD4QT_QT_SUPPORT
#include <md4qt/parser.hpp>

// md-pdf include.
#include "main_window.hpp"
#include "renderer.hpp"
#include "progress.hpp"
#include "const.hpp"
#include "version.hpp"
#include "cfg.hpp"

// Qt include.
#include <QToolButton>
#include <QColorDialog>
#include <QFileDialog>
#include <QPushButton>
#include <QMessageBox>
#include <QThread>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QApplication>
#include <QComboBox>
#include <QStandardPaths>
#include <QTextStream>

// podofo include.
#include <podofo/podofo.h>

// shared include.
#include <license_dialog.hpp>

// cfgfile include.
#include <cfgfile/all.hpp>


//
// MainWindow
//

MainWidget::MainWidget( QWidget * parent )
	:	QWidget( parent )
	,	m_ui( new Ui::MainWindow() )
	,	m_thread( new QThread( this ) )
	,	m_textFontOk( false )
	,	m_codeFontOk( false )
	,	m_syntax( new Syntax )
{
	m_ui->setupUi( this );

	m_ui->m_linkColor->setColor( QColor( 33, 122, 255 ) );
	m_ui->m_borderColor->setColor( QColor( 81, 81, 81 ) );

	connect( m_ui->m_linkColor, &ColorWidget::clicked, this, &MainWidget::changeLinkColor );
	connect( m_ui->m_borderColor, &ColorWidget::clicked, this, &MainWidget::changeBorderColor );
	connect( m_ui->m_fileNameBtn, &QToolButton::clicked,
		this, &MainWidget::selectMarkdown );
	connect( m_ui->m_startBtn, &QPushButton::clicked,
		this, &MainWidget::process );
	connect( m_ui->m_textFont, &QFontComboBox::currentFontChanged,
		this, &MainWidget::textFontChanged );
	connect( m_ui->m_codeFont, &QFontComboBox::currentFontChanged,
		this, &MainWidget::codeFontChanged );

	void (QSpinBox::*signal) ( int ) = &QSpinBox::valueChanged;

	connect( m_ui->m_codeFontSize, signal, this, &MainWidget::codeFontSizeChanged );
	connect( m_ui->m_textFontSize, signal, this, &MainWidget::textFontSizeChanged );

	connect( m_ui->m_mm, &QToolButton::toggled, this, &MainWidget::mmButtonToggled );

	m_ui->m_left->setMaximum( 50 );
	m_ui->m_right->setMaximum( 50 );
	m_ui->m_top->setMaximum( 50 );
	m_ui->m_bottom->setMaximum( 50 );

	adjustSize();

	m_thread->start();

	if( !PdfRenderer::isFontCreatable( m_ui->m_textFont->currentText() ) )
	{
		for( int i = 0; i < m_ui->m_textFont->count(); ++i )
		{
			if( PdfRenderer::isFontCreatable( m_ui->m_textFont->itemText( i ) ) )
			{
				m_ui->m_textFont->setCurrentIndex( i );

				break;
			}
		}
	}

	if( !PdfRenderer::isFontCreatable( m_ui->m_codeFont->currentText() ) )
	{
		for( int i = 0; i < m_ui->m_codeFont->count(); ++i )
		{
			if( PdfRenderer::isFontCreatable( m_ui->m_codeFont->itemText( i ) ) )
			{
				m_ui->m_codeFont->setCurrentIndex( i );

				break;
			}
		}
	}

	textFontChanged( m_ui->m_textFont->currentFont() );
	codeFontChanged( m_ui->m_codeFont->currentFont() );

	QStringList themeNames;
	const auto themes = m_syntax->repository().themes();

	for( const auto & t : themes )
		themeNames.push_back( t.name() );

	themeNames.sort( Qt::CaseInsensitive );

	m_ui->m_codeTheme->addItems( themeNames );
	m_ui->m_codeTheme->setCurrentText( QStringLiteral( "GitHub Light" ) );
	
	readCfg();
}

MainWidget::~MainWidget()
{
	m_thread->quit();
	m_thread->wait();
}

static const QString c_appCfgFileName = QStringLiteral( "md-pdf-gui.cfg" );
static const QString c_appCfgFolderName = QStringLiteral( "Markdown" );

QString
MainWidget::configFileName( bool inPlace ) const
{
	const auto folders = QStandardPaths::standardLocations( QStandardPaths::ConfigLocation );
	
	if( !folders.isEmpty() && !inPlace )
		return folders.front() + QDir::separator() + c_appCfgFolderName + QDir::separator() +
			c_appCfgFileName;
	else
		return QApplication::applicationDirPath() + QDir::separator() + c_appCfgFileName;
}

void
MainWidget::readCfg()
{
	auto fileName = configFileName( false );
	
	if( !QFileInfo::exists( fileName ) )
		fileName = configFileName( true );
	
	QFile file( fileName );

	if( file.open( QIODevice::ReadOnly ) )
	{
		try {
			MdPdf::tag_Cfg< cfgfile::qstring_trait_t > tag;

			QTextStream stream( &file );

			cfgfile::read_cfgfile( tag, stream, c_appCfgFileName );

			file.close();

			const auto cfg = tag.get_cfg();
			
			const auto & tFont = cfg.textFont();

			if( !tFont.name().isEmpty() )
			{
				auto fs = tFont.size();
				
				if( fs < 6 )
					fs = 6;
				
				if( fs > 16 )
					fs = 16;
				
				const QFont f( tFont.name(), fs );

				m_ui->m_textFont->setCurrentFont( f );
				m_ui->m_textFontSize->setValue( fs );
			}
			
			const auto & cFont = cfg.codeFont();
			
			if( !cFont.name().isEmpty() )
			{
				auto fs = cFont.size();
				
				if( fs < 5 )
					fs = 5;
				
				if( fs > 14 )
					fs = 14;
				
				const QFont f( cFont.name(), fs );

				m_ui->m_codeFont->setCurrentFont( f );
				m_ui->m_codeFontSize->setValue( fs );
			}
			
			const auto & mFont = cfg.mathFont();
			
			if( !mFont.name().isEmpty() )
			{
				auto fs = mFont.size();
				
				if( fs < 5 )
					fs = 5;
				
				if( fs > 14 )
					fs = 14;
				
				const QFont f( mFont.name(), fs );

				m_ui->m_mathFont->setCurrentFont( f );
				m_ui->m_mathFontSize->setValue( fs );
			}

			if( !cfg.linkColor().isEmpty() )
				m_ui->m_linkColor->setColor( QColor( cfg.linkColor() ) );

			if( !cfg.borderColor().isEmpty() )
				m_ui->m_borderColor->setColor( QColor( cfg.borderColor() ) );

			if( !cfg.codeTheme().isEmpty() )
				m_ui->m_codeTheme->setCurrentText( cfg.codeTheme() );
		
			m_ui->m_dpi->setValue( cfg.dpiForImages() );
			
			const auto & m = cfg.margins();
			
			if( m.units() == QStringLiteral( "mm" ) )
				m_ui->m_mm->setChecked( true );
			else
				m_ui->m_pt->setChecked( true );
			
			m_ui->m_left->setValue( m.left() );
			m_ui->m_right->setValue( m.right() );
			m_ui->m_top->setValue( m.top() );
			m_ui->m_bottom->setValue( m.bottom() );
		}
		catch( const cfgfile::exception_t< cfgfile::qstring_trait_t > & )
		{
			file.close();
		}
	}
}

void
MainWidget::saveCfg()
{
	auto fileName = configFileName( false );
	
	const QDir dir( "./" );
	
	if( !dir.mkpath( QFileInfo( fileName ).absolutePath() ) )
		fileName = configFileName( true );
	
	QFile file( fileName );

	if( file.open( QIODevice::WriteOnly ) )
	{
		try {
			MdPdf::Cfg cfg;
			
			{
				MdPdf::Font f;
				f.set_name( m_ui->m_textFont->currentFont().family() );
				f.set_size( m_ui->m_textFontSize->value() );
				cfg.set_textFont( f );
			}
			
			{
				MdPdf::Font f;
				f.set_name( m_ui->m_codeFont->currentFont().family() );
				f.set_size( m_ui->m_codeFontSize->value() );
				cfg.set_codeFont( f );
			}
			
			{
				MdPdf::Font f;
				f.set_name( m_ui->m_mathFont->currentFont().family() );
				f.set_size( m_ui->m_mathFontSize->value() );
				cfg.set_mathFont( f );
			}
			
			cfg.set_linkColor( m_ui->m_linkColor->color().name( QColor::HexRgb ) );
			cfg.set_borderColor( m_ui->m_borderColor->color().name( QColor::HexRgb ) );
			cfg.set_codeTheme( m_ui->m_codeTheme->currentText() );
			cfg.set_dpiForImages( m_ui->m_dpi->value() );
			
			MdPdf::Margins m;
			
			if( m_ui->m_mm->isChecked() )
				m.set_units( QStringLiteral( "mm" ) );
			else
				m.set_units( QStringLiteral( "pt" ) );
				
			m.set_left( m_ui->m_left->value() );
			m.set_right( m_ui->m_right->value() );
			m.set_top( m_ui->m_top->value() );
			m.set_bottom( m_ui->m_bottom->value() );
			
			cfg.set_margins( m );

			MdPdf::tag_Cfg< cfgfile::qstring_trait_t > tag( cfg );

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
MainWidget::setMarkdownFile( const QString & fileName )
{
	m_ui->m_fileName->setText( fileName );

	changeStateOfStartButton();
}

void
MainWidget::changeLinkColor()
{
	QColorDialog dlg( m_ui->m_linkColor->color(), this );

	if( QDialog::Accepted == dlg.exec() )
		m_ui->m_linkColor->setColor( dlg.currentColor() );
}

void
MainWidget::changeBorderColor()
{
	QColorDialog dlg( m_ui->m_borderColor->color(), this );

	if( QDialog::Accepted == dlg.exec() )
		m_ui->m_borderColor->setColor( dlg.currentColor() );
}

void
MainWidget::changeStateOfStartButton()
{
	if( m_textFontOk && m_codeFontOk )
		m_ui->m_startBtn->setEnabled( true );
	else
		m_ui->m_startBtn->setEnabled( false );
}

void
MainWidget::selectMarkdown()
{
	const auto folder = m_ui->m_fileName->text().isEmpty() ? QDir::homePath() :
		QFileInfo( m_ui->m_fileName->text() ).absolutePath();

	const auto fileName = QFileDialog::getOpenFileName( this, tr( "Select Markdown" ),
		folder, tr( "Markdown (*.md *.markdown)" ) );

	if( !fileName.isEmpty() )
	{
		m_ui->m_fileName->setText( fileName );

		changeStateOfStartButton();
	}
}

void
MainWidget::process()
{
	saveCfg();
	
	auto fileName = QFileDialog::getSaveFileName( this, tr( "Save as" ),
		QDir::homePath(),
		tr( "PDF (*.pdf)" ) );

	if( !fileName.isEmpty() )
	{
		if( !fileName.endsWith( QLatin1String( ".pdf" ), Qt::CaseInsensitive ) )
			fileName.append( QLatin1String( ".pdf" ) );

		MD::Parser< MD::QStringTrait > parser;

		auto doc = parser.parse( m_ui->m_fileName->text(), m_ui->m_recursive->isChecked() );

		if( !doc->isEmpty() )
		{
			auto * pdf = new PdfRenderer();
			pdf->moveToThread( m_thread );

			RenderOpts opts;

			opts.m_textFont = m_ui->m_textFont->currentFont().family();
			opts.m_textFontSize = m_ui->m_textFontSize->value();
			opts.m_codeFont = m_ui->m_codeFont->currentFont().family();
			opts.m_codeFontSize = m_ui->m_codeFontSize->value();
			opts.m_mathFont = m_ui->m_mathFont->currentFont().family();
			opts.m_mathFontSize = m_ui->m_mathFontSize->value();
			opts.m_linkColor = m_ui->m_linkColor->color();
			opts.m_borderColor = m_ui->m_borderColor->color();
			opts.m_left = ( m_ui->m_pt->isChecked() ? m_ui->m_left->value() :
				m_ui->m_left->value() / c_mmInPt );
			opts.m_right = ( m_ui->m_pt->isChecked() ? m_ui->m_right->value() :
				m_ui->m_right->value() / c_mmInPt );
			opts.m_top = ( m_ui->m_pt->isChecked() ? m_ui->m_top->value() :
				m_ui->m_top->value() / c_mmInPt );
			opts.m_bottom = ( m_ui->m_pt->isChecked() ? m_ui->m_bottom->value() :
				m_ui->m_bottom->value() / c_mmInPt );
			opts.m_dpi = m_ui->m_dpi->value();
			opts.m_syntax = m_syntax;
			m_syntax->setTheme( m_syntax->themeForName( m_ui->m_codeTheme->currentText() ) );


			ProgressDlg progress( pdf, this );

			pdf->render( fileName, doc, opts );

			if( progress.exec() == QDialog::Accepted )
				QMessageBox::information( this, tr( "Markdown processed..." ),
					tr( "PDF generated. Have a look at the result. Thank you." ) );
			else
			{
				if( !progress.errorMsg().isEmpty() )
					QMessageBox::critical( this, tr( "Error during rendering PDF..." ),
						tr( "%1\n\nOutput PDF is broken. Sorry." )
							.arg( progress.errorMsg() ) );
				else
					QMessageBox::information( this, tr( "Canceled..." ),
						tr( "PDF generation is canceled." ) );
			}
		}
		else
			QMessageBox::warning( this, tr( "Markdown is empty..." ),
				tr( "Input Markdown file is empty. Nothing saved." ) );
	}
}

void
MainWidget::codeFontSizeChanged( int i )
{
	if( i > m_ui->m_textFontSize->value() )
		m_ui->m_codeFontSize->setValue( m_ui->m_textFontSize->value() );
}

void
MainWidget::textFontSizeChanged( int i )
{
	if( i < m_ui->m_codeFontSize->value() )
		m_ui->m_codeFontSize->setValue( m_ui->m_textFontSize->value() );
}

void
MainWidget::mmButtonToggled( bool on )
{
	if( on )
	{
		m_ui->m_left->setValue( qRound( m_ui->m_left->value() * c_mmInPt ) );
		m_ui->m_right->setValue( qRound( m_ui->m_right->value() * c_mmInPt ) );
		m_ui->m_top->setValue( qRound( m_ui->m_top->value() * c_mmInPt ) );
		m_ui->m_bottom->setValue( qRound( m_ui->m_bottom->value() * c_mmInPt ) );

		m_ui->m_left->setMaximum( 50 );
		m_ui->m_right->setMaximum( 50 );
		m_ui->m_top->setMaximum( 50 );
		m_ui->m_bottom->setMaximum( 50 );
	}
	else
	{
		m_ui->m_left->setMaximum( qRound( 50 / c_mmInPt ) );
		m_ui->m_right->setMaximum( qRound( 50 / c_mmInPt ) );
		m_ui->m_top->setMaximum( qRound( 50 / c_mmInPt ) );
		m_ui->m_bottom->setMaximum( qRound( 50 / c_mmInPt ) );

		m_ui->m_left->setValue( qRound( m_ui->m_left->value() / c_mmInPt ) );
		m_ui->m_right->setValue( qRound( m_ui->m_right->value() / c_mmInPt ) );
		m_ui->m_top->setValue( qRound( m_ui->m_top->value() / c_mmInPt ) );
		m_ui->m_bottom->setValue( qRound( m_ui->m_bottom->value() / c_mmInPt ) );
	}
}

void
MainWidget::textFontChanged( const QFont & f )
{
	static const QString defaultColor = m_ui->m_textFont->palette().color( QPalette::Text ).name();

	if( !PdfRenderer::isFontCreatable( f.family() ) )
	{
		m_ui->m_textFont->setStyleSheet( QStringLiteral( "QFontComboBox { color: red }" ) );
		m_textFontOk = false;

		m_ui->m_startBtn->setEnabled( false );
	}
	else
	{
		m_ui->m_textFont->setStyleSheet( QStringLiteral( "QFontComboBox { color: %1 }" )
			.arg( defaultColor ) );
		m_textFontOk = true;

		if( !m_ui->m_fileName->text().isEmpty() && m_codeFontOk )
			m_ui->m_startBtn->setEnabled( true );
		else
			m_ui->m_startBtn->setEnabled( false );
	}
}

void
MainWidget::codeFontChanged( const QFont & f )
{
	static const QString defaultColor = m_ui->m_codeFont->palette().color( QPalette::Text ).name();

	if( !PdfRenderer::isFontCreatable( f.family() ) )
	{
		m_ui->m_codeFont->setStyleSheet( QStringLiteral( "QFontComboBox { color: red }" ) );
		m_codeFontOk = false;

		m_ui->m_startBtn->setEnabled( false );
	}
	else
	{
		m_ui->m_codeFont->setStyleSheet( QStringLiteral( "QFontComboBox { color: %1 }" )
			.arg( defaultColor ) );
		m_codeFontOk = true;

		if( !m_ui->m_fileName->text().isEmpty() && m_textFontOk )
			m_ui->m_startBtn->setEnabled( true );
		else
			m_ui->m_startBtn->setEnabled( false );
	}
}


//
// MainWindow
//

MainWindow::MainWindow()
{
	setWindowTitle( tr( "MD-PDF Converter" ) );

	auto file = menuBar()->addMenu( tr( "&File" ) );
	file->addAction( QIcon( QStringLiteral( ":/img/application-exit.png" ) ), tr( "&Quit" ),
		this, &MainWindow::quit );

	auto help = menuBar()->addMenu( tr( "&Help" ) );
	help->addAction( QIcon( QStringLiteral( ":/img/icon_24x24.png" ) ), tr( "About" ),
		this, &MainWindow::about );
	help->addAction( QIcon( QStringLiteral( ":/img/qt.png" ) ), tr( "About Qt" ),
		this, &MainWindow::aboutQt );
	help->addAction( QIcon( QStringLiteral( ":/img/bookmarks-organize.png" ) ),
		tr( "Licenses" ), this, &MainWindow::licenses );

	ui = new MainWidget( this );

	setCentralWidget( ui );
}

void
MainWindow::setMarkdownFile( const QString & fileName )
{
	ui->setMarkdownFile( fileName );
}

void
MainWindow::about()
{
	QMessageBox::about( this, tr( "About MD-PDF Converter" ),
		tr( "MD-PDF Converter.\n\n"
			"Version %1\n\n"
			"md4qt version %2\n\n"
			"Author - Igor Mironchik (igor.mironchik at gmail dot com).\n\n"
			"Copyright (c) 2019-2024 Igor Mironchik.\n\n"
			"Licensed under GNU GPL 3.0." )
				.arg( c_version )
				.arg( c_md4qtVersion ) );
}

void
MainWindow::aboutQt()
{
	QMessageBox::aboutQt( this );
}

void
MainWindow::licenses()
{
	LicenseDialog msg( this );
	msg.addLicense( QStringLiteral( "PoDoFo" ),
		QStringLiteral( "<p><b>PoDoFo License\n\n</b></p>"
		"<p>GNU LIBRARY GENERAL PUBLIC LICENSE\n</p>"
		"<p>Version 2, June 1991\n</p>"
		"\n"
		"<p>Copyright (C) 1991 Free Software Foundation, Inc.\n</p>"
		"<p>59 Temple Place - Suite 330\n</p>"
		"<p>Boston, MA 02111-1307, USA.\n</p>"
		"<p>Everyone is permitted to copy and distribute verbatim copies "
		"of this license document, but changing it is not allowed.\n</p>"
		"\n"
		"<p>[This is the first released version of the library GPL.  It is "
		"numbered 2 because it goes with version 2 of the ordinary GPL.]\n</p>"
		"\n"
		"<p>Preamble\n</p>"
		"\n"
		"<p>The licenses for most software are designed to take away your "
		"freedom to share and change it.  By contrast, the GNU General Public "
		"Licenses are intended to guarantee your freedom to share and change "
		"free software--to make sure the software is free for all its users.\n</p>"
		"\n"
		"<p>This license, the Library General Public License, applies to some "
		"specially designated Free Software Foundation software, and to any "
		"other libraries whose authors decide to use it.  You can use it for "
		"your libraries, too.\n</p>"
		"\n"
		"<p>When we speak of free software, we are referring to freedom, not "
		"price.  Our General Public Licenses are designed to make sure that you "
		"have the freedom to distribute copies of free software (and charge for "
		"this service if you wish), that you receive source code or can get it "
		"if you want it, that you can change the software or use pieces of it "
		"in new free programs; and that you know you can do these things.\n</p>"
		"\n"
		"<p>To protect your rights, we need to make restrictions that forbid "
		"anyone to deny you these rights or to ask you to surrender the rights. "
		"These restrictions translate to certain responsibilities for you if "
		"you distribute copies of the library, or if you modify it.\n</p>"
		"\n"
		"<p>For example, if you distribute copies of the library, whether gratis "
		"or for a fee, you must give the recipients all the rights that we gave "
		"you.  You must make sure that they, too, receive or can get the source "
		"code.  If you link a program with the library, you must provide "
		"complete object files to the recipients so that they can relink them "
		"with the library, after making changes to the library and recompiling "
		"it.  And you must show them these terms so they know their rights.\n</p>"
		"\n"
		"<p>Our method of protecting your rights has two steps: (1) copyright "
		"the library, and (2) offer you this license which gives you legal "
		"permission to copy, distribute and/or modify the library.\n</p>"
		"\n"
		"<p>Also, for each distributor's protection, we want to make certain "
		"that everyone understands that there is no warranty for this free "
		"library.  If the library is modified by someone else and passed on, we "
		"want its recipients to know that what they have is not the original "
		"version, so that any problems introduced by others will not reflect on "
		"the original authors' reputations.\n</p>"
		"\n"
		"<p>Finally, any free program is threatened constantly by software "
		"patents.  We wish to avoid the danger that companies distributing free "
		"software will individually obtain patent licenses, thus in effect "
		"transforming the program into proprietary software.  To prevent this, "
		"we have made it clear that any patent must be licensed for everyone's "
		"free use or not licensed at all.\n</p>"
		"\n"
		"<p>Most GNU software, including some libraries, is covered by the ordinary "
		"GNU General Public License, which was designed for utility programs.  This "
		"license, the GNU Library General Public License, applies to certain "
		"designated libraries.  This license is quite different from the ordinary "
		"one; be sure to read it in full, and don't assume that anything in it is "
		"the same as in the ordinary license.\n</p>"
		"\n"
		"<p>The reason we have a separate public license for some libraries is that "
		"they blur the distinction we usually make between modifying or adding to a "
		"program and simply using it.  Linking a program with a library, without "
		"changing the library, is in some sense simply using the library, and is "
		"analogous to running a utility program or application program.  However, in "
		"a textual and legal sense, the linked executable is a combined work, a "
		"derivative of the original library, and the ordinary General Public License "
		"treats it as such.\n</p>"
		"\n"
		"<p>Because of this blurred distinction, using the ordinary General "
		"Public License for libraries did not effectively promote software "
		"sharing, because most developers did not use the libraries.  We "
		"concluded that weaker conditions might promote sharing better.\n</p>"
		"\n"
		"</p>However, unrestricted linking of non-free programs would deprive the "
		"users of those programs of all benefit from the free status of the "
		"libraries themselves.  This Library General Public License is intended to "
		"permit developers of non-free programs to use free libraries, while "
		"preserving your freedom as a user of such programs to change the free "
		"libraries that are incorporated in them.  (We have not seen how to achieve "
		"this as regards changes in header files, but we have achieved it as regards "
		"changes in the actual functions of the Library.)  The hope is that this "
		"will lead to faster development of free libraries.\n</p>"
		"\n"
		"<p>The precise terms and conditions for copying, distribution and "
		"modification follow.  Pay close attention to the difference between a "
		"\"work based on the library\" and a \"work that uses the library\".  The "
		"former contains code derived from the library, while the latter only "
		"works together with the library.\n</p>"
		"\n"
		"<p>Note that it is possible for a library to be covered by the ordinary "
		"General Public License rather than by this special one.\n</p>"
		"\n"
		"<p><b>GNU LIBRARY GENERAL PUBLIC LICENSE "
		"TERMS AND CONDITIONS FOR COPYING, DISTRIBUTION AND MODIFICATION\n</b></p>"
		"\n"
		"<p><b>0.</b> This License Agreement applies to any software library which "
		"contains a notice placed by the copyright holder or other authorized "
		"party saying it may be distributed under the terms of this Library "
		"General Public License (also called \"this License\").  Each licensee is "
		"addressed as \"you\".\n</p>"
		"\n"
		"<p>A \"library\" means a collection of software functions and/or data "
		"prepared so as to be conveniently linked with application programs "
		"(which use some of those functions and data) to form executables.\n</p>"
		"\n"
		"<p>The \"Library\", below, refers to any such software library or work "
		"which has been distributed under these terms.  A \"work based on the "
		"Library\" means either the Library or any derivative work under "
		"copyright law: that is to say, a work containing the Library or a "
		"portion of it, either verbatim or with modifications and/or translated "
		"straightforwardly into another language.  (Hereinafter, translation is "
		"included without limitation in the term \"modification\".)\n</p>"
		"\n"
		"<p>\"Source code\" for a work means the preferred form of the work for "
		"making modifications to it.  For a library, complete source code means "
		"all the source code for all modules it contains, plus any associated "
		"interface definition files, plus the scripts used to control compilation "
		"and installation of the library.\n</p>"
		"\n"
		"<p>Activities other than copying, distribution and modification are not "
		"covered by this License; they are outside its scope.  The act of "
		"running a program using the Library is not restricted, and output from "
		"such a program is covered only if its contents constitute a work based "
		"on the Library (independent of the use of the Library in a tool for "
		"writing it).  Whether that is true depends on what the Library does "
		"and what the program that uses the Library does.\n</p>"
		"\n"
		"<p><b>1.</b> You may copy and distribute verbatim copies of the Library's "
		"complete source code as you receive it, in any medium, provided that "
		"you conspicuously and appropriately publish on each copy an "
		"appropriate copyright notice and disclaimer of warranty; keep intact "
		"all the notices that refer to this License and to the absence of any "
		"warranty; and distribute a copy of this License along with the "
		"Library.\n</p>"
		"\n"
		"<p>You may charge a fee for the physical act of transferring a copy, "
		"and you may at your option offer warranty protection in exchange for a "
		"fee.\n</p>"
		"\n"
		"<p><b>2.</b> You may modify your copy or copies of the Library or any portion "
		"of it, thus forming a work based on the Library, and copy and "
		"distribute such modifications or work under the terms of Section 1 "
		"above, provided that you also meet all of these conditions:\n</p>"
		"\n"
		"<p>  <b>a)</b> The modified work must itself be a software library.\n</p>"
		"\n"
		"<p>  <b>b)</b> You must cause the files modified to carry prominent notices "
		"stating that you changed the files and the date of any change.\n</p>"
		"\n"
		"<p>  <b>c)</b> You must cause the whole of the work to be licensed at no "
		"charge to all third parties under the terms of this License.\n</p>"
		"\n"
		"<p>  <b>d)</b> If a facility in the modified Library refers to a function or a "
		"table of data to be supplied by an application program that uses "
		"the facility, other than as an argument passed when the facility "
		"is invoked, then you must make a good faith effort to ensure that, "
		"in the event an application does not supply such function or "
		"table, the facility still operates, and performs whatever part of "
		"its purpose remains meaningful.\n</p>"
		"\n"
		"<p>(For example, a function in a library to compute square roots has "
		"a purpose that is entirely well-defined independent of the "
		"application.  Therefore, Subsection 2d requires that any "
		"application-supplied function or table used by this function must "
		"be optional: if the application does not supply it, the square "
		"root function must still compute square roots.)\n</p>"
		"\n"
		"<p>These requirements apply to the modified work as a whole.  If "
		"identifiable sections of that work are not derived from the Library, "
		"and can be reasonably considered independent and separate works in "
		"themselves, then this License, and its terms, do not apply to those "
		"sections when you distribute them as separate works.  But when you "
		"distribute the same sections as part of a whole which is a work based "
		"on the Library, the distribution of the whole must be on the terms of "
		"this License, whose permissions for other licensees extend to the "
		"entire whole, and thus to each and every part regardless of who wrote "
		"it.\n</p>"
		"\n"
		"<p>Thus, it is not the intent of this section to claim rights or contest "
		"your rights to work written entirely by you; rather, the intent is to "
		"exercise the right to control the distribution of derivative or "
		"collective works based on the Library.\n</p>"
		"\n"
		"<p>In addition, mere aggregation of another work not based on the Library "
		"with the Library (or with a work based on the Library) on a volume of "
		"a storage or distribution medium does not bring the other work under "
		"the scope of this License.\n</p>"
		"\n"
		"<p><b>3.</b> You may opt to apply the terms of the ordinary GNU General Public "
		"License instead of this License to a given copy of the Library.  To do "
		"this, you must alter all the notices that refer to this License, so "
		"that they refer to the ordinary GNU General Public License, version 2, "
		"instead of to this License.  (If a newer version than version 2 of the "
		"ordinary GNU General Public License has appeared, then you can specify "
		"that version instead if you wish.)  Do not make any other change in "
		"these notices.\n</p>"
		"\n"
		"<p>Once this change is made in a given copy, it is irreversible for "
		"that copy, so the ordinary GNU General Public License applies to all "
		"subsequent copies and derivative works made from that copy.\n</p>"
		"\n"
		"<p>This option is useful when you wish to copy part of the code of "
		"the Library into a program that is not a library.\n</p>"
		"\n"
		"<p><b>4.</b> You may copy and distribute the Library (or a portion or "
		"derivative of it, under Section 2) in object code or executable form "
		"under the terms of Sections 1 and 2 above provided that you accompany "
		"it with the complete corresponding machine-readable source code, which "
		"must be distributed under the terms of Sections 1 and 2 above on a "
		"medium customarily used for software interchange.\n</p>"
		"\n"
		"<p>If distribution of object code is made by offering access to copy "
		"from a designated place, then offering equivalent access to copy the "
		"source code from the same place satisfies the requirement to "
		"distribute the source code, even though third parties are not "
		"compelled to copy the source along with the object code.\n</p>"
		"\n"
		"<p><b>5.</b> A program that contains no derivative of any portion of the "
		"Library, but is designed to work with the Library by being compiled or "
		"linked with it, is called a \"work that uses the Library\".  Such a "
		"work, in isolation, is not a derivative work of the Library, and "
		"therefore falls outside the scope of this License.\n</p>"
		"\n"
		"<p>However, linking a \"work that uses the Library\" with the Library "
		"creates an executable that is a derivative of the Library (because it "
		"contains portions of the Library), rather than a \"work that uses the "
		"library\".  The executable is therefore covered by this License. "
		"Section 6 states terms for distribution of such executables.\n</p>"
		"\n"
		"<p>When a \"work that uses the Library\" uses material from a header file "
		"that is part of the Library, the object code for the work may be a "
		"derivative work of the Library even though the source code is not. "
		"Whether this is true is especially significant if the work can be "
		"linked without the Library, or if the work is itself a library.  The "
		"threshold for this to be true is not precisely defined by law.\n</p>"
		"\n"
		"<p>If such an object file uses only numerical parameters, data "
		"structure layouts and accessors, and small macros and small inline "
		"functions (ten lines or less in length), then the use of the object "
		"file is unrestricted, regardless of whether it is legally a derivative "
		"work.  (Executables containing this object code plus portions of the "
		"Library will still fall under Section 6.)\n</p>"
		"\n"
		"<p>Otherwise, if the work is a derivative of the Library, you may "
		"distribute the object code for the work under the terms of Section 6. "
		"Any executables containing that work also fall under Section 6, "
		"whether or not they are linked directly with the Library itself.\n</p>"
		"\n"
		"<p><b>6.</b> As an exception to the Sections above, you may also compile or "
		"link a \"work that uses the Library\" with the Library to produce a "
		"work containing portions of the Library, and distribute that work "
		"under terms of your choice, provided that the terms permit "
		"modification of the work for the customer's own use and reverse "
		"engineering for debugging such modifications.\n</p>"
		"\n"
		"<p>You must give prominent notice with each copy of the work that the "
		"Library is used in it and that the Library and its use are covered by "
		"this License.  You must supply a copy of this License.  If the work "
		"during execution displays copyright notices, you must include the "
		"copyright notice for the Library among them, as well as a reference "
		"directing the user to the copy of this License.  Also, you must do one "
		"of these things:\n</p>"
		"\n"
		"<p>  <b>a)</b> Accompany the work with the complete corresponding "
		"machine-readable source code for the Library including whatever "
		"changes were used in the work (which must be distributed under "
		"Sections 1 and 2 above); and, if the work is an executable linked "
		"with the Library, with the complete machine-readable \"work that "
		"uses the Library\", as object code and/or source code, so that the "
		"user can modify the Library and then relink to produce a modified "
		"executable containing the modified Library.  (It is understood "
		"that the user who changes the contents of definitions files in the "
		"Library will not necessarily be able to recompile the application "
		"to use the modified definitions.)\n</p>"
		"\n"
		"<p>  <b>b)</b> Accompany the work with a written offer, valid for at "
		"least three years, to give the same user the materials "
		"specified in Subsection 6a, above, for a charge no more "
		"than the cost of performing this distribution.\n</p>"
		"\n"
		"<p>  <b>c)</b> If distribution of the work is made by offering access to copy "
		"from a designated place, offer equivalent access to copy the above "
		"specified materials from the same place.\n</p>"
		"\n"
		"<p>  <b>d)</b> Verify that the user has already received a copy of these "
		"materials or that you have already sent this user a copy.\n</p>"
		"\n"
		"<p>For an executable, the required form of the \"work that uses the "
		"Library\" must include any data and utility programs needed for "
		"reproducing the executable from it.  However, as a special exception, "
		"the source code distributed need not include anything that is normally "
		"distributed (in either source or binary form) with the major "
		"components (compiler, kernel, and so on) of the operating system on "
		"which the executable runs, unless that component itself accompanies "
		"the executable.\n</p>"
		"\n"
		"<p>It may happen that this requirement contradicts the license "
		"restrictions of other proprietary libraries that do not normally "
		"accompany the operating system.  Such a contradiction means you cannot "
		"use both them and the Library together in an executable that you "
		"distribute.\n</p>"
		"\n"
		"<p><b>7.</b> You may place library facilities that are a work based on the "
		"Library side-by-side in a single library together with other library "
		"facilities not covered by this License, and distribute such a combined "
		"library, provided that the separate distribution of the work based on "
		"the Library and of the other library facilities is otherwise "
		"permitted, and provided that you do these two things:\n</p>"
		"\n"
		"<p>  <b>a)</b> Accompany the combined library with a copy of the same work "
		"based on the Library, uncombined with any other library "
		"facilities.  This must be distributed under the terms of the "
		"Sections above.\n</p>"
		"\n"
		"<p>  <b>b)</b> Give prominent notice with the combined library of the fact "
		"that part of it is a work based on the Library, and explaining "
		"where to find the accompanying uncombined form of the same work.\n</p>"
		"\n"
		"<p><b>8.</b> You may not copy, modify, sublicense, link with, or distribute "
		"the Library except as expressly provided under this License.  Any "
		"attempt otherwise to copy, modify, sublicense, link with, or "
		"distribute the Library is void, and will automatically terminate your "
		"rights under this License.  However, parties who have received copies, "
		"or rights, from you under this License will not have their licenses "
		"terminated so long as such parties remain in full compliance.\n</p>"
		"\n"
		"<p><b>9.</b> You are not required to accept this License, since you have not "
		"signed it.  However, nothing else grants you permission to modify or "
		"distribute the Library or its derivative works.  These actions are "
		"prohibited by law if you do not accept this License.  Therefore, by "
		"modifying or distributing the Library (or any work based on the "
		"Library), you indicate your acceptance of this License to do so, and "
		"all its terms and conditions for copying, distributing or modifying "
		"the Library or works based on it.\n</p>"
		"\n"
		"<p><b>10.</b> Each time you redistribute the Library (or any work based on the "
		"Library), the recipient automatically receives a license from the "
		"original licensor to copy, distribute, link with or modify the Library "
		"subject to these terms and conditions.  You may not impose any further "
		"restrictions on the recipients' exercise of the rights granted herein. "
		"You are not responsible for enforcing compliance by third parties to "
		"this License.\n</p>"
		"\n"
		"<p><b>11.</b> If, as a consequence of a court judgment or allegation of patent "
		"infringement or for any other reason (not limited to patent issues), "
		"conditions are imposed on you (whether by court order, agreement or "
		"otherwise) that contradict the conditions of this License, they do not "
		"excuse you from the conditions of this License.  If you cannot "
		"distribute so as to satisfy simultaneously your obligations under this "
		"License and any other pertinent obligations, then as a consequence you "
		"may not distribute the Library at all.  For example, if a patent "
		"license would not permit royalty-free redistribution of the Library by "
		"all those who receive copies directly or indirectly through you, then "
		"the only way you could satisfy both it and this License would be to "
		"refrain entirely from distribution of the Library.\n</p>"
		"\n"
		"<p>If any portion of this section is held invalid or unenforceable under any "
		"particular circumstance, the balance of the section is intended to apply, "
		"and the section as a whole is intended to apply in other circumstances.\n</p>"
		"\n"
		"<p>It is not the purpose of this section to induce you to infringe any "
		"patents or other property right claims or to contest validity of any "
		"such claims; this section has the sole purpose of protecting the "
		"integrity of the free software distribution system which is "
		"implemented by public license practices.  Many people have made "
		"generous contributions to the wide range of software distributed "
		"through that system in reliance on consistent application of that "
		"system; it is up to the author/donor to decide if he or she is willing "
		"to distribute software through any other system and a licensee cannot "
		"impose that choice.\n</p>"
		"\n"
		"<p>This section is intended to make thoroughly clear what is believed to "
		"be a consequence of the rest of this License.\n</p>"
		"\n"
		"<p><b>12.</b> If the distribution and/or use of the Library is restricted in "
		"certain countries either by patents or by copyrighted interfaces, the "
		"original copyright holder who places the Library under this License may add "
		"an explicit geographical distribution limitation excluding those countries, "
		"so that distribution is permitted only in or among countries not thus "
		"excluded.  In such case, this License incorporates the limitation as if "
		"written in the body of this License.\n</p>"
		"\n"
		"<p><b>13.</b> The Free Software Foundation may publish revised and/or new "
		"versions of the Library General Public License from time to time. "
		"Such new versions will be similar in spirit to the present version, "
		"but may differ in detail to address new problems or concerns.\n</p>"
		"\n"
		"<p>Each version is given a distinguishing version number.  If the Library "
		"specifies a version number of this License which applies to it and "
		"\"any later version\", you have the option of following the terms and "
		"conditions either of that version or of any later version published by "
		"the Free Software Foundation.  If the Library does not specify a "
		"license version number, you may choose any version ever published by "
		"the Free Software Foundation.\n</p>"
		"\n"
		"<p><b>14.</b> If you wish to incorporate parts of the Library into other free "
		"programs whose distribution conditions are incompatible with these, "
		"write to the author to ask for permission.  For software which is "
		"copyrighted by the Free Software Foundation, write to the Free "
		"Software Foundation; we sometimes make exceptions for this.  Our "
		"decision will be guided by the two goals of preserving the free status "
		"of all derivatives of our free software and of promoting the sharing "
		"and reuse of software generally.\n</p>"
		"\n"
		"<p>NO WARRANTY\n</p>"
		"\n"
		"<p><b>15.</b> BECAUSE THE LIBRARY IS LICENSED FREE OF CHARGE, THERE IS NO "
		"WARRANTY FOR THE LIBRARY, TO THE EXTENT PERMITTED BY APPLICABLE LAW. "
		"EXCEPT WHEN OTHERWISE STATED IN WRITING THE COPYRIGHT HOLDERS AND/OR "
		"OTHER PARTIES PROVIDE THE LIBRARY \"AS IS\" WITHOUT WARRANTY OF ANY "
		"KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, THE "
		"IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR "
		"PURPOSE.  THE ENTIRE RISK AS TO THE QUALITY AND PERFORMANCE OF THE "
		"LIBRARY IS WITH YOU.  SHOULD THE LIBRARY PROVE DEFECTIVE, YOU ASSUME "
		"THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.\n</p>"
		"\n"
		"<p><b>16.</b> IN NO EVENT UNLESS REQUIRED BY APPLICABLE LAW OR AGREED TO IN "
		"WRITING WILL ANY COPYRIGHT HOLDER, OR ANY OTHER PARTY WHO MAY MODIFY "
		"AND/OR REDISTRIBUTE THE LIBRARY AS PERMITTED ABOVE, BE LIABLE TO YOU "
		"FOR DAMAGES, INCLUDING ANY GENERAL, SPECIAL, INCIDENTAL OR "
		"CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE "
		"LIBRARY (INCLUDING BUT NOT LIMITED TO LOSS OF DATA OR DATA BEING "
		"RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR THIRD PARTIES OR A "
		"FAILURE OF THE LIBRARY TO OPERATE WITH ANY OTHER SOFTWARE), EVEN IF "
		"SUCH HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH "
		"DAMAGES.\n</p>"
		"\n"
		"<p>END OF TERMS AND CONDITIONS\n\n</p>"
		"<p>How to Apply These Terms to Your New Libraries\n</p>"
		"\n"
		"<p>If you develop a new library, and you want it to be of the greatest "
		"possible use to the public, we recommend making it free software that "
		"everyone can redistribute and change.  You can do so by permitting "
		"redistribution under these terms (or, alternatively, under the terms of the "
		"ordinary General Public License).\n</p>"
		"\n"
		"<p>To apply these terms, attach the following notices to the library.  It is "
		"safest to attach them to the start of each source file to most effectively "
		"convey the exclusion of warranty; and each file should have at least the "
		"\"copyright\" line and a pointer to where the full notice is found.\n</p>"
		"\n"
		"<p>&lt;one line to give the library's name and a brief idea of what it does.&gt;\n</p>"
		"<p>Copyright (C) &lt;year&gt;  &lt;name of author&gt;\n</p>"
		"\n"
		"<p>This library is free software; you can redistribute it and/or "
		"modify it under the terms of the GNU Lesser General Public "
		"License as published by the Free Software Foundation; either "
		"version 2 of the License, or (at your option) any later version.\n</p>"
		"\n"
		"<p>This library is distributed in the hope that it will be useful, "
		"but WITHOUT ANY WARRANTY; without even the implied warranty of "
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU "
		"Lesser General Public License for more details.\n</p>"
		"\n"
		"<p>You should have received a copy of the GNU Lesser General Public "
		"License along with this library; if not, write to the Free Software "
		"Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n</p>"
		"\n"
		"<p>Also add information on how to contact you by electronic and paper mail.\n</p>"
		"\n"
		"<p>You should also get your employer (if you work as a programmer) or your "
		"school, if any, to sign a \"copyright disclaimer\" for the library, if "
		"necessary.  Here is a sample; alter the names:\n</p>"
		"\n"
		"<p>Yoyodyne, Inc., hereby disclaims all copyright interest in the "
		"library `Frob' (a library for tweaking knobs) written by James Random Hacker.\n</p>"
		"\n"
		"<p>&lt;signature of Ty Coon&gt;, 1 April 1990\n</p>"
		"<p>Ty Coon, President of Vice\n</p>"
		"\n"
		"<p>That's all there is to it!</p>" ) );

	msg.addLicense( QStringLiteral( "ImageMagick" ),
		QStringLiteral( "<p><b>ImageMagick License</b>\n\n</p>"
		"<p>Terms and Conditions for Use, Reproduction, and Distribution\n</p>"
		"\n"
		"<p>The legally binding and authoritative terms and conditions for use, reproduction, "
		"and distribution of ImageMagick follow:\n</p>"
		"\n"
		"<p>Copyright (c) 1999-2021 ImageMagick Studio LLC, a non-profit organization dedicated "
		"to making software imaging solutions freely available.\n</p>"
		"\n"
		"<p><b>1.</b> Definitions.\n</p>"
		"\n"
		"<p>License shall mean the terms and conditions for use, reproduction, and distribution as "
		"defined by Sections 1 through 9 of this document.\n</p>"
		"\n"
		"<p>Licensor shall mean the copyright owner or entity authorized by the copyright owner "
		"that is granting the License.\n</p>"
		"\n"
		"<p>Legal Entity shall mean the union of the acting entity and all other entities that "
		"control, are controlled by, or are under common control with that entity. For the "
		"purposes of this definition, control means (i) the power, direct or indirect, to cause "
		"the direction or management of such entity, whether by contract or otherwise, or (ii) "
		"ownership of fifty percent (50%) or more of the outstanding shares, or (iii) beneficial "
		"ownership of such entity.\n</p>"
		"\n"
		"<p>You (or Your) shall mean an individual or Legal Entity exercising permissions granted "
		"by this License.\n</p>"
		"\n"
		"<p>Source form shall mean the preferred form for making modifications, including but not "
		"limited to software source code, documentation source, and configuration files.\n</p>"
		"\n"
		"<p>Object form shall mean any form resulting from mechanical transformation or translation "
		"of a Source form, including but not limited to compiled object code, generated "
		"documentation, and conversions to other media types.\n</p>"
		"\n"
		"<p>Work shall mean the work of authorship, whether in Source or Object form, made available "
		"under the License, as indicated by a copyright notice that is included in or attached to "
		"the work (an example is provided in the Appendix below).\n</p>"
		"\n"
		"<p>Derivative Works shall mean any work, whether in Source or Object form, that is based on "
		"(or derived from) the Work and for which the editorial revisions, annotations, "
		"elaborations, or other modifications represent, as a whole, an original work of "
		"authorship. For the purposes of this License, Derivative Works shall not include works "
		"that remain separable from, or merely link (or bind by name) to the interfaces of, the "
		"Work and Derivative Works thereof.\n</p>"
		"\n"
		"<p>Contribution shall mean any work of authorship, including the original version of the "
		"Work and any modifications or additions to that Work or Derivative Works thereof, that is "
		"intentionally submitted to Licensor for inclusion in the Work by the copyright owner or "
		"by an individual or Legal Entity authorized to submit on behalf of the copyright owner. "
		"For the purposes of this definition, \"submitted\" means any form of electronic, verbal, "
		"or written communication sent to the Licensor or its representatives, including but not "
		"limited to communication on electronic mailing lists, source code control systems, and "
		"issue tracking systems that are managed by, or on behalf of, the Licensor for the purpose "
		"of discussing and improving the Work, but excluding communication that is conspicuously "
		"marked or otherwise designated in writing by the copyright owner as Not a Contribution.\n</p>"
		"\n"
		"<p>Contributor shall mean Licensor and any individual or Legal Entity on behalf of whom a "
		"Contribution has been received by Licensor and subsequently incorporated within the Work.\n</p>"
		"\n"
		"<p><b>2.</b> Grant of Copyright License. Subject to the terms and conditions of this License, each "
		"Contributor hereby grants to You a perpetual, worldwide, non-exclusive, no-charge, "
		"royalty-free, irrevocable copyright license to reproduce, prepare Derivative Works of, "
		"publicly display, publicly perform, sublicense, and distribute the Work and such "
		"Derivative Works in Source or Object form.\n</p>"
		"\n"
		"<p><b>3.</b> Grant of Patent License. Subject to the terms and conditions of this License, each "
		"Contributor hereby grants to You a perpetual, worldwide, non-exclusive, no-charge, "
		"royalty-free, irrevocable (except as stated in this section) patent license to make, have "
		"made, use, offer to sell, sell, import, and otherwise transfer the Work, where such "
		"license applies only to those patent claims licensable by such Contributor that are "
		"necessarily infringed by their Contribution(s) alone or by combination of their "
		"Contribution(s) with the Work to which such Contribution(s) was submitted. If You "
		"institute patent litigation against any entity (including a cross-claim or counterclaim "
		"in a lawsuit) alleging that the Work or a Contribution incorporated within the Work "
		"constitutes direct or contributory patent infringement, then any patent licenses granted "
		"to You under this License for that Work shall terminate as of the date such litigation "
		"is filed.\n</p>"
		"\n"
		"<p><b>4.</b> Redistribution. You may reproduce and distribute copies of the Work or Derivative "
		"Works thereof in any medium, with or without modifications, and in Source or Object "
		"form, provided that You meet the following conditions:\n</p>"
		"\n"
		"<p> <b>a.</b> You must give any other recipients of the Work or Derivative Works a copy of this "
		"License; and\n</p>"
		"<p> <b>b.</b> You must cause any modified files to carry prominent notices stating that You changed "
		"the files; and\n</p>"
		"<p> <b>c.</b> You must retain, in the Source form of any Derivative Works that You distribute, all "
		"copyright, patent, trademark, and attribution notices from the Source form of the Work, "
		"excluding those notices that do not pertain to any part of the Derivative Works; and\n</p>"
		"<p> <b>d.</b> If the Work includes a \"NOTICE\" text file as part of its distribution, then any "
		"Derivative Works that You distribute must include a readable copy of the attribution "
		"notices contained within such NOTICE file, excluding those notices that do not pertain "
		"to any part of the Derivative Works, in at least one of the following places: within a "
		"NOTICE text file distributed as part of the Derivative Works; within the Source form or "
		"documentation, if provided along with the Derivative Works; or, within a display "
		"generated by the Derivative Works, if and wherever such third-party notices normally "
		"appear. The contents of the NOTICE file are for informational purposes only and do not "
		"modify the License. You may add Your own attribution notices within Derivative Works "
		"that You distribute, alongside or as an addendum to the NOTICE text from the Work, "
		"provided that such additional attribution notices cannot be construed as modifying the "
		"License.\n</p>"
		"\n"
		"<p>You may add Your own copyright statement to Your modifications and may provide additional "
		"or different license terms and conditions for use, reproduction, or distribution of Your "
		"modifications, or for any such Derivative Works as a whole, provided Your use, "
		"reproduction, and distribution of the Work otherwise complies with the conditions stated "
		"in this License.\n</p>"
		"\n"
		"<p><b>5.</b> Submission of Contributions. Unless You explicitly state otherwise, any Contribution "
		"intentionally submitted for inclusion in the Work by You to the Licensor shall be under "
		"the terms and conditions of this License, without any additional terms or conditions. "
		"Notwithstanding the above, nothing herein shall supersede or modify the terms of any "
		"separate license agreement you may have executed with Licensor regarding such Contributions.\n</p>"
		"\n"
		"<p><b>6.</b> Trademarks. This License does not grant permission to use the trade names, trademarks, "
		"service marks, or product names of the Licensor, except as required for reasonable and "
		"customary use in describing the origin of the Work and reproducing the content of the "
		"NOTICE file.\n</p>"
		"\n"
		"<p><b>7.</b> Disclaimer of Warranty. Unless required by applicable law or agreed to in writing, "
		"Licensor provides the Work (and each Contributor provides its Contributions) on an AS IS "
		"BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied, "
		"including, without limitation, any warranties or conditions of TITLE, NON-INFRINGEMENT, "
		"MERCHANTABILITY, or FITNESS FOR A PARTICULAR PURPOSE. You are solely responsible for "
		"determining the appropriateness of using or redistributing the Work and assume any risks "
		"associated with Your exercise of permissions under this License.\n</p>"
		"\n"
		"<p><b>8.</b> Limitation of Liability. In no event and under no legal theory, whether in tort "
		"(including negligence), contract, or otherwise, unless required by applicable law (such "
		"as deliberate and grossly negligent acts) or agreed to in writing, shall any Contributor "
		"be liable to You for damages, including any direct, indirect, special, incidental, or "
		"consequential damages of any character arising as a result of this License or out of the "
		"use or inability to use the Work (including but not limited to damages for loss of "
		"goodwill, work stoppage, computer failure or malfunction, or any and all other "
		"commercial damages or losses), even if such Contributor has been advised of the "
		"possibility of such damages.\n</p>"
		"\n"
		"<p><b>9.</b> Accepting Warranty or Additional Liability. While redistributing the Work or "
		"Derivative Works thereof, You may choose to offer, and charge a fee for, acceptance of "
		"support, warranty, indemnity, or other liability obligations and/or rights consistent "
		"with this License. However, in accepting such obligations, You may act only on Your own "
		"behalf and on Your sole responsibility, not on behalf of any other Contributor, and only "
		"if You agree to indemnify, defend, and hold each Contributor harmless for any liability "
		"incurred by, or claims asserted against, such Contributor by reason of your accepting "
		"any such warranty or additional liability.\n</p>"
		"\n"
		"<p>How to Apply the License to your Work\n</p>"
		"\n"
		"<p>To apply the ImageMagick License to your work, attach the following boilerplate notice, "
		"with the fields enclosed by brackets \"[]\" replaced with your own identifying "
		"information (don't include the brackets). The text should be enclosed in the appropriate "
		"comment syntax for the file format. We also recommend that a file or class name and "
		"description of purpose be included on the same \"printed page\" as the copyright notice "
		"for easier identification within third-party archives.\n</p>"
		"\n"
		"<p>    Copyright [yyyy] [name of copyright owner]\n</p>"
		"\n"
		"<p>    Licensed under the ImageMagick License (the \"License\"); you may not use\n"
		"    this file except in compliance with the License.  You may obtain a copy\n"
		"    of the License at\n</p>"
		"\n"
		"<p>    <a href=\"https://imagemagick.org/script/license.php\">https://imagemagick.org/script/license.php</a>\n</p>"
		"\n"
		"<p>    Unless required by applicable law or agreed to in writing, software\n"
		"    distributed under the License is distributed on an \"AS IS\" BASIS, WITHOUT\n"
		"    WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the\n"
		"    License for the specific language governing permissions and limitations\n"
		"    under the License.</p>" ) );

	msg.addLicense( QStringLiteral( "KSyntaxHighlighting" ),
		QStringLiteral( "<p><b>KSyntaxHighlighting License</b>\n\n</p>"
		"<p>MIT License Copyright (c) &lt;year&gt; &lt;copyright holders&gt;</p>\n"
		"<p>Permission is hereby granted, free of charge, to any person obtaining a copy "
		"of this software and associated documentation files (the \"Software\"), to deal "
		"in the Software without restriction, including without limitation the rights "
		"to use, copy, modify, merge, publish, distribute, sublicense, and/or sell "
		"copies of the Software, and to permit persons to whom the Software is furnished "
		"to do so, subject to the following conditions:</p>"
		"<p>The above copyright notice and this permission notice (including the next "
		"paragraph) shall be included in all copies or substantial portions of the "
		"Software.</p>"
		"<p>THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR "
		"IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS "
		"FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS "
		"OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, "
		"WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF "
		"OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.</p>" ) );

	msg.exec();
}

void
MainWindow::quit()
{
	QApplication::quit();
}
