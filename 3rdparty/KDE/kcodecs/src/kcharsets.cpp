/*
    This file is part of the KDE libraries

    SPDX-FileCopyrightText: 1999 Lars Knoll <knoll@kde.org>
    SPDX-FileCopyrightText: 2001, 2003, 2004, 2005, 2006 Nicolas GOUTTE <goutte@kde.org>
    SPDX-FileCopyrightText: 2007 Nick Shaforostoff <shafff@ukr.net>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#include "kcharsets.h"
#include "kcodecs_debug.h"

#include <algorithm>
#include <array>
#include <assert.h>
#include <span>
#include <string_view>

/*
 * The encoding names (like "ISO 8859-1") in this list are user-visible,
 * and should be mostly uppercase.
 * Generate with generate_string_table.pl (located in kde-dev-scripts),
 * input data:
ISO 8859-1
i18n:Western European
ISO 8859-15
i18n:Western European
ISO 8859-14
i18n:Western European
cp 1252
i18n:Western European
IBM850
i18n:Western European
ISO 8859-2
i18n:Central European
ISO 8859-3
i18n:Central European
ISO 8859-4
i18n:Baltic
ISO 8859-13
i18n:Baltic
ISO 8859-16
i18n:South-Eastern Europe
cp 1250
i18n:Central European
cp 1254
i18n:Turkish
cp 1257
i18n:Baltic
KOI8-R
i18n:Cyrillic
ISO 8859-5
i18n:Cyrillic
cp 1251
i18n:Cyrillic
KOI8-U
i18n:Cyrillic
IBM866
i18n:Cyrillic
Big5
i18n:Chinese Traditional
Big5-HKSCS
i18n:Chinese Traditional
GB18030
i18n:Chinese Simplified
GBK
i18n:Chinese Simplified
GB2312
i18n:Chinese Simplified
EUC-KR
i18n:Korean
windows-949
i18n:Korean
sjis
i18n:Japanese
ISO-2022-JP
i18n:Japanese
EUC-JP
i18n:Japanese
ISO 8859-7
i18n:Greek
cp 1253
i18n:Greek
ISO 8859-6
i18n:Arabic
cp 1256
i18n:Arabic
ISO 8859-8
i18n:Hebrew
ISO 8859-8-I
i18n:Hebrew
cp 1255
i18n:Hebrew
ISO 8859-9
i18n:Turkish
TIS620
i18n:Thai
ISO 8859-11
i18n:Thai
UTF-8
i18n:Unicode
UTF-16
i18n:Unicode
utf7
i18n:Unicode
ucs2
i18n:Unicode
ISO 10646-UCS-2
i18n:Unicode
windows-1258
i18n:Other
IBM874
i18n:Other
TSCII
i18n:Other
 */
/*
 * Notes about the table:
 *
 * - The following entries were disabled and removed from the table:
ibm852
i18n:Central European
pt 154
i18n:Cyrillic              // ### TODO "PT 154" seems to have been removed from Qt
 *
 * - ISO 8559-11 is the deprecated name of TIS-620
 * - utf7 is not in Qt
 * - UTF-16 is duplicated as "ucs2" and "ISO 10646-UCS-2"
 * - windows-1258: TODO
 * - IBM874: TODO
 * - TSCII: TODO
 */

/*
 * This redefines the QT_TRANSLATE_NOOP3 macro provided by Qt to indicate that
 * statically initialised text should be translated so that it expands to just
 * the string that should be translated, making it possible to use it in the
 * single string construct below.
 */
#undef QT_TRANSLATE_NOOP3
#define QT_TRANSLATE_NOOP3(a, b, c) b

/*
 * THE FOLLOWING CODE IS GENERATED. PLEASE DO NOT EDIT BY HAND.
 * The script used was generate_string_table.pl which can be found in kde-dev-scripts.
 * It was then edited to use QT_TRANSLATE_NOOP3 instead of I18N_NOOP.
 */

