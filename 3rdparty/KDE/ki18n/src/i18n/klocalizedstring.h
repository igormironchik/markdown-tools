/*  This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2006, 2013 Chusslove Illich <caslav.ilic@gmx.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KLOCALIZEDSTRING_H
#define KLOCALIZEDSTRING_H

#include <ki18n_export.h>

#include <QChar>
#include <QLatin1Char>
#include <QSet>
#include <QString>
#include <QStringList>

#include <kuitsetup.h>

// enforce header to be parsed before redefining i18n* with preprocessor macros
// depending on TRANSLATION_DOMAIN (see bottom of file)
#include <klocalizedcontext.h>

class KLocalizedStringPrivate;
class KLazyLocalizedString;

/*!
 * \class KLocalizedString
 * \inmodule KI18n
 *
 * \brief Class for producing and handling localized messages.
 *
 * KLocalizedString handles translation and
 * argument substitution and formatting of user-visible text.
 *
 * KLocalizedString instances are usually not constructed directly,
 * but through one of the wrapper *i18n* calls.
 *
 * For detailed information on how to use KI18n functions please refer
 * to prg_guide.
 */
class KI18N_EXPORT KLocalizedString
{
    friend class KLocalizedStringPrivate;
    friend class KLazyLocalizedString;

    friend KLocalizedString KI18N_EXPORT ki18n(const char *text);
    friend KLocalizedString KI18N_EXPORT ki18nc(const char *context, const char *text);
    friend KLocalizedString KI18N_EXPORT ki18np(const char *singular, const char *plural);
    friend KLocalizedString KI18N_EXPORT ki18ncp(const char *context, const char *singular, const char *plural);
    friend KLocalizedString KI18N_EXPORT ki18nd(const char *domain, const char *text);
    friend KLocalizedString KI18N_EXPORT ki18ndc(const char *domain, const char *context, const char *text);
    friend KLocalizedString KI18N_EXPORT ki18ndp(const char *domain, const char *singular, const char *plural);
    friend KLocalizedString KI18N_EXPORT ki18ndcp(const char *domain, const char *context, const char *singular, const char *plural);

    friend KLocalizedString KI18N_EXPORT kxi18n(const char *text);
    friend KLocalizedString KI18N_EXPORT kxi18nc(const char *context, const char *text);
    friend KLocalizedString KI18N_EXPORT kxi18np(const char *singular, const char *plural);
    friend KLocalizedString KI18N_EXPORT kxi18ncp(const char *context, const char *singular, const char *plural);
    friend KLocalizedString KI18N_EXPORT kxi18nd(const char *domain, const char *text);
    friend KLocalizedString KI18N_EXPORT kxi18ndc(const char *domain, const char *context, const char *text);
    friend KLocalizedString KI18N_EXPORT kxi18ndp(const char *domain, const char *singular, const char *plural);
    friend KLocalizedString KI18N_EXPORT kxi18ndcp(const char *domain, const char *context, const char *singular, const char *plural);

public:
    /*!
     * Construct an empty message.
     *
     * Direct construction is used when another KLocalizedString instance,
     * obtained by one of ki18n* calls, should later be assigned
     * to directly constructed instance.
     * Before the assignment happens, directly constructed instance
     * is not valid for finalization by toString methods.
     *
     * \sa isEmpty
     */
    explicit KLocalizedString();

    KLocalizedString(const KLocalizedString &rhs);

    KLocalizedString &operator=(const KLocalizedString &rhs);

    ~KLocalizedString();

    /*!
     * Check whether the message is empty.
     *
     * The message is considered empty if the object was constructed
     * via the default constructor.
     *
     * Empty messages are not valid for finalization.
     * The behavior of calling toString on them is undefined.
     * In debug mode, an error mark may appear in the returned string.
     *
     * Returns \c true if the message is empty, \c false otherwise
     */
    bool isEmpty() const;

    /*!
     * Finalize the translation.
     *
     * Creates translated QString, with placeholders substituted
     * by arguments given by KLocalizedString::subs methods.
     * Translated text is searched for and arguments are formatted
     * based on the global locale.
     *
     * If there was any mismatch between placeholders and arguments,
     * in debug mode the returned string may contain error marks.
     *
     * Returns finalized translation
     */
    Q_REQUIRED_RESULT QString toString() const;

    /*!
     * Like toString, but look for translation only in given languages.
     *
     * Given languages override languages defined by the global locale,
     * and any languages set earlier using withLanguages.
     * If \a languages is empty, original message is returned.
     *
     * \a languages list of language codes (by decreasing priority)
     * Returns finalized translation
     */
    Q_REQUIRED_RESULT QString toString(const QStringList &languages) const;

    /*!
     * Like toString, but look for translation in the given domain.
     *
     * Given domain overrides any set earlier using withDomain.
     *
     * \a domain the translation domain
     *
     * Returns finalized translation
     */
    Q_REQUIRED_RESULT QString toString(const char *domain) const;

    /*!
     * Like toString, but resolve KUIT markup into given visual format.
     *
     * Given visual format overrides that implied by the context UI marker
     * or set earlier using withFormat.
     * If the message is not markup-aware,
     * this is same as toString without arguments.
     *
     * \a format the target visual format
     *
     * Returns finalized translation
     */
    Q_REQUIRED_RESULT QString toString(Kuit::VisualFormat format) const;

    /*!
     * Indicate to look for translation only in given languages.
     *
     * \a languages list of language codes (by decreasing priority)
     *
     * Returns updated KLocalizedString
     */
    Q_REQUIRED_RESULT KLocalizedString withLanguages(const QStringList &languages) const;

    /*!
     * Indicate to look for translation in the given domain.
     *
     * \a domain the translation domain
     * Returns updated KLocalizedString
     */
    Q_REQUIRED_RESULT KLocalizedString withDomain(const char *domain) const;

    /*!
     * Indicate to resolve KUIT markup into given visual format.
     *
     * If the message is not markup-aware, this has no effect.
     *
     * \a format the target visual format
     *
     * Returns updated KLocalizedString
     */
    Q_REQUIRED_RESULT KLocalizedString withFormat(Kuit::VisualFormat format) const;

    /*!
     * Substitute an int argument into the message.
     *
     * \a a the argument
     *
     * \a fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     *
     * \a base the radix used to represent the number as a string.
     *             Valid values range from 2 to 36
     *
     * \a fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     *
     * Returns updated KLocalizedString
     */
    Q_REQUIRED_RESULT KLocalizedString subs(int a, int fieldWidth = 0, int base = 10, QChar fillChar = QLatin1Char(' ')) const;

    /*!
     * Substitute an unsigned int argument into the message.
     *
     * \a a the argument
     *
     * \a fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     *
     * \a base the radix used to represent the number as a string.
     *             Valid values range from 2 to 36
     *
     * \a fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     *
     * Returns updated KLocalizedString
     */
    Q_REQUIRED_RESULT KLocalizedString subs(uint a, int fieldWidth = 0, int base = 10, QChar fillChar = QLatin1Char(' ')) const;

    /*!
     * Substitute a long argument into the message.
     *
     * \a a the argument
     *
     * \a fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     *
     * \a base the radix used to represent the number as a string.
     *             Valid values range from 2 to 36
     *
     * \a fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     *
     * Returns updated KLocalizedString
     */
    Q_REQUIRED_RESULT KLocalizedString subs(long a, int fieldWidth = 0, int base = 10, QChar fillChar = QLatin1Char(' ')) const;

    /*!
     * Substitute an unsigned long argument into the message.
     *
     * \a a the argument
     *
     * \a fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     *
     * \a base the radix used to represent the number as a string.
     *             Valid values range from 2 to 36
     *
     * \a fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     *
     * Returns updated KLocalizedString
     */
    Q_REQUIRED_RESULT KLocalizedString subs(ulong a, int fieldWidth = 0, int base = 10, QChar fillChar = QLatin1Char(' ')) const;

    /*!
     * Substitute a long long argument into the message.
     *
     * \a a the argument
     *
     * \a fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     *
     * \a base the radix used to represent the number as a string.
     *             Valid values range from 2 to 36
     *
     * \a fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     *
     * Returns updated KLocalizedString
     */
    Q_REQUIRED_RESULT KLocalizedString subs(qlonglong a, int fieldWidth = 0, int base = 10, QChar fillChar = QLatin1Char(' ')) const;

