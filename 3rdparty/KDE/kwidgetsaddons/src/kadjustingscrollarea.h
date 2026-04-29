// SPDX-FileCopyrightText: 2007 Klarälvdalens Datakonsult AB
// SPDX-FileCopyrightText: 2022 g10 Code GmbH
// SPDX-FileContributor: Ingo Klöcker <dev@ingo-kloecker.de>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef KADJUSTINGSCROLLAREA_H
#define KADJUSTINGSCROLLAREA_H

#include <kwidgetsaddons_export.h>

#include <QScrollArea>

/*!
 * \class KAdjustingScrollArea
 * \inmodule KWidgetsAddons
 *
 * \brief Special scroll area widget which adjust its size to avoid scroll bars
 * as much as possible
 *
 * This widget improves a few aspects of QScrollArea in particular, for
 * vertically scrollable widgets and dialogs.
 *
 * If sizeAdjustPolicy is set to QAbstractScrollArea::AdjustToContents,
 * then the scroll area will (try to) adjust its size to the widget to avoid
 * scroll bars as much as possible.
 *
 * In particular, this widget will automatically increase the height of its
 * window when the content of the scroll area grows in height (e.g. because
 * more widgets are added dynamically). The automatic resizing stops at 2/3
 * of the screen's height.
 *
 * Instead of setting the scrollarea's content via QScrollArea::setWidget,
 * add the child widgets to the QVBoxLayout of the scrollarea's widget.
 *
 * \code
 * auto scrollArea = new KAdjustingScrollArea(this);
 * scrollArea->setSizeAdjustPolicy(QScrollArea::AdjustToContents);
 * scrollArea->setFocusPolicy(Qt::NoFocus);
 * scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
 * scrollArea->setWidget(new QWidget);
 *
 * auto scrollAreaLayout = new QVBoxLayout(scrollArea->widget());
 * \endcode
 *
 * \since 6.9
 */
class KWIDGETSADDONS_EXPORT KAdjustingScrollArea : public QScrollArea
{
    Q_OBJECT

public:
    /*!
     * Creates a scroll area with a QWidget with QVBoxLayout that is flagged
     * as resizable.
     */
    explicit KAdjustingScrollArea(QWidget *parent = nullptr);
    ~KAdjustingScrollArea() override;

    QSize minimumSizeHint() const override;

    /*
     * Reimplemented to remove the caching of the size/size hint of the
     * widget and to add the horizontal size hint of the vertical scroll bar
     * unless it is explicitly turned off.
     */
    QSize sizeHint() const override;

    bool event(QEvent *event) override;

private:
    bool eventFilter(QObject *obj, QEvent *ev) override;
};

#endif // KADJUSTINGSCROLLAREA_H
