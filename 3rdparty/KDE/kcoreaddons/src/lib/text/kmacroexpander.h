/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2002-2003 Oswald Buddenhagen <ossi@kde.org>
    SPDX-FileCopyrightText: 2003 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KMACROEXPANDER_H
#define KMACROEXPANDER_H

#include <QChar>
#include <QStringList>

#include <kcoreaddons_export.h>
#include <memory>

class QString;
template<typename KT, typename VT>
class QHash;
class KMacroExpanderBasePrivate;

/*!
 * \class KMacroExpanderBase
 * \inmodule KCoreAddons
 * \inheaderfile KMacroExpander
 *
 * \brief Abstract base class for the worker classes behind the KMacroExpander namespace
 * and the KCharMacroExpander and KWordMacroExpander classes.
 *
 */
class KCOREADDONS_EXPORT KMacroExpanderBase
{
public:
    /*!
     * Constructor.
     * \a c escape char indicating start of macros, or QChar::null for none
     */
    explicit KMacroExpanderBase(QChar c = QLatin1Char('%'));

    virtual ~KMacroExpanderBase();

    /*!
     * Perform safe macro expansion (substitution) on a string.
     *
     * \a str the string in which macros are expanded in-place
     */
    void expandMacros(QString &str);

    // TODO: This documentation is relevant for end-users. Where to put it?
    /*!
     * Perform safe macro expansion (substitution) on a string for use
     * in shell commands.
     *
     * *NIX notes:
     *
     * Explicitly supported shell constructs:
     *   \ '' "" $'' $"" {} () $(()) ${} $() ``
     *
     * Implicitly supported shell constructs:
     *   (())
     *
     * Unsupported shell constructs that will cause problems:
     *  Shortened \tt{case $v in pat)} syntax. Use
     *   \tt{case $v in (pat)} instead.
     *
     * The rest of the shell (incl. bash) syntax is simply ignored,
     * as it is not expected to cause problems.
     *
     * Note that bash contains a bug which makes macro expansion within
     * double quoted substitutions (\tt{"${VAR:-%macro}"}) inherently
     * insecure.
     *
     * For security reasons, \e never put expandos in command line arguments
     * that are shell commands by themselves -
     * \tt{sh -c 'foo \%f'} is taboo.
     * \tt{file=\%f sh -c 'foo "$file"'} is OK.
     *
     * Windows notes:
     *
     * All quoting syntax supported by KShell is supported here as well.
     * Additionally, command grouping via parentheses is recognized - note
     * however, that the parser is much stricter about unquoted parentheses
     * than cmd itself.
     * The rest of the cmd syntax is simply ignored, as it is not expected
     * to cause problems - do not use commands that embed other commands,
     * though - \tt{for /f ...} is taboo.
     *
     * \a str the string in which macros are expanded in-place
     *
     * \a pos the position inside the string at which parsing/substitution
     *  should start, and upon exit where processing stopped
     *
     * Returns false if the string could not be parsed and therefore no safe
     *  substitution was possible. Note that macros will have been processed
     *  up to the point where the error occurred. An unmatched closing paren
     *  or brace outside any shell construct is \e not an error (unlike in
     *  the function below), but still prematurely terminates processing.
     */
    bool expandMacrosShellQuote(QString &str, int &pos);

    /*!
     * Same as above, but always starts at position 0, and unmatched closing
     * parens and braces are treated as errors.
     */
    bool expandMacrosShellQuote(QString &str);

    /*!
     * Set the macro escape character.
     *
     * \a c escape char indicating start of macros, or QChar::null if none
     */
    void setEscapeChar(QChar c);

    /*!
     * Obtain the macro escape character.
     * Returns escape char indicating start of macros, or QChar::null if none
     */
    QChar escapeChar() const;

protected:
    /*!
     * This function is called for every single char within the string if
     * the escape char is QChar::null. It should determine whether the
     * string starting at \a pos within \a str is a valid macro and return
     * the substitution value for it if so.
     *
     * \a str the input string
     *
     * \a pos the offset within \a str
     *
     * \a ret return value: the string to substitute for the macro
     *
     * Returns If greater than zero, the number of chars at \a pos in \a str
     *  to substitute with \a ret (i.e., a valid macro was found). If less
     *  than zero, subtract this value from \a pos (to skip a macro, i.e.,
     *  substitute it with itself). If zero, no macro starts at \a pos.
     */
    virtual int expandPlainMacro(const QString &str, int pos, QStringList &ret);

