/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Urs Wolfer <uwolfer at kde.org>

    Parts of this class have been take from the KAboutApplication class, which was:
    SPDX-FileCopyrightText: 2000 Waldo Bastian <bastian@kde.org>
    SPDX-FileCopyrightText: 2000 Espen Sand <espen@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KABOUT_APPLICATION_DIALOG_H
#define KABOUT_APPLICATION_DIALOG_H

#include <QDialog>
#include <memory>

#include <kxmlgui_export.h>

class KAboutData;

/*!
 * \class KAboutApplicationDialog
 * \inmodule KXmlGui
 *
 * \brief Standard "About Application" dialog box.
 *
 * This class provides the standard "About Application" dialog box
 * that is used by KHelpMenu. It uses the information of the global
 * KAboutData that is specified at the start of your program in
 * main(). Normally you should not use this class directly but rather
 * the KHelpMenu class or even better just subclass your toplevel
 * window from KMainWindow. If you do the latter, the help menu and
 * thereby this dialog box is available through the
 * KMainWindow::helpMenu() function.
 *
 * \image kaboutapplicationdialog.png "KAboutApplicationDialog"
 */

class KXMLGUI_EXPORT KAboutApplicationDialog : public QDialog
{
    Q_OBJECT
public:
    /*!
     * \enum KAboutApplicationDialog::Option
     *
     * Defines some options that can be applied to the about dialog.
     *
     * \value NoOptions
     *        Shows the standard about dialog.
     * \value HideTranslators
     *        Hides the translators tab.
     * \value HideLibraries
     *        Since 5.77, hides the libraries tab.
     *        Since 5.87, hides the components tab (which replaces the libraries tab).
     * \since 4.4
     */
    enum Option {
        NoOptions = 0x0,
        HideTranslators = 0x1,
        HideLibraries = 0x2,
    };
    Q_DECLARE_FLAGS(Options, Option)
    Q_FLAG(Options)

    /*!
     * Constructs a fully featured "About Application" dialog box
     * using existing \a aboutData and the specified \a opts.
     *
     * You should set the toplevel window as the parent
     * so that the dialog becomes centered.
     *
     * \sa Options
     *
     * \since 4.4
     */
    explicit KAboutApplicationDialog(const KAboutData &aboutData, Options opts, QWidget *parent = nullptr);

    /*!
     * \brief Constructs a fully featured "About Application" dialog box
     * using existing \a aboutData.
     *
     * You should set the toplevel window as the \a parent
     * so that the dialog becomes centered.
     */
    explicit KAboutApplicationDialog(const KAboutData &aboutData, QWidget *parent = nullptr);

    ~KAboutApplicationDialog() override;

private:
    std::unique_ptr<class KAboutApplicationDialogPrivate> const d;

    Q_DISABLE_COPY(KAboutApplicationDialog)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KAboutApplicationDialog::Options)

#endif