static const char language_for_encoding_string[] =
    "ISO 8859-1\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Western European", "@item Text character set")"\0"
    "ISO 8859-15\0"
    "ISO 8859-14\0"
    "cp 1252\0"
    "IBM850\0"
    "ISO 8859-2\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Central European", "@item Text character set")"\0"
    "ISO 8859-3\0"
    "ISO 8859-4\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Baltic", "@item Text character set")"\0"
    "ISO 8859-13\0"
    "ISO 8859-16\0"
    QT_TRANSLATE_NOOP3("KCharsets", "South-Eastern Europe", "@item Text character set")"\0"
    "cp 1250\0"
    "cp 1254\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Turkish", "@item Text character set")"\0"
    "cp 1257\0"
    "KOI8-R\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Cyrillic", "@item Text character set")"\0"
    "ISO 8859-5\0"
    "cp 1251\0"
    "KOI8-U\0"
    "IBM866\0"
    "Big5\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Chinese Traditional", "@item Text character set")"\0"
    "Big5-HKSCS\0"
    "GB18030\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Chinese Simplified", "@item Text character set")"\0"
    "GBK\0"
    "GB2312\0"
    "EUC-KR\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Korean", "@item Text character set")"\0"
    "windows-949\0"
    "sjis\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Japanese", "@item Text character set")"\0"
    "ISO-2022-JP\0"
    "EUC-JP\0"
    "ISO 8859-7\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Greek", "@item Text character set")"\0"
    "cp 1253\0"
    "ISO 8859-6\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Arabic", "@item Text character set")"\0"
    "cp 1256\0"
    "ISO 8859-8\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Hebrew", "@item Text character set")"\0"
    "ISO 8859-8-I\0"
    "cp 1255\0"
    "ISO 8859-9\0"
    "TIS620\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Thai", "@item Text character set")"\0"
    "ISO 8859-11\0"
    "UTF-8\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Unicode", "@item Text character set")"\0"
    "UTF-16\0"
    "utf7\0"
    "ucs2\0"
    "ISO 10646-UCS-2\0"
    "windows-1258\0"
    QT_TRANSLATE_NOOP3("KCharsets", "Other", "@item Text character set")"\0"
    "IBM874\0"
    "TSCII\0"
    "\0";

static const int language_for_encoding_indices[] = {
    0,   11,  28,  11,  40,  11,  52,  11,  60,  11,  67,  78,  95,  78,  106, 117, 124, 117, 136, 148, 169, 78,  177, 185, 193, 117, 201, 208, 217, 208, 228,
    208, 236, 208, 243, 208, 250, 255, 275, 255, 286, 294, 313, 294, 317, 294, 324, 331, 338, 331, 350, 355, 364, 355, 376, 355, 383, 394, 400, 394, 408, 419,
    426, 419, 434, 445, 452, 445, 465, 445, 473, 185, 484, 491, 496, 491, 508, 514, 522, 514, 529, 514, 534, 514, 539, 514, 555, 568, 574, 568, 581, 568, -1};

/*
 * GENERATED CODE ENDS HERE
 */

// search an array of items index/data, find first matching index
// and return data, or return 0
static inline const char *kcharsets_array_search(const char *start, const int *indices, const char *entry)
{
    for (int i = 0; indices[i] != -1; i += 2) {
        if (qstrcmp(start + indices[i], entry) == 0) {
            return start + indices[i + 1];
        }
    }
    return nullptr;
}

class KCharsetsPrivate
{
    // empty class required for binary compatibility
};

// --------------------------------------------------------------------------

constexpr KCharsets::KCharsets() = default;

KCharsets::~KCharsets() = default;

struct KCharsetsSingletonPrivate {
    static constinit KCharsets instance;
};
constinit KCharsets KCharsetsSingletonPrivate::instance{};

// sorted entities list for lookup
struct Entity {
    static constexpr auto MAX_ENTITY_LEN = 8;
    using PaddedName = std::array<char, MAX_ENTITY_LEN>;
    static constexpr PaddedName buildName(std::string_view n)
    {
        PaddedName padded = {0};
        for (std::size_t i = 0; i < n.size(); ++i) {
            padded[i] = n[i];
        }
        return padded;
    }

    constexpr inline Entity(std::string_view n, uint32_t c)
        : name(buildName(n))
        , code(c)
    {
    }

