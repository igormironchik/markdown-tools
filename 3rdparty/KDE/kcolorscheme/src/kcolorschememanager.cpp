/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2013 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kcolorschememanager.h"
#include "kcolorschememanager_p.h"

#include "kcolorscheme.h"
#include "kcolorschememodel.h"

#include <KConfigGroup>
#include <KConfigGui>
#include <KLocalizedString>
#include <KSharedConfig>

#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QPainter>
#include <QPointer>
#include <QStandardPaths>
#include <QStyleHints>

#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
#include <QAccessibilityHints>
#endif

#include <private/qguiapplication_p.h>
#include <qpa/qplatformtheme.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

// ensure we are linking KConfigGui, so QColor I/O from KConfig works
KCONFIGGUI_EXPORT int initKConfigGroupGui();
static int s_init = initKConfigGroupGui();

constexpr int defaultSchemeRow = 0;

static bool isKdePlatformTheme()
{
    if (!QGuiApplicationPrivate::platformTheme()) {
        return false;
    }

    if (QGuiApplicationPrivate::platformTheme()->name() == QLatin1String("kde")) {
        return true;
    }

    if (qgetenv("XDG_CURRENT_DESKTOP") == "KDE" && QGuiApplicationPrivate::platformTheme()->name() == QLatin1String("xdgdesktopportal")) {
        return true;
    }

    return false;
}

void KColorSchemeManagerPrivate::activateSchemeInternal(const QString &colorSchemePath)
{
    // hint for plasma-integration to synchronize the color scheme with the window manager/compositor
    // The property needs to be set before the palette change because is is checked upon the
    // ApplicationPaletteChange event.
    qApp->setProperty("KDE_COLOR_SCHEME_PATH", colorSchemePath);
    if (colorSchemePath.isEmpty()) {
        qApp->setPalette(QPalette());
    } else {
        qApp->setPalette(KColorScheme::createApplicationPalette(KSharedConfig::openConfig(colorSchemePath)));
    }
}

QString KColorSchemeManagerPrivate::automaticColorSchemeId() const
{
    QString platformThemeSchemePath = qApp->property("KDE_COLOR_SCHEME_PATH").toString();
    if (isKdePlatformTheme() || !platformThemeSchemePath.isEmpty()) {
        return QString();
    }

    if (contrastPreference() == ContrastPreference::HighContrast) {
        return QString();
    }

    if (qGuiApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark) {
        return getDarkColorScheme();
    }

    return getLightColorScheme();
}

// The meaning of the Default entry depends on the platform
// On KDE we apply a default KColorScheme
// On other platforms we automatically apply Breeze/Breeze Dark depending on the system preference
QString KColorSchemeManagerPrivate::automaticColorSchemePath() const
{
    const QString colorSchemeId = automaticColorSchemeId();
    if (colorSchemeId.isEmpty()) {
        return QString();
    } else {
        return indexForSchemeId(colorSchemeId).data(KColorSchemeModel::PathRole).toString();
    }
}

QIcon KColorSchemeManagerPrivate::createPreview(const QString &path)
{
    KSharedConfigPtr schemeConfig = KSharedConfig::openConfig(path, KConfig::SimpleConfig);
    QIcon result;

    KColorScheme activeWindow(QPalette::Active, KColorScheme::Window, schemeConfig);
    KColorScheme activeButton(QPalette::Active, KColorScheme::Button, schemeConfig);
    KColorScheme activeView(QPalette::Active, KColorScheme::View, schemeConfig);
    KColorScheme activeSelection(QPalette::Active, KColorScheme::Selection, schemeConfig);

    auto pixmap = [&](int size) {
        QPixmap pix(size, size);
        pix.fill(Qt::black);
        QPainter p;
        p.begin(&pix);
        const int itemSize = size / 2 - 1;
        p.fillRect(1, 1, itemSize, itemSize, activeWindow.background());
        p.fillRect(1 + itemSize, 1, itemSize, itemSize, activeButton.background());
        p.fillRect(1, 1 + itemSize, itemSize, itemSize, activeView.background());
        p.fillRect(1 + itemSize, 1 + itemSize, itemSize, itemSize, activeSelection.background());
        p.end();
        result.addPixmap(pix);
    };
    // 16x16
    pixmap(16);
    // 24x24
    pixmap(24);

    return result;
}

