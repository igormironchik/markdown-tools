/*
    SPDX-FileCopyrightText: 2016 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: MIT
*/

#include "repository_test_base.h"
#include "test-config.h"

#include <KSyntaxHighlighting/AbstractHighlighter>
#include <KSyntaxHighlighting/Definition>
#include <KSyntaxHighlighting/Format>
#include <KSyntaxHighlighting/Repository>
#include <KSyntaxHighlighting/State>
#include <KSyntaxHighlighting/Theme>

#include <QFileInfo>
#include <QPalette>
#include <QTest>

#include <algorithm>

namespace KSyntaxHighlighting
{
class NullHighlighter : public AbstractHighlighter
{
public:
    using AbstractHighlighter::highlightLine;
    void applyFormat(int offset, int length, const Format &format) override
    {
        Q_UNUSED(offset);
        Q_UNUSED(length);
        // only here to ensure we don't crash
        format.isDefaultTextStyle(theme());
        format.textColor(theme());
    }
};

class RepositoryTest : public RepositoryTestBase
{
    Q_OBJECT
private Q_SLOTS:
    void testDefinitionByExtension_data()
    {
        definitionByExtensionTestData();
    }

    void testDefinitionByExtension()
    {
        QFETCH(QString, fileName);
        QFETCH(QString, definitionName);

        definitionByExtensionTest(fileName, definitionName);
    }

    void testDefinitionsForFileName_data()
    {
        definitionsForFileNameTestData();
    }

    void testDefinitionsForFileName()
    {
        QFETCH(QString, fileName);
        QFETCH(QStringList, definitionNames);

        definitionsForFileNameTest(fileName, definitionNames);
    }

    void testDefinitionForMimeType_data()
    {
        definitionForMimeTypeTestData();
    }

    void testDefinitionForMimeType()
    {
        QFETCH(QString, mimeTypeName);
        QFETCH(QString, definitionName);

        definitionForMimeTypeTest(mimeTypeName, definitionName);
    }

    void testDefinitionsForMimeType_data()
    {
        definitionsForMimeTypeTestData();
    }

    void testDefinitionsForMimeType()
    {
        QFETCH(QString, mimeTypeName);
        QFETCH(QStringList, definitionNames);

        definitionsForMimeTypeTest(mimeTypeName, definitionNames);
    }

    void testLoadAll()
    {
        for (const auto &def : m_repo.definitions()) {
            QVERIFY(!def.name().isEmpty());
            QVERIFY(!def.translatedName().isEmpty());
            QVERIFY(!def.isValid() || !def.section().isEmpty());
            QVERIFY(!def.isValid() || !def.translatedSection().isEmpty());
            // indirectly trigger loading, as we can't reach that from public API
            // if the loading fails the highlighter will produce empty states
            NullHighlighter hl;
            State initialState;
            hl.setDefinition(def);
            const auto state = hl.highlightLine(u"This should not crash } ] ) !", initialState);
            QVERIFY(!def.isValid() || state != initialState || def.name() == QLatin1String("Broken Syntax"));
        }
    }

    void testMetaData()
    {
        auto def = m_repo.definitionForName(QLatin1String("Alerts"));
        QVERIFY(def.isValid());
        QVERIFY(def.extensions().isEmpty());
        QVERIFY(def.mimeTypes().isEmpty());
        QVERIFY(def.version() >= 1.11f);
        QVERIFY(def.isHidden());
        QCOMPARE(def.section(), QLatin1String("Other"));
        QCOMPARE(def.license(), QLatin1String("MIT"));
        QVERIFY(def.author().contains(QLatin1String("Dominik")));
        QFileInfo fi(def.filePath());
        QVERIFY(fi.isAbsolute());
        QVERIFY(def.filePath().endsWith(QLatin1String("alert.xml")));

        def = m_repo.definitionForName(QLatin1String("C++"));
        QVERIFY(def.isValid());
        QCOMPARE(def.section(), QLatin1String("Sources"));
        QCOMPARE(def.indenter(), QLatin1String("cstyle"));
        QCOMPARE(def.style(), QLatin1String("C++"));
        QVERIFY(def.mimeTypes().contains(QLatin1String("text/x-c++hdr")));
        QVERIFY(def.extensions().contains(QLatin1String("*.h")));
        QCOMPARE(def.priority(), 9);

        def = m_repo.definitionForName(QLatin1String("Apache Configuration"));
        QVERIFY(def.isValid());
        QVERIFY(def.extensions().contains(QLatin1String("httpd.conf")));
        QVERIFY(def.extensions().contains(QLatin1String(".htaccess*")));
    }