    PaddedName name;
    uint32_t code;
};
static_assert(Entity::buildName("thetasym") == std::array{'t', 'h', 'e', 't', 'a', 's', 'y', 'm'});
static_assert(Entity::buildName("lt") == std::array{'l', 't', '\0', '\0', '\0', '\0', '\0', '\0'});

static constexpr inline const Entity entities[] = {
    {"AElig", 0x00c6},   {"AMP", 38},         {"Aacute", 0x00c1},  {"Acirc", 0x00c2},   {"Agrave", 0x00c0},   {"Alpha", 0x0391},   {"Aring", 0x00c5},
    {"Atilde", 0x00c3},  {"Auml", 0x00c4},    {"Beta", 0x0392},    {"Ccaron", 0x010c},  {"Ccedil", 0x00c7},   {"Chi", 0x03a7},     {"Dagger", 0x2021},
    {"Dcaron", 0x010e},  {"Delta", 0x0394},   {"ETH", 0x00d0},     {"Eacute", 0x00c9},  {"Eague", 0x00c9},    {"Ecaron", 0x011a},  {"Ecirc", 0x00ca},
    {"Egrave", 0x00c8},  {"Epsilon", 0x0395}, {"Eta", 0x0397},     {"Euml", 0x00cb},    {"GT", 62},           {"Gamma", 0x0393},   {"Iacute", 0x00cd},
    {"Icirc", 0x00ce},   {"Igrave", 0x00cc},  {"Iota", 0x0399},    {"Iuml", 0x00cf},    {"Kappa", 0x039a},    {"LT", 60},          {"Lambda", 0x039b},
    {"Mu", 0x039c},      {"Ncaron", 0x0147},  {"Ntilde", 0x00d1},  {"Nu", 0x039d},      {"OElig", 0x0152},    {"Oacute", 0x00d3},  {"Ocirc", 0x00d4},
    {"Ograve", 0x00d2},  {"Omega", 0x03a9},   {"Omicron", 0x039f}, {"Oslash", 0x00d8},  {"Otilde", 0x00d5},   {"Ouml", 0x00d6},    {"Phi", 0x03a6},
    {"Pi", 0x03a0},      {"Prime", 0x2033},   {"Psi", 0x03a8},     {"QUOT", 34},        {"Rcaron", 0x0158},   {"Rho", 0x03a1},     {"Scaron", 0x0160},
    {"Sigma", 0x03a3},   {"THORN", 0x00de},   {"Tau", 0x03a4},     {"Tcaron", 0x0164},  {"Theta", 0x0398},    {"Uacute", 0x00da},  {"Ucirc", 0x00db},
    {"Ugrave", 0x00d9},  {"Upsilon", 0x03a5}, {"Uring", 0x016e},   {"Uuml", 0x00dc},    {"Xi", 0x039e},       {"Yacute", 0x00dd},  {"Yuml", 0x0178},
    {"Zcaron", 0x017d},  {"Zeta", 0x0396},    {"aacute", 0x00e1},  {"acirc", 0x00e2},   {"acute", 0x00b4},    {"aelig", 0x00e6},   {"agrave", 0x00e0},
    {"alefsym", 0x2135}, {"alpha", 0x03b1},   {"amp", 38},         {"and", 0x2227},     {"ang", 0x2220},      {"apos", 0x0027},    {"aring", 0x00e5},
    {"asymp", 0x2248},   {"atilde", 0x00e3},  {"auml", 0x00e4},    {"bdquo", 0x201e},   {"beta", 0x03b2},     {"brvbar", 0x00a6},  {"bull", 0x2022},
    {"cap", 0x2229},     {"ccaron", 0x010d},  {"ccedil", 0x00e7},  {"cedil", 0x00b8},   {"cent", 0x00a2},     {"chi", 0x03c7},     {"circ", 0x02c6},
    {"clubs", 0x2663},   {"cong", 0x2245},    {"copy", 0x00a9},    {"crarr", 0x21b5},   {"cup", 0x222a},      {"curren", 0x00a4},  {"dArr", 0x21d3},
    {"dagger", 0x2020},  {"darr", 0x2193},    {"dcaron", 0x10f},   {"deg", 0x00b0},     {"delta", 0x03b4},    {"diams", 0x2666},   {"divide", 0x00f7},
    {"dol", 0x0024},     {"dollar", 0x0024},  {"eacute", 0x00e9},  {"eague", 0x00e9},   {"ecaron", 0x011b},   {"ecirc", 0x00ea},   {"egrave", 0x00e8},
    {"emdash", 0x2014},  {"empty", 0x2205},   {"emsp", 0x2003},    {"endash", 0x2013},  {"ensp", 0x2002},     {"epsilon", 0x03b5}, {"equiv", 0x2261},
    {"eta", 0x03b7},     {"eth", 0x00f0},     {"euml", 0x00eb},    {"euro", 0x20ac},    {"exist", 0x2203},    {"fnof", 0x0192},    {"forall", 0x2200},
    {"frac12", 0x00bd},  {"frac14", 0x00bc},  {"frac34", 0x00be},  {"frasl", 0x2044},   {"gamma", 0x03b3},    {"ge", 0x2265},      {"gt", 62},
    {"hArr", 0x21d4},    {"harr", 0x2194},    {"hearts", 0x2665},  {"hellip", 0x2026},  {"iacute", 0x00ed},   {"icirc", 0x00ee},   {"iexcl", 0x00a1},
    {"igrave", 0x00ec},  {"image", 0x2111},   {"infin", 0x221e},   {"int", 0x222b},     {"iota", 0x03b9},     {"iquest", 0x00bf},  {"isin", 0x2208},
    {"iuml", 0x00ef},    {"kappa", 0x03ba},   {"lArr", 0x21d0},    {"lambda", 0x03bb},  {"lang", 0x2329},     {"laquo", 0x00ab},   {"larr", 0x2190},
    {"lceil", 0x2308},   {"ldquo", 0x201c},   {"le", 0x2264},      {"lfloor", 0x230a},  {"lowast", 0x2217},   {"loz", 0x25ca},     {"lrm", 0x200e},
    {"lsaquo", 0x2039},  {"lsquo", 0x2018},   {"lt", 60},          {"macr", 0x00af},    {"mdash", 0x2014},    {"micro", 0x00b5},   {"middot", 0x00b7},
    {"minus", 0x2212},   {"mu", 0x03bc},      {"nabla", 0x2207},   {"nbsp", 0x00a0},    {"ncaron", 0x0148},   {"ndash", 0x2013},   {"ne", 0x2260},
    {"ni", 0x220b},      {"not", 0x00ac},     {"notin", 0x2209},   {"nsub", 0x2284},    {"ntilde", 0x00f1},   {"nu", 0x03bd},      {"oacute", 0x00f3},
    {"ocirc", 0x00f4},   {"oelig", 0x0153},   {"ograve", 0x00f2},  {"oline", 0x203e},   {"omega", 0x03c9},    {"omicron", 0x03bf}, {"oplus", 0x2295},
    {"or", 0x2228},      {"ordf", 0x00aa},    {"ordm", 0x00ba},    {"oslash", 0x00f8},  {"otilde", 0x00f5},   {"otimes", 0x2297},  {"ouml", 0x00f6},
    {"para", 0x00b6},    {"part", 0x2202},    {"percnt", 0x0025},  {"permil", 0x2030},  {"perp", 0x22a5},     {"phi", 0x03c6},     {"pi", 0x03c0},
    {"piv", 0x03d6},     {"plusmn", 0x00b1},  {"pound", 0x00a3},   {"prime", 0x2032},   {"prod", 0x220f},     {"prop", 0x221d},    {"psi", 0x03c8},
    {"quot", 34},        {"rArr", 0x21d2},    {"radic", 0x221a},   {"rang", 0x232a},    {"raquo", 0x00bb},    {"rarr", 0x2192},    {"rcaron", 0x0159},
    {"rceil", 0x2309},   {"rdquo", 0x201d},   {"real", 0x211c},    {"reg", 0x00ae},     {"rfloor", 0x230b},   {"rho", 0x03c1},     {"rlm", 0x200f},
    {"rsaquo", 0x203a},  {"rsquo", 0x2019},   {"sbquo", 0x201a},   {"scaron", 0x0161},  {"sdot", 0x22c5},     {"sect", 0x00a7},    {"shy", 0x00ad},
    {"sigma", 0x03c3},   {"sigmaf", 0x03c2},  {"sim", 0x223c},     {"spades", 0x2660},  {"sub", 0x2282},      {"sube", 0x2286},    {"sum", 0x2211},
    {"sup", 0x2283},     {"sup1", 0x00b9},    {"sup2", 0x00b2},    {"sup3", 0x00b3},    {"supe", 0x2287},     {"supl", 0x00b9},    {"szlig", 0x00df},
    {"tau", 0x03c4},     {"tcaron", 0x0165},  {"there4", 0x2234},  {"theta", 0x03b8},   {"thetasym", 0x03d1}, {"thinsp", 0x2009},  {"thorn", 0x00fe},
    {"tilde", 0x02dc},   {"times", 0x00d7},   {"trade", 0x2122},   {"uArr", 0x21d1},    {"uacute", 0x00fa},   {"uarr", 0x2191},    {"ucirc", 0x00fb},
    {"ugrave", 0x00f9},  {"uml", 0x00a8},     {"upsih", 0x03d2},   {"upsilon", 0x03c5}, {"uring", 0x016f},    {"uuml", 0x00fc},    {"weierp", 0x2118},
    {"xi", 0x03be},      {"yacute", 0x00fd},  {"yen", 0x00a5},     {"yuml", 0x00ff},    {"zcaron", 0x017e},   {"zeta", 0x03b6},    {"zwj", 0x200d},
    {"zwnj", 0x200c}};