KColorSchemeManagerPrivate::KColorSchemeManagerPrivate()
    : model(new KColorSchemeModel())
{
}

KColorSchemeManager::KColorSchemeManager(GuardApplicationConstructor, QGuiApplication *app)
    : QObject(app)
    , d(new KColorSchemeManagerPrivate)
{
    init();
}

#if KCOLORSCHEME_BUILD_DEPRECATED_SINCE(6, 6)
KColorSchemeManager::KColorSchemeManager(QObject *parent)
    : QObject(parent)
    , d(new KColorSchemeManagerPrivate())
{
    init();
}
#endif

KColorSchemeManager::~KColorSchemeManager()
{
}

void KColorSchemeManager::init()
{
    QString platformThemeSchemePath = qApp->property("KDE_COLOR_SCHEME_PATH").toString();

    auto schemeChanged = [this] {
        if (!d->m_activatedScheme.isEmpty()) {
            // Don't override what has been manually set
            return;
        }

        d->activateSchemeInternal(d->automaticColorSchemePath());
    };

    connect(qApp->styleHints(), &QStyleHints::colorSchemeChanged, this, schemeChanged);

#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    connect(qApp->styleHints()->accessibility(), &QAccessibilityHints::contrastPreferenceChanged, this, schemeChanged);
#endif

    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup cg(config, QStringLiteral("UiSettings"));

    const QString scheme = cg.readEntry("ColorScheme", QString());
    auto index = indexForSchemeId(scheme);
    if (!scheme.isEmpty() && !index.isValid()) {
        // No sucess treating value as ID maybe it is a scheme name?
        // Until 6.16 we saved the scheme name instead of the id to ColorScheme
        index = indexForScheme(scheme);

        if (index.isValid()) {
            saveSchemeIdToConfigFile(index.data(KColorSchemeModel::IdRole).toString());
        }
    }

    QString schemePath;

    if (scheme.isEmpty()) {
        // Color scheme might be already set from a platform theme
        // This is used for example by QGnomePlatform that can set color scheme
        // matching GNOME settings. This avoids issues where QGnomePlatform sets
        // QPalette for dark theme, but end up mixing it also with Breeze light
        // that is going to be used as a fallback for apps using KColorScheme.
        // BUG: 447029
        if (platformThemeSchemePath.isEmpty()) {
            schemePath = d->automaticColorSchemePath();
        }
    } else {
        schemePath = index.data(KColorSchemeModel::PathRole).toString();
        d->m_activatedScheme = index.data(KColorSchemeModel::IdRole).toString();
    }

    if (!schemePath.isEmpty()) {
        d->activateSchemeInternal(schemePath);
    }
}

QAbstractItemModel *KColorSchemeManager::model() const
{
    return d->model.get();
}

QModelIndex KColorSchemeManagerPrivate::indexForSchemeId(const QString &id) const
{
    // Empty string is mapped to "reset to the system scheme"
    if (id.isEmpty()) {
        return model->index(defaultSchemeRow);
    }
    for (int i = 1; i < model->rowCount(); ++i) {
        QModelIndex index = model->index(i);
        if (index.data(KColorSchemeModel::IdRole).toString() == id) {
            return index;
        }
    }
    return QModelIndex();
}

void KColorSchemeManager::setAutosaveChanges(bool autosaveChanges)
{
    d->m_autosaveChanges = autosaveChanges;
}

QModelIndex KColorSchemeManager::indexForSchemeId(const QString &id) const
{
    return d->indexForSchemeId(id);
}

QModelIndex KColorSchemeManager::indexForScheme(const QString &name) const
{
    // Empty string is mapped to "reset to the system scheme"
    if (name.isEmpty()) {
        return d->model->index(defaultSchemeRow);
    }
    for (int i = 1; i < d->model->rowCount(); ++i) {
        QModelIndex index = d->model->index(i);
        if (index.data(KColorSchemeModel::NameRole).toString() == name) {
            return index;
        }
    }
    return QModelIndex();
}

