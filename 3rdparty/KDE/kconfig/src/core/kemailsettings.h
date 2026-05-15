/*
    SPDX-FileCopyrightText: 2000 Alex Zepeda <zipzippy@sonic.net>

    SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef _KEMAILSETTINGS_H
#define _KEMAILSETTINGS_H

#include <QCoreApplication> // Q_DECLARE_TR_FUNCTIONS
#include <QStringList>

#include <kconfigcore_export.h>

class KEMailSettingsPrivate;

/*!
 * \class KEMailSettings
 * \inmodule KConfigCore
 *
 * \brief Access to e-mail settings.
 *
 * Facilitates accessing e-mail settings
 * and allows any program to manage multiple e-mail profiles.
 *
 * The default profile is automatically selected in the constructor.
 **/
class KCONFIGCORE_EXPORT KEMailSettings
{
    Q_DECLARE_TR_FUNCTIONS(KEMailSettings)
public:
    /*!
     * \value ClientProgram
     * \value ClientTerminal
     * \value RealName
     * \value EmailAddress
     * \value ReplyToAddress
     * \value Organization
     * \sa getSetting()
     * \sa setSetting()
     **/
    enum Setting {
        ClientProgram,
        ClientTerminal,
        RealName,
        EmailAddress,
        ReplyToAddress,
        Organization,
    };

    /*!
     * Constructs a new KEMailSettings with the default profile.
     **/
    KEMailSettings();

    KEMailSettings(const KEMailSettings &) = delete;
    KEMailSettings &operator=(const KEMailSettings &) = delete;

    ~KEMailSettings();

    /*!
     * Returns the list of profiles available.
     **/
    QStringList profiles() const;

    /*!
     * Sets the current profile to the new name \a s.
     **/
    void setProfile(const QString &s);

    /*!
     * Returns the name of the default profile.
     **/
    QString defaultProfileName() const;

    /*!
     * Sets a new default profile \a def.
     **/
    void setDefault(const QString &def);

    /*!
     * Returns a setting \a s from the predefined "basic" settings, or QString() if unset.
     * \sa Setting
     **/
    QString getSetting(KEMailSettings::Setting s) const;

    /*!
     * Sets the setting \a s from the predefined "basic" settings to the new value \a v.
     * \sa Setting
     **/
    void setSetting(KEMailSettings::Setting s, const QString &v);

private:
    KEMailSettingsPrivate *const p;
};

#endif