    /*!
     * Substitute an unsigned long long argument into the message.
     *
     * \a a the argument
     *
     * \a fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     *
     * \a base the radix used to represent the number as a string.
     *             Valid values range from 2 to 36
     *
     * \a fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     *
     * Returns updated KLocalizedString
     */
    Q_REQUIRED_RESULT KLocalizedString subs(qulonglong a, int fieldWidth = 0, int base = 10, QChar fillChar = QLatin1Char(' ')) const;

    /*!
     * Substitute a double argument into the message.
     *
     * \a a the argument
     *
     * \a fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     *
     * \a format type of floating point formatting, like in QString::arg
     *
     * \a precision number of digits after the decimal separator
     *
     * \a fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     *
     * Returns updated KLocalizedString
     */
    Q_REQUIRED_RESULT KLocalizedString subs(double a, int fieldWidth = 0, char format = 'g', int precision = -1, QChar fillChar = QLatin1Char(' ')) const;

    /*!
     * Substitute a QChar argument into the message.
     *
     * \a a the argument
     *
     * \a fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     *
     * \a fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     *
     * Returns updated KLocalizedString
     */
    Q_REQUIRED_RESULT KLocalizedString subs(QChar a, int fieldWidth = 0, QChar fillChar = QLatin1Char(' ')) const;

    /*!
     * Substitute a QString argument into the message.
     *
     * \a a the argument
     *
     * \a fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     *
     * \a fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     *
     * Returns updated KLocalizedString
     */
    Q_REQUIRED_RESULT KLocalizedString subs(const QString &a, int fieldWidth = 0, QChar fillChar = QLatin1Char(' ')) const;

    /*!
     * Substitute another KLocalizedString into the message.
     *
     * \a a the argument
     *
     * \a fieldWidth width of the formatted field, padded by spaces.
     *                   Positive value aligns right, negative aligns left
     *
     * \a fillChar the character used to fill up the empty places when
     *                 field width is greater than argument width
     *
     * Returns updated KLocalizedString
     */
    Q_REQUIRED_RESULT KLocalizedString subs(const KLocalizedString &a, int fieldWidth = 0, QChar fillChar = QLatin1Char(' ')) const;

    /*!
     * Add dynamic context to the message.
     *
     * See dyn_ctxt for use cases.
     *
     * \a key context key
     *
     * \a value context value
     *
     * Returns updated KLocalizedString
     */
    Q_REQUIRED_RESULT KLocalizedString inContext(const QString &key, const QString &value) const;

    /*!
     * Relax matching between placeholders and arguments.
     *
     * Normally the placeholders should start from %1 and have no gaps,
     * and on finalization there must be exactly as many arguments
     * supplied through subs methods as there are unique plaecholders.
     * If this is not satisfied, in debug mode warnings are printed
     * and the finalized string may contain error marks.
     *
     * This method relaxes the placeholder-argument matching,
     * such that there must only be an argument available for
     * every present unique placeholder (taking placeholder numbers
     * to be 1-based indices into the argument list).
     * This can come useful in some situations.
     *
     * Returns updated KLocalizedString
     */
    Q_REQUIRED_RESULT KLocalizedString relaxSubs() const;

    /*!
     * Do not resolve KUIT markup.
     *
     * If the message is markup-aware
     * (constructed by one of *xi18n* calls),
     * this function can be used to make it non-markup-aware.
     * This may be useful for debugging markup.
     *
     * Returns updated KLocalizedString
     */
    Q_REQUIRED_RESULT KLocalizedString ignoreMarkup() const;

    /*!
     * Returns the untranslated text.
     *
     * \since 5.64
     */
    Q_REQUIRED_RESULT QByteArray untranslatedText() const;

    /*!
     * Set the given domain as application's main domain.
     *
     * This function must be called in applications, in order to have
     * any translations at all. It should never be called in libraries.
     * This allows to check whether the application is translated
     * into a given language, so that if it is not, translations from
     * underlying libraries will not appear even if they are translated.
     * This prevents mixing of translated and untranslated text
     * in the user interface.
     *
     * This function should be called right after creating the instance
     * of QCoreApplication or one of its subclasses. At that time the locale
     * setup has been made, including what is hooked into the
     * QCoreApplication startup, like KXMLGUI's language switching support.
     * So the initialisation done by this function sees all the data it should.
     *
     * \a domain the translation domain of the application
     */
    static void setApplicationDomain(const QByteArray &domain);

    /*!
     * Get the application's main translation domain.
     *
     * Returns the domain set by setApplicationDomain.
     */
    static QByteArray applicationDomain();

    /*!
     * Get the languages for which translations will be made.
     *
     * Returned languages are ordered with decreasing priority.
     *
     * Returns languages ordered list of language codes
     * \sa setLanguages
     * \sa clearLanguages
     *
     * \since 5.20.0
     */
    static QStringList languages();

    /*!
     * Set the languages for which translations will be made.
     *
     * This overrides the languages provided by the locale.
     * Languages should be ordered with decreasing priority.
     *
     * \a languages ordered list of language codes
     *
     * \sa setLocale
     * \sa clearLanguages
     * \sa languages
     */
    static void setLanguages(const QStringList &languages);

    /*!
     * Clear override languages.
     *
     * This clears the override languages, going back to those
     * provided by the locale.
     *
     * \sa setLanguages
     * \sa languages
     */
    static void clearLanguages();

    /*!
     * Check whether the translation catalog file in the given language
     * for the set application translation domain exists.
     *
     * \a language the language code to check
     *
     * Returns \c true if the translation catalog for \a language exits,
     *         \c false otherwise
     * \sa setApplicationDomain
     */
    static bool isApplicationTranslatedInto(const QString &language);

    /*!
     * \since 5.0
     *
     * Get the languages for which there exists the translation catalog file
     * for the set application translation domain.
     *
     * The application domain is set by setApplicationDomain.
     * If the application domain was not set, empty set is returned.
     * If the application domain was set, the language set will always
     * contain at least the source code language (en_US).
     *
     * Returns set of language codes for existing translation catalogs
     * \sa setApplicationDomain
     */
    static QSet<QString> availableApplicationTranslations();

    /*!
     * \since 5.0
     *
     * Get the languages for which a translation catalog file
     * for the passed translation domain exists.
     *
     * If the translation domain was not specified in the
     * domain parameter an empty set is returned.
     *
     * If the application domain was set, the language set will always
     * contain at least the source code language (en_US).
     *
     * \a domain query for translations of a specific domain, if an empty
     * QByteArray is passed, an empty set will be returned
     *
     * Returns set of language codes for existing translation catalogs
     * \sa setApplicationDomain
     * \sa availableApplicationTranslations
     */
    static QSet<QString> availableDomainTranslations(const QByteArray &domain);

    /*!
     * Load locales for a domain from a specific location
     * This is useful for resources which have their translation files
     * outside of the usual $XDG_DATA_DIRS/locales location
     *
     * \a domain the domain to load resources from
     *
     * \a path the full file path to the locale directory
     */
    static void addDomainLocaleDir(const QByteArray &domain, const QString &path);

    /*!
     * Find a path to the localized file for the given original path.
     *
     * This is intended mainly for non-text resources (images, sounds, etc).
     * Text resources should be handled in more specific ways.
     *
     * Possible localized paths are checked in turn by priority of set
     * languages, in form of <dirname>/l10n/<lang>/<basename>,
     * where <dirname> and <basename> are those of
     * the original path, and <lang> is the language code.
     *
     * \a filePath path to the original file
     *
     * Returns path to the localized file if found, original path otherwise
     */
    Q_REQUIRED_RESULT static QString localizedFilePath(const QString &filePath);

    /*!
     * Remove accelerator marker from a UI text label.
     *
     * Accelerator marker is not always a plain ampersand (&),
     * so it is not enough to just remove it by QString::remove.
     * The label may contain escaped markers ("&&") which must be resolved
     * and skipped, as well as CJK-style markers ("Foo (&F)") where
     * the whole parenthesis construct should be removed.
     * Therefore always use this function to remove accelerator marker
     * from UI labels.
     *
     * \a label UI label which may contain an accelerator marker
     *
     * Returns label without the accelerator marker
     */
    Q_REQUIRED_RESULT static QString removeAcceleratorMarker(const QString &label);

private:
    // exported because called from inline KLazyLocalizedString::operator KLocalizedString()
    KLocalizedString(const char *domain, const char *context, const char *text, const char *plural, bool markupAware);

private:
    // intentionally not a unique_ptr as this file gets included a lot and using a unique_ptr
    // results in too many template instantiations
    KLocalizedStringPrivate *const d;
};

