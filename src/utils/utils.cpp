/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// shared include.
#include "utils.h"

// md-pdf-lib include.
#include <md-pdf-lib/src/utils.h>

// Qt include.
#include <QIcon>
#include <QModelIndex>
#include <QPixmapCache>
#include <QStyle>
#include <QStyleHints>
#include <QWindow>
#include <QtResource>

#if defined(MD_BREEZE) && defined(Q_OS_WIN)
#include <KColorSchemeManager>
#include <KConfigGroup>
#include <KIconEngine>
#include <KIconLoader>
#include <KIconTheme>
#include <KSharedConfig>

#include <Windows.h>
#include <dwmapi.h>

#include <QMainWindow>
#include <QMenuBar>
#include <QStyleFactory>
#endif

void initTheme(QApplication &app)
{
#ifdef Q_OS_WIN
    app.setStyle(QStyleFactory::create("Breeze"));
#endif

#ifdef Q_OS_LINUX
    setFallbackPathForIcons(app.styleHints()->colorScheme() == Qt::ColorScheme::Dark);
#endif

#if defined(MD_BREEZE) && defined(Q_OS_WIN)
    const auto isDark = KColorSchemeManager::instance()->activeSchemeId().toLower().endsWith(QStringLiteral("dark"));

    setFallbackPathForIcons(isDark);

    if (isDark) {
        app.styleHints()->setColorScheme(Qt::ColorScheme::Dark);
    } else {
        app.styleHints()->setColorScheme(Qt::ColorScheme::Light);
    }
#endif
}

void setFallbackPathForIcons(bool isDark)
{
    QIcon::setFallbackSearchPaths(QStringList() << QStringLiteral(":/pics/%1/scalable/apps")
                                                       .arg(isDark ? QStringLiteral("md-dark") : QStringLiteral("md")));
}

void applyTheme(const QString &name,
                bool isDark)
{
    setFallbackPathForIcons(isDark);

#ifdef Q_OS_LINUX
    QPixmapCache::clear();
#endif

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

    QEvent themeEvent(QEvent::ThemeChange);
    QApplication::sendEvent(qApp, &themeEvent);

    const auto windows = QApplication::topLevelWidgets();

    for (const auto &w : windows) {
        if (w) {
            auto mw = dynamic_cast<QMainWindow *>(w);

            if (mw && mw->menuBar()) {
                mw->menuBar()->setPalette(qApp->palette());
            }

            if (w->winId()) {
                HWND hwnd = reinterpret_cast<HWND>(w->winId());
                BOOL useDarkMode = isDark;
                DWORD attribute = DWMWA_USE_IMMERSIVE_DARK_MODE;
                DwmSetWindowAttribute(hwnd, attribute, &useDarkMode, sizeof(useDarkMode));
                SendMessage(hwnd, WM_NCACTIVATE, FALSE, 0);
                SendMessage(hwnd, WM_NCACTIVATE, TRUE, 0);
            }
        }
    }
#endif

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
    MdPdf::initSharedResources();
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
