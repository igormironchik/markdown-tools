// SPDX-FileCopyrightText: 2024 Mathis Brüchert <mbb@kaidan.im>
// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#ifndef KWINDOWINSETSCONTROLLER_H
#define KWINDOWINSETSCONTROLLER_H

#include "kguiaddons_export.h"

#include <QColor>
#include <QObject>

#include <memory>

class KWindowInsetsControllerPrivate;

/*!
 * \qmlsingletontype WindowInsetsController
 * \inqmlmodule org.kde.guiaddons
 * \nativetype KWindowInsetsController
 *
 * \brief Access to window insets colors.
 *
 * On most platforms this does nothing, on Android it allows
 * to customize the (top) status bar and (botton) navigation bar
 * background colors, e.g. to match the current window or application
 * colors.
 *
 * Note that the foreground colors on Android are automatically
 * chosen based on the background color.
 *
 * \code
 * Component.onComplete: {
 *     WindowInsetsController.statusBarBackgroundColor = Kirigami.Theme.backgroundColor;
 *     WindowInsetsController.navigationBarBackgroundColor = Kirigami.Theme.backgroundColor;
 * }
 * \endcode
 *
 * \since 6.7
 *
 */

/*!
 * \qmlproperty color WindowInsetsController::statusBarBackgroundColor
 * Background color of the status bar.
 */

/*!
 * \qmlproperty color WindowInsetsController::navigationBarBackgroundColor
 * Background color of the navigation bar.
 */

/*!
 * \class KWindowInsetsController
 * \inmodule KGuiAddons
 * \brief Access to window insets colors.
 *
 * On most platforms this does nothing, on Android it allows
 * to customize the (top) status bar and (botton) navigation bar
 * background colors, e.g. to match the current window or application
 * colors.
 *
 * Note that the foreground colors on Android are automatically
 * chosen based on the background color.
 *
 * \code
 * Component.onComplete: {
 *     WindowInsetsController.statusBarBackgroundColor = Kirigami.Theme.backgroundColor;
 *     WindowInsetsController.navigationBarBackgroundColor = Kirigami.Theme.backgroundColor;
 * }
 * \endcode
 *
 * \since 6.7
 */
class KGUIADDONS_EXPORT KWindowInsetsController : public QObject
{
    Q_OBJECT

    /*!
     * \property KWindowInsetsController::statusBarBackgroundColor
     * Background color of the status bar.
     */
    Q_PROPERTY(QColor statusBarBackgroundColor READ statusBarBackgroundColor WRITE setStatusBarBackgroundColor NOTIFY statusBarBackgroundColorChanged)
    /*!
     * \property KWindowInsetsController::navigationBarBackgroundColor
     * Background color of the navigation bar.
     */
    Q_PROPERTY(
        QColor navigationBarBackgroundColor READ navigationBarBackgroundColor WRITE setNavigationBarBackgroundColor NOTIFY navigationBarBackgroundColorChanged)

public:
    explicit KWindowInsetsController(QObject *parent = nullptr);
    ~KWindowInsetsController();

    [[nodiscard]] QColor statusBarBackgroundColor() const;
    void setStatusBarBackgroundColor(const QColor &color);

    [[nodiscard]] QColor navigationBarBackgroundColor() const;
    void setNavigationBarBackgroundColor(const QColor &color);

Q_SIGNALS:
    void statusBarBackgroundColorChanged();
    void navigationBarBackgroundColorChanged();

private:
    std::unique_ptr<KWindowInsetsControllerPrivate> d;
};

#endif // KWINDOWINSETSCONTROLLER_H
