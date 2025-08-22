/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QAbstractButton>
#include <QColor>
#include <QDialog>
#include <QScopedPointer>

// md-editor include.
#include "colors.h"

// shared include.
#include "syntax.h"

namespace MdShared
{

class ColorWidget;

} /* namespace MdShared */

namespace MdEditor
{

//
// ColorsDialog
//

struct ColorsDialogPrivate;

//! Colors dialog.
class ColorsDialog : public QDialog
{
    Q_OBJECT

public:
    ColorsDialog(const Colors &cols,
                 std::shared_ptr<MdShared::Syntax> syntax,
                 QWidget *parent = nullptr);
    ~ColorsDialog() override;

    //! \return Current colors scheme.
    const Colors &colors() const;

private slots:
    //! On button in buttons group clicked.
    void clicked(QAbstractButton *btn);

private:
    Q_DISABLE_COPY(ColorsDialog)

    QScopedPointer<ColorsDialogPrivate> m_d;
}; // class ColorsDialog

} /* namespace MdEditor */
