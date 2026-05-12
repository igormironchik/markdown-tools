/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2001 Hans Petter Bieker <bieker@kde.org>
    SPDX-FileCopyrightText: 2012, 2013 Chusslove Illich <caslav.ilic@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCATALOG_H
#define KCATALOG_H

#include "ki18n_export.h"

#include <QByteArray>
#include <QSet>
#include <QString>
#include <memory>

class KCatalogPrivate;

/*!
 * This class abstracts a Gettext message catalog.
 * It takes care of needed Gettext bindings.
 *
 * \sa KLocalizedString
 * \internal exported only for use in KI18nLocaleData.
 */
class KI18N_EXPORT KCatalog
{
public:
    /*!
     * Constructor.
     *
     * \a name translation domain
     *
     * \a language translation language
     */
    KCatalog(const QByteArray &domain, const QString &language);

    /*!
     * Destructor.
     */
    ~KCatalog();

    /*!
     * Get translation of the given message text.
     *
     * Do not pass empty message text.
     *
     * \a msgid message text
     *
     * Returns translated message if found, QString() otherwise
     */
    QString translate(const QByteArray &msgid) const;

    /*!
     * Get translation of the given message text with given context.
     *
     * Do not pass empty message text.
     *
     * \a msgctxt message context
     *
     * \a msgid message text
     *
     * Returns translated message if found, QString() otherwise
     */
    QString translate(const QByteArray &msgctxt, const QByteArray &msgid) const;

    /*!
     * Get translation of given message with plural forms.
     *
     * Do not pass empty message text.
     *
     * \a msgid singular message text
     *
     * \a msgid_plural plural message text
     *
     * \a n number for which the plural form is needed
     *
     * Returns translated message if found, QString() otherwise
     */
    QString translate(const QByteArray &msgid, const QByteArray &msgid_plural, qulonglong n) const;

    /*!
     * Get translation of given message with plural forms with given context.
     *
     * Do not pass empty message text.
     *
     * \a msgctxt message context
     *
     * \a msgid singular message text
     *
     * \a msgid_plural plural message text
     *
     * \a n number for which the plural form is needed
     *
     * Returns translated message if found, QString() otherwise
     */
    QString translate(const QByteArray &msgctxt, const QByteArray &msgid, const QByteArray &msgid_plural, qulonglong n) const;

    /*!
     * Find the locale directory for the given domain in the given language.
     *
     * \a domain translation domain
     *
     * \a language language of the catalog
     *
     * Returns the locale directory if found, QByteArray() otherwise.
     */
    static QString catalogLocaleDir(const QByteArray &domain, const QString &language);

    /*!
     * Find the all languages for which the translation catalog
     * of given domain exists.
     *
     * \a domain translation domain
     *
     * Returns set of language codes
     */
    static QSet<QString> availableCatalogLanguages(const QByteArray &domain);

    static void addDomainLocaleDir(const QByteArray &domain, const QString &path);

private:
    Q_DISABLE_COPY(KCatalog)

    std::unique_ptr<KCatalogPrivate> const d;
};

#endif
