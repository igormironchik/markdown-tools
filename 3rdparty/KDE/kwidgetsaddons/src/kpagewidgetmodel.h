/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 2006 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPAGEWIDGETMODEL_H
#define KPAGEWIDGETMODEL_H

#include "kpagemodel.h"
#include <memory>

class QWidget;
class QAction;

/*!
 * \class KPageWidgetItem
 * \inmodule KWidgetsAddons
 * \inheaderfile KPageWidgetModel
 *
 * \brief KPageWidgetItem is used by KPageWidget and represents
 * a page.
 *
 * Example:
 *
 * \code
 *  ColorPage *page = new ColorPage;
 *
 *  KPageWidgetItem *item = new KPageWidgetItem( page, i18n( "Colors" ) );
 *  item->setHeader( i18n( "Colors of Main Window" ) );
 *  item->setIcon( QIcon::fromTheme( "colors" ) );
 *
 *  KPageWidget *pageWidget = new KPageWidget( this );
 *  pageWidget->addPage( item );
 * \endcode
 */
class KWIDGETSADDONS_EXPORT KPageWidgetItem : public QObject
{
    Q_OBJECT

    /*!
     * \property KPageWidgetItem::name
     */
    Q_PROPERTY(QString name READ name WRITE setName)

    /*!
     * \property KPageWidgetItem::header
     */
    Q_PROPERTY(QString header READ header WRITE setHeader)

    /*!
     * \property KPageWidgetItem::icon
     */
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)

    /*!
     * \property KPageWidgetItem::checkable
     */
    Q_PROPERTY(bool checkable READ isCheckable WRITE setCheckable)

    /*!
     * \property KPageWidgetItem::checked
     */
    Q_PROPERTY(bool checked READ isChecked WRITE setChecked)

    /*!
     * \property KPageWidgetItem::enabled
     *
     * This property holds whether the item is enabled.
     *
     * It dis-/enables both the widget and the item in the list-/treeview.
     */
    Q_PROPERTY(bool enabled READ isEnabled WRITE setEnabled)

    /*!
     * \property KPageWidgetItem::headerVisible
     * \since 5.52
     */
    Q_PROPERTY(bool headerVisible READ isHeaderVisible WRITE setHeaderVisible)

    /*!
     * \property KPageWidgetItem::actions
     *
     * This property holds the actions associated to the page.
     *
     * \warning This is not supported when using a KPageView/KPageWidget/KPageDialog with
     * the Tabbed face type.
     * \since 6.6
     */
    Q_PROPERTY(QList<QAction *> actions READ actions WRITE setActions NOTIFY actionsChanged)
public:
    /*!
     * Creates a new page widget item.
     *
     * \a widget The widget that is shown as page in the KPageWidget.
     */
    KPageWidgetItem(QWidget *widget);

    /*!
     * Creates a new page widget item.
     *
     * \a widget The widget that is shown as page in the KPageWidget.
     *
     * \a name The localized string that is show in the navigation view
     *             of the KPageWidget.
     */
    KPageWidgetItem(QWidget *widget, const QString &name);

    ~KPageWidgetItem() override;

    /*!
     * Returns the widget of the page widget item.
     */
    QWidget *widget() const;

    /*!
     * Sets the name of the item as shown in the navigation view of the page
     * widget.
     */
    void setName(const QString &name);

    /*!
     * Returns the name of the page widget item.
     */
    QString name() const;

    /*!
     * Sets the header of the page widget item.
     *
     * If setHeader(QString()) is used, what is the default if the header
     * does not got set explicit, then the defined name() will also be used
     * for the header.
     *
     * \a header Header of the page widget item.
     */
    void setHeader(const QString &header);

    /*!
     * Returns the header of the page widget item.
     */
    QString header() const;

    /*!
     * Sets the icon of the page widget item.
     * \a icon Icon of the page widget item.
     */
    void setIcon(const QIcon &icon);

    /*!
     * Returns the icon of the page widget item.
     */
    QIcon icon() const;

    /*!
     * Sets whether the page widget item is checkable in the view.
     * \a checkable True if the page widget is checkable,
     *                  otherwise false.
     */
    void setCheckable(bool checkable);

    /*!
     * Returns whether the page widget item is checkable.
     */
    bool isCheckable() const;

    /*!
     * Returns whether the page widget item is checked.
     */
    bool isChecked() const;

    /*!
     * Returns whether the page widget item is enabled.
     */
    bool isEnabled() const;

    /*!
     * Returns whether the page will show the header title
     * \since 5.52
     */
    bool isHeaderVisible() const;

    /*!
     * Set whether the page should show the header title
     * \since 5.52
     */
    void setHeaderVisible(bool visible);

    /*!
     * Returns the actions associated to the page.
     * \since 6.6
     */
    QList<QAction *> actions() const;

    /*!
     * Set the actions associated to the page.
     * \since 6.6
     */
    void setActions(QList<QAction *> actions);

