/*
 *  SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick
import org.kde.kirigami.platform as Platform
import org.kde.kirigami.templates as KT

KT.Heading {
    id: heading

    font.pointSize: {
      let factor = 1;
      switch (heading.level) {
        case 1:
          factor = 1.35;
          break;
        case 2:
          factor = 1.20;
          break;
        case 3:
          factor = 1.15;
          break;
        case 4:
          factor = 1.10;
          break;
        default:
          break;
      }
      return Platform.Theme.defaultFont.pointSize * factor;
    }

    font.weight: type === Heading.Type.Primary ? Font.DemiBold : Font.Normal

    opacity: type === Heading.Type.Secondary ? 0.75 : 1
}