[[nodiscard]] static constexpr auto operator<=>(const Entity &lhs, std::span<const char, Entity::MAX_ENTITY_LEN> rhs)
{
    return std::string_view{lhs.name.data(), lhs.name.size()} <=> std::string_view{rhs.data(), rhs.size()};
}
static_assert(entities[0] < entities[1].name);
static_assert((entities[0] < entities[0].name) == false);
static_assert([] {
    for (size_t i = 1; (std::begin(entities) + i) < std::end(entities); i++) {
        if (entities[i] < entities[i - 1].name)
            return false;
    }
    return true;
}());
static_assert(std::is_eq(Entity{"lt", 0} <=> Entity::buildName("lt")));
static_assert(std::is_eq(Entity{"thetasym", 0} <=> Entity::buildName("thetasym")));
static_assert(std::is_neq(Entity{"thetasym", 0} <=> Entity::buildName("thetasy")));

QChar KCharsets::fromEntity(QStringView str)
{
    QChar res = QChar::Null;

    if (str.isEmpty()) {
        return QChar::Null;
    }

    int pos = 0;
    if (str[pos] == QLatin1Char('&')) {
        pos++;
    }

    // Check for '&#000' or '&#x0000' sequence
    if (str[pos] == QLatin1Char('#') && str.length() - pos > 1) {
        bool ok;
        pos++;
        if (str[pos] == QLatin1Char('x') || str[pos] == QLatin1Char('X')) {
            pos++;
            // '&#x0000', hexadecimal character reference
            const auto tmp = str.mid(pos);
            res = QChar(tmp.toInt(&ok, 16));
        } else {
            //  '&#0000', decimal character reference
            const auto tmp = str.mid(pos);
            res = QChar(tmp.toInt(&ok, 10));
        }
        if (ok) {
            return res;
        } else {
            return QChar::Null;
        }
    }

    const QByteArray raw(str.toLatin1());
    if (raw.size() > 8) {
        return QChar::Null;
    }
    const auto paddedRaw = Entity::buildName({raw.constData(), static_cast<size_t>(raw.size())});

    const auto e = std::lower_bound(std::begin(entities), std::end(entities), paddedRaw);

    if (e == std::end(entities) || std::is_neq(*e <=> paddedRaw)) {
        return QChar::Null;
    }

    return QChar(e->code);
}