public Q_SLOTS:
    /*!
     * Sets whether the page widget item is enabled.
     */
    void setEnabled(bool);

    /*!
     * Sets whether the page widget item is checked.
     */
    void setChecked(bool checked);

Q_SIGNALS:
    /*!
     * This signal is emitted whenever the icon or header
     * is changed.
     */
    void changed();

    /*!
     * This signal is emitted whenever the user checks or
     * unchecks the item of setChecked() is called.
     */
    void toggled(bool checked);

    /*!
     * This signal is emitted whenever the actions associated to the
     * page are changed.
     * \since 6.6
     */
    void actionsChanged();

private:
    std::unique_ptr<class KPageWidgetItemPrivate> const d;
};

class KPageWidgetModelPrivate;

/*!
 * \class KPageWidgetModel
 * \inmodule KWidgetsAddons
 *
 * \brief This page model is used by KPageWidget to provide
 * a hierarchical layout of pages.
 */
class KWIDGETSADDONS_EXPORT KPageWidgetModel : public KPageModel
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KPageWidgetModel)

public:
    /*!
     * Creates a new page widget model.
     *
     * \a parent The parent object.
     */
    explicit KPageWidgetModel(QObject *parent = nullptr);

    /*!
     * Destroys the page widget model.
     */
    ~KPageWidgetModel() override;

    /*!
     * Adds a new top level page to the model.
     *
     * \a widget The widget of the page.
     *
     * \a name The name which is displayed in the navigation view.
     *
     * Returns the associated KPageWidgetItem.
     */
    KPageWidgetItem *addPage(QWidget *widget, const QString &name);

    /*!
     * Adds a new top level page to the model.
     *
     * \a item The KPageWidgetItem which describes the page.
     */
    void addPage(KPageWidgetItem *item);

    /*!
     * Inserts a new page in the model.
     *
     * \a before The new page will be insert before this KPageWidgetItem
     *               on the same level in hierarchy.
     * \a widget The widget of the page.
     *
     * \a name The name which is displayed in the navigation view.
     *
     * Returns the associated KPageWidgetItem.
     */
    KPageWidgetItem *insertPage(KPageWidgetItem *before, QWidget *widget, const QString &name);

    /*!
     * Inserts a new page in the model.
     *
     * \a before The new page will be insert before this KPageWidgetItem
     *               on the same level in hierarchy.
     *
     * \a item The KPageWidgetItem which describes the page.
     */
    void insertPage(KPageWidgetItem *before, KPageWidgetItem *item);

    /*!
     * Inserts a new sub page in the model.
     *
     * \a parent The new page will be insert as child of this KPageWidgetItem.
     *
     * \a widget The widget of the page.
     *
     * \a name The name which is displayed in the navigation view.
     *
     * Returns the associated KPageWidgetItem.
     */
    KPageWidgetItem *addSubPage(KPageWidgetItem *parent, QWidget *widget, const QString &name);

    /*!
     * Inserts a new sub page in the model.
     *
     * \a parent The new page will be insert as child of this KPageWidgetItem.
     *
     * \a item The KPageWidgetItem which describes the page.
     */
    void addSubPage(KPageWidgetItem *parent, KPageWidgetItem *item);

    /*!
     * Removes the page associated with the given KPageWidgetItem.
     */
    void removePage(KPageWidgetItem *item);

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    /*!
     * Returns the KPageWidgetItem for a given index or a null pointer if the index is invalid.
     */
    KPageWidgetItem *item(const QModelIndex &index) const;

    /*!
     * Returns the index for a given KPageWidgetItem. The index is invalid if the
     * item can't be found in the model.
     */
    QModelIndex index(const KPageWidgetItem *item) const;

Q_SIGNALS:
    /*!
     * This signal is emitted whenever a checkable page changes its state. \a checked is true
     * when the \a page is checked, or false if the \a page is unchecked.
     */
    void toggled(KPageWidgetItem *page, bool checked);

private:
    Q_PRIVATE_SLOT(d_func(), void _k_itemChanged())
    Q_PRIVATE_SLOT(d_func(), void _k_itemToggled(bool))
};

#endif