void KColorSchemeManager::activateScheme(const QModelIndex &index)
{
    const bool isDefaultEntry = index.data(KColorSchemeModel::PathRole).toString().isEmpty();

    if (index.isValid() && index.model() == d->model.get() && !isDefaultEntry) {
        d->m_activatedScheme = index.data(KColorSchemeModel::IdRole).toString();
        if (d->m_autosaveChanges) {
            saveSchemeIdToConfigFile(index.data(KColorSchemeModel::IdRole).toString());
        }
        d->activateSchemeInternal(index.data(KColorSchemeModel::PathRole).toString());
    } else {
        d->m_activatedScheme = QString();
        if (d->m_autosaveChanges) {
            saveSchemeIdToConfigFile(QString());
        }
        d->activateSchemeInternal(d->automaticColorSchemePath());
    }
}

void KColorSchemeManager::activateSchemeId(const QString &schemeId)
{
    auto index = d->indexForSchemeId(schemeId);
    const bool isDefaultEntry = index.data(KColorSchemeModel::IdRole).toString().isEmpty();

    if (index.isValid() && !isDefaultEntry) {
        d->activateSchemeInternal(index.data(KColorSchemeModel::PathRole).toString());
        d->m_activatedScheme = index.data(KColorSchemeModel::IdRole).toString();
        if (d->m_autosaveChanges) {
            saveSchemeIdToConfigFile(index.data(KColorSchemeModel::IdRole).toString());
        }
    } else {
        d->activateSchemeInternal(d->automaticColorSchemePath());
        d->m_activatedScheme = QString();
        if (d->m_autosaveChanges) {
            saveSchemeIdToConfigFile(QString());
        }
    }
}

#if KCOLORSCHEME_ENABLE_DEPRECATED_SINCE(6, 19)
void KColorSchemeManager::saveSchemeToConfigFile(const QString &schemeName) const
{
    const auto index = indexForScheme(schemeName);
    QString schemeId;
    if (index.isValid()) {
        schemeId = index.data(KColorSchemeModel::IdRole).toString();
    }
    saveSchemeIdToConfigFile(schemeId);
}
#endif

void KColorSchemeManager::saveSchemeIdToConfigFile(const QString &schemeId) const
{
    KSharedConfigPtr config = KSharedConfig::openConfig();
    KConfigGroup cg(config, QStringLiteral("UiSettings"));
    if (schemeId.isEmpty() && !cg.hasDefault("ColorScheme")) {
        cg.revertToDefault("ColorScheme");
    } else {
        cg.writeEntry("ColorScheme", schemeId);
    }
    cg.sync();
}

QString KColorSchemeManager::activeSchemeId() const
{
    return d->m_activatedScheme;
}

QString KColorSchemeManager::activeSchemeName() const
{
    return d->indexForSchemeId(d->m_activatedScheme).data(KColorSchemeModel::NameRole).toString();
}

#ifdef Q_OS_WIN
static bool isWindowsHighContrastModeActive()
{
    HIGHCONTRAST result;
    result.cbSize = sizeof(HIGHCONTRAST);
    if (SystemParametersInfo(SPI_GETHIGHCONTRAST, result.cbSize, &result, 0)) {
        return (result.dwFlags & HCF_HIGHCONTRASTON);
    }
    return false;
}
#endif

KColorSchemeManagerPrivate::ContrastPreference KColorSchemeManagerPrivate::contrastPreference()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 10, 0)
    return qGuiApp->styleHints()->accessibility()->contrastPreference() == Qt::ContrastPreference::HighContrast ? HighContrast : NoPreference;
#else
#ifdef Q_OS_WIN
    return isWindowsHighContrastModeActive() ? HighContrast : NoPreference;
#endif
#endif
    return NoPreference;
}

KColorSchemeManager *KColorSchemeManager::instance()
{
    Q_ASSERT(qApp);
    static QPointer<KColorSchemeManager> manager;
    if (!manager) {
        manager = new KColorSchemeManager(GuardApplicationConstructor{}, qApp);
    }
    return manager;
}

#include "moc_kcolorschememanager.cpp"