QChar KCharsets::fromEntity(QStringView str, int &len)
{
    // entities are never longer than 8 chars... we start from
    // that length and work backwards...
    len = 8;
    while (len > 0) {
        const auto tmp = str.left(len);
        QChar res = fromEntity(tmp);
        if (res != QChar::Null) {
            return res;
        }
        len--;
    }
    return QChar::Null;
}

QString KCharsets::toEntity(const QChar &ch)
{
    return QString::asprintf("&#x%x;", ch.unicode());
}

QString KCharsets::resolveEntities(const QString &input)
{
    QString text = input;
    const QChar *p = text.unicode();
    const QChar *end = p + text.length();
    const QChar *ampersand = nullptr;
    bool scanForSemicolon = false;

    for (; p < end; ++p) {
        const QChar ch = *p;

        if (ch == QLatin1Char('&')) {
            ampersand = p;
            scanForSemicolon = true;
            continue;
        }

        if (ch != QLatin1Char(';') || scanForSemicolon == false) {
            continue;
        }

        assert(ampersand);

        scanForSemicolon = false;

        const QChar *entityBegin = ampersand + 1;

        const uint entityLength = p - entityBegin;
        if (entityLength == 0) {
            continue;
        }

        const QChar entityValue = KCharsets::fromEntity(QStringView(entityBegin, entityLength));
        if (entityValue.isNull()) {
            continue;
        }

        const uint ampersandPos = ampersand - text.unicode();

        text[(int)ampersandPos] = entityValue;
        text.remove(ampersandPos + 1, entityLength + 1);
        p = text.unicode() + ampersandPos;
        end = text.unicode() + text.length();
        ampersand = nullptr;
    }

    return text;
}