    void testGeneralMetaData()
    {
        auto def = m_repo.definitionForName(QLatin1String("C++"));
        QVERIFY(def.isValid());
        QVERIFY(!def.indentationBasedFoldingEnabled());

        // comment markers
        QCOMPARE(def.singleLineCommentMarker(), QLatin1String("//"));
        QCOMPARE(def.singleLineCommentPosition(), KSyntaxHighlighting::CommentPosition::AfterWhitespace);
        const auto cppMultiLineCommentMarker = QPair<QString, QString>(QLatin1String("/*"), QLatin1String("*/"));
        QCOMPARE(def.multiLineCommentMarker(), cppMultiLineCommentMarker);

        def = m_repo.definitionForName(QLatin1String("Python"));
        QVERIFY(def.isValid());

        // indentation
        QVERIFY(def.indentationBasedFoldingEnabled());
        QCOMPARE(def.foldingIgnoreList(), QStringList() << QLatin1String("(?:\\s+|\\s*#.*)"));

        // keyword lists
        QVERIFY(!def.keywordLists().isEmpty());
        QVERIFY(def.keywordList(QLatin1String("operators")).contains(QLatin1String("and")));
        QVERIFY(!def.keywordList(QLatin1String("does not exits")).contains(QLatin1String("and")));
    }

    void testFormatData()
    {
        auto def = m_repo.definitionForName(QLatin1String("ChangeLog"));
        QVERIFY(def.isValid());
        auto formats = def.formats();
        QVERIFY(!formats.isEmpty());

        // verify that the formats are sorted, such that the order matches the order of the itemDatas in the xml files.
        auto sortComparator = [](const KSyntaxHighlighting::Format &lhs, const KSyntaxHighlighting::Format &rhs) {
            return lhs.id() < rhs.id();
        };
        QVERIFY(std::is_sorted(formats.begin(), formats.end(), sortComparator));

        // check all names are listed
        QStringList formatNames;
        for (const auto &format : std::as_const(formats)) {
            formatNames.append(format.name());
        }

        const QStringList expectedItemDatas = {QStringLiteral("Normal Text"),
                                               QStringLiteral("Name"),
                                               QStringLiteral("E-Mail"),
                                               QStringLiteral("Date"),
                                               QStringLiteral("Entry")};
        QCOMPARE(formatNames, expectedItemDatas);
    }

    void testIncludedDefinitions()
    {
        auto def = m_repo.definitionForName(QLatin1String("PHP (HTML)"));
        QVERIFY(def.isValid());
        auto defs = def.includedDefinitions();

        QStringList expectedDefinitionNames = {QStringLiteral("PHP/PHP"),
                                               QStringLiteral("Alerts"),
                                               QStringLiteral("Comments"),
                                               QStringLiteral("CSS/PHP"),
                                               QStringLiteral("JavaScript/PHP"),
                                               QStringLiteral("JavaScript React (JSX)/PHP"),
                                               QStringLiteral("JavaScript"),
                                               QStringLiteral("TypeScript/PHP"),
                                               QStringLiteral("Mustache/Handlebars (HTML)/PHP"),
                                               QStringLiteral("Doxygen"),
                                               QStringLiteral("Modelines"),
                                               QStringLiteral("SPDX-Comments"),
                                               QStringLiteral("HTML"),
                                               QStringLiteral("CSS"),
                                               QStringLiteral("SQL (MySQL)"),
                                               QStringLiteral("JavaScript React (JSX)"),
                                               QStringLiteral("TypeScript"),
                                               QStringLiteral("Mustache/Handlebars (HTML)")};
        QStringList definitionNames;
        for (auto d : defs) {
            QVERIFY(d.isValid());
            definitionNames.push_back(d.name());
        }

        // work on sorted lists to make test more stable to changes in structure of XML files
        std::sort(expectedDefinitionNames.begin(), expectedDefinitionNames.end());
        std::sort(definitionNames.begin(), definitionNames.end());
        QCOMPARE(definitionNames, expectedDefinitionNames);
    }