    /*!
     * This function is called every time the escape char is found if it is
     * not QChar::null. It should determine whether the
     * string starting at \a pos witin \a str is a valid macro and return
     * the substitution value for it if so.
     *
     * \a str the input string
     *
     * \a pos the offset within \a str. Note that this is the position of
     *  the occurrence of the escape char
     *
     * \a ret return value: the string to substitute for the macro
     *
     * Returns If greater than zero, the number of chars at \a pos in \a str
     *  to substitute with \a ret (i.e., a valid macro was found). If less
     *  than zero, subtract this value from \a pos (to skip a macro, i.e.,
     *  substitute it with itself). If zero, scanning continues as if no
     *  escape char was encountered at all.
     */
    virtual int expandEscapedMacro(const QString &str, int pos, QStringList &ret);

private:
    std::unique_ptr<KMacroExpanderBasePrivate> const d;
};

/*!
 * \class KWordMacroExpander
 * \inmodule KCoreAddons
 * \inheaderfile KMacroExpander
 *
 * \brief Abstract base class for simple word macro substitutors.
 *
 * Use this instead of  the functions in the KMacroExpander namespace if speculatively pre-filling
 * the substitution map would be too expensive.
 *
 * A typical application:
 *
 * \code
 * class MyClass {
 * ...
 *   private:
 *     QString m_str;
 * ...
 *   friend class MyExpander;
 * };
 *
 * class MyExpander : public KWordMacroExpander {
 *   public:
 *     MyExpander( MyClass *_that ) : KWordMacroExpander(), that( _that ) {}
 *   protected:
 *     virtual bool expandMacro( const QString &str, QStringList &ret );
 *   private:
 *     MyClass *that;
 * };
 *
 * bool MyExpander::expandMacro( const QString &str, QStringList &ret )
 * {
 *   if (str == "macro") {
 *     ret += complexOperation( that->m_str );
 *     return true;
 *   }
 *   return false;
 * }
 *
 * ... MyClass::...(...)
 * {
 *   QString str;
 *   ...
 *   MyExpander mx( this );
 *   mx.expandMacrosShellQuote( str );
 *   ...
 * }
 * \endcode
 *
 * Alternatively MyClass could inherit from KWordMacroExpander directly.
 *
 */
class KCOREADDONS_EXPORT KWordMacroExpander : public KMacroExpanderBase
{
public:
    /*!
     * Constructor.
     *
     * \a c escape char indicating start of macros, or QChar::null for none
     */
    explicit KWordMacroExpander(QChar c = QLatin1Char('%'))
        : KMacroExpanderBase(c)
    {
    }

protected:
    /*! \internal Not to be called or reimplemented. */
    int expandPlainMacro(const QString &str, int pos, QStringList &ret) override;
    /*! \internal Not to be called or reimplemented. */
    int expandEscapedMacro(const QString &str, int pos, QStringList &ret) override;

    /*!
     * Return substitution list \a ret for string macro \a str.
     *
     * \a str the macro to expand
     *
     * \a ret return variable reference. It is guaranteed to be empty
     *  when expandMacro is entered.
     *
     * Returns \c true iff \a str was a recognized macro name
     */
    virtual bool expandMacro(const QString &str, QStringList &ret) = 0;
};

/*!
 * \class KCharMacroExpander
 * \inmodule KCoreAddons
 * \inheaderfile KMacroExpander
 *
 * \brief Abstract base class for single char macro substitutors.
 *
 * Use this instead of the functions in the KMacroExpander namespace
 * if speculatively pre-filling the substitution map would be too expensive.
 *
 * See KWordMacroExpander for a sample application.
 *
 */
class KCOREADDONS_EXPORT KCharMacroExpander : public KMacroExpanderBase
{
public:
    /*!
     * Constructor.
     *
     * \a c escape char indicating start of macros, or QChar::null for none
     */
    explicit KCharMacroExpander(QChar c = QLatin1Char('%'))
        : KMacroExpanderBase(c)
    {
    }

protected:
    /*! \internal Not to be called or reimplemented. */
    int expandPlainMacro(const QString &str, int pos, QStringList &ret) override;
    /*! \internal Not to be called or reimplemented. */
    int expandEscapedMacro(const QString &str, int pos, QStringList &ret) override;

    /*!
     * Return substitution list \a ret for single-character macro \a chr.
     *
     * \a chr the macro to expand
     *
     * \a ret return variable reference. It is guaranteed to be empty
     *  when expandMacro is entered.
     *
     * Returns \c true iff \a chr was a recognized macro name
     */
    virtual bool expandMacro(QChar chr, QStringList &ret) = 0;
};

/*!
 * \namespace KMacroExpander
 * \inmodule KCoreAddons
 * \brief A group of functions providing macro expansion (substitution) in strings,
 * optionally with quoting appropriate for shell execution.
 */
