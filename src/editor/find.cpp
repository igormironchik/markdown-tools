/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-editor include.
#include "find.hpp"
#include "ui_find.h"
#include "editor.hpp"
#include "mainwindow.hpp"

// Qt include.
#include <QPalette>


namespace MdEditor {

//
// FindPrivate
//

struct FindPrivate {
	FindPrivate( MainWindow * w, Editor * e, Find * parent )
		:	q( parent )
		,	editor( e )
		,	window( w )
	{
	}

	void initUi()
	{
		ui.setupUi( q );

		QObject::connect( ui.findEdit, &QLineEdit::textChanged,
			q, &Find::onFindTextChanged );
		QObject::connect( ui.replaceEdit, &QLineEdit::textChanged,
			q, &Find::onReplaceTextChanged );

		auto findPrevAction = new QAction( Find::tr( "Find Previous" ), q );
		findPrevAction->setShortcutContext( Qt::ApplicationShortcut );
		findPrevAction->setShortcut( Find::tr( "Shift+F3" ) );
		findPrevAction->setToolTip( Find::tr( "Find Previous <small>Shift+F3</small>" ) );
		ui.findPrevBtn->setDefaultAction( findPrevAction );
		ui.findPrevBtn->setEnabled( false );

		auto findNextAction = new QAction( Find::tr( "Find Next" ), q );
		findNextAction->setShortcutContext( Qt::ApplicationShortcut );
		findNextAction->setShortcut( Find::tr( "F3" ) );
		findNextAction->setToolTip( Find::tr( "Find Next <small>F3</small>" ) );
		ui.findNextBtn->setDefaultAction( findNextAction );
		ui.findNextBtn->setEnabled( false );

		auto replaceAction = new QAction( Find::tr( "Replace" ), q );
		replaceAction->setToolTip( Find::tr( "Replace Selected" ) );
		ui.replaceBtn->setDefaultAction( replaceAction );
		ui.replaceBtn->setEnabled( false );

		auto replaceAllAction = new QAction( Find::tr( "Replace All" ), q );
		replaceAllAction->setToolTip( Find::tr( "Replace All" ) );
		ui.replaceAllBtn->setDefaultAction( replaceAllAction );
		ui.replaceAllBtn->setEnabled( false );

		textColor = ui.findEdit->palette().color( QPalette::Text );

		QObject::connect( findPrevAction, &QAction::triggered,
			editor, &Editor::onFindPrev );
		QObject::connect( findNextAction, &QAction::triggered,
			editor, &Editor::onFindNext );
		QObject::connect( replaceAction, &QAction::triggered,
			q, &Find::onReplace );
		QObject::connect( replaceAllAction, &QAction::triggered,
			q, &Find::onReplaceAll );
		QObject::connect( editor, &QPlainTextEdit::selectionChanged,
			q, &Find::onSelectionChanged );
		QObject::connect( ui.close, &QAbstractButton::clicked,
			q, &Find::onClose );
	}

	Find * q = nullptr;
	Editor * editor = nullptr;
	MainWindow * window = nullptr;
	QColor textColor;
	Ui::Find ui;
}; // struct FindPrivate


//
// Find
//

Find::Find( MainWindow * window, Editor * editor, QWidget * parent )
	:	QFrame( parent )
	,	d( new FindPrivate( window, editor, this ) )
{
	d->initUi();
}

Find::~Find()
{
}

QLineEdit *
Find::editLine() const
{
	return d->ui.findEdit;
}

QLineEdit *
Find::replaceLine() const
{
	return d->ui.replaceEdit;
}

void
Find::onFindTextChanged( const QString & str )
{
	d->editor->highlight( d->ui.findEdit->text(), true );

	QColor c = d->textColor;

	if( !d->editor->foundHighlighted() )
		c = Qt::red;

	d->ui.findNextBtn->setEnabled( d->editor->foundHighlighted() );
	d->ui.findPrevBtn->setEnabled( d->editor->foundHighlighted() );
	d->ui.findNextBtn->defaultAction()->setEnabled( d->editor->foundHighlighted() );
	d->ui.findPrevBtn->defaultAction()->setEnabled( d->editor->foundHighlighted() );

	QPalette palette = d->ui.findEdit->palette();
	palette.setColor( QPalette::Text, c );
	d->ui.findEdit->setPalette( palette );
}

void
Find::onReplaceTextChanged( const QString & )
{
	onSelectionChanged();
}

void
Find::setFindText( const QString & text )
{
	d->ui.findEdit->setText( text );

	setFocusOnFind();

	onSelectionChanged();
}

void
Find::setFocusOnFind()
{
	d->ui.findEdit->setFocus();
	d->ui.findEdit->selectAll();

	d->editor->highlight( d->ui.findEdit->text(), true );
}

void
Find::onReplace()
{
	d->editor->replaceCurrent( d->ui.replaceEdit->text() );
}

void
Find::onReplaceAll()
{
	d->editor->replaceAll( d->ui.replaceEdit->text() );
}

void
Find::onSelectionChanged()
{
	d->ui.replaceBtn->setEnabled( d->editor->foundSelected() &&
		!d->ui.replaceEdit->text().isEmpty() );
	d->ui.replaceAllBtn->setEnabled( d->editor->foundHighlighted() &&
		!d->ui.replaceEdit->text().isEmpty() );
}

void
Find::onClose()
{
	hide();

	d->window->onToolHide();
}

} /* namespace MdEditor */
