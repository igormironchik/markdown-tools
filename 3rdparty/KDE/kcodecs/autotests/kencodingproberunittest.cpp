/*
    SPDX-FileCopyrightText: 2025 Stefan Brüns <stefan.bruens@rwth-aachen.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include <QTest>

#include "../src/probers/nsEscSM.h"
#include "../src/probers/nsMBCSSM.h"

#include "../src/probers/nsCodingStateMachine.h"
#include "../src/probers/nsHebrewProber.h"
#include "../src/probers/nsLatin1Prober.h"

class KEncodingProberUnitTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testUtf8();
    void testUtf8_data();
    void testUtf16BE();
    void testUtf16BE_data();
    void testUtf16LE();
    void testUtf16LE_data();
    void testUtf16_common_data();
    void testWindows1252();
    void testWindows1252_data();
    void testWindows1255();
    void testWindows1255_data();
    void testEsc_common_data();
    void testHzCharset();
    void testHzCharset_data();
    void testIso2022JPCharset();
    void testIso2022JPCharset_data();
};

void KEncodingProberUnitTest::testUtf8()
{
    QFETCH(QByteArray, data);
    QFETCH(bool, utf8Valid);

    using namespace kencodingprober;

    nsCodingStateMachine stateMachine{&UTF8SMModel};
    nsSMState state = eStart;

    for (auto b : data) {
        state = stateMachine.NextState(b);
    }

    QCOMPARE((state != eError), utf8Valid);
}

void KEncodingProberUnitTest::testUtf8_data()
{
    using namespace Qt::StringLiterals;

    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<bool>("utf8Valid");

    QTest::addRow("UTF-8 Latin1") << "abcdxyzABCDXYZ 0129;,"_ba << true;
    QTest::addRow("BOM UTF-8") << "\xef\xbb\xbfZ"_ba << true; // "<UTF-8 BOM>Z"

    // multibyte sequences - length 2
    QTest::addRow("UTF-8 Latin1 Supplement") //
        << "Latin1 Text \xC3\xA4\xC3\xB6\xC3\xBC\xC3\x9F"_ba // "Latin1 Text äöüß"
        << true;
    QTest::addRow("UTF-8 len 2") << "Text \xC3\xA4  "_ba << true;
    QTest::addRow("UTF-8 len 2 short") << "Text \xC3  "_ba << false;
    QTest::addRow("UTF-8 len 2 invalid range") << "Text \xC0\x90   "_ba << false;

    // multibyte sequences - length 3
    QTest::addRow("UTF-8 CJK") //
        << QByteArray::fromHex("e998bfe5b094e58d91e696afe5b1b1e88489") // 阿尔卑斯山脉
        << true;
    QTest::addRow("UTF-8 len 3 a") << "Text \xE2\x80\x90 "_ba << true; // "‐" (HYPHEN)
    QTest::addRow("UTF-8 len 3-1 short") << "Text \xE2\x80 "_ba << false;
    QTest::addRow("UTF-8 len 3-2 short") << "Text \xE2 "_ba << false;

    QTest::addRow("UTF-8 len 3 b") << "Text \xE0\xbf\xbf "_ba << true; // "๏" (THAI CHARACTER FONGMAN)
    QTest::addRow("UTF-8 len 3 invalid range") << "Text \xE0\x9f\x90 "_ba << false;

    QTest::addRow("UTF-8 len 3 c") << "Text \xED\x80\x80 "_ba << true; // "퀀" (HANGUL SYLLABLE KWEON)
    QTest::addRow("UTF-8 invalid CESU") << "Text \xED\xbf\x80 "_ba << false;

    // multibyte sequences - length 4
    QTest::addRow("UTF-8 SMP Symbols") << "\xF0\x9F\x82\xA1 "_ba << true; // "🂡 " (ACE OF SPADES)
    QTest::addRow("UTF-8 len 4-1 short") << "\xF0\x9F\x82  "_ba << false;
    QTest::addRow("UTF-8 len 4-2 short") << "\xF0\x9F   "_ba << false;
    QTest::addRow("UTF-8 len 4-3 short") << "\xF0    "_ba << false;
    QTest::addRow("UTF-8 len 4 invalid long") << "\xF0\x8F\x90\x90 "_ba << false;
    QTest::addRow("UTF-8 len 4 invalid range") << "\xF5\x90\x90\x90 "_ba << false;

    // multibyte sequences - length 5/6 (invalid)
    QTest::addRow("UTF-8 len 5 invalid") << "\xF8\x90\x90\x90\x90 "_ba << false;
    QTest::addRow("UTF-8 len 6 invalid") << "\xFC\x90\x90\x90\x90\x90 "_ba << false;

    QTest::addRow("UTF-8 0xFE invalid") << "\xFE "_ba << false;
    QTest::addRow("UTF-8 0xFF invalid") << "\xFF "_ba << false;

    // continuation without leading 2/3/4 byte start byte
    QTest::addRow("UTF-8 invalid isolate high 0x80") << "\x80 "_ba << false;
    QTest::addRow("UTF-8 invalid isolate high 0x92") << "\x92 "_ba << false;
    QTest::addRow("UTF-8 invalid isolate high 0xAA") << "\xAA "_ba << false;
    QTest::addRow("UTF-8 invalid isolate high 0xBF") << "\xBF "_ba << false;

    // Either Windows-1252/-1254/-1255 (binary identical)
    // "One pound, i.e. ½ a kilogramm of butter costs 2 £."
    QTest::addRow("Windows-125x English") << "One pound, i.e. \xAF a kilogramm of butter costs 2 \xA3."_ba << false;
    // Example texts with Windows-125x encoding which are definitely not UTF-8 -- see Wikipedia "Pangram"
    // "Příliš žluťoučký kůň úpěl ďábelské ódy" - "A horse that was too yellow moaned devilish odes"
    QTest::addRow("Windows-1250 Czech") << //
        "P\xf8\xedli\x9a \x9elu\x9dou\xe8k\xfd k\xf9\xf2 \xfap\xecl \xef\xe1\x62\x65lsk\xe9 \xf3\x64y."_ba << false;
    // "Под южно дърво, цъфтящо в синьо, бягаше малко пухкаво зайче" - "Under a southern tree, blooming in blue, ran a little fluffy bunny"
    QTest::addRow("Windows-1251 Bulgarian") << QByteArray::fromHex( //
        "cfeee420fee6edee20e4faf0e2ee2c20f6faf4f2fff9ee20"
        "e220f1e8edfcee2c20e1ffe3e0f8e520ece0ebeaee20eff3"
        "f5eae0e2ee20e7e0e9f7e5") << false;
    // "Victor jagt zwölf Boxkämpfer quer über den großen Sylter Deich." - "Victor chases twelve boxers across the Great Levee of Sylt"
    QTest::addRow("Windows-1252 German") << //
        "Victor jagt zw\xf6lf Boxk\xe4mpfer quer \xfc\x62\x65r den gro\xdf\x65n Sylter Deich."_ba << false;
    // "שפן אכל קצת גזר בטעם חסה, ודי" - "A bunny ate some lettuce-flavored carrots, and he had enough"
    QTest::addRow("Windows-1255 Hebrew") //
        << QByteArray::fromHex("f9f4ef20e0ebec20f7f6fa20e2e6f820e1e8f2ed20e7f1e42c20e5e3e9") << false;
}

void KEncodingProberUnitTest::testUtf16BE()
{
    QFETCH(QByteArray, data);
    QFETCH(bool, utf16BEValid);

    using namespace kencodingprober;

    nsCodingStateMachine stateMachine{&UCS2BESMModel};
    nsSMState state = eStart;

    QEXPECT_FAIL("UTF16 ZWNBSP little", "zero width no-break space rejected", Abort);
    for (auto b : data) {
        state = stateMachine.NextState(b);
    }

    QCOMPARE((state != eError), utf16BEValid);
}

void KEncodingProberUnitTest::testUtf16LE()
{
    QFETCH(QByteArray, data);
    QFETCH(bool, utf16LEValid);

    using namespace kencodingprober;

    nsCodingStateMachine stateMachine{&UCS2LESMModel};
    nsSMState state = eStart;

    QEXPECT_FAIL("UTF16 ZWNBSP big", "zero width no-break space rejected", Abort);
    for (auto b : data) {
        state = stateMachine.NextState(b);
    }

    QCOMPARE((state != eError), utf16LEValid);
}

void KEncodingProberUnitTest::testUtf16_common_data()
{
    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<bool>("utf16BEValid");
    QTest::addColumn<bool>("utf16LEValid");

    QTest::addRow("empty") << QByteArray() << true << true;
    // BOM must be detected
    QTest::addRow("BE BOM") << QByteArray("\xFE\xFF") << true << false;
    QTest::addRow("LE BOM") << QByteArray("\xFF\xFE") << false << true;
    // swapped endianess does not cause an error, as the codepoint is still valid
    QTest::addRow("BE HS+LS") << QByteArray("\x01\x01\xD8\x00\xDC\x00", 6) << true << true;
    QTest::addRow("LE HS+LS") << QByteArray("\x01\x01\x00\xD8\x00\xDC", 6) << true << true;
    QTest::addRow("BE HS+LS at start") << QByteArray("\xD8\x00\xDC\x00", 4) << true << true;
    QTest::addRow("LE HS+LS at start") << QByteArray("\x00\xD8\x00\xDC", 4) << true << true;
    // if swapped endianess results in a low surrogate, the input is invalid
    QTest::addRow("BE LS") << QByteArray("\x01\x01\xDC\x00", 4) << false << true;
    QTest::addRow("LE LS") << QByteArray("\x01\x01\x00\xDC", 4) << true << false;
    QTest::addRow("BE LS at start") << QByteArray("\xDC\x00", 2) << false << true;
    QTest::addRow("LE LS at start") << QByteArray("\x00\xDC", 2) << true << false;

    struct Utf16TestData {
        const char *name;
        const std::span<const char16_t> data;
        bool validBig;
        bool validLittle;
    };
    using namespace std::string_view_literals;
    constexpr std::array<Utf16TestData, 7> utf16TestData = {
        // syntactically correct even with wrong endianess
        Utf16TestData{"UTF16 XY", u"XY"sv, true, true},
        Utf16TestData{"UTF16 ab", u"ab"sv, true, true},
        Utf16TestData{"UTF16 äöü", u"äöü"sv, true, true},
        Utf16TestData{"UTF16 BOM", u"\xFEFF"sv, true, false},
        // "‛" or "ᬠ" (U+1B20 BALINESE LETTER DA MURDA MAHAPRANA)
        Utf16TestData{"UTF16 Interpunctuation", u"\x201B"sv, true, true},
        // "⨯" or "⼪" (U+2F2A KANGXI RADICAL LAME)
        Utf16TestData{"UTF16 Math supplement", u"\x2A2F"sv, true, true},
        // ZWNBSP aka BOM inside the document is deprecated, but valid
        Utf16TestData{"UTF16 ZWNBSP", u" \xFEFF"sv, true, true},
    };

    for (const auto &tc : utf16TestData) {
        QByteArray data;
        data.resize(tc.data.size() * 2);

        qToBigEndian<quint16>(tc.data.data(), tc.data.size(), data.data());
        QTest::addRow("%s big", tc.name) << data << tc.validBig << tc.validLittle;

        qToLittleEndian<quint16>(tc.data.data(), tc.data.size(), data.data());
        QTest::addRow("%s little", tc.name) << data << tc.validLittle << tc.validBig;
    }
}

void KEncodingProberUnitTest::testUtf16BE_data()
{
    testUtf16_common_data();
}

void KEncodingProberUnitTest::testUtf16LE_data()
{
    testUtf16_common_data();
}

void KEncodingProberUnitTest::testWindows1252()
{
    using namespace kencodingprober;

    QFETCH(QByteArray, data);
    QFETCH(bool, win1252Valid);
    QFETCH(double, minConfidence);

    nsLatin1Prober win1252Prober{};

    auto state = win1252Prober.HandleData(data.constData(), data.size());

    QCOMPARE(win1252Valid, (state != eNotMe));

    QEXPECT_FAIL("Windows-1252 German", "Valid vowel + sharp s considered unlikely", Abort);
    QEXPECT_FAIL("Windows-1252 French", "Accented vowel pair considered unlikely", Abort);
    QEXPECT_FAIL("Windows-1252 Spanish", "Accented vowel pair considered unlikely", Abort);
    if (win1252Valid) {
        QCOMPARE_GE(win1252Prober.GetConfidence(), minConfidence);
    }
}

void KEncodingProberUnitTest::testWindows1252_data()
{
    using namespace Qt::StringLiterals;

    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<bool>("win1252Valid");
    QTest::addColumn<double>("minConfidence");

    // Verify "undefined" codes in CP1252 are rejected
    // https://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP1252.TXT
    for (auto c : std::array<unsigned char, 5>{0x81, 0x8D, 0x8F, 0x90, 0x9D}) {
        QTest::addRow("Undefined 0x%x", c) << QByteArray(1, c) + " "_ba << false << 0.0;
    }

    // "Victor jagt zwölf Boxkämpfer quer über den großen Sylter Deich." - "Victor chases twelve boxers across the Great Levee of Sylt"
    QTest::addRow("Windows-1252 German") << //
        "Victor jagt zw\xf6lf Boxk\xe4mpfer quer \xfc\x62\x65r den gro\xdf\x65n Sylter Deich."_ba << true << 0.45;

    // Same "Victor jagt zwölf Boxkämpfer quer über den großen Sylter Deich.", but using UTF-8
    // Unfortunately, these are all valid codepoints for Windows-1252, but the confidence should be lower than for UTF-8,
    // as it contains UTF-8 multibyte sequences, and several capital characters following small characters for Windows-1252.
    QTest::addRow("UTF-8 German") << //
        "Victor jagt zw\xc3\xb6lf Boxk\xc3\xa4mpfer quer \xc3\xbc\x62\x65r den gro\xc3\x9f\x65n Sylter Deich."_ba << true << 0.0;

    // "L'ergothérapeute effectue des prestations de rééducation et de réadaptation."
    // - "The occupational therapist provides rehabilitation and re-adaptation services."
    QTest::addRow("Windows-1252 French") << //
        "L'ergoth\xe9rapeute effectue des prestations de r\xe9\xe9\x64ucation et de r\xe9\x61\x64\x61ptation."_ba << true << 0.45;

    // "La sociolingüística sincrónica se centra en la estructura sociolingüística y en las variaciones lingüísticas."
    // - "Synchronic sociolinguistics focuses on sociolinguistic structure and linguistic variations."
    QTest::addRow("Windows-1252 Spanish") << //
        "La socioling\xfc\xedstica sincr\xf3nica se centra en la estructura socioling\xfc\xedstica y en las variaciones ling\xfc\xedsticas."_ba << true << 0.45;

    // "שפן אכל קצת גזר בטעם חסה, ודי" - "A bunny ate some lettuce-flavored carrots, and he had enough"
    QTest::addRow("Windows-1255 Hebrew") //
        << QByteArray::fromHex("f9f4ef20e0ebec20f7f6fa20e2e6f820e1e8f2ed20e7f1e42c20e5e3e9") << true << 0.0;
    // "שפן אכל קצת גזר בטעם חסה, ודי", but using UTF-8
    QTest::addRow("UTF-8 Hebrew") << QByteArray::fromHex( //
        "d7a9d7a4d79f20d790d79bd79c20d7a7d7a6d7aa20d792d7"
        "96d7a820d791d798d7a2d79d20d797d7a1d7942c20d795d7"
        "93d799") << false << 0.0;
}

void KEncodingProberUnitTest::testWindows1255()
{
    using namespace kencodingprober;

    QFETCH(QByteArray, data);
    QFETCH(bool, win1255Valid);

    nsHebrewProber win1255Prober{};

    if (QByteArray(QTest::currentDataTag(), -1).startsWith("Undefined 0x")) {
        QEXPECT_FAIL("", "Invalid value accepted", Abort);
    }
    QEXPECT_FAIL("UTF-8 Hebrew", "Invalid 0x9f accepted", Abort);

    auto state = win1255Prober.HandleData(data.constData(), data.size());

    QCOMPARE(win1255Valid, (state != eNotMe));
}

void KEncodingProberUnitTest::testWindows1255_data()
{
    using namespace Qt::StringLiterals;

    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<bool>("win1255Valid");

    // Verify "undefined" codes in CP1255 are rejected
    // https://www.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP1255.TXT
    for (auto c : std::array<unsigned char, 23>{
             0x81, 0x8A, 0x8C, 0x8D, 0x8E, 0x8F, 0x90, 0x9A, 0x9C, 0x9D, 0x9E, 0x9F, //
             0xCA, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, 0xFB, 0xFC, 0xFF,
         }) {
        QTest::addRow("Undefined 0x%x", c) << QByteArray(1, c) + " "_ba << false;
    }

    // "שפן אכל קצת גזר בטעם חסה, ודי" - "A bunny ate some lettuce-flavored carrots, and he had enough"
    QTest::addRow("Windows-1255 Hebrew") //
        << QByteArray::fromHex("f9f4ef20e0ebec20f7f6fa20e2e6f820e1e8f2ed20e7f1e42c20e5e3e9") << true;

    // Same "שפן אכל קצת גזר בטעם חסה, ודי", but using UTF-8
    QTest::addRow("UTF-8 Hebrew") << QByteArray::fromHex( //
        "d7a9d7a4d79f20d790d79bd79c20d7a7d7a6d7aa20d792d7"
        "96d7a820d791d798d7a2d79d20d797d7a1d7942c20d795d7"
        "93d799") << false;
}

void KEncodingProberUnitTest::testHzCharset()
{
    QFETCH(QByteArray, data);
    QFETCH(bool, hzValid);

    using namespace kencodingprober;

    nsCodingStateMachine stateMachine{&HZSMModel};

    nsSMState state = eStart;

    QEXPECT_FAIL("HZ odd", "0x7d '}' accepted as lead byte", Abort);
    for (auto b : data) {
        state = stateMachine.NextState(b);
    }

    QCOMPARE((state != eError), hzValid);
}

void KEncodingProberUnitTest::testIso2022JPCharset()
{
    QFETCH(QByteArray, data);
    QFETCH(bool, iso2022JPValid);

    using namespace kencodingprober;

    nsCodingStateMachine stateMachine{&ISO2022JPSMModel};

    nsSMState state = eStart;

    for (auto b : data) {
        state = stateMachine.NextState(b);
    }

    qDebug() << data;
    QCOMPARE((state != eError), iso2022JPValid);
}

void KEncodingProberUnitTest::testEsc_common_data()
{
    using namespace Qt::StringLiterals;

    QTest::addColumn<QByteArray>("data");
    QTest::addColumn<bool>("hzValid");
    QTest::addColumn<bool>("iso2022JPValid");

    // ASCII without escape sequence is always valid
    QTest::addRow("empty") << QByteArray() << true << true;
    QTest::addRow("Latin1") << "abcdxyzABCDXYZ 0129;,"_ba << true << true;

    // All codes >= 0x80 are invalid
    QTest::addRow("UTF-8 BOM") << "\xef\xbb\xbfZ"_ba << false << false; // "<UTF-8 BOM>Z"
    QTest::addRow("UTF-16BE BOM") << "\xFE\xFF"_ba << false << false;
    QTest::addRow("UTF-16LE BOM") << "\xFF\xFE"_ba << false << false;
    QTest::addRow("UTF-8 Latin1 Supplement") //
        << "Latin1 Text \xC3\xA4\xC3\xB6\xC3\xBC\xC3\x9F"_ba // "Latin1 Text äöüß"
        << false << false;

    // HZ encoding is (a subset of) plain ASCII, i.e. also valid ISO-2022
    QTest::addRow("HZ empty") << "~{~}"_ba << true << true;
    QTest::addRow("HZ odd") << "~{R~}"_ba << false << true;
    QTest::addRow("HZ valid") << "~{R;~}"_ba << true << true; // "一" (one)
    QTest::addRow("HZ valid with ASCII") << "R~{R;~}R"_ba << true << true; // "R一R"

    // "こんにちは" / "Kon'nichiwa" - "Hello"
    // ESC $ B <JIS X 208-1983 Double Byte> ESC ( B
    QTest::addRow("ISO-2022-JP") << QByteArray::fromHex( //
        "1b 24 42 24 33 24 73 24  4b 24 41 24 4f 1b 28 42"
        "") << false << true;
}

void KEncodingProberUnitTest::testHzCharset_data()
{
    testEsc_common_data();
}

void KEncodingProberUnitTest::testIso2022JPCharset_data()
{
    testEsc_common_data();
}

QTEST_MAIN(KEncodingProberUnitTest)

#include "kencodingproberunittest.moc"
