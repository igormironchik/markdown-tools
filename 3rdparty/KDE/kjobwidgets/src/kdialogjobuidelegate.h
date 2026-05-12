/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 Stephan Kulow <coolo@kde.org>
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2006, 2007 Kevin Ottens <ervin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KDIALOGJOBUIDELEGATE_H
#define KDIALOGJOBUIDELEGATE_H

#include <KJobUiDelegate>
#include <kjobwidgets_export.h>

#include <memory>

/*!
 * \class KDialogJobUiDelegate
 *
 * \inmodule KJobWidgets
 *
 * \brief A UI delegate using KMessageBox for interaction (showing errors and warnings).
 *
 * The KMessageBox will use window as a parent in an application-modal.
 */
class KJOBWIDGETS_EXPORT KDialogJobUiDelegate : public KJobUiDelegate
{
    Q_OBJECT

public:
    /*!
     * Constructs a new KDialogJobUiDelegate.
     */
    KDialogJobUiDelegate();

    /*!
     * Constructs a new KDialogJobUiDelegate.
     *
     * \a flags allows to enable automatic error/warning handling
     *
     * \a window the window associated with this delegate, see setWindow.
     *
     * \since 5.70
     */
    explicit KDialogJobUiDelegate(KJobUiDelegate::Flags flags, QWidget *window);

    ~KDialogJobUiDelegate() override;

public:
    bool setJob(KJob *job) override;

    /*!
     * Associate this delegate with a window given by \a window.
     *
     * Needed for dialog boxes to respect stacking order, centering to parent, focus going back to parent after closing...
     *
     * \a window the window to associate to
     *
     * \sa window()
     */
    virtual void setWindow(QWidget *window);

    /*!
     * Returns the window this delegate is associated with.
     *
     * \sa setWindow()
     */
    QWidget *window() const;

    /*!
     * Updates the last user action timestamp to the given time.
     */
    void updateUserTimestamp(unsigned long time);

    /*!
     * \internal
     */
    unsigned long userTimestamp() const;

    void showErrorMessage() override;

protected Q_SLOTS:
    void slotWarning(KJob *job, const QString &message) override;

private:
    std::unique_ptr<class KDialogJobUiDelegatePrivate> const d;
};

#endif