namespace KMacroExpander
{
/*!
 * Perform safe macro expansion (substitution) on a string.
 * The escape char must be quoted with itself to obtain its literal
 * representation in the resulting string.
 *
 * \a str The string to expand
 *
 * \a map map with substitutions
 *
 * \a c escape char indicating start of macro, or QChar::null if none
 *
 * Returns the string with all valid macros expanded
 *
 * \code
 * // Code example
 * QHash<QChar,QString> map;
 * map.insert('u', "/tmp/myfile.txt");
 * map.insert('n', "My File");
 * QString s = "%% Title: %u:%n";
 * s = KMacroExpander::expandMacros(s, map);
 * // s is now "% Title: /tmp/myfile.txt:My File";
 * \endcode
 */
KCOREADDONS_EXPORT QString expandMacros(const QString &str, const QHash<QChar, QString> &map, QChar c = QLatin1Char('%'));

/*!
 * Perform safe macro expansion (substitution) on a string for use
 * in shell commands.
 * The escape char must be quoted with itself to obtain its literal
 * representation in the resulting string.
 *
 * \a str The string to expand
 *
 * \a map map with substitutions
 *
 * \a c escape char indicating start of macro, or QChar::null if none
 *
 * Returns the string with all valid macros expanded, or a null string
 *  if a shell syntax error was detected in the command
 *
 * \code
 * // Code example
 * QHash<QChar,QString> map;
 * map.insert('u', "/tmp/myfile.txt");
 * map.insert('n', "My File");
 * QString s = "kwrite --qwindowtitle %n %u";
 * s = KMacroExpander::expandMacrosShellQuote(s, map);
 * // s is now "kwrite --qwindowtitle 'My File' '/tmp/myfile.txt'";
 * system(QFile::encodeName(s));
 * \endcode
 */
KCOREADDONS_EXPORT QString expandMacrosShellQuote(const QString &str, const QHash<QChar, QString> &map, QChar c = QLatin1Char('%'));

/*!
 * Perform safe macro expansion (substitution) on a string.
 * The escape char must be quoted with itself to obtain its literal
 * representation in the resulting string.
 * Macro names can consist of chars in the range [A-Za-z0-9_];
 * use braces to delimit macros from following words starting
 * with these chars, or to use other chars for macro names.
 *
 * \a str The string to expand
 *
 * \a map map with substitutions
 *
 * \a c escape char indicating start of macro, or QChar::null if none
 *
 * Returns the string with all valid macros expanded
 *
 * \code
 * // Code example
 * QHash<QString,QString> map;
 * map.insert("url", "/tmp/myfile.txt");
 * map.insert("name", "My File");
 * QString s = "Title: %{url}-%name";
 * s = KMacroExpander::expandMacros(s, map);
 * // s is now "Title: /tmp/myfile.txt-My File";
 * \endcode
 */
KCOREADDONS_EXPORT QString expandMacros(const QString &str, const QHash<QString, QString> &map, QChar c = QLatin1Char('%'));

/*!
 * Perform safe macro expansion (substitution) on a string for use
 * in shell commands. See KMacroExpanderBase::expandMacrosShellQuote()
 * for the exact semantics.
 * The escape char must be quoted with itself to obtain its literal
 * representation in the resulting string.
 * Macro names can consist of chars in the range [A-Za-z0-9_];
 * use braces to delimit macros from following words starting
 * with these chars, or to use other chars for macro names.
 *
 * \a str The string to expand
 *
 * \a map map with substitutions
 *
 * \a c escape char indicating start of macro, or QChar::null if none
 *
 * Returns the string with all valid macros expanded, or a null string
 *  if a shell syntax error was detected in the command
 *
 * \code
 * // Code example
 * QHash<QString,QString> map;
 * map.insert("url", "/tmp/myfile.txt");
 * map.insert("name", "My File");
 * QString s = "kwrite --qwindowtitle %name %{url}";
 * s = KMacroExpander::expandMacrosShellQuote(s, map);
 * // s is now "kwrite --qwindowtitle 'My File' '/tmp/myfile.txt'";
 * system(QFile::encodeName(s));
 * \endcode
 */
KCOREADDONS_EXPORT QString expandMacrosShellQuote(const QString &str, const QHash<QString, QString> &map, QChar c = QLatin1Char('%'));

/*!
 * Same as above, except that the macros expand to string lists that
 * are simply join(" ")ed together.
 */
KCOREADDONS_EXPORT QString expandMacros(const QString &str, const QHash<QChar, QStringList> &map, QChar c = QLatin1Char('%'));
KCOREADDONS_EXPORT QString expandMacros(const QString &str, const QHash<QString, QStringList> &map, QChar c = QLatin1Char('%'));

/*!
 * Same as above, except that the macros expand to string lists.
 * If the macro appears inside a quoted string, the list is simply
 * join(" ")ed together; otherwise every element expands to a separate
 * quoted string.
 */
KCOREADDONS_EXPORT QString expandMacrosShellQuote(const QString &str, const QHash<QChar, QStringList> &map, QChar c = QLatin1Char('%'));
KCOREADDONS_EXPORT QString expandMacrosShellQuote(const QString &str, const QHash<QString, QStringList> &map, QChar c = QLatin1Char('%'));
}

#endif /* KMACROEXPANDER_H */
