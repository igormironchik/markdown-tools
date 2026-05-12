/*
    SPDX-FileCopyrightText: 2024 Mathis Brüchert <mbb@kaidan.im>
    SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

package org.kde.guiaddons;

import android.app.Activity;
import android.os.Build;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.WindowInsetsController;

public class KWindowInsetsController
{
    public static void setStatusBarBackground(android.app.Activity activity, int color)
    {
        Window window = activity.getWindow();
        window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
        window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
        window.setStatusBarColor(color);

        if (Build.VERSION.SDK_INT >= 30) {
            if (isDark(color)) {
                window.getInsetsController().setSystemBarsAppearance(0, WindowInsetsController.APPEARANCE_LIGHT_STATUS_BARS);
            } else {
                window.getInsetsController().setSystemBarsAppearance(WindowInsetsController.APPEARANCE_LIGHT_STATUS_BARS, WindowInsetsController.APPEARANCE_LIGHT_STATUS_BARS);
            }
        } else {
            int visibility = window.getDecorView().getSystemUiVisibility();
            if (isDark(color)) {
                visibility &= ~View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR;
            } else {
                visibility |= View.SYSTEM_UI_FLAG_LIGHT_STATUS_BAR;
            }
            window.getDecorView().setSystemUiVisibility(visibility);
        }
    }

    public static void setNavigationBarBackground(android.app.Activity activity, int color)
    {
        Window window = activity.getWindow();
        window.clearFlags(WindowManager.LayoutParams.FLAG_TRANSLUCENT_STATUS);
        window.addFlags(WindowManager.LayoutParams.FLAG_DRAWS_SYSTEM_BAR_BACKGROUNDS);
        window.setNavigationBarColor(color);

        if (Build.VERSION.SDK_INT >= 30) {
            if (isDark(color)) {
                window.getInsetsController().setSystemBarsAppearance(0, WindowInsetsController.APPEARANCE_LIGHT_NAVIGATION_BARS);
            } else {
                window.getInsetsController().setSystemBarsAppearance(WindowInsetsController.APPEARANCE_LIGHT_NAVIGATION_BARS, WindowInsetsController.APPEARANCE_LIGHT_NAVIGATION_BARS);
            }
        } else {
            int visibility = window.getDecorView().getSystemUiVisibility();
            if (isDark(color)) {
                visibility &= ~View.SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR;
            } else {
                visibility |= View.SYSTEM_UI_FLAG_LIGHT_NAVIGATION_BAR;
            }
            window.getDecorView().setSystemUiVisibility(visibility);
        }
    }

    private static double luma(int color)
    {
        return (0.299 * (color >> 24) + 0.587 * ((color >> 16) & 0xff) + 0.114 * ((color >> 8) & 0xff)) / 255.0;
    }

    private static boolean isDark(int color)
    {
        return luma(color) <= 0.5;
    }
}
