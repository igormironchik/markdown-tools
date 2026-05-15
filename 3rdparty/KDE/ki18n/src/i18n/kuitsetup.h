/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2013 Chusslove Illich <caslav.ilic@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KUITSETUP_H
#define KUITSETUP_H

#include <ki18n_export.h>

#include <QHash>
#include <QString>
#include <QStringList>

class KuitSetup;

/*!
 * \namespace Kuit
 * \inmodule KI18n
 *
 * \brief Global constants and functions related to KUIT markup.
 */
namespace Kuit
{
/*!
 * Visual formats into which KUIT markup can be resolved.
 *
 * \value UndefinedFormat format not defined. This value can be explicitly set (e.g. through KLocalizedString::withFormat) to indicate that the format should be
 * decided by another mechanism (e.g. context UI marker).
 * \value PlainText Plain Text
 * \value RichText Qt rich text (HTML subset).
 * \value TermText Terminal escape sequences.
 */
enum VisualFormat {
    UndefinedFormat = 0,
    PlainText = 10,
    RichText = 20,
    TermText = 30,
};

/*!
 * Classification of KUIT tags.
 *
 * \value PhraseTag Tags wrapping text inserted into running text.
 * \value StructTag Tags splitting text into paragraph-level blocks
 */
enum TagClass {
    PhraseTag = 0,
    StructTag = 1,
};

/*!
 * Functions accepted by tag formatting functions.
 *
 * \a languages the target languages (by decreasing priority)
 *
 * \a tagName the wrapping tag name
 *
 * \a attributes the attribute name-value pairs in the tag
 *
 * \a text the wrapped text
 *
 * \a tagPath the ordered list of ancestor tag names, parent first
 *
 * \a format the target visual format
 *
 * Returns formatted text
 */
typedef QString (*TagFormatter)(const QStringList &languages,
                                const QString &tagName,
                                const QHash<QString, QString> &attributes,
                                const QString &text,
                                const QStringList &tagPath,
                                Kuit::VisualFormat format);

/*!
 * Get hold of the KUIT setup object for a given domain.
 *
 * \a domain the translation domain
 *
 * Returns pointer to KUIT setup object
 */
KI18N_EXPORT KuitSetup &setupForDomain(const QByteArray &domain);
}

class KLocalizedString;
class KuitSetupPrivate;
class KuitFormatterPrivate;

/*!
 * \class KuitSetup
 * \inmodule KI18n
 *
 * \brief Class for modifying KUIT markup in a given domain.
 *
 * Not directly constructed, but obtained through Kuit::setupForDomain.
 */
class KI18N_EXPORT KuitSetup
{
    friend KuitSetup &Kuit::setupForDomain(const QByteArray &domain);
    friend class KuitFormatterPrivate;

public:
    ~KuitSetup();

    /*!
     * Set the formatting string for a tag with attributes combination.
     *
     * If a new tag name is given, this effectively defines a new tag.
     * The same holds for attribute names.
     *
     * The pattern string \a pattern should contain placeholders
     * for inserting the text and the attribute values.
     * %1 will be replaced with the wrapped text, and %2 and upwards
     * with attribute values in the order given by \a attribNames.
     * Non markup-aware translation call with context (ki18nc)
     * should be used to create the pattern string.
     *
     * In addition to the pattern, a formatting function
     * of the type TagFormatter can be given.
     * This function receives the full markup parsing context,
     * so that it can do whatever is necessary with the wrapped text.
     * The result of this function is then substituted into the pattern.
     * You can also give an empty pattern (as KLocalizedString())
     * together with the formatting function, in which case the function
     * is assumed to do everything and no substitution is performed.
     *
     * \a tagName the name of the tag
     *
     * \a attribNames the names of the attributes (empty names are ignored)
     *
     * \a format the target visual format
     *
     * \a pattern the pattern string
     *
     * \a leadingNewlines the number of new lines (\\n) to be maintained
     *                        between any preceding text and the text wrapped
     *                        with this tag (for formats where it matters)
     */
    void setTagPattern(const QString &tagName,
                       const QStringList &attribNames,
                       Kuit::VisualFormat format,
                       const KLocalizedString &pattern,
                       Kuit::TagFormatter formatter = nullptr,
                       int leadingNewlines = 0);

    /*!
     * Set the KUIT class of the tag.
     *
     * \a tagName the name of the tag
     *
     * \a aClass the KUIT tag class
     */
    void setTagClass(const QString &tagName, Kuit::TagClass aClass);

    /*!
     * Set the default visual format for a given UI marker.
     *
     * Giving "@<major>" for \a marker means to set the format
     * only for standalone @<major> marker,
     * while "@<major>:" (with trailing colon) means to set
     * the same format for all @<major>:<minor> combinations.
     *
     * Defined UI marker major/minor combinations are listed in the section
     * uimark_ctxt. If an UI marker combination outside of the defined
     * is given as \a marker, it will be ignored.
     *
     * Setting Kuit::UndefinedFormat as \a format
     * means to fall back to default format for the given UI marker.
     *
     * \a marker the UI marker
     *
     * \a format the visual format
     */
    void setFormatForMarker(const QString &marker, Kuit::VisualFormat format);

private:
    KI18N_NO_EXPORT explicit KuitSetup(const QByteArray &domain);
    Q_DISABLE_COPY(KuitSetup)

    // intentionally not a unique_ptr as this file gets included a lot and using a unique_ptr
    // results in too many template instantiations
    KuitSetupPrivate *const d;
};

#endif // KUITSETUP_H