    void testIncludedFormats()
    {
        // ensure we have a efficient numbering of formats
        // do this just with one example definition that includes a lot of stuff
        // before: checked for all definitions we have, that is n^2 run-time wise
        Repository repo;
        initRepositorySearchPaths(repo);
        auto def = repo.definitionForName(QLatin1String("PHP (HTML)"));
        QVERIFY(def.isValid());
        auto includedDefs = def.includedDefinitions();
        includedDefs.push_front(def);

        // collect all formats, shall be numbered from 1..
        QSet<int> formatIds;
        for (const auto &d : std::as_const(includedDefs)) {
            const auto formats = d.formats();
            for (const auto &format : formats) {
                // no duplicates
                QVERIFY(!formatIds.contains(format.id()));
                formatIds.insert(format.id());
            }
        }

        // ensure all ids are there from 1..size
        // this is no longer feasible, as we e.g. skip the "Comments"
        // syntax definition in includedDefinitions as it is not contributing
        // any context/rule
        // for (int i = 1; i <= formatIds.size(); ++i) {
        //    QVERIFY(formatIds.contains(i));
        //}
    }

    void testReload()
    {
        auto def = m_repo.definitionForName(QLatin1String("QML"));
        QVERIFY(!m_repo.definitions().isEmpty());
        QVERIFY(def.isValid());

        NullHighlighter hl;
        hl.setDefinition(def);
        auto oldState = hl.highlightLine(u"/* TODO this should not crash */", State());

        m_repo.reload();
        QVERIFY(!m_repo.definitions().isEmpty());
        QVERIFY(!def.isValid());

        hl.highlightLine(u"/* TODO this should not crash */", State());
        hl.highlightLine(u"/* FIXME neither should this crash */", oldState);
        QVERIFY(hl.definition().isValid());
        QCOMPARE(hl.definition().name(), QLatin1String("QML"));
    }

    void testLifetime()
    {
        // common mistake with value-type like Repo API, make sure this doesn'T crash
        NullHighlighter hl;
        {
            Repository repo;
            hl.setDefinition(repo.definitionForName(QLatin1String("C++")));
            hl.setTheme(repo.defaultTheme());
        }
        hl.highlightLine(u"/**! @todo this should not crash .*/", State());
    }

    void testCustomPath()
    {
        QString testInputPath = QStringLiteral(TESTSRCDIR "/input");

        Repository repo;
        QVERIFY(repo.customSearchPaths().empty());
        repo.addCustomSearchPath(testInputPath);
        QCOMPARE(repo.customSearchPaths().size(), 1);
        QCOMPARE(repo.customSearchPaths()[0], testInputPath);

        auto customDefinition = repo.definitionForName(QLatin1String("Test Syntax"));
        QVERIFY(customDefinition.isValid());
        auto customTheme = repo.theme(QLatin1String("Test Theme"));
        QVERIFY(customTheme.isValid());
    }

