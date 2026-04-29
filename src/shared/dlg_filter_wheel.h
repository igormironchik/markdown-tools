/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// KDE include.
#include <KPageDialog>

namespace MdShared
{

//
// DlgWheelFilter
//

/*!
 * Filter of wheel event for QComboBox and QSpinBox.
 */
class DlgWheelFilter : public KPageDialog
{
    Q_OBJECT

public:
    explicit DlgWheelFilter(QWidget *parent);
    ~DlgWheelFilter() override = default;

protected:
    bool eventFilter(QObject *watched,
                     QEvent *event) override;

protected:
    virtual bool isFilteredType(QObject *o);
    virtual void installFilterForChildren(QObject *parent);
}; // class DlgWheelFilter

} /* namespace MdShared */
