
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

// md-pdf include.
#include "progress.hpp"

// Qt include.
#include <QCloseEvent>
#include <QMessageBox>


//
// ProgressDlg
//

ProgressDlg::ProgressDlg( PdfRenderer * render, QWidget * parent )
	:	QDialog( parent )
	,	m_render( render )
	,	m_ui( new Ui::ProgressDlg() )
{
	m_ui->setupUi( this );

	connect( m_render, &PdfRenderer::progress, this, &ProgressDlg::progress );
	connect( m_render, &PdfRenderer::done, this, &ProgressDlg::finished );
	connect( m_render, &PdfRenderer::error, this, &ProgressDlg::error );
	connect( m_render, &PdfRenderer::status, this, &ProgressDlg::status );

	connect( m_ui->m_cancel, &QPushButton::clicked,
		this, &ProgressDlg::cancel );
}

void
ProgressDlg::closeEvent( QCloseEvent * e )
{
	cancel();

	e->ignore();
}

const QString &
ProgressDlg::errorMsg() const
{
	return m_error;
}

void
ProgressDlg::progress( int value )
{
	m_ui->m_progress->setValue( value );
}

void
ProgressDlg::finished( bool terminated )
{
	if( terminated )
		reject();
	else
		accept();
}

void
ProgressDlg::cancel()
{
	const auto answer = QMessageBox::question( this, tr( "Cancel rendering?" ),
		tr( "Do you really want to terminate rendering?" ) );

	if( answer == QMessageBox::Yes )
		m_render->terminate();
}

void
ProgressDlg::error( const QString & msg )
{
	m_error = msg;

	reject();
}

void
ProgressDlg::status( const QString & msg )
{
	m_ui->m_status->setText( msg );
}
