/*
    This file is part of the KDE Libraries
    SPDX-FileCopyrightText: 1999-2001 Mirko Boehm <mirko@kde.org>
    SPDX-FileCopyrightText: 1999-2001 Espen Sand <espen@kde.org>
    SPDX-FileCopyrightText: 1999-2001 Holger Freyther <freyther@kde.org>
    SPDX-FileCopyrightText: 2005-2006 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2006 Tobias Koenig <tokoe@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KPAGEDIALOG_H
#define KPAGEDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <kpagewidget.h>
#include <memory>

class KPageDialogPrivate;

/*!
 * \class KPageDialog
 * \inmodule KWidgetsAddons
 *
 * \brief A dialog base class which can handle multiple pages.
 *
 * This class provides a dialog base class which handles multiple
 * pages and allows the user to switch between these pages in
 * different ways.
 *
 * Currently, Auto, Plain, List, Tree and Tabbed face
 * types are available (cmp. KPageView).
 *
 * By default a QDialogButtonBox is added to the dialog with two buttons,
 * OK (QDialogButtonBox::Ok) and Cancel (QDialogButtonBox::Cancel).
 * You can customize which buttons are added to the dialog by using any of the
 * available buttons-related methods.
 *
 * Note that if there is a QDialogButtonBox (either the one added by default, or
 * one you added manually) some logical connections are created:
 * \list
 * \li QDialogButtonBox::accepted() is connected to QDialog::accept()
 * \li QDialogButtonBox::rejected() is connected to QDialog::reject()
 * \endlist
 * this means that you shouldn't create these connections again (otherwise you
 * would end up receiving two duplicate accepted() signals for example).
 *
 * Example:
 *
 * \code
 * UrlDialog::UrlDialog( QWidget *parent )
 *   : KPageDialog( parent )
 * {
 *   setFaceType(List);
 *
 *   QLabel *label = new QLabel("Test Page");
 *   addPage(label, i18n("My Test Page"));
 *
 *   label = new QLabel("Second Test Page");
 *   KPageWidgetItem *page = new KPageWidgetItem(label, i18n("My Second Test Page"));
 *   page->setHeader(i18n("My header string"));
 *   page->setIcon(QIcon::fromTheme("file"));
 *
 *   addPage(page);
 *
 *   // Change the buttons added to the dialog
 *   setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel);
 *
 *   // Alternatively you can create a QDialogButtonBox, add the buttons you want to it,
 *   // then add that button box to the dialog
 *   QDialogButtonBox *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel,
 *                                                   Qt::Horizontal,
 *                                                   this);
 *   setButtonBox(btnBox);
 * }
 * \endcode
 */
class KWIDGETSADDONS_EXPORT KPageDialog : public QDialog
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(KPageDialog)

public:
    /*!
     * The face types supported.
     *
     * \value Auto A dialog with a face based on the structure of the available pages. If only a single page is added, the dialog behaves like in Plain mode,
     * with multiple pages without sub pages it behaves like in List mode and like in Tree mode otherwise.
     * \value Plain A normal dialog
     * \value List A dialog with an icon list on the left side and a representation of the contents on the right side
     * \value Tree A dialog with a tree on the left side and a representation of the contents on the right side
     * \value Tabbed A dialog with a tab bar above the representation of the contents
     * \value FlatList A dialog with an flat list with small icons on the left side and a representation of the contents on the right side
     */
    enum FaceType {
        Auto = KPageView::Auto,
        Plain = KPageView::Plain,
        List = KPageView::List,
        Tree = KPageView::Tree,
        Tabbed = KPageView::Tabbed,
        FlatList = KPageView::FlatList,
    };

