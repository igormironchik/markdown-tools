/*
	SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
	SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QDialog>
#include <QColor>
#include <QScopedPointer>
#include <QAbstractButton>

// md-editor include.
#include "colors.hpp"


namespace MdShared {

class ColorWidget;

} /* namespace MdShared */

namespace MdEditor {

//
// ColorsDialog
//

struct ColorsDialogPrivate;

//! Colors dialog.
class ColorsDialog
	:	public QDialog
{
	Q_OBJECT

public:
	explicit ColorsDialog( const Colors & cols, QWidget * parent = nullptr );
	~ColorsDialog() override;

	const Colors & colors() const;

private slots:
	void clicked( QAbstractButton * btn );

private:
	Q_DISABLE_COPY( ColorsDialog )

	QScopedPointer< ColorsDialogPrivate > d;
}; // class ColorsDialog

} /* namespace MdEditor */
