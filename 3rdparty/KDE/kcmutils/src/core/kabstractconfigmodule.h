/*
    SPDX-FileCopyrightText: 2022 Alexander Lohnau <alexander.lohnau@gmx.de>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KABSTRACTCONFIGMODULE_H
#define KABSTRACTCONFIGMODULE_H

#include "kcmutilscore_export.h"

#include <QObject>

#include <memory>

class KPluginMetaData;
class KAbstractConfigModulePrivate;

/*!
 * \class KAbstractConfigModule
 * \inmodule KCMUtilsCore
 * \brief Base class for QML and QWidgets config modules.
 *
 * \since 6.0
 */
class KCMUTILSCORE_EXPORT KAbstractConfigModule : public QObject
{
    Q_OBJECT

    /*! \property KAbstractConfigModule::buttons */
    Q_PROPERTY(KAbstractConfigModule::Buttons buttons READ buttons WRITE setButtons NOTIFY buttonsChanged)
    /*! \property KAbstractConfigModule::defaultsIndicatorsVisible */
    Q_PROPERTY(bool defaultsIndicatorsVisible READ defaultsIndicatorsVisible WRITE setDefaultsIndicatorsVisible NOTIFY defaultsIndicatorsVisibleChanged)
    /*! \property KAbstractConfigModule::needsAuthorization */
    Q_PROPERTY(bool needsAuthorization READ needsAuthorization NOTIFY authActionNameChanged)
    /*! \property KAbstractConfigModule::representsDefaults */
    Q_PROPERTY(bool representsDefaults READ representsDefaults WRITE setRepresentsDefaults NOTIFY representsDefaultsChanged)
    /*! \property KAbstractConfigModule::needsSave */
    Q_PROPERTY(bool needsSave READ needsSave WRITE setNeedsSave NOTIFY needsSaveChanged)
    /*! \property KAbstractConfigModule::name */
    Q_PROPERTY(QString name READ name CONSTANT)
    /*! \property KAbstractConfigModule::description */
    Q_PROPERTY(QString description READ description CONSTANT)
    /*!
     * \property KAbstractConfigModule::supportsInstantApply
     *
     * \since 6.16
     */
    Q_PROPERTY(bool supportsInstantApply READ supportsInstantApply WRITE setSupportsInstantApply NOTIFY supportsInstantApplyChanged)
public:
    /*!
     * \enum KAbstractConfigModule::Button
     * An enumeration type for the buttons used by this module.
     *
     * If Apply is not specified, kcmshell will show a "Close" button.
     *
     * \value NoAdditionalButton
     *        Shows the "Ok" and "Cancel" buttons.
     * \value Help
     *        Shows a "Help" button.
     * \value Default
     *        Shows a "Use Defaults" button.
     * \value Apply
     *        Shows the "Ok", "Apply", and "Cancel" buttons.
     * \value Export
     *        Obsolete.
     *
     * \sa buttons
     * \sa setButtons
     */
    enum Button {
        NoAdditionalButton = 0,
        Help = 1,
        Default = 2,
        Apply = 4,
        Export = 8,
    };
    Q_ENUM(Button)
    Q_DECLARE_FLAGS(Buttons, Button)
    Q_FLAG(Buttons)

    /*!
     *
     */
    explicit KAbstractConfigModule(QObject *parent, const KPluginMetaData &metaData);

    ~KAbstractConfigModule() override;

    /*!
     * \brief Set if the module's save() method requires authorization to be executed.
     *
     * It will still have to execute the action itself using the KAuth library, so
     * this method is not technically needed to perform the action, but
     * using this method will ensure that hosting
     * applications like System Settings or kcmshell behave correctly.
     *
     * \a action The action that will be used by this ConfigModule.
     */
    void setAuthActionName(const QString &action);

    /*!
     * \brief Returns the action previously set with setAuthActionName() and that is authorized to execute the save() method.
     *
     * By default this will be a empty string.
     */
    QString authActionName() const;

    /*!
     * \brief Emitted when the auth action name has changed.
     */
    Q_SIGNAL void authActionNameChanged();

    /*!
     * \brief Set this property to \c true when the user changes something in the module,
     * signaling that it \a needs a save (such as user pressing Ok or Apply).
     */
    void setNeedsSave(bool needs);

    /*!
     * \brief Returns \c true when the module has something changed and needs save.
     */
    bool needsSave() const;

