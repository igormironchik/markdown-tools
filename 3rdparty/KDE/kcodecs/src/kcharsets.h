/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Lars Knoll <knoll@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KCHARSETS_H
#define KCHARSETS_H

#include <kcodecs_export.h>

#include <QCoreApplication>
#include <QList>
#include <QStringList>
#include <memory>

class KCharsetsPrivate;

class QChar;
class QString;

/*!
 * \class KCharsets
 * \inmodule KCodecs
 *
 * \brief Charset font and encoder/decoder handling.
 *
 * This is needed, because Qt's encoding name matching in
 * QTextCodec::codecForName() matches only closely-related encoded names
 * but not alternate names, e.g. found in the reality of the Internet.
 */
class KCODECS_EXPORT KCharsets final
{
    Q_DECLARE_TR_FUNCTIONS(KCharsets)

protected:
    /*
     * Protected constructor. If you need the kcharsets object, use
     * KCharsets::charsets() instead.
     */
    constexpr KCharsets();

public:
    ~KCharsets();

    /*!
     * The global charset manager.
     */
    static KCharsets *charsets();

    /*!
     * Converts an entity to a character.
     *
     * The string must contain only the entity \e {without}
     * the trailing ';'. For numeric character references, the
     * leading '&' \e {may} be included.
     *
     * \a str the entity
     *
     * Returns QChar::Null if the entity could not be decoded.
     */
    static QChar fromEntity(QStringView str);

    /*!
     * Tries to find an entity in the
     * QString str.
     *
     * \a str the string containing entified
     *
     * \a len is a return value, that gives the length of the decoded
     * entity.
     *
     * Returns a decoded entity if one could be found, QChar::null
     * otherwise
     *
     * \overload fromEntity(QStringView)
     */
    static QChar fromEntity(QStringView str, int &len);

    /*!
     * Converts a QChar to an entity. The returned string does already
     * contain the leading '&' and the trailing ';'.
     *
     * \a ch the char to convert
     *
     * Returns the entity
     */
    static QString toEntity(const QChar &ch);

    /*!
     * Scans the given string for entities (like &amp;amp;) and resolves them
     * using fromEntity.
     *
     * \a text the string containing the entities
     *
     * Returns the clean string
     */
    static QString resolveEntities(const QString &text);

    /*!
     * Lists all available encodings as names
     */
    QStringList availableEncodingNames() const;

    /*!
     * Lists the available encoding names together with a more descriptive language
     */
    QStringList descriptiveEncodingNames() const;

    /*!
     * Lists the available encoding names grouped by script (or language that uses them).
     *
     * Returns the list of lists consisting of description followed by encoding names (i.e. encodingsByScript().at(i).at(0) is a description for
     * encodingsByScript().at(i).at(k), k>0)
     */
    QList<QStringList> encodingsByScript() const;

    /*!
     * Returns a long description for an encoding name.
     *
     * \a encoding the encoding for the language
     *
     */
    QString descriptionForEncoding(QStringView encoding) const;

    /*!
     * Returns the encoding for a string obtained with descriptiveEncodingNames().
     *
     * \a descriptiveName the descriptive name for the encoding
     */
    QString encodingForName(const QString &descriptiveName) const;

private:
    std::unique_ptr<KCharsetsPrivate> const d;
    friend struct KCharsetsSingletonPrivate;
};

#endif