// Do not document every multi-argument i18n* call separately,
// but provide special quasi-calls that only qdoc sees.
// Placed in front of ki18n* calls, because i18n* are more basic.
#ifdef Q_QDOC

/*!
 * Translate a string and substitute any arguments.
 *
 * \a text string to translate
 *
 * \a arg arguments to insert (0 to 9),
 *            admissible types according to KLocalizedString::subs methods
 *
 * Returns translated string
 *
 * \relates KLocalizedString
 */
QString i18n(const char *text, const TYPE &arg...);

/*!
 * Translate a string with context and substitute any arguments.
 *
 * \a context context of the string
 *
 * \a text string to translate
 *
 * \a arg arguments to insert (0 to 9),
 *            admissible types according to KLocalizedString::subs methods
 *
 * Returns translated string
 *
 * \relates KLocalizedString
 */
QString i18nc(const char *context, const char *text, const TYPE &arg...);

/*!
 * Translate a string with plural and substitute any arguments.
 *
 * \a singular singular form of the string to translate
 *
 * \a plural plural form of the string to translate
 *
 * \a arg arguments to insert (0 to 9),
 *            admissible types according to KLocalizedString::subs methods
 *
 * Returns translated string
 *
 * \relates KLocalizedString
 */
QString i18np(const char *singular, const char *plural, const TYPE &arg...);

/*!
 * Translate a string with context and plural and substitute any arguments.
 *
 * \a context context of the string
 *
 * \a singular singular form of the string to translate
 *
 * \a plural plural form of the string to translate
 *
 * \a arg arguments to insert (0 to 9),
 *            admissible types according to KLocalizedString::subs methods
 *
 * Returns translated string
 *
 * \relates KLocalizedString
 */
QString i18ncp(const char *context, const char *singular, const char *plural, const TYPE &arg...);

/*!
 * Translate a string from domain and substitute any arguments.
 *
 * \a domain domain in which to look for translations
 *
 * \a text string to translate
 *
 * \a arg arguments to insert (0 to 9),
 *            admissible types according to KLocalizedString::subs methods
 *
 * Returns translated string
 *
 * \relates KLocalizedString
 */
QString i18nd(const char *domain, const char *text, const TYPE &arg...);

/*!
 * Translate a string from domain with context and substitute any arguments.
 *
 * \a domain domain in which to look for translations
 *
 * \a context context of the string
 *
 * \a text string to translate
 *
 * \a arg arguments to insert (0 to 9),
 *            admissible types according to KLocalizedString::subs methods
 *
 * Returns translated string
 *
 * \relates KLocalizedString
 */
QString i18ndc(const char *domain, const char *context, const char *text, const TYPE &arg...);

/*!
 * Translate a string from domain with plural and substitute any arguments.
 *
 * \a domain domain in which to look for translations
 *
 * \a singular singular form of the string to translate
 *
 * \a plural plural form of the string to translate
 *
 * \a arg arguments to insert (0 to 9),
 *            admissible types according to KLocalizedString::subs methods
 *
 * Returns translated string
 *
 * \relates KLocalizedString
 */
QString i18ndp(const char *domain, const char *singular, const char *plural, const TYPE &arg...);

/*!
 * Translate a string from domain with context and plural
 * and substitute any arguments.
 *
 * \a domain domain in which to look for translations
 *
 * \a context context of the string
 *
 * \a singular singular form of the string to translate
 *
 * \a plural plural form of the string to translate
 *
 * \a arg arguments to insert (0 to 9),
 *            admissible types according to KLocalizedString::subs methods
 *
 * Returns translated string
 *
 * \relates KLocalizedString
 */
QString i18ndcp(const char *domain, const char *context, const char *singular, const char *plural, const TYPE &arg...);

/*!
 * Translate a markup-aware string and substitute any arguments.
 *
 * \a text string to translate
 *
 * \a arg arguments to insert (0 to 9),
 *            admissible types according to KLocalizedString::subs methods
 *
 * Returns translated string
 *
 * \relates KLocalizedString
 */
QString xi18n(const char *text, const TYPE &arg...);

/*!
 * Translate a markup-aware string with context and substitute any arguments.
 *
 * \a context context of the string
 *
 * \a text string to translate
 *
 * \a arg arguments to insert (0 to 9),
 *            admissible types according to KLocalizedString::subs methods
 *
 * Returns translated string
 *
 * \relates KLocalizedString
 */
QString xi18nc(const char *context, const char *text, const TYPE &arg...);

/*!
 * Translate a markup-aware string with plural and substitute any arguments.
 *
 * \a singular singular form of the string to translate
 *
 * \a plural plural form of the string to translate
 *
 * \a arg arguments to insert (0 to 9),
 *            admissible types according to KLocalizedString::subs methods
 *
 * Returns translated string
 *
 * \relates KLocalizedString
 */
QString xi18np(const char *singular, const char *plural, const TYPE &arg...);

/*!
 * Translate a markup-aware string with context and plural
 * and substitute any arguments.
 *
 * \a context context of the string
 *
 * \a singular singular form of the string to translate
 *
 * \a plural plural form of the string to translate
 *
 * \a arg arguments to insert (0 to 9),
 *            admissible types according to KLocalizedString::subs methods
 *
 * Returns translated string
 *
 * \relates KLocalizedString
 */
QString xi18ncp(const char *context, const char *singular, const char *plural, const TYPE &arg...);

/*!
 * Translate a markup-aware string from domain and substitute any arguments.
 *
 * \a domain domain in which to look for translations
 *
 * \a text string to translate
 *
 * \a arg arguments to insert (0 to 9),
 *            admissible types according to KLocalizedString::subs methods
 *
 * Returns translated string
 *
 * \relates KLocalizedString
 */
QString xi18nd(const char *domain, const char *text, const TYPE &arg...);

/*!
 * Translate a markup-aware string from domain with context
 * and substitute any arguments.
 *
 * \a domain domain in which to look for translations
 *
 * \a context context of the string
 *
 * \a text string to translate
 *
 * \a arg arguments to insert (0 to 9),
 *            admissible types according to KLocalizedString::subs methods
 *
 * Returns translated string
 *
 * \relates KLocalizedString
 */
QString xi18ndc(const char *domain, const char *context, const char *text, const TYPE &arg...);

/*!
 * Translate a markup-aware string from domain with plural
 * and substitute any arguments.
 *
 * \a domain domain in which to look for translations
 *
 * \a singular singular form of the string to translate
 *
 * \a plural plural form of the string to translate
 *
 * \a arg arguments to insert (0 to 9),
 *            admissible types according to KLocalizedString::subs methods
 *
 * Returns translated string
 *
 * \relates KLocalizedString
 */
QString xi18ndp(const char *domain, const char *singular, const char *plural, const TYPE &arg...);

/*!
 * Translate a markup-aware string from domain with context and plural
 * and substitute any arguments.
 *
 * \a domain domain in which to look for translations
 *
 * \a context context of the string
 *
 * \a singular singular form of the string to translate
 *
 * \a plural plural form of the string to translate
 *
 * \a arg arguments to insert (0 to 9),
 *            admissible types according to KLocalizedString::subs methods
 *
 * Returns translated string
 *
 * \relates KLocalizedString
 */
QString xi18ndcp(const char *domain, const char *context, const char *singular, const char *plural, const TYPE &arg...);

#endif // Q_QDOC

/*!
 * Create non-finalized translated string.
 *
 * \a text string to translate
 *
 * Returns non-finalized translated string
 *
 * \relates KLocalizedString
 */
KLocalizedString KI18N_EXPORT ki18n(const char *text);

/*!
 * Create non-finalized translated string with context.
 *
 * \a context context of the string
 *
 * \a text string to translate
 *
 * Returns non-finalized translated string
 *
 * \relates KLocalizedString
 */
KLocalizedString KI18N_EXPORT ki18nc(const char *context, const char *text);

