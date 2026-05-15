/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2007 Chusslove Illich <caslav.ilic@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KTRANSCRIPT_P_H
#define KTRANSCRIPT_P_H

#include <QHash>
#include <QList>
#include <QStringList>
#include <QVariant>

/*!
 * \internal
 * (used by KLocalizedString)
 *
 * Class for supporting programmable translations
 *
 * KTranscript provides support for programmable translations.
 * The class is abstract in order to facilitate dynamic loading.
 */
class KTranscript
{
public:
    /*!
     * Evaluates interpolation.
     *
     * \a argv list of interpolation tokens
     * \a lang language of the translation
     * \a ctry locale country
     * \a msgctxt message context
     * \a dynctxt dynamic context
     * \a msgid original message
     * \a subs substitutions for message placeholders
     * \a vals values that were formatted to substitutions
     * \a ftrans finalized ordinary translation
     * \a mods scripting modules to load; the list is cleared after loading
     * \a error set to the message detailing the problem, if the script
                    failed; set to empty otherwise
     * \a fallback set to true if the script requested fallback to ordinary
                       translation; set to false otherwise
     * Returns resolved interpolation if evaluation succeeded,
     *         empty string otherwise
     */
    virtual QString eval(const QList<QVariant> &argv,
                         const QString &lang,
                         const QString &ctry,
                         const QString &msgctxt,
                         const QHash<QString, QString> &dynctxt,
                         const QString &msgid,
                         const QStringList &subs,
                         const QList<QVariant> &vals,
                         const QString &ftrans,
                         QList<QStringList> &mods,
                         QString &error,
                         bool &fallback) = 0;

    /*!
     * Returns the list of calls to execute an all messages after the
     * interpolations are done, as evaluations with no parameters.
     *
     * \a lang language of the translation
     * Returns list of post calls
     */
    virtual QStringList postCalls(const QString &lang) = 0;

    /*!
     * Destructor.
     */
    virtual ~KTranscript()
    {
    }
};

#ifdef KTRANSCRIPT_TESTBUILD
KTranscript *autotestCreateKTranscriptImp();
void autotestDestroyKTranscriptImp();
#endif

#endif
