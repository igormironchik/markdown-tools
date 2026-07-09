/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// shared include.
#include "utils.h"
#include "emoji_parser.h"
#include "emphasis.h"
#include "syntax.h"

// Qt include.
#include <QModelIndex>
#include <QStyle>
#include <QWindow>
#include <QtResource>

// md4qt inclide.
#include <md4qt/src/asterisk_emphasis_parser.h>
#include <md4qt/src/atx_heading_parser.h>
#include <md4qt/src/autolink_parser.h>
#include <md4qt/src/blockquote_parser.h>
#include <md4qt/src/emphasis_parser.h>
#include <md4qt/src/fenced_code_parser.h>
#include <md4qt/src/footnote_parser.h>
#include <md4qt/src/gfm_autolink_parser.h>
#include <md4qt/src/hard_line_break_parser.h>
#include <md4qt/src/html_parser.h>
#include <md4qt/src/indented_code_parser.h>
#include <md4qt/src/inline_code_parser.h>
#include <md4qt/src/inline_html_parser.h>
#include <md4qt/src/inline_math_parser.h>
#include <md4qt/src/link_image_parser.h>
#include <md4qt/src/list_parser.h>
#include <md4qt/src/paragraph_parser.h>
#include <md4qt/src/setext_heading_parser.h>
#include <md4qt/src/strikethrough_emphasis_parser.h>
#include <md4qt/src/table_parser.h>
#include <md4qt/src/thematic_break_parser.h>
#include <md4qt/src/underline_emphasis_parser.h>
#include <md4qt/src/yaml_parser.h>

#ifdef MD_BREEZE
#include <KColorSchemeManager>
#include <KConfigGroup>
#include <KIconEngine>
#include <KIconLoader>
#include <KIconTheme>
#include <KSharedConfig>

#include <QIcon>
#include <QPixmapCache>
#include <QStyleHints>
#endif // MD_BREEZE

#ifdef Q_OS_WIN
#include <Windows.h>
#include <dwmapi.h>
#endif

void refreshStyleRecursively(QWidget *widget,
                             const QPalette &p)
{
    if (!widget) {
        return;
    }

    widget->setPalette(p);
    widget->style()->unpolish(widget);
    widget->style()->polish(widget);

    for (QObject *child : std::as_const(widget->children())) {
        if (QWidget *childWidget = qobject_cast<QWidget *>(child)) {
            refreshStyleRecursively(childWidget, p);
        }
    }

    widget->repaint();
}

void applyTheme(const QString &name,
                bool isDark)
{
#if defined(MD_BREEZE) && defined(Q_OS_WIN)
    auto upper = name;
    upper[0] = upper[0].toUpper();
    const auto scheme = upper + (isDark ? QStringLiteral("Dark") : QStringLiteral("Light"));
    const auto idx = KColorSchemeManager::instance()->indexForSchemeId(scheme);

    if (idx.isValid()) {
        qApp->styleHints()->setColorScheme(isDark ? Qt::ColorScheme::Dark : Qt::ColorScheme::Light);
        auto cfg = KSharedConfig::openConfig();
        const auto iconThemeName = QStringLiteral("breeze") + (isDark ? QStringLiteral("-dark") : QString());
        auto cg = cfg->group(QStringLiteral("Icons"));
        cg.writeEntry(QStringLiteral("Theme"), iconThemeName);
        cg.sync();
        KIconTheme::forceThemeForTests(iconThemeName);
        KIconLoader::global()->reconfigure(qApp->applicationName());
        QPixmapCache::clear();
        KColorSchemeManager::instance()->activateScheme(idx);
    }
#endif

    QIcon::setFallbackSearchPaths(
        QStringList() << QStringLiteral(":/pics/%1").arg(isDark ? QStringLiteral("md-dark") : QStringLiteral("md")));

    const auto windows = QApplication::topLevelWidgets();

    for (const auto &w : windows) {
        if (w) {
            refreshStyleRecursively(w, qApp->palette());

#ifdef Q_OS_WIN
            if (w->winId()) {
                HWND hwnd = reinterpret_cast<HWND>(w->winId());
                BOOL useDarkMode = isDark;
                DWORD attribute = DWMWA_USE_IMMERSIVE_DARK_MODE;
                DwmSetWindowAttribute(hwnd, attribute, &useDarkMode, sizeof(useDarkMode));
                SendMessage(hwnd, WM_NCACTIVATE, FALSE, 0);
                SendMessage(hwnd, WM_NCACTIVATE, TRUE, 0);
            }
#endif
        }
    }

    if (isDark) {
        QIcon appIcon(QStringLiteral(":/pics/icon_256x256-dark.png"));
        appIcon.addFile(QStringLiteral(":/pics/icon_128x128-dark.png"));
        appIcon.addFile(QStringLiteral(":/pics/icon_64x64-dark.png"));
        appIcon.addFile(QStringLiteral(":/pics/icon_48x48-dark.png"));
        appIcon.addFile(QStringLiteral(":/pics/icon_32x32-dark.png"));
        appIcon.addFile(QStringLiteral(":/pics/icon_24x24-dark.png"));
        appIcon.addFile(QStringLiteral(":/pics/icon_16x16-dark.png"));
        qApp->setWindowIcon(appIcon);
    } else {
        QIcon appIcon(QStringLiteral(":/pics/icon_256x256.png"));
        appIcon.addFile(QStringLiteral(":/pics/icon_128x128.png"));
        appIcon.addFile(QStringLiteral(":/pics/icon_64x64.png"));
        appIcon.addFile(QStringLiteral(":/pics/icon_48x48.png"));
        appIcon.addFile(QStringLiteral(":/pics/icon_32x32.png"));
        appIcon.addFile(QStringLiteral(":/pics/icon_24x24.png"));
        appIcon.addFile(QStringLiteral(":/pics/icon_16x16.png"));
        qApp->setWindowIcon(appIcon);
    }
}