/*!
 * Create non-finalized translated string with plural.
 *
 * \a singular singular form of the string to translate
 *
 * \a plural plural form of the string to translate
 *
 * Returns non-finalized translated string
 *
 * \relates KLocalizedString
 */
KLocalizedString KI18N_EXPORT ki18np(const char *singular, const char *plural);

/*!
 * Create non-finalized translated string with context and plural.
 *
 * \a context context of the string
 *
 * \a singular singular form of the string to translate
 *
 * \a plural plural form of the string to translate
 *
 * Returns non-finalized translated string
 *
 * \relates KLocalizedString
 */
KLocalizedString KI18N_EXPORT ki18ncp(const char *context, const char *singular, const char *plural);

/*!
 * Create non-finalized translated string from domain.
 *
 * \a domain domain in which to look for translations
 *
 * \a text string to translate
 *
 * Returns non-finalized translated string
 *
 * \relates KLocalizedString
 */
KLocalizedString KI18N_EXPORT ki18nd(const char *domain, const char *text);

/*!
 * Create non-finalized translated string from domain with context.
 *
 * \a domain domain in which to look for translations
 *
 * \a context context of the string
 *
 * \a text string to translate
 *
 * Returns non-finalized translated string
 *
 * \relates KLocalizedString
 */
KLocalizedString KI18N_EXPORT ki18ndc(const char *domain, const char *context, const char *text);

/*!
 * Create non-finalized translated string from domain with plural.
 *
 * \a domain domain in which to look for translations
 *
 * \a singular singular form of the string to translate
 *
 * \a plural plural form of the string to translate
 *
 * Returns non-finalized translated string
 *
 * \relates KLocalizedString
 */
KLocalizedString KI18N_EXPORT ki18ndp(const char *domain, const char *singular, const char *plural);

/*!
 * Create non-finalized translated string from domain with context and plural.
 *
 * \a domain domain in which to look for translations
 *
 * \a context context of the string
 *
 * \a singular singular form of the string to translate
 *
 * \a plural plural form of the string to translate
 *
 * Returns non-finalized translated string
 *
 * \relates KLocalizedString
 */
KLocalizedString KI18N_EXPORT ki18ndcp(const char *domain, const char *context, const char *singular, const char *plural);

/*!
 * Create non-finalized markup-aware translated string.
 *
 * \a text string to translate
 *
 * Returns non-finalized translated string
 *
 * \relates KLocalizedString
 */
KLocalizedString KI18N_EXPORT kxi18n(const char *text);

/*!
 * Create non-finalized markup-aware translated string with context.
 *
 * \a context context of the string
 *
 * \a text string to translate
 *
 * Returns non-finalized translated string
 *
 * \relates KLocalizedString
 */
KLocalizedString KI18N_EXPORT kxi18nc(const char *context, const char *text);

/*!
 * Create non-finalized markup-aware translated string with plural.
 *
 * \a singular singular form of the string to translate
 *
 * \a plural plural form of the string to translate
 *
 * Returns non-finalized translated string
 *
 * \relates KLocalizedString
 */
KLocalizedString KI18N_EXPORT kxi18np(const char *singular, const char *plural);

/*!
 * Create non-finalized markup-aware translated string.
 * with context and plural.
 *
 * \a context context of the string
 *
 * \a singular singular form of the string to translate
 *
 * \a plural plural form of the string to translate
 *
 * Returns non-finalized translated string
 *
 * \relates KLocalizedString
 */
KLocalizedString KI18N_EXPORT kxi18ncp(const char *context, const char *singular, const char *plural);

/*!
 * Create non-finalized markup-aware translated string from domain.
 *
 * \a domain domain in which to look for translations
 *
 * \a text string to translate
 *
 * Returns non-finalized translated string
 *
 * \relates KLocalizedString
 */
KLocalizedString KI18N_EXPORT kxi18nd(const char *domain, const char *text);

/*!
 * Create non-finalized markup-aware translated string from domain with context.
 *
 * \a domain domain in which to look for translations
 *
 * \a context context of the string
 *
 * \a text string to translate
 *
 * Returns non-finalized translated string
 *
 * \relates KLocalizedString
 */
KLocalizedString KI18N_EXPORT kxi18ndc(const char *domain, const char *context, const char *text);

/*!
 * Create non-finalized markup-aware translated string from domain with plural.
 *
 * \a domain domain in which to look for translations
 *
 * \a singular singular form of the string to translate
 *
 * \a plural plural form of the string to translate
 *
 * Returns non-finalized translated string
 *
 * \relates KLocalizedString
 */
KLocalizedString KI18N_EXPORT kxi18ndp(const char *domain, const char *singular, const char *plural);

/*!
 * Create non-finalized markup-aware translated string from domain
 * with context and plural.
 *
 * \a domain domain in which to look for translations
 *
 * \a context context of the string
 *
 * \a singular singular form of the string to translate
 *
 * \a plural plural form of the string to translate
 *
 * Returns non-finalized translated string
 *
 * \relates KLocalizedString
 */
KLocalizedString KI18N_EXPORT kxi18ndcp(const char *domain, const char *context, const char *singular, const char *plural);

/*!
 * Redirect Qt's uic-generated translation calls to Ki18n.
 *
 * Use -tr tr2i18n option to uic to have it redirect calls.
 *
 * \a text string to translate
 *
 * \a comment Qt equivalent of disambiguation context
 *
 * Returns translated string
 *
 * \relates KLocalizedString
 */
inline QString tr2i18n(const char *text, const char *comment = nullptr)
{
    if (comment && comment[0] && text && text[0]) {
        return ki18nc(comment, text).toString();
    } else if (text && text[0]) {
        return ki18n(text).toString();
    } else {
        return QString();
    }
}

/*!
 * Like tr2i18n, but look for translation in a specific domain.
 *
 * Use -tr tr2i18nd option to uic to have it redirect calls.
 *
 * \a domain domain in which to look for translations
 *
 * \a text string to translate
 *
 * \a comment Qt equivalent of disambiguation context
 *
 * Returns translated string
 *
 * \relates KLocalizedString
 */
inline QString tr2i18nd(const char *domain, const char *text, const char *comment = nullptr)
{
    if (comment && comment[0] && text && text[0]) {
        return ki18ndc(domain, comment, text).toString();
    } else if (text && text[0]) {
        return ki18nd(domain, text).toString();
    } else {
        return QString();
    }
}

/*!
 * Like tr2i18n, but when UI strings are KUIT markup-aware.
 *
 * Use -tr tr2xi18n option to uic to have it redirect calls.
 *
 * \a text markup-aware string to translate
 *
 * \a comment Qt equivalent of disambiguation context
 *
 * Returns translated string
 *
 * \relates KLocalizedString
 */
inline QString tr2xi18n(const char *text, const char *comment = nullptr)
{
    if (comment && comment[0] && text && text[0]) {
        return kxi18nc(comment, text).toString();
    } else if (text && text[0]) {
        return kxi18n(text).toString();
    } else {
        return QString();
    }
}

/*!
 * Like tr2xi18n, but look for translation in a specific domain.
 *
 * Use -tr tr2xi18nd option to uic to have it redirect calls.
 *
 * \a domain domain in which to look for translations
 *
 * \a text markup-aware string to translate
 *
 * \a comment Qt equivalent of disambiguation context
 *
 * Returns translated string
 *
 * \relates KLocalizedString
 */
inline QString tr2xi18nd(const char *domain, const char *text, const char *comment = nullptr)
{
    if (comment && comment[0] && text && text[0]) {
        return kxi18ndc(domain, comment, text).toString();
    } else if (text && text[0]) {
        return kxi18nd(domain, text).toString();
    } else {
        return QString();
    }
}

#ifndef Q_QDOC

#ifndef NDEBUG
#define I18N_ERR_MSG String_literal_as_second_argument_to_i18n___Perhaps_you_need_i18nc_or_i18np
template<typename T, int s>
class I18nTypeCheck
{
public:
    static void I18N_ERR_MSG()
    {
    }
};
template<int s>
class I18nTypeCheck<char[s], s>
{
};
#define STATIC_ASSERT_NOT_LITERAL_STRING(T) I18nTypeCheck<T, sizeof(T)>::I18N_ERR_MSG();
#else
#define STATIC_ASSERT_NOT_LITERAL_STRING(T)
#endif

