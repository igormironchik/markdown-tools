/*
 *  SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
 *  SPDX-FileCopyrightText: 2016 Aleix Pol Gonzalez <aleixpol@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick

import org.kde.kirigami.platform as Platform
import org.kde.kirigami.primitives


/*!
  \qmltype Separator
  \inqmlmodule org.kde.kirigami.primitives

  \brief A visual separator.

  Useful for splitting one set of items from another.

 */
Item {
    id: root
    implicitWidth: 1
    implicitHeight: 1
    Accessible.role: Accessible.Separator
    Accessible.focusable: false

    enum Weight {
        Light,
        Normal
    }

    /*!
      \brief This property holds the visual weight of the separator.

      Weight options:
      \list
      \li Kirigami.Separator.Weight.Light
      \li Kirigami.Separator.Weight.Normal
      \endlist

      default: Kirigami.Separator.Weight.Normal

      \since 5.72
     */
    property int weight: Separator.Weight.Normal


    property alias color: internal.color

    // The separator is drawn by an internal rectangle, which gets moved and resized
    // in a way to align exactly to the pixel grid in fractional scaling
    Rectangle {
        id: internal

        AlignedSize.width: root.width
        AlignedSize.height: root.height

        /* TODO: If we get a separator color role, change this to
         * mix weights lower than Normal with the background color
         * and mix weights higher than Normal with the text color.
         */
        color: Platform.ColorUtils.linearInterpolation(
                Platform.Theme.backgroundColor,
                Platform.Theme.textColor,
                root.weight === Separator.Weight.Light ? Platform.Theme.lightFrameContrast : Platform.Theme.frameContrast
        )
    }
}