QStringList KCharsets::availableEncodingNames() const
{
    QStringList available;
    for (const int *p = language_for_encoding_indices; *p != -1; p += 2) {
        available.append(QString::fromUtf8(language_for_encoding_string + *p));
    }
    available.sort();
    return available;
}

QString KCharsets::descriptionForEncoding(QStringView encoding) const
{
    const char *lang = kcharsets_array_search(language_for_encoding_string, language_for_encoding_indices, encoding.toUtf8().data());
    if (lang) {
        return tr("%1 ( %2 )", "@item %1 character set, %2 encoding").arg(tr(lang, "@item Text character set"), encoding);
    } else {
        return tr("Other encoding (%1)", "@item").arg(encoding);
    }
}

QString KCharsets::encodingForName(const QString &descriptiveName) const
{
    const int left = descriptiveName.lastIndexOf(QLatin1Char('('));

    if (left < 0) { // No parenthesis, so assume it is a normal encoding name
        return descriptiveName.trimmed();
    }

    QString name(descriptiveName.mid(left + 1));

    const int right = name.lastIndexOf(QLatin1Char(')'));

    if (right < 0) {
        return name;
    }

    return name.left(right).trimmed();
}

QStringList KCharsets::descriptiveEncodingNames() const
{
    QStringList encodings;

    for (const auto scriptsAndEncodings = encodingsByScript(); const auto &encodingList : scriptsAndEncodings) {
        const auto script = encodingList[0];
        for (qsizetype i = 1; i < encodingList.size(); i++) {
            const auto encoding = encodingList[i];
            encodings.append(tr("%1 ( %2 )", "@item Text encoding: %1 character set, %2 encoding").arg(script, encoding));
        }
    }

    encodings.sort();
    return encodings;
}

QList<QStringList> KCharsets::encodingsByScript() const
{
    static auto encodings = []() {
        QList<QStringList> encodings;

        for (const int *p = language_for_encoding_indices; *p != -1; p += 2) {
            const QString encoding = QString::fromUtf8(language_for_encoding_string + p[0]);
            const QString script = tr(language_for_encoding_string + p[1], "@item Text character set");

            auto match = std::find_if(encodings.begin(), encodings.end(), [&script](auto &e) {
                return e.at(0) == script;
            });

            if (match == encodings.end()) {
                encodings.append(QStringList{script, encoding});
            } else {
                match->append(encoding);
            }
        }
        return encodings;
    }();

    return encodings;
}

KCharsets *KCharsets::charsets()
{
    return &KCharsetsSingletonPrivate::instance;
}
