/*
    SPDX-FileCopyrightText: 2018 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

package org.kde.guiaddons;

import android.app.Activity;
import android.view.WindowManager;

public class KSystemInhibitor
{
    public static void setLockInhibitionOn(Activity activity) {
        activity.runOnUiThread(() -> activity.getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON));
    }

    public static void setLockInhibitionOff(Activity activity) {
        activity.runOnUiThread(() -> activity.getWindow().clearFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON));
    }
}