    void testInvalidDefinition()
    {
        Definition def;
        QVERIFY(!def.isValid());
        QVERIFY(def.filePath().isEmpty());
        QCOMPARE(def.name(), QLatin1String("None"));
        QVERIFY(def.section().isEmpty());
        QVERIFY(def.translatedSection().isEmpty());
        QVERIFY(def.mimeTypes().isEmpty());
        QVERIFY(def.extensions().isEmpty());
        QCOMPARE(def.version(), 0);
        QCOMPARE(def.priority(), 0);
        QVERIFY(!def.isHidden());
        QVERIFY(def.style().isEmpty());
        QVERIFY(def.indenter().isEmpty());
        QVERIFY(def.author().isEmpty());
        QVERIFY(def.license().isEmpty());
        QVERIFY(!def.foldingEnabled());
        QVERIFY(!def.indentationBasedFoldingEnabled());
        QVERIFY(def.foldingIgnoreList().isEmpty());
        QVERIFY(def.keywordLists().isEmpty());
        QVERIFY(def.formats().isEmpty());
        QVERIFY(def.includedDefinitions().isEmpty());
        QVERIFY(def.singleLineCommentMarker().isEmpty());
        QCOMPARE(def.singleLineCommentPosition(), KSyntaxHighlighting::CommentPosition::StartOfLine);
        const auto emptyPair = QPair<QString, QString>();
        QCOMPARE(def.multiLineCommentMarker(), emptyPair);
        QVERIFY(def.characterEncodings().isEmpty());

        for (QChar c : QStringLiteral("\t !%&()*+,-./:;<=>?[\\]^{|}~")) {
            QVERIFY(def.isWordDelimiter(c));
            QVERIFY(def.isWordWrapDelimiter(c));
        }
    }

    void testDelimiters()
    {
        auto def = m_repo.definitionForName(QLatin1String("LaTeX"));
        QVERIFY(def.isValid());

        // check that backslash '\' and '*' are removed
        for (QChar c : QStringLiteral("\t !%&()+,-./:;<=>?[]^{|}~")) {
            QVERIFY(def.isWordDelimiter(c));
        }
        QVERIFY(!def.isWordDelimiter(QLatin1Char('\\')));

        // check where breaking a line is valid
        for (QChar c : QStringLiteral(",{}[]")) {
            QVERIFY(def.isWordWrapDelimiter(c));
        }
    }

    void testFoldingEnabled()
    {
        // test invalid folding
        Definition def;
        QVERIFY(!def.isValid());
        QVERIFY(!def.foldingEnabled());
        QVERIFY(!def.indentationBasedFoldingEnabled());

        // test no folding
        def = m_repo.definitionForName(QLatin1String("ChangeLog"));
        QVERIFY(def.isValid());
        QVERIFY(!def.foldingEnabled());
        QVERIFY(!def.indentationBasedFoldingEnabled());

        // C++ itself has no regions, but it includes ISO C++
        def = m_repo.definitionForName(QLatin1String("C++"));
        QVERIFY(def.isValid());
        QVERIFY(def.foldingEnabled());
        QVERIFY(!def.indentationBasedFoldingEnabled());

        // ISO C++ itself has folding regions
        def = m_repo.definitionForName(QLatin1String("ISO C++"));
        QVERIFY(def.isValid());
        QVERIFY(def.foldingEnabled());
        QVERIFY(!def.indentationBasedFoldingEnabled());

        // Python has indentation based folding
        def = m_repo.definitionForName(QLatin1String("Python"));
        QVERIFY(def.isValid());
        QVERIFY(def.foldingEnabled());
        QVERIFY(def.indentationBasedFoldingEnabled());
    }

    void testCharacterEncodings()
    {
        auto def = m_repo.definitionForName(QLatin1String("LaTeX"));
        QVERIFY(def.isValid());
        const auto encodings = def.characterEncodings();
        QVERIFY(!encodings.isEmpty());
        QVERIFY(encodings.contains({QChar(196), QLatin1String("\\\"{A}")}));
        QVERIFY(encodings.contains({QChar(227), QLatin1String("\\~{a}")}));
    }

