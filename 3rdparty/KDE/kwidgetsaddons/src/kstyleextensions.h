/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2014 Thomas Lübking <thomas.luebking@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KSTYLEEXTENSIONS_H
#define KSTYLEEXTENSIONS_H

#include <QStyle>
#include <kwidgetsaddons_export.h>

/*!
 * \namespace KStyleExtensions
 * \inmodule KWidgetsAddons
 *
 * \brief Runtime style extensions.
 *
 * You can use this to have a supporting QStyle implementation paint your widget
 * This is just convenience and does /not/ require the using widgets style to inherit KStyle (i.e.
 * calling this while using cleanlooks won't segfault but just return "0")
 *
 * For simplicity, only StyleHints, ControlElements and their SubElements are supported
 * If you don't need extended SubElement functionality, just skip its usage
 *
 * The element string has to be of the form: "appname.(2-char-element-type)_element"
 * The 2-char-element-type is the corresponding {SH, CE, SE}
 * Widgets in KWidgetsAddons don't have to pass the appname
 *
 * Examples: "CE_CapacityBar", "amarok.CE_Analyzer"
 *
 * Important notes:
 *
 * 1) If your string lacks the matching "SH_", "CE_" or "SE_" token the element request will be ignored (return is 0)
 *
 * 2) Try to avoid custom elements and use default ones (if possible) to get better style support and keep UI coherency
 *
 * 3) If you cache this value (good idea, this requires a map lookup) do not forget to catch style changes in QWidget::changeEvent()!
 */
namespace KStyleExtensions
{
/*!
 * Resolve a dynamic QStyle::ControlElement for eg. QStyle::drawControl()
 *
 * Returns a unique QStyle::ControlElement or 0 in case the style doesn't support this element
 *
 * \a element a valid element string appname.CE_element, eg. "amarok.CE_Analyzer"
 *
 * \a widget the widget to paint this element for. This parameter is mandatory, nullptr will return 0!
 *
 * \since 5.3
 */
KWIDGETSADDONS_EXPORT QStyle::ControlElement customControlElement(const QString &element, const QWidget *widget);
/*!
 * Resolve a dynamic QStyle::StyleHint to query QStyle::styleHint()
 *
 * Returns a unique QStyle::StyleHint or 0 in case the style doesn't support this element
 *
 * \a element a valid element string appname.SH_element, eg. "amarok.SH_Analyzer"
 *
 * \a widget the widget to paint this element for. This parameter is mandatory, nullptr will return 0!
 *
 * \since 5.3
 */
KWIDGETSADDONS_EXPORT QStyle::StyleHint customStyleHint(const QString &element, const QWidget *widget);
/*!
 * Resolve a dynamic QStyle::SubElement for eg. QStyle::subElementRect()
 *
 * Returns a unique QStyle::SubElement or 0 in case the style doesn't support this element
 *
 * \a element a valid element string appname.SE_element, eg. "amarok.SE_AnalyzerCanvas"
 *
 * \a widget the widget to paint this element for. This parameter is mandatory, nullptr will return 0!
 *
 * \since 5.3
 */
KWIDGETSADDONS_EXPORT QStyle::SubElement customSubElement(const QString &element, const QWidget *widget);
}

#endif // KSTYLEEXTENSIONS_H