public:
    /*!
     * Creates a new page dialog.
     */
    explicit KPageDialog(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

    ~KPageDialog() override;

    /*!
     * Sets the face type of the dialog.
     */
    void setFaceType(FaceType faceType);

    /*!
     * Adds a new top level page to the dialog.
     *
     * \a widget The widget of the page.
     *
     * \a name The name which is displayed in the navigation view.
     *
     * Returns the associated KPageWidgetItem.
     */
    KPageWidgetItem *addPage(QWidget *widget, const QString &name);

    /*!
     * Adds a new top level page to the dialog.
     *
     * \a item The KPageWidgetItem which describes the page.
     */
    void addPage(KPageWidgetItem *item);

    /*!
     * Inserts a new page in the dialog.
     *
     * \a before the new page will be insert before this KPageWidgetItem
     *               on the same level in hierarchy.
     *
     * \a widget the widget of the page.
     *
     * \a name the name which is displayed in the navigation view.
     *
     * Returns the associated KPageWidgetItem.
     */
    KPageWidgetItem *insertPage(KPageWidgetItem *before, QWidget *widget, const QString &name);

    /*!
     * Inserts a new page in the dialog.
     *
     * \a before The new page will be insert before this KPageWidgetItem
     *               on the same level in hierarchy.
     *
     * \a item The KPageWidgetItem which describes the page.
     */
    void insertPage(KPageWidgetItem *before, KPageWidgetItem *item);

    /*!
     * Inserts a new sub page in the dialog.
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
     * Inserts a new sub page in the dialog.
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

    /*!
     * Sets the page which is associated with the given KPageWidgetItem to
     * be the current page and emits the currentPageChanged() signal.
     */
    void setCurrentPage(KPageWidgetItem *item);

    /*!
     * Returns the KPageWidgetItem for the current page or a null pointer if there is no
     * current page.
     */
    KPageWidgetItem *currentPage() const;

    /*!
     * Sets the collection of standard buttons displayed by this dialog.
     */
    void setStandardButtons(QDialogButtonBox::StandardButtons buttons);

    /*!
     * Returns the QPushButton corresponding to the standard button which, or a null pointer if the standard
     * button doesn't exist in this dialog.
     */
    QPushButton *button(QDialogButtonBox::StandardButton which) const;

    /*!
     * Set an action button.
     */
    void addActionButton(QAbstractButton *button);

Q_SIGNALS:
    /*!
     * This signal is emitted whenever the current page has changed.
     *
     * \a current The new current page or a null pointer if no current page is available.
     *
     * \a before The page that was current before the new current page has changed.
     */
    void currentPageChanged(KPageWidgetItem *current, KPageWidgetItem *before);

    /*!
     * This signal is emitted whenever a page has been removed.
     *
     * \a page The page which has been removed
     */
    void pageRemoved(KPageWidgetItem *page);

protected:
    /*!
     * This constructor can be used by subclasses to provide a custom page widget.
     *
     * \a widget The KPageWidget object will be reparented to this object, so you can create
     * it without parent and you are not allowed to delete it.
     */
    KPageDialog(KPageWidget *widget, QWidget *parent, Qt::WindowFlags flags = Qt::WindowFlags());
    KWIDGETSADDONS_NO_EXPORT KPageDialog(KPageDialogPrivate &dd, KPageWidget *widget, QWidget *parent, Qt::WindowFlags flags = Qt::WindowFlags());

    /*!
     * Returns the page widget of the dialog or a null pointer if no page widget is set.
     */
    KPageWidget *pageWidget();

    /*!
     * Returns the page widget of the dialog or a null pointer if no page widget is set.
     */
    const KPageWidget *pageWidget() const;

    /*!
     * Set the page widget of the dialog.
     *
     * \note the previous pageWidget will be deleted.
     *
     * \a widget The KPageWidget object will be reparented to this object, so you can create
     * it without parent and you are not allowed to delete it.
     */
    void setPageWidget(KPageWidget *widget);

    /*!
     * Returns the button box of the dialog or a null pointer if no button box is set.
     */
    QDialogButtonBox *buttonBox();

    /*!
     * Returns the button box of the dialog or a null pointer if no button box is set.
     */
    const QDialogButtonBox *buttonBox() const;

    /*!
     * Set the button box of the dialog
     *
     * \note the previous buttonBox will be deleted.
     *
     * \a box The QDialogButtonBox object will be reparented to this object, so you can create
     * it without parent and you are not allowed to delete it.
     */
    void setButtonBox(QDialogButtonBox *box);

protected:
    std::unique_ptr<class KPageDialogPrivate> const d_ptr;
};

#endif