void initSharedResources()
{
    Q_INIT_RESOURCE(qt);
    Q_INIT_RESOURCE(icon);
    Q_INIT_RESOURCE(tr);
    MdShared::Syntax::init();
}

bool isRightToLeft(const QChar &ch)
{
    switch (ch.direction()) {
    case QChar::DirAL:
    case QChar::DirAN:
    case QChar::DirR:
        return true;

    default:
        return false;
    }
}

QVector<QPair<QString,
              bool>>
splitString(const QString &str,
            bool skipSpaces)
{
    QVector<QPair<QString, bool>> res;
    qsizetype first = 0;
    bool space = false;
    bool previousRTL = false;

    for (qsizetype i = 0; i < str.length(); ++i) {
        if (str[i].isSpace() && !space) {
            if (first < i) {
                const auto word = str.sliced(first, i - first);
                bool rtl = false;

                if (word[0].direction() != QChar::DirON) {
                    previousRTL = isRightToLeft(word[0]);
                }

                rtl = previousRTL;

                res.append({word, rtl});
            }

            if (!skipSpaces) {
                res.append({QStringLiteral(" "), false});
            }

            space = true;
        } else {
            if (space) {
                first = i;
            }

            space = false;
        }
    }

    if (!space && first < str.length()) {
        const auto word = str.sliced(first);
        res.append({word, isRightToLeft(word[0])});
    }

    return res;
}

void orderWords(QVector<QPair<QString,
                              bool>> &text)
{
    qsizetype start = -1;
    qsizetype end = -1;

    auto reverseItems = [](qsizetype start, qsizetype end, QVector<QPair<QString, bool>> &data) {
        if (start > -1 && end > start) {
            while (end - start > 0) {
                data.swapItemsAt(start, end);
                ++start;
                --end;
            }
        }
    };

    for (qsizetype i = 0; i < text.size(); ++i) {
        if (text[i].first != QStringLiteral(" ")) {
            if (!text[i].second) {
                if (start == -1) {
                    start = i;
                    end = i;
                } else {
                    end = i;
                }
            } else {
                reverseItems(start, end, text);

                start = -1;
                end = -1;
            }
        }
    }

    reverseItems(start, end, text);
}

void setPlugins(MD::Parser &parser,
                const MdShared::PluginsCfg &cfg,
                bool enableEmoji)
{
    MD::Parser::InlineParsers inlineParsers;

    MD::Parser::appendInlineParser<MD::InlineCodeParser>(inlineParsers);

    auto linkParser = MD::Parser::appendInlineParser<MD::LinkImageParser>(inlineParsers);

    MD::Parser::appendInlineParser<MD::AutolinkParser>(inlineParsers);
    MD::Parser::appendInlineParser<MD::InlineHtmlParser>(inlineParsers);
    MD::Parser::appendInlineParser<MD::InlineMathParser>(inlineParsers);
    MD::Parser::appendInlineParser<MD::AsteriskEmphasisParser>(inlineParsers);
    MD::Parser::appendInlineParser<MD::UnderlineEmphasisParser>(inlineParsers);
    MD::Parser::appendInlineParser<MD::StrikethroughEmphasisParser>(inlineParsers);

    if (enableEmoji) {
        MD::Parser::appendInlineParser<MdShared::EmojiParser>(inlineParsers);
    }

    if (cfg.m_sup.m_on) {
        inlineParsers.append(QSharedPointer<MdShared::SupEmphasisParser>::create(cfg.m_sup.m_delimiter));
    }

    if (cfg.m_sub.m_on) {
        inlineParsers.append(QSharedPointer<MdShared::SubEmphasisParser>::create(cfg.m_sub.m_delimiter));
    }

    if (cfg.m_mark.m_on) {
        inlineParsers.append(QSharedPointer<MdShared::HighlightEmphasisParser>::create(cfg.m_mark.m_delimiter));
    }

    inlineParsers.append(QSharedPointer<MD::GfmAutolinkParser>::create(linkParser));

    MD::Parser::appendInlineParser<MD::HardLineBreakParser>(inlineParsers);

    MD::Parser::BlockParsers blockParsers;

    if (cfg.m_yamlEnabled) {
        MD::Parser::appendBlockParser<MD::YAMLParser>(blockParsers, &parser);
    }

    MD::Parser::appendBlockParser<MD::BlockquoteParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::SetextHeadingParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::ThematicBreakParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::ListParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::ATXHeadingParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::FencedCodeParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::HTMLParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::IndentedCodeParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::FootnoteParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::TableParser>(blockParsers, &parser);
    MD::Parser::appendBlockParser<MD::ParagraphParser>(blockParsers, &parser);

    parser.setBlockParsers(blockParsers);
    parser.setInlineParsers(inlineParsers);
}

static const QChar s_dash = QLatin1Char('-');
static const QString s_english = QStringLiteral("en");

bool hasEnglish(const QStringList &langs)
{
    for (const auto &lang : langs) {
        const auto i = lang.indexOf(s_dash);

        if (i != -1) {
            if (lang.left(i).toLower() == s_english) {
                return true;
            }
        }
    }

    return false;
}
