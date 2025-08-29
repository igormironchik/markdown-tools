/*
    SPDX-FileCopyrightText: 2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

#pragma once

// Qt include.
#include <QFrame>

/*!
 * \brief Namespace for shared stuff.
 *
 * Namespace for shared stuff used both in Markdown editor and converter to PDF.
 */
namespace MdShared
{

//
// ColorWidget
//

/*!
 * \brief Color widget.
 *
 * Colored clickable rectangle - color picker.
 */
class ColorWidget final : public QFrame
{
    Q_OBJECT

signals:
    /*!
     * Clicked.
     */
    void clicked();

public:
    /*!
     * Constructor.
     *
     * \param parent Parent widget.
     */
    ColorWidget(QWidget *parent);
    ~ColorWidget() override = default;

    /*!
     * Returns current color.
     */
    const QColor &color() const;
    /*!
     * Set current color.
     *
     * \param c Color.
     */
    void setColor(const QColor &c);

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;

private:
    QColor m_color;
    bool m_pressed;

    Q_DISABLE_COPY(ColorWidget)
}; // class ColorWidget

} /* namespace MdShared */