// >>>>> Basic calls
// Autogenerated; contact maintainer for batch changes.
inline QString i18n(const char *text)
{
    return ki18n(text).toString();
}
template<typename A1>
inline QString i18n(const char *text, const A1 &a1)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18n(text).subs(a1).toString();
}
template<typename A1, typename A2>
inline QString i18n(const char *text, const A1 &a1, const A2 &a2)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18n(text).subs(a1).subs(a2).toString();
}
template<typename A1, typename A2, typename A3>
inline QString i18n(const char *text, const A1 &a1, const A2 &a2, const A3 &a3)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18n(text).subs(a1).subs(a2).subs(a3).toString();
}
template<typename A1, typename A2, typename A3, typename A4>
inline QString i18n(const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18n(text).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString i18n(const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18n(text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline QString i18n(const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18n(text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline QString i18n(const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18n(text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline QString i18n(const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18n(text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline QString
i18n(const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8, const A9 &a9)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18n(text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).subs(a9).toString();
}
// <<<<<<< End of basic calls

// >>>>> Context calls
// Autogenerated; contact maintainer for batch changes.
inline QString i18nc(const char *context, const char *text)
{
    return ki18nc(context, text).toString();
}
template<typename A1>
inline QString i18nc(const char *context, const char *text, const A1 &a1)
{
    return ki18nc(context, text).subs(a1).toString();
}
template<typename A1, typename A2>
inline QString i18nc(const char *context, const char *text, const A1 &a1, const A2 &a2)
{
    return ki18nc(context, text).subs(a1).subs(a2).toString();
}
template<typename A1, typename A2, typename A3>
inline QString i18nc(const char *context, const char *text, const A1 &a1, const A2 &a2, const A3 &a3)
{
    return ki18nc(context, text).subs(a1).subs(a2).subs(a3).toString();
}
template<typename A1, typename A2, typename A3, typename A4>
inline QString i18nc(const char *context, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    return ki18nc(context, text).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString i18nc(const char *context, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    return ki18nc(context, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline QString i18nc(const char *context, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
    return ki18nc(context, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline QString i18nc(const char *context, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7)
{
    return ki18nc(context, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline QString
i18nc(const char *context, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8)
{
    return ki18nc(context, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline QString i18nc(const char *context,
                     const char *text,
                     const A1 &a1,
                     const A2 &a2,
                     const A3 &a3,
                     const A4 &a4,
                     const A5 &a5,
                     const A6 &a6,
                     const A7 &a7,
                     const A8 &a8,
                     const A9 &a9)
{
    return ki18nc(context, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).subs(a9).toString();
}
// <<<<< End of context calls

// >>>>> Plural calls
// Autogenerated; contact maintainer for batch changes.
template<typename A1>
inline QString i18np(const char *singular, const char *plural, const A1 &a1)
{
    return ki18np(singular, plural).subs(a1).toString();
}
template<typename A1, typename A2>
inline QString i18np(const char *singular, const char *plural, const A1 &a1, const A2 &a2)
{
    return ki18np(singular, plural).subs(a1).subs(a2).toString();
}
template<typename A1, typename A2, typename A3>
inline QString i18np(const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3)
{
    return ki18np(singular, plural).subs(a1).subs(a2).subs(a3).toString();
}
template<typename A1, typename A2, typename A3, typename A4>
inline QString i18np(const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    return ki18np(singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString i18np(const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    return ki18np(singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline QString i18np(const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
    return ki18np(singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline QString i18np(const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7)
{
    return ki18np(singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline QString
i18np(const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8)
{
    return ki18np(singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline QString i18np(const char *singular,
                     const char *plural,
                     const A1 &a1,
                     const A2 &a2,
                     const A3 &a3,
                     const A4 &a4,
                     const A5 &a5,
                     const A6 &a6,
                     const A7 &a7,
                     const A8 &a8,
                     const A9 &a9)
{
    return ki18np(singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).subs(a9).toString();
}
// <<<<< End of plural calls

// >>>>> Context-plural calls
// Autogenerated; contact maintainer for batch changes.
template<typename A1>
inline QString i18ncp(const char *context, const char *singular, const char *plural, const A1 &a1)
{
    return ki18ncp(context, singular, plural).subs(a1).toString();
}
template<typename A1, typename A2>
inline QString i18ncp(const char *context, const char *singular, const char *plural, const A1 &a1, const A2 &a2)
{
    return ki18ncp(context, singular, plural).subs(a1).subs(a2).toString();
}
template<typename A1, typename A2, typename A3>
inline QString i18ncp(const char *context, const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3)
{
    return ki18ncp(context, singular, plural).subs(a1).subs(a2).subs(a3).toString();
}
template<typename A1, typename A2, typename A3, typename A4>
inline QString i18ncp(const char *context, const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    return ki18ncp(context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString i18ncp(const char *context, const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    return ki18ncp(context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline QString
i18ncp(const char *context, const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
    return ki18ncp(context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline QString i18ncp(const char *context,
                      const char *singular,
                      const char *plural,
                      const A1 &a1,
                      const A2 &a2,
                      const A3 &a3,
                      const A4 &a4,
                      const A5 &a5,
                      const A6 &a6,
                      const A7 &a7)
{
    return ki18ncp(context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline QString i18ncp(const char *context,
                      const char *singular,
                      const char *plural,
                      const A1 &a1,
                      const A2 &a2,
                      const A3 &a3,
                      const A4 &a4,
                      const A5 &a5,
                      const A6 &a6,
                      const A7 &a7,
                      const A8 &a8)
{
    return ki18ncp(context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline QString i18ncp(const char *context,
                      const char *singular,
                      const char *plural,
                      const A1 &a1,
                      const A2 &a2,
                      const A3 &a3,
                      const A4 &a4,
                      const A5 &a5,
                      const A6 &a6,
                      const A7 &a7,
                      const A8 &a8,
                      const A9 &a9)
{
    return ki18ncp(context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).subs(a9).toString();
}
// <<<<< End of context-plural calls

// >>>>> Basic calls with domain
// Autogenerated; contact maintainer for batch changes.
inline QString i18nd(const char *domain, const char *text)
{
    return ki18nd(domain, text).toString();
}
template<typename A1>
inline QString i18nd(const char *domain, const char *text, const A1 &a1)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18nd(domain, text).subs(a1).toString();
}
template<typename A1, typename A2>
inline QString i18nd(const char *domain, const char *text, const A1 &a1, const A2 &a2)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18nd(domain, text).subs(a1).subs(a2).toString();
}
template<typename A1, typename A2, typename A3>
inline QString i18nd(const char *domain, const char *text, const A1 &a1, const A2 &a2, const A3 &a3)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18nd(domain, text).subs(a1).subs(a2).subs(a3).toString();
}
template<typename A1, typename A2, typename A3, typename A4>
inline QString i18nd(const char *domain, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18nd(domain, text).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString i18nd(const char *domain, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18nd(domain, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline QString i18nd(const char *domain, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18nd(domain, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline QString i18nd(const char *domain, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18nd(domain, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline QString
i18nd(const char *domain, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18nd(domain, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline QString i18nd(const char *domain,
                     const char *text,
                     const A1 &a1,
                     const A2 &a2,
                     const A3 &a3,
                     const A4 &a4,
                     const A5 &a5,
                     const A6 &a6,
                     const A7 &a7,
                     const A8 &a8,
                     const A9 &a9)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return ki18nd(domain, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).subs(a9).toString();
}
// <<<<<<< End of basic calls with domain

// >>>>> Context calls with domain
// Autogenerated; contact maintainer for batch changes.
inline QString i18ndc(const char *domain, const char *context, const char *text)
{
    return ki18ndc(domain, context, text).toString();
}
template<typename A1>
inline QString i18ndc(const char *domain, const char *context, const char *text, const A1 &a1)
{
    return ki18ndc(domain, context, text).subs(a1).toString();
}
template<typename A1, typename A2>
inline QString i18ndc(const char *domain, const char *context, const char *text, const A1 &a1, const A2 &a2)
{
    return ki18ndc(domain, context, text).subs(a1).subs(a2).toString();
}
template<typename A1, typename A2, typename A3>
inline QString i18ndc(const char *domain, const char *context, const char *text, const A1 &a1, const A2 &a2, const A3 &a3)
{
    return ki18ndc(domain, context, text).subs(a1).subs(a2).subs(a3).toString();
}
template<typename A1, typename A2, typename A3, typename A4>
inline QString i18ndc(const char *domain, const char *context, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    return ki18ndc(domain, context, text).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString i18ndc(const char *domain, const char *context, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    return ki18ndc(domain, context, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline QString
i18ndc(const char *domain, const char *context, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
    return ki18ndc(domain, context, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline QString i18ndc(const char *domain,
                      const char *context,
                      const char *text,
                      const A1 &a1,
                      const A2 &a2,
                      const A3 &a3,
                      const A4 &a4,
                      const A5 &a5,
                      const A6 &a6,
                      const A7 &a7)
{
    return ki18ndc(domain, context, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline QString i18ndc(const char *domain,
                      const char *context,
                      const char *text,
                      const A1 &a1,
                      const A2 &a2,
                      const A3 &a3,
                      const A4 &a4,
                      const A5 &a5,
                      const A6 &a6,
                      const A7 &a7,
                      const A8 &a8)
{
    return ki18ndc(domain, context, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline QString i18ndc(const char *domain,
                      const char *context,
                      const char *text,
                      const A1 &a1,
                      const A2 &a2,
                      const A3 &a3,
                      const A4 &a4,
                      const A5 &a5,
                      const A6 &a6,
                      const A7 &a7,
                      const A8 &a8,
                      const A9 &a9)
{
    return ki18ndc(domain, context, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).subs(a9).toString();
}
// <<<<< End of context calls with domain

// >>>>> Plural calls with domain
// Autogenerated; contact maintainer for batch changes.
template<typename A1>
inline QString i18ndp(const char *domain, const char *singular, const char *plural, const A1 &a1)
{
    return ki18ndp(domain, singular, plural).subs(a1).toString();
}
template<typename A1, typename A2>
inline QString i18ndp(const char *domain, const char *singular, const char *plural, const A1 &a1, const A2 &a2)
{
    return ki18ndp(domain, singular, plural).subs(a1).subs(a2).toString();
}
template<typename A1, typename A2, typename A3>
inline QString i18ndp(const char *domain, const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3)
{
    return ki18ndp(domain, singular, plural).subs(a1).subs(a2).subs(a3).toString();
}
template<typename A1, typename A2, typename A3, typename A4>
inline QString i18ndp(const char *domain, const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    return ki18ndp(domain, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString i18ndp(const char *domain, const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    return ki18ndp(domain, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline QString
i18ndp(const char *domain, const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
    return ki18ndp(domain, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline QString i18ndp(const char *domain,
                      const char *singular,
                      const char *plural,
                      const A1 &a1,
                      const A2 &a2,
                      const A3 &a3,
                      const A4 &a4,
                      const A5 &a5,
                      const A6 &a6,
                      const A7 &a7)
{
    return ki18ndp(domain, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline QString i18ndp(const char *domain,
                      const char *singular,
                      const char *plural,
                      const A1 &a1,
                      const A2 &a2,
                      const A3 &a3,
                      const A4 &a4,
                      const A5 &a5,
                      const A6 &a6,
                      const A7 &a7,
                      const A8 &a8)
{
    return ki18ndp(domain, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline QString i18ndp(const char *domain,
                      const char *singular,
                      const char *plural,
                      const A1 &a1,
                      const A2 &a2,
                      const A3 &a3,
                      const A4 &a4,
                      const A5 &a5,
                      const A6 &a6,
                      const A7 &a7,
                      const A8 &a8,
                      const A9 &a9)
{
    return ki18ndp(domain, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).subs(a9).toString();
}
// <<<<< End of plural calls with domain

// >>>>> Context-plural calls with domain
// Autogenerated; contact maintainer for batch changes.
template<typename A1>
inline QString i18ndcp(const char *domain, const char *context, const char *singular, const char *plural, const A1 &a1)
{
    return ki18ndcp(domain, context, singular, plural).subs(a1).toString();
}
template<typename A1, typename A2>
inline QString i18ndcp(const char *domain, const char *context, const char *singular, const char *plural, const A1 &a1, const A2 &a2)
{
    return ki18ndcp(domain, context, singular, plural).subs(a1).subs(a2).toString();
}
template<typename A1, typename A2, typename A3>
inline QString i18ndcp(const char *domain, const char *context, const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3)
{
    return ki18ndcp(domain, context, singular, plural).subs(a1).subs(a2).subs(a3).toString();
}
template<typename A1, typename A2, typename A3, typename A4>
inline QString
i18ndcp(const char *domain, const char *context, const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    return ki18ndcp(domain, context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString
i18ndcp(const char *domain, const char *context, const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    return ki18ndcp(domain, context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline QString i18ndcp(const char *domain,
                       const char *context,
                       const char *singular,
                       const char *plural,
                       const A1 &a1,
                       const A2 &a2,
                       const A3 &a3,
                       const A4 &a4,
                       const A5 &a5,
                       const A6 &a6)
{
    return ki18ndcp(domain, context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline QString i18ndcp(const char *domain,
                       const char *context,
                       const char *singular,
                       const char *plural,
                       const A1 &a1,
                       const A2 &a2,
                       const A3 &a3,
                       const A4 &a4,
                       const A5 &a5,
                       const A6 &a6,
                       const A7 &a7)
{
    return ki18ndcp(domain, context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline QString i18ndcp(const char *domain,
                       const char *context,
                       const char *singular,
                       const char *plural,
                       const A1 &a1,
                       const A2 &a2,
                       const A3 &a3,
                       const A4 &a4,
                       const A5 &a5,
                       const A6 &a6,
                       const A7 &a7,
                       const A8 &a8)
{
    return ki18ndcp(domain, context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline QString i18ndcp(const char *domain,
                       const char *context,
                       const char *singular,
                       const char *plural,
                       const A1 &a1,
                       const A2 &a2,
                       const A3 &a3,
                       const A4 &a4,
                       const A5 &a5,
                       const A6 &a6,
                       const A7 &a7,
                       const A8 &a8,
                       const A9 &a9)
{
    return ki18ndcp(domain, context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).subs(a9).toString();
}
// <<<<< End of context-plural calls with domain

// >>>>> Markup-aware basic calls
// Autogenerated; contact maintainer for batch changes.
inline QString xi18n(const char *text)
{
    return kxi18n(text).toString();
}
template<typename A1>
inline QString xi18n(const char *text, const A1 &a1)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return kxi18n(text).subs(a1).toString();
}
template<typename A1, typename A2>
inline QString xi18n(const char *text, const A1 &a1, const A2 &a2)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return kxi18n(text).subs(a1).subs(a2).toString();
}
template<typename A1, typename A2, typename A3>
inline QString xi18n(const char *text, const A1 &a1, const A2 &a2, const A3 &a3)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return kxi18n(text).subs(a1).subs(a2).subs(a3).toString();
}
template<typename A1, typename A2, typename A3, typename A4>
inline QString xi18n(const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return kxi18n(text).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString xi18n(const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return kxi18n(text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline QString xi18n(const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return kxi18n(text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline QString xi18n(const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return kxi18n(text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline QString xi18n(const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return kxi18n(text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline QString
xi18n(const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8, const A9 &a9)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return kxi18n(text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).subs(a9).toString();
}
// <<<<<<< End of markup-aware basic calls

// >>>>> Markup-aware context calls
// Autogenerated; contact maintainer for batch changes.
inline QString xi18nc(const char *context, const char *text)
{
    return kxi18nc(context, text).toString();
}
template<typename A1>
inline QString xi18nc(const char *context, const char *text, const A1 &a1)
{
    return kxi18nc(context, text).subs(a1).toString();
}
template<typename A1, typename A2>
inline QString xi18nc(const char *context, const char *text, const A1 &a1, const A2 &a2)
{
    return kxi18nc(context, text).subs(a1).subs(a2).toString();
}
template<typename A1, typename A2, typename A3>
inline QString xi18nc(const char *context, const char *text, const A1 &a1, const A2 &a2, const A3 &a3)
{
    return kxi18nc(context, text).subs(a1).subs(a2).subs(a3).toString();
}
template<typename A1, typename A2, typename A3, typename A4>
inline QString xi18nc(const char *context, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    return kxi18nc(context, text).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString xi18nc(const char *context, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    return kxi18nc(context, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline QString xi18nc(const char *context, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
    return kxi18nc(context, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline QString xi18nc(const char *context, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7)
{
    return kxi18nc(context, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline QString
xi18nc(const char *context, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8)
{
    return kxi18nc(context, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline QString xi18nc(const char *context,
                      const char *text,
                      const A1 &a1,
                      const A2 &a2,
                      const A3 &a3,
                      const A4 &a4,
                      const A5 &a5,
                      const A6 &a6,
                      const A7 &a7,
                      const A8 &a8,
                      const A9 &a9)
{
    return kxi18nc(context, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).subs(a9).toString();
}
// <<<<< End of markup-aware context calls

// >>>>> Markup-aware plural calls
// Autogenerated; contact maintainer for batch changes.
template<typename A1>
inline QString xi18np(const char *singular, const char *plural, const A1 &a1)
{
    return kxi18np(singular, plural).subs(a1).toString();
}
template<typename A1, typename A2>
inline QString xi18np(const char *singular, const char *plural, const A1 &a1, const A2 &a2)
{
    return kxi18np(singular, plural).subs(a1).subs(a2).toString();
}
template<typename A1, typename A2, typename A3>
inline QString xi18np(const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3)
{
    return kxi18np(singular, plural).subs(a1).subs(a2).subs(a3).toString();
}
template<typename A1, typename A2, typename A3, typename A4>
inline QString xi18np(const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    return kxi18np(singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString xi18np(const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    return kxi18np(singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline QString xi18np(const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
    return kxi18np(singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline QString
xi18np(const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7)
{
    return kxi18np(singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline QString
xi18np(const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8)
{
    return kxi18np(singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline QString xi18np(const char *singular,
                      const char *plural,
                      const A1 &a1,
                      const A2 &a2,
                      const A3 &a3,
                      const A4 &a4,
                      const A5 &a5,
                      const A6 &a6,
                      const A7 &a7,
                      const A8 &a8,
                      const A9 &a9)
{
    return kxi18np(singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).subs(a9).toString();
}
// <<<<< End of markup-aware plural calls

// >>>>> Markup-aware context-plural calls
// Autogenerated; contact maintainer for batch changes.
template<typename A1>
inline QString xi18ncp(const char *context, const char *singular, const char *plural, const A1 &a1)
{
    return kxi18ncp(context, singular, plural).subs(a1).toString();
}
template<typename A1, typename A2>
inline QString xi18ncp(const char *context, const char *singular, const char *plural, const A1 &a1, const A2 &a2)
{
    return kxi18ncp(context, singular, plural).subs(a1).subs(a2).toString();
}
template<typename A1, typename A2, typename A3>
inline QString xi18ncp(const char *context, const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3)
{
    return kxi18ncp(context, singular, plural).subs(a1).subs(a2).subs(a3).toString();
}
template<typename A1, typename A2, typename A3, typename A4>
inline QString xi18ncp(const char *context, const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    return kxi18ncp(context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString xi18ncp(const char *context, const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    return kxi18ncp(context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline QString
xi18ncp(const char *context, const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
    return kxi18ncp(context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline QString xi18ncp(const char *context,
                       const char *singular,
                       const char *plural,
                       const A1 &a1,
                       const A2 &a2,
                       const A3 &a3,
                       const A4 &a4,
                       const A5 &a5,
                       const A6 &a6,
                       const A7 &a7)
{
    return kxi18ncp(context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline QString xi18ncp(const char *context,
                       const char *singular,
                       const char *plural,
                       const A1 &a1,
                       const A2 &a2,
                       const A3 &a3,
                       const A4 &a4,
                       const A5 &a5,
                       const A6 &a6,
                       const A7 &a7,
                       const A8 &a8)
{
    return kxi18ncp(context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline QString xi18ncp(const char *context,
                       const char *singular,
                       const char *plural,
                       const A1 &a1,
                       const A2 &a2,
                       const A3 &a3,
                       const A4 &a4,
                       const A5 &a5,
                       const A6 &a6,
                       const A7 &a7,
                       const A8 &a8,
                       const A9 &a9)
{
    return kxi18ncp(context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).subs(a9).toString();
}
// <<<<< End of markup-aware context-plural calls

// >>>>> Markup-aware basic calls with domain
// Autogenerated; contact maintainer for batch changes.
inline QString xi18nd(const char *domain, const char *text)
{
    return kxi18nd(domain, text).toString();
}
template<typename A1>
inline QString xi18nd(const char *domain, const char *text, const A1 &a1)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return kxi18nd(domain, text).subs(a1).toString();
}
template<typename A1, typename A2>
inline QString xi18nd(const char *domain, const char *text, const A1 &a1, const A2 &a2)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return kxi18nd(domain, text).subs(a1).subs(a2).toString();
}
template<typename A1, typename A2, typename A3>
inline QString xi18nd(const char *domain, const char *text, const A1 &a1, const A2 &a2, const A3 &a3)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return kxi18nd(domain, text).subs(a1).subs(a2).subs(a3).toString();
}
template<typename A1, typename A2, typename A3, typename A4>
inline QString xi18nd(const char *domain, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return kxi18nd(domain, text).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString xi18nd(const char *domain, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return kxi18nd(domain, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline QString xi18nd(const char *domain, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return kxi18nd(domain, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline QString xi18nd(const char *domain, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return kxi18nd(domain, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline QString
xi18nd(const char *domain, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6, const A7 &a7, const A8 &a8)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return kxi18nd(domain, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline QString xi18nd(const char *domain,
                      const char *text,
                      const A1 &a1,
                      const A2 &a2,
                      const A3 &a3,
                      const A4 &a4,
                      const A5 &a5,
                      const A6 &a6,
                      const A7 &a7,
                      const A8 &a8,
                      const A9 &a9)
{
    STATIC_ASSERT_NOT_LITERAL_STRING(A1)
    return kxi18nd(domain, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).subs(a9).toString();
}
// <<<<<<< End of markup-aware basic calls with domain

// >>>>> Markup-aware context calls with domain
// Autogenerated; contact maintainer for batch changes.
inline QString xi18ndc(const char *domain, const char *context, const char *text)
{
    return kxi18ndc(domain, context, text).toString();
}
template<typename A1>
inline QString xi18ndc(const char *domain, const char *context, const char *text, const A1 &a1)
{
    return kxi18ndc(domain, context, text).subs(a1).toString();
}
template<typename A1, typename A2>
inline QString xi18ndc(const char *domain, const char *context, const char *text, const A1 &a1, const A2 &a2)
{
    return kxi18ndc(domain, context, text).subs(a1).subs(a2).toString();
}
template<typename A1, typename A2, typename A3>
inline QString xi18ndc(const char *domain, const char *context, const char *text, const A1 &a1, const A2 &a2, const A3 &a3)
{
    return kxi18ndc(domain, context, text).subs(a1).subs(a2).subs(a3).toString();
}
template<typename A1, typename A2, typename A3, typename A4>
inline QString xi18ndc(const char *domain, const char *context, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    return kxi18ndc(domain, context, text).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString xi18ndc(const char *domain, const char *context, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    return kxi18ndc(domain, context, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline QString
xi18ndc(const char *domain, const char *context, const char *text, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
    return kxi18ndc(domain, context, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline QString xi18ndc(const char *domain,
                       const char *context,
                       const char *text,
                       const A1 &a1,
                       const A2 &a2,
                       const A3 &a3,
                       const A4 &a4,
                       const A5 &a5,
                       const A6 &a6,
                       const A7 &a7)
{
    return kxi18ndc(domain, context, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline QString xi18ndc(const char *domain,
                       const char *context,
                       const char *text,
                       const A1 &a1,
                       const A2 &a2,
                       const A3 &a3,
                       const A4 &a4,
                       const A5 &a5,
                       const A6 &a6,
                       const A7 &a7,
                       const A8 &a8)
{
    return kxi18ndc(domain, context, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline QString xi18ndc(const char *domain,
                       const char *context,
                       const char *text,
                       const A1 &a1,
                       const A2 &a2,
                       const A3 &a3,
                       const A4 &a4,
                       const A5 &a5,
                       const A6 &a6,
                       const A7 &a7,
                       const A8 &a8,
                       const A9 &a9)
{
    return kxi18ndc(domain, context, text).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).subs(a9).toString();
}
// <<<<< End of markup-aware context calls with domain

// >>>>> Markup-aware plural calls with domain
// Autogenerated; contact maintainer for batch changes.
template<typename A1>
inline QString xi18ndp(const char *domain, const char *singular, const char *plural, const A1 &a1)
{
    return kxi18ndp(domain, singular, plural).subs(a1).toString();
}
template<typename A1, typename A2>
inline QString xi18ndp(const char *domain, const char *singular, const char *plural, const A1 &a1, const A2 &a2)
{
    return kxi18ndp(domain, singular, plural).subs(a1).subs(a2).toString();
}
template<typename A1, typename A2, typename A3>
inline QString xi18ndp(const char *domain, const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3)
{
    return kxi18ndp(domain, singular, plural).subs(a1).subs(a2).subs(a3).toString();
}
template<typename A1, typename A2, typename A3, typename A4>
inline QString xi18ndp(const char *domain, const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    return kxi18ndp(domain, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString xi18ndp(const char *domain, const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5)
{
    return kxi18ndp(domain, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline QString
xi18ndp(const char *domain, const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4, const A5 &a5, const A6 &a6)
{
    return kxi18ndp(domain, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline QString xi18ndp(const char *domain,
                       const char *singular,
                       const char *plural,
                       const A1 &a1,
                       const A2 &a2,
                       const A3 &a3,
                       const A4 &a4,
                       const A5 &a5,
                       const A6 &a6,
                       const A7 &a7)
{
    return kxi18ndp(domain, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline QString xi18ndp(const char *domain,
                       const char *singular,
                       const char *plural,
                       const A1 &a1,
                       const A2 &a2,
                       const A3 &a3,
                       const A4 &a4,
                       const A5 &a5,
                       const A6 &a6,
                       const A7 &a7,
                       const A8 &a8)
{
    return kxi18ndp(domain, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline QString xi18ndp(const char *domain,
                       const char *singular,
                       const char *plural,
                       const A1 &a1,
                       const A2 &a2,
                       const A3 &a3,
                       const A4 &a4,
                       const A5 &a5,
                       const A6 &a6,
                       const A7 &a7,
                       const A8 &a8,
                       const A9 &a9)
{
    return kxi18ndp(domain, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).subs(a9).toString();
}
// <<<<< End of markup-aware plural calls with domain

// >>>>> Markup-aware context-plural calls with domain
// Autogenerated; contact maintainer for batch changes.
template<typename A1>
inline QString xi18ndcp(const char *domain, const char *context, const char *singular, const char *plural, const A1 &a1)
{
    return kxi18ndcp(domain, context, singular, plural).subs(a1).toString();
}
template<typename A1, typename A2>
inline QString xi18ndcp(const char *domain, const char *context, const char *singular, const char *plural, const A1 &a1, const A2 &a2)
{
    return kxi18ndcp(domain, context, singular, plural).subs(a1).subs(a2).toString();
}
template<typename A1, typename A2, typename A3>
inline QString xi18ndcp(const char *domain, const char *context, const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3)
{
    return kxi18ndcp(domain, context, singular, plural).subs(a1).subs(a2).subs(a3).toString();
}
template<typename A1, typename A2, typename A3, typename A4>
inline QString
xi18ndcp(const char *domain, const char *context, const char *singular, const char *plural, const A1 &a1, const A2 &a2, const A3 &a3, const A4 &a4)
{
    return kxi18ndcp(domain, context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5>
inline QString xi18ndcp(const char *domain,
                        const char *context,
                        const char *singular,
                        const char *plural,
                        const A1 &a1,
                        const A2 &a2,
                        const A3 &a3,
                        const A4 &a4,
                        const A5 &a5)
{
    return kxi18ndcp(domain, context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6>
inline QString xi18ndcp(const char *domain,
                        const char *context,
                        const char *singular,
                        const char *plural,
                        const A1 &a1,
                        const A2 &a2,
                        const A3 &a3,
                        const A4 &a4,
                        const A5 &a5,
                        const A6 &a6)
{
    return kxi18ndcp(domain, context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7>
inline QString xi18ndcp(const char *domain,
                        const char *context,
                        const char *singular,
                        const char *plural,
                        const A1 &a1,
                        const A2 &a2,
                        const A3 &a3,
                        const A4 &a4,
                        const A5 &a5,
                        const A6 &a6,
                        const A7 &a7)
{
    return kxi18ndcp(domain, context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8>
inline QString xi18ndcp(const char *domain,
                        const char *context,
                        const char *singular,
                        const char *plural,
                        const A1 &a1,
                        const A2 &a2,
                        const A3 &a3,
                        const A4 &a4,
                        const A5 &a5,
                        const A6 &a6,
                        const A7 &a7,
                        const A8 &a8)
{
    return kxi18ndcp(domain, context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).toString();
}
template<typename A1, typename A2, typename A3, typename A4, typename A5, typename A6, typename A7, typename A8, typename A9>
inline QString xi18ndcp(const char *domain,
                        const char *context,
                        const char *singular,
                        const char *plural,
                        const A1 &a1,
                        const A2 &a2,
                        const A3 &a3,
                        const A4 &a4,
                        const A5 &a5,
                        const A6 &a6,
                        const A7 &a7,
                        const A8 &a8,
                        const A9 &a9)
{
    return kxi18ndcp(domain, context, singular, plural).subs(a1).subs(a2).subs(a3).subs(a4).subs(a5).subs(a6).subs(a7).subs(a8).subs(a9).toString();
}
// <<<<< End of markup-aware context-plural calls with domain

#endif // Q_QDOC

#endif // KLOCALIZEDSTRING_H

#ifndef Q_QDOC

// Outside of include guards, to be able to map and unmap domains
// by successive inclusions of this header
// preceded with different definitions of TRANSLATION_DOMAIN.
#ifdef TRANSLATION_DOMAIN
#define i18n(...) i18nd(TRANSLATION_DOMAIN, __VA_ARGS__)
#define i18nc(...) i18ndc(TRANSLATION_DOMAIN, __VA_ARGS__)
#define i18np(...) i18ndp(TRANSLATION_DOMAIN, __VA_ARGS__)
#define i18ncp(...) i18ndcp(TRANSLATION_DOMAIN, __VA_ARGS__)
#define ki18n(...) ki18nd(TRANSLATION_DOMAIN, __VA_ARGS__)
#define ki18nc(...) ki18ndc(TRANSLATION_DOMAIN, __VA_ARGS__)
#define ki18np(...) ki18ndp(TRANSLATION_DOMAIN, __VA_ARGS__)
#define ki18ncp(...) ki18ndcp(TRANSLATION_DOMAIN, __VA_ARGS__)
#define tr2i18n(...) tr2i18nd(TRANSLATION_DOMAIN, __VA_ARGS__)
#define xi18n(...) xi18nd(TRANSLATION_DOMAIN, __VA_ARGS__)
#define xi18nc(...) xi18ndc(TRANSLATION_DOMAIN, __VA_ARGS__)
#define xi18np(...) xi18ndp(TRANSLATION_DOMAIN, __VA_ARGS__)
#define xi18ncp(...) xi18ndcp(TRANSLATION_DOMAIN, __VA_ARGS__)
#define kxi18n(...) kxi18nd(TRANSLATION_DOMAIN, __VA_ARGS__)
#define kxi18nc(...) kxi18ndc(TRANSLATION_DOMAIN, __VA_ARGS__)
#define kxi18np(...) kxi18ndp(TRANSLATION_DOMAIN, __VA_ARGS__)
#define kxi18ncp(...) kxi18ndcp(TRANSLATION_DOMAIN, __VA_ARGS__)
#define tr2xi18n(...) tr2xi18nd(TRANSLATION_DOMAIN, __VA_ARGS__)
#else
#undef i18n
#undef i18nc
#undef i18np
#undef i18ncp
#undef ki18n
#undef ki18nc
#undef ki18np
#undef ki18ncp
#undef tr2i18n
#undef xi18n
#undef xi18nc
#undef xi18np
#undef xi18ncp
#undef kxi18n
#undef kxi18nc
#undef kxi18np
#undef kxi18ncp
#undef tr2xi18n
#endif

#endif // Q_QDOC
