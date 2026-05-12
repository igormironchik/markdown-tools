/*
    SPDX-FileCopyrightText: 2014 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "windoweffects.h"

#include <QDebug>
#include <QExposeEvent>
#include <QGuiApplication>

#include <qpa/qplatformwindow_p.h>

#include <QWaylandClientExtension>
#include <QWaylandClientExtensionTemplate>

#include "qwayland-blur.h"
#include "qwayland-contrast.h"
#include "qwayland-ext-background-effect-v1.h"
#include "qwayland-slide.h"

#include "helpers.h"

#include <wayland-client-protocol.h>

static wl_region *createRegion(const QRegion &region)
{
    auto native = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
    if (!native) {
        return nullptr;
    }
    auto compositor = native->compositor();
    if (!compositor) {
        return nullptr;
    }
    auto wl_region = wl_compositor_create_region(compositor);
    for (const auto &rect : region) {
        wl_region_add(wl_region, rect.x(), rect.y(), rect.width(), rect.height());
    }
    return wl_region;
}

class BlurManager : public QWaylandClientExtensionTemplate<BlurManager>, public QtWayland::org_kde_kwin_blur_manager
{
public:
    BlurManager()
        : QWaylandClientExtensionTemplate<BlurManager>(1)
    {
        initialize();
    }
};

class Blur : public QObject, public QtWayland::org_kde_kwin_blur
{
public:
    Blur(struct ::org_kde_kwin_blur *object, QObject *parent)
        : QObject(parent)
        , QtWayland::org_kde_kwin_blur(object)
    {
    }

    ~Blur() override
    {
        if (isQpaAlive()) {
            release();
        }
    }
};

class ContrastManager : public QWaylandClientExtensionTemplate<ContrastManager>, public QtWayland::org_kde_kwin_contrast_manager
{
public:
    ContrastManager()
        : QWaylandClientExtensionTemplate<ContrastManager>(2)
    {
        initialize();
    }
};

class Contrast : public QObject, public QtWayland::org_kde_kwin_contrast
{
public:
    Contrast(struct ::org_kde_kwin_contrast *object, QObject *parent)
        : QObject(parent)
        , QtWayland::org_kde_kwin_contrast(object)
    {
    }

    ~Contrast() override
    {
        if (isQpaAlive()) {
            release();
        }
    }
};

class SlideManager : public QWaylandClientExtensionTemplate<SlideManager>, public QtWayland::org_kde_kwin_slide_manager
{
public:
    SlideManager()
        : QWaylandClientExtensionTemplate<SlideManager>(1)
    {
        initialize();
    }
};

class Slide : public QObject, public QtWayland::org_kde_kwin_slide
{
public:
    Slide(struct ::org_kde_kwin_slide *object, QObject *parent)
        : QObject(parent)
        , QtWayland::org_kde_kwin_slide(object)
    {
    }

    ~Slide() override
    {
        if (isQpaAlive()) {
            release();
        }
    }
};

class BackgroundEffectManager : public QWaylandClientExtensionTemplate<BackgroundEffectManager>, public QtWayland::ext_background_effect_manager_v1
{
    Q_OBJECT
public:
    BackgroundEffectManager()
        : QWaylandClientExtensionTemplate<BackgroundEffectManager>(1)
    {
        initialize();
    }

    ~BackgroundEffectManager()
    {
        if (isQpaAlive() && isActive()) {
            destroy();
        }
    }

    void ext_background_effect_manager_v1_capabilities(uint32_t flags) override
    {
        supportsBlur = flags & EXT_BACKGROUND_EFFECT_MANAGER_V1_CAPABILITY_BLUR;
        Q_EMIT capabilitiesChanged();
    }

    bool supportsBlur = false;

    Q_SIGNAL void capabilitiesChanged();
};

class BackgroundEffect : public QtWayland::ext_background_effect_surface_v1
{
public:
    BackgroundEffect(struct ::ext_background_effect_surface_v1 *object)
        : QtWayland::ext_background_effect_surface_v1(object)
    {
    }

    ~BackgroundEffect() override
    {
        if (isQpaAlive()) {
            destroy();
        }
    }
};

WindowEffects::WindowEffects()
    : QObject()
    , KWindowEffectsPrivate()
    , m_blurManager(std::make_unique<BlurManager>())
    , m_contrastManager(std::make_unique<ContrastManager>())
    , m_slideManager(std::make_unique<SlideManager>())
    , m_backgroundEffectManager(std::make_unique<BackgroundEffectManager>())
{
    // The KWindowEffects API doesn't provide any signals to notify that the particular
    // effect has become unavailable. So we re-install effects when the corresponding globals
    // are added.

    connect(m_blurManager.get(), &BlurManager::activeChanged, this, [this] {
        if (m_backgroundEffectManager->isActive()) {
            return;
        }
        for (auto it = m_blurRegions.constBegin(); it != m_blurRegions.constEnd(); ++it) {
            installBlur(it.key(), m_blurManager->isActive(), *it);
        }
    });

    connect(m_contrastManager.get(), &ContrastManager::activeChanged, this, [this] {
        for (auto it = m_backgroundConstrastRegions.constBegin(); it != m_backgroundConstrastRegions.constEnd(); ++it) {
            if (m_contrastManager->isActive()) {
                installContrast(it.key(), true, it->contrast, it->intensity, it->saturation, it->region);
            } else {
                installContrast(it.key(), false);
            }
        }
    });

    connect(m_slideManager.get(), &SlideManager::activeChanged, this, [this] {
        for (auto it = m_slideMap.constBegin(); it != m_slideMap.constEnd(); ++it) {
            if (m_slideManager->isActive()) {
                installSlide(it.key(), it->location, it->offset);
            } else {
                installSlide(it.key(), KWindowEffects::SlideFromLocation::NoEdge, 0);
            }
        }
    });

    connect(m_backgroundEffectManager.get(), &BackgroundEffectManager::activeChanged, this, [this]() {
        if (!m_backgroundEffectManager->isActive()) {
            m_backgroundEffects.clear();
        }
    });

    connect(m_backgroundEffectManager.get(), &BackgroundEffectManager::capabilitiesChanged, this, [this]() {
        for (auto it = m_blurRegions.constBegin(); it != m_blurRegions.constEnd(); ++it) {
            installBlur(it.key(), m_backgroundEffectManager->supportsBlur, *it);
        }
    });
}

WindowEffects::~WindowEffects()
{
}

void WindowEffects::trackWindow(QWindow *window)
{
    if (!m_windowDestroyedWatchers.contains(window)) {
        window->installEventFilter(this);
        m_windowDestroyedWatchers[window] = connect(window, &QObject::destroyed, this, [this, window]() {
            resetBlur(window);
            m_blurRegions.remove(window);
            resetContrast(window);
            m_backgroundConstrastRegions.remove(window);
            m_slideMap.remove(window);
            m_windowDestroyedWatchers.remove(window);
            m_surfaceDestroyedWatchers.remove(window);
        });
    }
    if (!m_surfaceDestroyedWatchers.contains(window)) {
        auto waylandWindow = window->nativeInterface<QNativeInterface::Private::QWaylandWindow>();
        if (waylandWindow) {
            m_surfaceDestroyedWatchers[window] = connect(waylandWindow, &QNativeInterface::Private::QWaylandWindow::surfaceDestroyed, this, [this, window]() {
                resetBlur(window);
                resetContrast(window);
            });
        }
    }
}

void WindowEffects::releaseWindow(QWindow *window)
{
    if (!m_blurRegions.contains(window) && !m_backgroundConstrastRegions.contains(window) && !m_slideMap.contains(window)) {
        if (auto it = m_windowDestroyedWatchers.find(window); it != m_windowDestroyedWatchers.end()) {
            disconnect(*it);
            m_windowDestroyedWatchers.erase(it);
        }
        if (auto it = m_surfaceDestroyedWatchers.find(window); it != m_surfaceDestroyedWatchers.end()) {
            disconnect(*it);
            m_surfaceDestroyedWatchers.erase(it);
        }
        window->removeEventFilter(this);
    }
}

// Helper function to replace a QObject value in the map and delete the old one.
template<typename MapType>
void replaceValue(MapType &map, typename MapType::key_type key, typename MapType::mapped_type value)
{
    if (auto oldValue = map.take(key)) {
        oldValue->deleteLater();
    }
    if (value) {
        map[key] = value;
    }
}

void WindowEffects::resetBlur(QWindow *window, Blur *blur)
{
    m_backgroundEffects.erase(window);
    replaceValue(m_blurs, window, blur);
}

void WindowEffects::resetContrast(QWindow *window, Contrast *contrast)
{
    replaceValue(m_contrasts, window, contrast);
}

bool WindowEffects::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Expose) {
        auto window = qobject_cast<QWindow *>(watched);
        if (!window || !window->isExposed()) {
            return false;
        }

        {
            auto it = m_blurRegions.constFind(window);
            if (it != m_blurRegions.constEnd()) {
                installBlur(window, true, *it);
            }
        }
        {
            auto it = m_backgroundConstrastRegions.constFind(window);
            if (it != m_backgroundConstrastRegions.constEnd()) {
                installContrast(window, true, it->contrast, it->intensity, it->saturation, it->region);
            }
        }
        {
            auto it = m_slideMap.constFind(window);
            if (it != m_slideMap.constEnd()) {
                installSlide(window, it->location, it->offset);
            }
        }
    } else if (event->type() == QEvent::PlatformSurface) {
        auto ps = static_cast<QPlatformSurfaceEvent *>(event);
        if (ps->surfaceEventType() != QPlatformSurfaceEvent::SurfaceCreated) {
            return false;
        }
        auto window = static_cast<QWindow *>(watched);
        auto waylandWindow = window->nativeInterface<QNativeInterface::Private::QWaylandWindow>();
        if (!waylandWindow) {
            return false;
        }
        if (auto it = m_surfaceDestroyedWatchers.find(window); it != m_surfaceDestroyedWatchers.end()) {
            disconnect(*it);
        }
        m_surfaceDestroyedWatchers[window] = connect(waylandWindow, &QNativeInterface::Private::QWaylandWindow::surfaceDestroyed, this, [this, window]() {
            resetBlur(window);
            resetContrast(window);
        });
    }
    return false;
}

bool WindowEffects::isEffectAvailable(KWindowEffects::Effect effect)
{
    switch (effect) {
    case KWindowEffects::BackgroundContrast:
        return m_contrastManager->isActive();
    case KWindowEffects::BlurBehind:
        if (m_backgroundEffectManager->isActive()) {
            return m_backgroundEffectManager->supportsBlur;
        }
        return m_blurManager->isActive();
    case KWindowEffects::Slide:
        return m_slideManager->isActive();
    default:
        return false;
    }
}

void WindowEffects::slideWindow(QWindow *window, KWindowEffects::SlideFromLocation location, int offset)
{
    if (location != KWindowEffects::SlideFromLocation::NoEdge) {
        trackWindow(window);

        const SlideData data{
            .location = location,
            .offset = offset,
        };

        if (auto it = m_slideMap.find(window); it != m_slideMap.end()) {
            if (*it == data) {
                return;
            } else {
                *it = data;
            }
        } else {
            m_slideMap.insert(window, data);
        }
    } else {
        if (!m_slideMap.remove(window)) {
            return;
        }
        releaseWindow(window);
    }

    installSlide(window, location, offset);
}

void WindowEffects::installSlide(QWindow *window, KWindowEffects::SlideFromLocation location, int offset)
{
    if (!m_slideManager->isActive()) {
        return;
    }
    wl_surface *surface = surfaceForWindow(window);
    if (surface) {
        if (location != KWindowEffects::SlideFromLocation::NoEdge) {
            auto slide = new Slide(m_slideManager->create(surface), window);

            Slide::location convertedLoc;
            switch (location) {
            case KWindowEffects::SlideFromLocation::TopEdge:
                convertedLoc = Slide::location::location_top;
                break;
            case KWindowEffects::SlideFromLocation::LeftEdge:
                convertedLoc = Slide::location::location_left;
                break;
            case KWindowEffects::SlideFromLocation::RightEdge:
                convertedLoc = Slide::location::location_right;
                break;
            case KWindowEffects::SlideFromLocation::BottomEdge:
            default:
                convertedLoc = Slide::location::location_bottom;
                break;
            }

            slide->set_location(convertedLoc);
            slide->set_offset(offset);
            slide->commit();
        } else {
            m_slideManager->unset(surface);
        }
    }
}

void WindowEffects::enableBlurBehind(QWindow *window, bool enable, const QRegion &region)
{
    if (enable) {
        trackWindow(window);

        if (auto it = m_blurRegions.find(window); it != m_blurRegions.end()) {
            if (*it == region) {
                return;
            } else {
                *it = region;
            }
        } else {
            m_blurRegions.insert(window, region);
        }
    } else {
        if (!m_blurRegions.remove(window)) {
            return;
        }
        resetBlur(window);
        releaseWindow(window);
    }

    installBlur(window, enable, region);
}

void WindowEffects::installBlur(QWindow *window, bool enable, const QRegion &region)
{
    wl_surface *surface = surfaceForWindow(window);
    if (!surface) {
        return;
    }
    if (m_backgroundEffectManager->isActive()) {
        replaceValue(m_blurs, window, nullptr);
        if (!m_backgroundEffectManager->supportsBlur) {
            // will be set if/when the capability changes
            return;
        }
        auto &effect = m_backgroundEffects[window];
        if (!effect) {
            effect = std::make_unique<BackgroundEffect>(m_backgroundEffectManager->get_background_effect(surface));
        }
        wl_region *wlRegion = nullptr;
        if (enable) {
            if (region.isEmpty()) {
                // empty region = cover the whole window
                wlRegion = createRegion(QRegion{
                    std::numeric_limits<int>::min() / 2,
                    std::numeric_limits<int>::min() / 2,
                    std::numeric_limits<int>::max(),
                    std::numeric_limits<int>::max(),
                });
            } else {
                wlRegion = createRegion(region);
            }
        }
        effect->set_blur_region(wlRegion);
        if (wlRegion) {
            wl_region_destroy(wlRegion);
        }
        return;
    }
    if (!m_blurManager->isActive()) {
        return;
    }
    if (enable) {
        auto wl_region = createRegion(region);
        if (!wl_region) {
            return;
        }
        auto blur = new Blur(m_blurManager->create(surface), window);
        blur->set_region(wl_region);
        blur->commit();
        wl_region_destroy(wl_region);
        resetBlur(window, blur);
    } else {
        resetBlur(window);
        m_blurManager->unset(surface);
    }
}

void WindowEffects::enableBackgroundContrast(QWindow *window, bool enable, qreal contrast, qreal intensity, qreal saturation, const QRegion &region)
{
    if (enable) {
        trackWindow(window);

        const BackgroundContrastData data{
            .contrast = contrast,
            .intensity = intensity,
            .saturation = saturation,
            .region = region,
        };

        if (auto it = m_backgroundConstrastRegions.find(window); it != m_backgroundConstrastRegions.end()) {
            if (*it == data) {
                return;
            } else {
                *it = data;
            }
        } else {
            m_backgroundConstrastRegions.insert(window, data);
        }
    } else {
        if (!m_backgroundConstrastRegions.remove(window)) {
            return;
        }
        resetContrast(window);
        releaseWindow(window);
    }

    installContrast(window, enable, contrast, intensity, saturation, region);
}

void WindowEffects::installContrast(QWindow *window, bool enable, qreal contrast, qreal intensity, qreal saturation, const QRegion &region)
{
    if (!m_contrastManager->isActive()) {
        return;
    }

    wl_surface *surface = surfaceForWindow(window);
    if (surface) {
        if (enable) {
            auto wl_region = createRegion(region);
            if (!wl_region) {
                return;
            }
            auto backgroundContrast = new Contrast(m_contrastManager->create(surface), window);
            backgroundContrast->set_region(wl_region);
            backgroundContrast->set_contrast(wl_fixed_from_double(contrast));
            backgroundContrast->set_intensity(wl_fixed_from_double(intensity));
            backgroundContrast->set_saturation(wl_fixed_from_double(saturation));
            backgroundContrast->commit();
            wl_region_destroy(wl_region);
            resetContrast(window, backgroundContrast);
        } else {
            resetContrast(window);
            m_contrastManager->unset(surface);
        }
    }
}

#include "moc_windoweffects.cpp"
#include "windoweffects.moc"
