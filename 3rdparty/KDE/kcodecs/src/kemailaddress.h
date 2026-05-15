/*
    SPDX-FileCopyrightText: 2004 Matt Douhan <matt@fruitsalad.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCODECS_EMAILADDRESS_H
#define KCODECS_EMAILADDRESS_H

#include <QUrl>

#include <QByteArray>
#include <QStringList>

#include <kcodecs_export.h>

/*!
 * \namespace KEmailAddress
 * \inmodule KCodecs
 *
 * \since 5.5
 */
namespace KEmailAddress
{

/*!
  \enum KEmailAddress::EmailParseResult

  Email validation result. The only 'success' code in
  this enumeration is AddressOK; all the other values
  indicate some specific problem with the address which
  is being validated.

  Result type for splitAddress(), isValidAddress()
  and isValidSimpleAddress().

  \value AddressOk Email is valid
  \value AddressEmpty The address is empty
  \value UnexpectedEnd Something is unbalanced
  \value UnbalancedParens Unbalanced ( )
  \value MissingDomainPart No domain in address
  \value UnclosedAngleAddr \< with no matching \>
  \value UnopenedAngleAddr \> with no preceding \<
  \value TooManyAts More than one \@ in address
  \value UnexpectedComma Comma not allowed here
  \value TooFewAts Missing \@ in address
  \value MissingLocalPart No address specified, only domain
  \value UnbalancedQuote Quotes (single or double) not matched
  \value NoAddressSpec
  \value DisallowedChar An invalid character detected in address
  \value InvalidDisplayName An invalid displayname detected in address
  \value TooFewDots Missing \. in address
*/
enum EmailParseResult {
    AddressOk,
    AddressEmpty,
    UnexpectedEnd,
    UnbalancedParens,
    MissingDomainPart,
    UnclosedAngleAddr,
    UnopenedAngleAddr,
    TooManyAts,
    UnexpectedComma,
    TooFewAts,
    MissingLocalPart,
    UnbalancedQuote,
    NoAddressSpec,
    DisallowedChar,
    InvalidDisplayName,
    TooFewDots,
};

/*! Split a comma separated list of email addresses.

\a aStr a single string representing a list of addresses

Returns a list of strings, where each string is one address
from the original list
*/
KCODECS_EXPORT
QStringList splitAddressList(const QString &aStr);

/*!
  Splits the given address into display name, email address and comment.
  Returns AddressOk if no error was encountered. Otherwise an appropriate
  error code is returned. In case of an error the values of displayName,
  addrSpec and comment are undefined.

  \a address      a single email address,
  example: Joe User (comment1) <joe.user@example.org> (comment2)

  \a displayName  only out: the display-name of the email address, i.e.
  "Joe User" in the example; in case of an error the
  return value is undefined

  \a addrSpec     only out: the addr-spec, i.e. "joe.user@example.org"
  in the example; in case of an error the return value is undefined

  \a comment      only out: the space-separated comments, i.e.
  "comment1 comment2" in the example; in case of an
  error the return value is undefined

  Returns             AddressOk if no error was encountered. Otherwise an
  appropriate error code is returned.
*/
KCODECS_EXPORT
EmailParseResult splitAddress(const QByteArray &address, QByteArray &displayName, QByteArray &addrSpec, QByteArray &comment);

/*!
  This is an overloaded member function, provided for convenience.
  It behaves essentially like the above function.

  Splits the given address into display name, email address and comment.
  Returns AddressOk if no error was encountered. Otherwise an appropriate
  error code is returned. In case of an error the values of displayName,
  addrSpec and comment are undefined.

  \a address      a single email address,
  example: Joe User (comment1) <joe.user@example.org> (comment2)

  \a displayName  only out: the display-name of the email address, i.e.
  "Joe User" in the example; in case of an error the
  return value is undefined

  \a addrSpec     only out: the addr-spec, i.e. "joe.user@example.org"
  in the example; in case of an error the return value is undefined

  \a comment      only out: the space-separated comments, i.e.
  "comment1 comment2" in the example; in case of an
  error the return value is undefined

  Returns             AddressOk if no error was encountered. Otherwise an
  appropriate error code is returned.
*/
KCODECS_EXPORT
EmailParseResult splitAddress(const QString &address, QString &displayName, QString &addrSpec, QString &comment);

/*!
  Validates an email address in the form of "Joe User" <joe@example.org>.
  Returns AddressOk if no error was encountered. Otherwise an appropriate
  error code is returned.

  \a aStr         a single email address,
  example: Joe User (comment1) <joe.user@example.org>

  Returns             AddressOk if no error was encountered. Otherwise an
  appropriate error code is returned.
*/
KCODECS_EXPORT
EmailParseResult isValidAddress(const QString &aStr);

/*!
  Validates a list of email addresses, and also allow aliases and
  distribution lists to be expanded before validation.

  \a aStr         a string containing a list of email addresses.

  \a badAddr      a string to hold the address that was faulty.

  Returns AddressOk if no error was encountered. Otherwise an
  appropriate error code is returned.
*/
KCODECS_EXPORT
EmailParseResult isValidAddressList(const QString &aStr, QString &badAddr);

/*!
  Translate the enum errorcodes from emailParseResult
  into i18n'd strings that can be used for msg boxes.

  \a errorCode an error code returned from one of the
  email validation functions. Do not pass
  AddressOk as a value, since that will yield
  a misleading error message

  Returns human-readable and already translated message describing
  the validation error.
*/
KCODECS_EXPORT
QString emailParseResultToString(EmailParseResult errorCode);

/*!
  Validates an email address in the form of joe@example.org.
  Returns true if no error was encountered.
  This method should be used when the input field should not
  allow a "full" email address with comments and other special
  cases that normally are valid in an email address.

  \a aStr         a single email address,
  example: joe.user@example.org

  Returns             true if no error was encountered.

  \note This method differs from calling isValidAddress()
  and checking that that returns AddressOk in two ways:
  it is faster, and it does not allow fancy addresses.
*/
KCODECS_EXPORT
bool isValidSimpleAddress(const QString &aStr);

/*!
  Returns a i18n string to be used in msgboxes. This allows for error
  messages to be the same across the board.

  Returns             An i18n ready string for use in msgboxes.
*/
KCODECS_EXPORT
QString simpleEmailAddressErrorMsg();

/*!
  Returns the pure email address (addr-spec in RFC2822) of the given address
  (mailbox in RFC2822).

  \a address  an email address, e.g. "Joe User <joe.user@example.org>"

  Returns         the addr-spec of \a address, i.e. joe.user@example.org
  in the example
*/
KCODECS_EXPORT
QByteArray extractEmailAddress(const QByteArray &address);

/*KF6 merge with above*/

/*!
  Returns the pure email address (addr-spec in RFC2822) of the given address
  (mailbox in RFC2822).

  \a address  an email address, e.g. "Joe User <joe.user@example.org>"

  \a errorMessage return error message when we can't parse email

  Returns         the addr-spec of \a address, i.e. joe.user@example.org
  in the example
  \since 5.11.0

*/
KCODECS_EXPORT
QByteArray extractEmailAddress(const QByteArray &address, QString &errorMessage);

/*!
  This is an overloaded member function, provided for convenience.
  It behaves essentially like the above function.

  Returns the pure email address (addr-spec in RFC2822) of the given
  address (mailbox in RFC2822).

  \a address  an email address, e.g. "Joe User <joe.user@example.org>"

  Returns         the addr-spec of \a address, i.e. joe.user@example.org
  in the example
*/
KCODECS_EXPORT
QString extractEmailAddress(const QString &address);

/*!
  Returns the pure email address (addr-spec in RFC2822) of the first
  email address of a list of addresses.

  \a address an email address, e.g. "Joe User <joe.user@example.org>"

  \a errorMessage return error message when we can't parse email

  Returns          the addr-spec of \a address, i.e. joe.user@example.org
  in the example
  \since 5.11
*/
KCODECS_EXPORT
QString extractEmailAddress(const QString &address, QString &errorMessage);

/*KF6 merge with above*/
/*!
  Returns the pure email address (addr-spec in RFC2822) of the first
  email address of a list of addresses.

  \a addresses an email address, e.g. "Joe User <joe.user@example.org>"

  Returns          the addr-spec of \a addresses, i.e. joe.user@example.org
  in the example
*/
KCODECS_EXPORT
QByteArray firstEmailAddress(const QByteArray &addresses);

/*!
  Returns the pure email address (addr-spec in RFC2822) of the first
  email address of a list of addresses.

  \a addresses an email address, e.g. "Joe User <joe.user@example.org>"

  \a errorMessage return error message when we can't parse email

  Returns          the addr-spec of \a addresses, i.e. joe.user@example.org
  in the example
  \since 5.11.0
*/

KCODECS_EXPORT
QByteArray firstEmailAddress(const QByteArray &addresses, QString &errorMessage);

/*!
  This is an overloaded member function, provided for convenience.
  It behaves essentially like the above function.

  Returns the pure email address (addr-spec in RFC2822) of the first
  email address of a list of addresses.

  \a addresses an email address, e.g. "Joe User <joe.user@example.org>"

  Returns          the addr-spec of \a addresses, i.e. joe.user@example.org
  in the example
*/
KCODECS_EXPORT
QString firstEmailAddress(const QString &addresses);

/*!
  This is an overloaded member function, provided for convenience.
  It behaves essentially like the above function.

  Returns the pure email address (addr-spec in RFC2822) of the first
  email address of a list of addresses.

  \a addresses an email address, e.g. "Joe User <joe.user@example.org>"

  \a errorMessage return error message when we can't parse email

  Returns          the addr-spec of \a addresses, i.e. joe.user@example.org
  in the example
  \since 5.11.0
*/
KCODECS_EXPORT
QString firstEmailAddress(const QString &addresses, QString &errorMessage);

/*!
  Return email address and name from string.
  Examples:
    "Stefan Taferner <taferner@example.org>" returns "taferner@example.org"
    and "Stefan Taferner". "joe@example.com" returns "joe@example.com"
    and "". Note that this only returns the first address.

  Also note that the return value is true if both the name and the
  mail are not empty: this does NOT tell you if mail contains a
  valid email address or just some rubbish.

  \a aStr  an email address, e.g "Joe User <joe.user@example.org>"

  \a name  only out: returns the displayname, "Joe User" in the example

  \a mail  only out: returns the email address "joe.user@example.org"
  in the example

  Returns true if both name and email address are not empty
*/
KCODECS_EXPORT
bool extractEmailAddressAndName(const QString &aStr, QString &mail, QString &name);

/*!
  Compare two email addresses. If matchName is false, it just checks
  the email address, and returns true if this matches. If matchName
  is true, both the name and the email must be the same.

  \a email1  the first email address to use for comparison

  \a email2  the second email address to use for comparison

  \a matchName  if set to true email address and displayname must match

  Returns true if the comparison matches true in all other cases
*/
KCODECS_EXPORT
bool compareEmail(const QString &email1, const QString &email2, bool matchName);

/*!
  Returns a normalized address built from the given parts. The normalized
  address is of one the following forms:
    - displayName (comment) &lt;addrSpec&gt;
    - displayName &lt;addrSpec&gt;
    - comment &lt;addrSpec&gt;
    - addrSpec

  \a displayName  the display name of the address

  \a addrSpec     the actual email address (addr-spec in RFC 2822)

  \a comment      a comment

  Returns             a normalized address built from the given parts
*/
KCODECS_EXPORT
QString normalizedAddress(const QString &displayName, const QString &addrSpec, const QString &comment = QString());

/*!
  Decodes the punycode domain part of the given addr-spec if it's an IDN.

  \a addrSpec  a pure 7-bit email address (addr-spec in RFC2822)

  Returns          the email address with Unicode domain
*/
KCODECS_EXPORT
QString fromIdn(const QString &addrSpec);

/*!
  Encodes the domain part of the given addr-spec in punycode if it's an IDN.

  \a addrSpec  a pure email address with Unicode domain

  Returns          the email address with domain in punycode
*/
KCODECS_EXPORT
QString toIdn(const QString &addrSpec);

/*!
  Normalizes all email addresses in the given list and decodes all IDNs.

  \a addresses  a list of email addresses with punycoded IDNs

  Returns           the email addresses in normalized form with Unicode IDNs
*/
KCODECS_EXPORT
QString normalizeAddressesAndDecodeIdn(const QString &addresses);

/*!
  Normalizes all email addresses in the given list and encodes all IDNs
  in punycode.

  \a str  a list of email addresses

  Returns     the email addresses in normalized form
*/
KCODECS_EXPORT
QString normalizeAddressesAndEncodeIdn(const QString &str);

/*!
  Add quote characters around the given string if it contains a
  character that makes that necessary, in an email name, such as ",".

  \a str  a string that may need quoting

  Returns     the string quoted if necessary
*/
KCODECS_EXPORT
QString quoteNameIfNecessary(const QString &str);

/*!
 * Creates a valid mailto: URL from the given mailbox.
 *
 * \a mailbox The mailbox, which means the display name and the address specification, for
 *                example "Thomas McGuire" <thomas@domain.com>. The display name is optional.
 *
 * Returns a valid mailto: URL for the given mailbox.
 */
KCODECS_EXPORT
QUrl encodeMailtoUrl(const QString &mailbox);

/*!
 * Extracts the mailbox out of the mailto: URL.
 *
 * \a mailtoUrl the URL with the mailto protocol, which contains the mailbox to be extracted
 *
 * Returns the mailbox, which means the display name and the address specification.
 */
KCODECS_EXPORT
QString decodeMailtoUrl(const QUrl &mailtoUrl);

} // namespace KEmailAddress

#endif
