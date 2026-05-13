/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KSHORTCUTWIDGET_H
#define KSHORTCUTWIDGET_H

#include <kxmlgui_export.h>

#include <QKeySequence>
#include <QList>
#include <QWidget>
#include <memory>

class KActionCollection;
class KShortcutWidgetPrivate;

/*!
 * \class KShortcutWidget
 * \inmodule KXmlGui
 *
 * Provides a widget that lets the user assign a main shortcut
 * and an alternate shortcut for a certain action.
 *
 * \image kshortcutwidget.png "KShortcutWidget"
 */
class KXMLGUI_EXPORT KShortcutWidget : public QWidget
{
    Q_OBJECT

    /*!
     * \property KShortcutWidget::modifierlessAllowed
     */
    Q_PROPERTY(bool modifierlessAllowed READ isModifierlessAllowed WRITE setModifierlessAllowed)
public:
    /*!
     * \brief Creates a new shortcut widget as a child of \a parent.
     */
    explicit KShortcutWidget(QWidget *parent = nullptr);
    /*!
     * \brief Destructs the shortcut widget.
     */
    ~KShortcutWidget() override;

    /*!
     * \brief Sets whether to \a allow a shortcut
     * that doesn't include a modifier key.
     */
    void setModifierlessAllowed(bool allow);

    /*!
     * \brief Returns whether the set widget shortcut
     * can be set without including a modifier key.
     */
    bool isModifierlessAllowed();

    /*!
     * \brief Sets whether to \a show the clear buttons next to the
     * main and alternate shortcuts of the widget.
     */
    void setClearButtonsShown(bool show);

    /*!
     * \brief Returns the set shortcut.
     */
    QList<QKeySequence> shortcut() const;

    /*!
     * \brief Sets a list of \a actionCollections to check against
     * for a conflictuous shortcut.
     *
     * If there is a conflictuous shortcut with a QAction,
     * and that this shortcut can be configured
     * (that is, KActionCollection::isShortcutConfigurable() returns \c true)
     * the user will be prompted to eventually steal the shortcut from this action.
     *
     * Global shortcuts are automatically checked for conflicts.
     *
     * Don't forget to call applyStealShortcut() to actually steal the shortcut.
     *
     * \since 4.1
     */
    void setCheckActionCollections(const QList<KActionCollection *> &actionCollections);

Q_SIGNALS:
    /*!
     * \brief Emitted when the given shortcut \a cut has changed.
     */
    void shortcutChanged(const QList<QKeySequence> &cut);

public Q_SLOTS:
    /*!
     * \brief Sets the given shortcut \a cut to the widget.
     */
    void setShortcut(const QList<QKeySequence> &cut);
    /*!
     * \brief Unassigns the widget's shortcut.
     */
    void clearShortcut();

    /*!
     * \brief Actually remove the shortcut that the user wanted to steal,
     * from the action that was using it.
     *
     * To be called before you apply your changes.
     * No shortcuts are stolen until this function is called.
     */
    void applyStealShortcut();

private:
    friend class KShortcutWidgetPrivate;
    std::unique_ptr<KShortcutWidgetPrivate> const d;
};

#endif // KSHORTCUTWIDGET_H
