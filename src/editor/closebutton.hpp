/*
    SPDX-FileCopyrightText: 2024 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QAbstractButton>
#include <QScopedPointer>

namespace MdEditor
{

//
// CloseButton
//

struct CloseButtonPrivate;

//! Close button.
class CloseButton : public QAbstractButton
{
    Q_OBJECT

public:
    explicit CloseButton(QWidget *parent);
    ~CloseButton() override;

    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *e) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    friend struct CloseButtonPrivate;

    Q_DISABLE_COPY(CloseButton)

    QScopedPointer<CloseButtonPrivate> m_d;
}; // class CloseButton

} /* namespace MdEditor */