    /*!
     * \brief Emitted when the state of the modules contents has changed.
     *
     * This signal is emitted whenever the state of the configuration
     * shown in the module changes. It allows the module container to
     * keep track of unsaved changes.
     */
    Q_SIGNAL void needsSaveChanged();

    /*!
     * \brief Set this property to \c true when the user sets the state of the module
     * to the \a defaults settings (e.g. clicking Defaults would do nothing).
     */
    void setRepresentsDefaults(bool defaults);

    /*!
     * \brief Returns \c true when the module state represents the default settings.
     */
    bool representsDefaults() const;

    /*!
     * \brief Emitted when the state of the modules contents has changed
     * in a way that it might represents the defaults settings, or
     * stopped representing them.
     */
    Q_SIGNAL void representsDefaultsChanged();

    /*!
     * \brief Sets the \a btn to display.
     * \sa buttons
     */
    void setButtons(const Buttons btn);

    /*!
     * \brief Indicate which buttons will be used.
     *
     * The return value is a value or'ed together from
     * the Button enumeration type.
     *
     * \sa setButtons
     */
    Buttons buttons() const;

    /*!
     * \brief Emitted when the buttons to display have changed.
     */
    Q_SIGNAL void buttonsChanged();

    /*!
     * \brief Returns \c true if the authActionName is not empty.
     * \sa setAuthActionName
     */
    bool needsAuthorization() const;

    /*!
     * \brief Returns the name of the config module.
     */
    QString name() const;

    /*!
     * \brief Returns the description of the config module.
     */
    QString description() const;

    /*!
     * \brief Changes the defaultness indicator visibility.
     *
     * \a visible Whether the indicator should be visible.
     */
    void setDefaultsIndicatorsVisible(bool visible);

    /*!
     * \brief Returns the defaultness indicator visibility.
     */
    bool defaultsIndicatorsVisible() const;

    /*!
     * \brief Emitted when the KCM needs to display indicators for field with non default value.
     */
    Q_SIGNAL void defaultsIndicatorsVisibleChanged();

    /*!
     * \brief Indicate whether a config module supports instant apply.
     *
     * \a supportes The config module supports instant apply. If false,
     * the settings application is exptected show Apply/OK buttons.
     *
     * The default is \c true.
     *
     * \since 6.16
     */
    void setSupportsInstantApply(bool supported);

    /*!
     * \brief Returns if the config module supports instant apply.
     *
     * Config modules that do not support instant apply only save their
     * settings once Apply/OK are clicked by the user. The settings
     * application is expected to show these buttons.
     *
     * The default is \c true.
     *
     * \since 6.16
     */
    bool supportsInstantApply() const;

    /*!
     * \brief Emitted when the KCM's supportsInstantApply property changes.
     *
     * \since 6.16
     */
    Q_SIGNAL void supportsInstantApplyChanged();


    /*!
     * \brief Returns the metaData that was used when instantiating the plugin.
     */
    KPluginMetaData metaData() const;

    /*!
     * \brief Emitted by a single-instance application (such as
     * System Settings) to request activation and update \a args to a module
     * that is already running.
     *
     * The module should connect to this signal when it needs to handle
     * the activation request and specially the new arguments.
     *
     * \a args A list of arguments that get passed to the module.
     */
    Q_SIGNAL void activationRequested(const QVariantList &args);

    /*!
     * \brief Loads the configuration data into the module.
     *
     * The load method sets the user interface elements of the
     * module to reflect the current settings stored in the
     * configuration files.
     *
     * This method is invoked whenever the module should read its configuration
     * (most of the times from a config file) and update the user interface.
     * This happens when the user clicks the "Reset" button in the control
     * center, to undo all of his changes and restore the currently valid
     * settings. It is also called right after construction.
     */
    virtual void load();

    /*!
     * \brief The save method stores the config information as shown
     * in the user interface in the config files.
     *
     * This method is called when the user clicks "Apply" or "Ok".
     */
    virtual void save();

    /*!
     * \brief Sets the configuration to default values.
     *
     * This method is called when the user clicks the "Default"
     * button.
     */
    virtual void defaults();

private:
    const std::unique_ptr<KAbstractConfigModulePrivate> d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KAbstractConfigModule::Buttons)

#endif // KABSTRACTCONFIGMODULE_H
