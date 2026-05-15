/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 2003, 2007 Oswald Buddenhagen <ossi@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KSHELL_H
#define KSHELL_H

#include <QStringList>
#include <kcoreaddons_export.h>
#include <qglobal.h>

class QString;

/*!
 * \namespace KShell
 * \inmodule KCoreAddons
 * Emulates some basic system shell functionality.
 * \sa KStringHandler
 */
namespace KShell
{
/*!
 * Flags for splitArgs().
 * \value NoOptions No options
 * \value TildeExpand
 * Perform tilde expansion.
 * On Windows, this flag is ignored, as the Windows shell has no
 * equivalent functionality.
 * \value AbortOnMeta
 * Put the parser into full shell mode and bail out if a too complex
 * construct is encountered.
 * A particular purpose of this flag is finding out whether the
 * command line being split would be executable directly (via
 * KProcess::setProgram()) or whether it needs to be run through
 * a real shell (via KProcess::setShellCommand()). Note, however,
 * that shell builtins are not recognized - you need to do that
 * yourself (compare with a list of known commands or verify that an
 * executable exists for the named command).
 *
 * Meta characters that cause a bail-out are the command separators
 * \c semicolon and \c ampersand, the redirection symbols \c less-than,
 * \c greater-than and the \c pipe \c symbol and the grouping symbols
 * opening and closing \c parentheses.
 *
 * Further meta characters on *NIX are the grouping symbols
 * opening and closing \c braces, the command substitution symbol
 * \c backquote, the generic substitution symbol \c dollar (if
 * not followed by an apostrophe), the wildcards \c asterisk,
 * \c question \c mark and opening and closing \c square \c brackets
 * and the comment symbol \c hash \c mark.
 * Additionally, a variable assignment in the first word is recognized.
 *
 * A further meta character on Windows is the environment variable
 * expansion symbol \c percent. Occurrences of \c \%PERCENT_SIGN% as
 * inserted by quoteArg() are converted back and cause no bail-out,
 * though.
 */
enum Option {
    NoOptions = 0,
    TildeExpand = 1,
    AbortOnMeta = 2,
};
Q_DECLARE_FLAGS(Options, Option)
Q_DECLARE_OPERATORS_FOR_FLAGS(Options)

/*!
 * Status codes from splitArgs()
 *
 * \value NoError Success
 * \value BadQuoting Indicates a parsing error, like an unterminated quoted string
 * \value FoundMeta The AbortOnMeta flag was set and an unhandled shell meta character was encountered
 */
enum Errors {
    NoError = 0,
    BadQuoting,
    FoundMeta,
};

/*!
 * Splits \a cmd according to system shell word splitting and quoting rules.
 * Can optionally perform tilde expansion and/or abort if it finds shell
 * meta characters it cannot process.
 *
 * On *NIX the behavior is based on the POSIX shell and bash:
 * \list
 * \li Whitespace splits tokens
 * \li The backslash quotes the following character
 * \li A string enclosed in single quotes is not split. No shell meta
 *   characters are interpreted.
 * \li A string enclosed in double quotes is not split. Within the string,
 *   the backslash quotes shell meta characters - if it is followed
 *   by a "meaningless" character, the backslash is output verbatim.
 * \li A string enclosed in $'' is not split. Within the string, the
 *   backslash has a similar meaning to the one in C strings. Consult
 *   the bash manual for more information.
 * \endlist
 *
 * On Windows, the behavior is defined by the Microsoft C runtime. Qt and
 * many other implementations comply with this standard, but many do not.
 * \list
 * \li Whitespace splits tokens
 * \li A string enclosed in double quotes is not split
 *   - 2N double quotes within a quoted string yield N literal quotes.
 *     This is not documented on MSDN.
 * \li Backslashes have special semantics iff they are followed by a double
 *   quote:
 *   - 2N backslashes + double quote => N backslashes and begin/end quoting
 *   - 2N+1 backslashes + double quote => N backslashes + literal quote
 * \endlist
 *
 * If AbortOnMeta is used on Windows, this function applies cmd shell
 * semantics before proceeding with word splitting:
 * \list
 * \li Cmd ignores all special chars between double quotes.
 *   Note that the quotes are not removed at this stage - the
 *   tokenization rules described above still apply.
 * \li The \c circumflex is the escape char for everything including
 *   itself.
 * \endlist
 *
 * \a cmd the command to split
 *
 * \a flags operation flags, see Option
 *
 * \a err if not NULL, a status code will be stored at the pointer
 *  target, see Errors
 *
 * Returns a list of unquoted words or an empty list if an error occurred
 */
KCOREADDONS_EXPORT QStringList splitArgs(const QString &cmd, Options flags = NoOptions, Errors *err = nullptr);

/*!
 * Quotes and joins \a args together according to system shell rules.
 *
 * If the output is fed back into splitArgs(), the AbortOnMeta flag
 * needs to be used on Windows. On *NIX, no such requirement exists.
 *
 * See quoteArg() for more info.
 *
 * \a args a list of strings to quote and join
 *
 * Returns a command suitable for shell execution
 */
KCOREADDONS_EXPORT QString joinArgs(const QStringList &args);

/*!
 * Quotes \a arg according to system shell rules.
 *
 * This function can be used to quote an argument string such that
 * the shell processes it properly. This is e.g. necessary for
 * user-provided file names which may contain spaces or quotes.
 * It also prevents expansion of wild cards and environment variables.
 *
 * On *NIX, the output is POSIX shell compliant.
 * On Windows, it is compliant with the argument splitting code of the
 * Microsoft C runtime and the cmd shell used together.
 * Occurrences of the \c percent \c sign are replaced with
 * \c % to prevent spurious variable expansion;
 * related KDE functions are prepared for this.
 *
 * \a arg the argument to quote
 *
 * Returns the quoted argument
 */
KCOREADDONS_EXPORT QString quoteArg(const QString &arg);

/*!
 * Performs tilde expansion on \a path. Interprets "~/path" and
 * "~user/path". If the path starts with an escaped tilde ("\~" on UNIX,
 * "^~" on Windows), the escape char is removed and the path is returned
 * as is.
 *
 * Note that if \a path starts with a tilde but cannot be properly expanded,
 * this function will return an empty string.
 *
 * \a path the path to tilde-expand
 *
 * Returns the expanded path
 */
KCOREADDONS_EXPORT QString tildeExpand(const QString &path);

/*!
 * Performs tilde collapse on \a path. If path did not start by the user
 * homedir returns path unchanged.
 *
 * \a path the path to tilde-collpase
 *
 * Returns the collapsed path
 * \since 5.67
 */
KCOREADDONS_EXPORT QString tildeCollapse(const QString &path);
}

#endif /* KSHELL_H */