    void testIncludeKeywordLists()
    {
        Repository repo;
        QTemporaryDir dir;

        // forge a syntax file
        {
            QVERIFY(QDir(dir.path()).mkpath(QLatin1String("syntax")));

            const char syntax[] = R"xml(<?xml version="1.0" encoding="UTF-8"?>
            <!DOCTYPE language SYSTEM "language.dtd">
            <language version="1" kateversion="5.1" name="AAA" section="Other" extensions="*.a" mimetype="" author="" license="MIT">
              <highlighting>
                <list name="a">
                  <include>c</include>
                  <item>a</item>
                </list>
                <list name="b">
                  <item>b</item>
                  <include>a</include>
                </list>
                <list name="c">
                  <item>c</item>
                  <include>b</include>
                </list>

                <list name="d">
                  <item>d</item>
                  <include>e##AAA</include>
                </list>
                <list name="e">
                  <item>e</item>
                  <include>f</include>
                </list>
                <list name="f">
                  <item>f</item>
                </list>
                <contexts>
                  <context attribute="Normal Text" lineEndContext="#stay" name="Normal Text">
                    <keyword attribute="x" context="#stay" String="a" />
                  </context>
                </contexts>
                <itemDatas>
                  <itemData name="Normal Text" defStyleNum="dsNormal"/>
                  <itemData name="x" defStyleNum="dsAlert" />
                </itemDatas>
              </highlighting>
            </language>
            )xml";

            QFile file(dir.path() + QLatin1String("/syntax/a.xml"));
            QVERIFY(file.open(QIODevice::WriteOnly));
            QTextStream stream(&file);
            stream << syntax;
        }

        repo.addCustomSearchPath(dir.path());
        auto def = repo.definitionForName(QLatin1String("AAA"));
        QCOMPARE(def.name(), QLatin1String("AAA"));

        auto klist1 = def.keywordList(QLatin1String("a"));
        auto klist2 = def.keywordList(QLatin1String("b"));
        auto klist3 = def.keywordList(QLatin1String("c"));

        // internal QHash<QString, KeywordList> is arbitrarily ordered and undeterministic
        auto &klist = klist1.size() == 3 ? klist1 : klist2.size() == 3 ? klist2 : klist3;

        QCOMPARE(klist.size(), 3);
        QVERIFY(klist.contains(QLatin1String("a")));
        QVERIFY(klist.contains(QLatin1String("b")));
        QVERIFY(klist.contains(QLatin1String("c")));

        klist = def.keywordList(QLatin1String("d"));
        QCOMPARE(klist.size(), 3);
        QVERIFY(klist.contains(QLatin1String("d")));
        QVERIFY(klist.contains(QLatin1String("e")));
        QVERIFY(klist.contains(QLatin1String("f")));

        klist = def.keywordList(QLatin1String("e"));
        QCOMPARE(klist.size(), 2);
        QVERIFY(klist.contains(QLatin1String("e")));
        QVERIFY(klist.contains(QLatin1String("f")));

        klist = def.keywordList(QLatin1String("f"));
        QCOMPARE(klist.size(), 1);
        QVERIFY(klist.contains(QLatin1String("f")));
    }

    void testKeywordListModification()
    {
        auto def = m_repo.definitionForName(QLatin1String("Python"));
        QVERIFY(def.isValid());

        const QStringList &lists = def.keywordLists();
        QVERIFY(!lists.isEmpty());

        const QString &listName = lists.first();
        const QStringList keywords = def.keywordList(listName);

        QStringList modified = keywords;
        modified.append(QLatin1String("test"));

        QVERIFY(def.setKeywordList(listName, modified) == true);
        QCOMPARE(keywords + QStringList(QLatin1String("test")), def.keywordList(listName));

        const QString &unexistedName = QLatin1String("unexisted-keyword-name");
        QVERIFY(lists.contains(unexistedName) == false);
        QVERIFY(def.setKeywordList(unexistedName, QStringList()) == false);
    }

    void testAutomaticThemeSelection()
    {
        QPalette palette;
        palette.setColor(QPalette::Base, QColor(27, 30, 32));
        palette.setColor(QPalette::Highlight, QColor(61, 174, 253));
        auto theme = m_repo.themeForPalette(palette);
        QCOMPARE(theme.name(), QStringLiteral("Breeze Dark"));
    }
};
}

QTEST_GUILESS_MAIN(KSyntaxHighlighting::RepositoryTest)

#include "repository_test.moc"
