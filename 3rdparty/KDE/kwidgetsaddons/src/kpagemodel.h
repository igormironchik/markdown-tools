/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2006 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPAGEMODEL_H
#define KPAGEMODEL_H

#include <kwidgetsaddons_export.h>

#include <QAbstractItemModel>
#include <memory>

class KPageModelPrivate;

/*!
 * \class KPageModel
 * \inmodule KWidgetsAddons
 *
 * \brief A base class for a model used by KPageView.
 *
 * This class is an abstract base class which must be used to
 * implement custom models for KPageView. Additional to the standard
 * Qt::ItemDataRoles it provides the two roles
 * \list
 * \li HeaderRole
 * \li HeaderVisibleRole
 * \li WidgetRole
 * \endlist
 *
 * which are used to return a header string for a page and a QWidget
 * pointer to the page itself.
 *
 * Example:
 *
 * \code
 *   KPageView *view = new KPageView( this );
 *   KPageModel *model = new MyPageModel( this );
 *
 *   view->setModel( model );
 * \endcode
 *
 * \sa KPageView
 */
class KWIDGETSADDONS_EXPORT KPageModel : public QAbstractItemModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KPageModel)

public:
    /*!
     * Additional roles that KPageView uses.
     *
     * \value HeaderRole A string to be rendered as page header.
     * \value WidgetRole A pointer to the page widget. This is the widget that is shown when the item is selected.
     * You can make QVariant take a QWidget using
     * \code
     * QWidget *myWidget = new QWidget;
     * QVariant v = QVariant::fromValue(myWidget);
     * \endcode
     * \value[since 5.52] When true, show the page header, if false don't
     * \value[since 6.6] The list of actions associated to the page
     */
    enum Role {
        HeaderRole = Qt::UserRole + 1,
        WidgetRole,
        HeaderVisibleRole,
        ActionsRole,
    };

    /*!
     * Constructs a page model with the given parent.
     */
    explicit KPageModel(QObject *parent = nullptr);

    ~KPageModel() override;

protected:
    KWIDGETSADDONS_NO_EXPORT KPageModel(KPageModelPrivate &dd, QObject *parent);

    std::unique_ptr<class KPageModelPrivate> const d_ptr;
};

#endif
