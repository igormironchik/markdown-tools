/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 1999 David Faure <faure@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KBUGREPORT_H
#define KBUGREPORT_H

#include <QDialog>
#include <kxmlgui_export.h>
#include <memory>

class KAboutData;
class KBugReportPrivate;

/*!
 * \class KBugReport
 * \inmodule KXmlGui
 *
 * \brief A dialog box for sending bug reports.
 *
 * All the information needed by the dialog box
 * (program name, version, bug-report address, etc.)
 * comes from the KAboutData class.
 * Make sure you create an instance of KAboutData and call
 * KAboutData::setApplicationData(<aboutData>).
 *
 * \image kbugreport.png "KBugReport"
 */
class KXMLGUI_EXPORT KBugReport : public QDialog
{
    Q_OBJECT

public:
    /*!
     * \brief Creates a bug-report dialog using information derived from
     * \a aboutData as a child of \a parent.
     *
     * Note that you shouldn't have to do this manually,
     * since KHelpMenu takes care of the menu item
     * for "Report Bug..." and of creating a KBugReport dialog.
     */
    explicit KBugReport(const KAboutData &aboutData, QWidget *parent = nullptr);

    /*!
     * \brief Destructor.
     */
    ~KBugReport() override;

    /*!
     * \brief OK has been clicked.
     */
    void accept() override;

protected:
    /*!
     * \brief Attempt to e-mail the bug report.
     *
     * Returns true on success.
     */
    bool sendBugReport();

private:
    friend class KBugReportPrivate;
    std::unique_ptr<KBugReportPrivate> const d;

    Q_DISABLE_COPY(KBugReport)
};

#endif
