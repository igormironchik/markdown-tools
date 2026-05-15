/*
    SPDX-FileCopyrightText: 2024 Lukas Sommer <sommerluk@gmail.com>
    SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KLOCALIZATION_H
#define KLOCALIZATION_H

#include "klocalizedstring.h"

#include <QObject>
#include <QVariant>

#include <type_traits>

class QDoubleSpinBox;
class QSpinBox;

/*!
 * \namespace KLocalization
 * \inmodule KI18n
 * \brief Namespace containing helpers for localization.
 * \since 6.5
 */
namespace KLocalization
{

namespace Private
{

constexpr inline const char SpinBoxFormatStringProperty[] = "__KLocalizationFormatStringPrivate";

}

/*!
 * Retranslates a previously set up format string to the current
 * language and updates the spin box.
 *
 * The format string is initially set up by setupSpinBoxFormatString().
 * This function updates the prefix and suffix of a spin box to reflect the
 * current language settings. It is useful for responding to language changes,
 * such as those triggered by QEvent::LanguageChange.
 *
 * \a T The type of the spin box, which must be either QSpinBox or
 * QDoubleSpinBox.
 *
 * \a spinBox Pointer to the spin box.
 *
 * The prefix and suffix of the spin box are updated to reflect the
 * current language.
 *
 * \sa setupSpinBoxFormatString
 *
 * \since 6.5
 */
template<typename T>
inline void retranslateSpinBoxFormatString(T *spinBox)
{
    constexpr bool isSpinBox = std::is_base_of_v<QSpinBox, T> || std::is_base_of_v<QDoubleSpinBox, T>;
    static_assert(isSpinBox, "First argument must be a QSpinBox or QDoubleSpinBox.");

    const auto lString = spinBox->property(Private::SpinBoxFormatStringProperty).template value<KLocalizedString>();
    // The KLocalizedString::subs() method performs two tasks:
    // 1. It replaces placeholders (%1, %2, ...) in the string with actual
    //    content.
    // 2. If the argument is an integer, it selects the appropriate plural form
    //    based on the value.
    // In this context, the string is expected not to contain any standard
    // placeholders (%1, %2, ...). Instead, it should contain a custom
    // placeholder (%v) which is ignored by KLocalizedString::subs().
    // The only purpose of calling KLocalizedString::subs() here is to ensure
    // the correct plural form is used when spinBox->value() is an integer.
    // If spinBox->value() is a double, KLocalizedString::subs() does not
    // perform any operations on the string since plural handling applies only
    // to integer values.
    const auto translation = lString.subs(spinBox->value()).toString();
    const auto parts = translation.split(QLatin1StringView("%v"));
    if (parts.count() == 2) {
        spinBox->setPrefix(parts.at(0));
        spinBox->setSuffix(parts.at(1));
    } else {
        spinBox->setPrefix(QString());
        spinBox->setSuffix(QString());
    }
}

/*!
 * Sets up a format string for internationalizing spin boxes.
 *
 * This function allows the customization of prefix and suffix for spin boxes
 * (QSpinBox and QDoubleSpinBox), considering internationalization needs.
 *
 * Spin boxes display a number and possibly a prefix and/or suffix. However,
 * in some languages, the position of the prefix and suffix may be reversed
 * compared to English. Example: In English, you write 50%, but in Turkish,
 * you write %50. Qt does not offer an out-of-the-box solution for this. This
 * helper now provides complete internationalization for prefixes and suffixes
 * of spin boxes.
 *
 * For QSpinBox it also provides correct plural handling by installing a
 * handler for the valueChanged() signal to update prefix and suffix whenever
 * the spin box value changes in the future.
 *
 * Example usage:
 * \code
 * QDoubleSpinBox doubleBox;
 * KLocalization::setupSpinBoxFormatString(
 *     &doubleBox,
 *     ki18nc("@item %v is a number and the second % is the percent sign", "%v%"));
 * // Turkish translation: "%%v"
 *
 * QSpinBox intBox;
 * KLocalization::setupSpinBoxFormatString(
 *     &intBox,
 *     ki18ncp("@item %v is a number", "Baking %v cake", "Baking %v cakes"));
 * \endcode
 *
 * \a T The type of the spin box, which must be either QSpinBox or QDoubleSpinBox.
 *
 * \a spinBox Pointer to the spin box.
 *
 * \a formatString A localized string in the format "PREFIX%vSUFFIX".
 * \list
 * \li For QDoubleSpinBox, plural forms in \a formatString are ignored
 *          and should be avoided. Use KLocalizedString::ki18nc "ki18nc()"
 *          to generate the format string.
 * \li For QSpinBox, if \a formatString includes plural forms, they are
 *          utilized. While optional, their use is highly recommended for
 *          accurate pluralization. Use KLocalizedString::ki18ncp "ki18ncp()"
 *          to generate the format string.
 * \endlist
 *
 * It is safe to call this function multiple times on the same spin box.
 *
 * \sa retranslateSpinBoxFormatString
 *
 * \since 6.5
 */
template<typename T>
inline void setupSpinBoxFormatString(T *spinBox, const KLocalizedString &formatString)
{
    constexpr bool isSpinBox = std::is_base_of_v<QSpinBox, T>;
    constexpr bool isDoubleSpinBox = std::is_base_of_v<QDoubleSpinBox, T>;
    static_assert(isSpinBox || isDoubleSpinBox, "First argument must be a QSpinBox or QDoubleSpinBox.");

    if constexpr (isSpinBox) {
        const bool hasSetup = !spinBox->property(Private::SpinBoxFormatStringProperty).isNull();
        if (!hasSetup) {
            QObject::connect(spinBox, &T::valueChanged, spinBox, [spinBox]() {
                retranslateSpinBoxFormatString(spinBox);
            });
        }
    }
    // Using relaxSubs() to avoid error marks if the library user did pass
    // a singular-only KLocalizedString.
    spinBox->setProperty(Private::SpinBoxFormatStringProperty, QVariant::fromValue(formatString.relaxSubs()));
    retranslateSpinBoxFormatString(spinBox);
}

}

#endif
