/*
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPLUGINWIDGET_H
#define KPLUGINWIDGET_H

#include <QList>
#include <QWidget>

#include <KPluginMetaData>
#include <KSharedConfig>
#include <kcmutils_export.h>

#include <memory>

class QPushButton;
class KPluginWidgetPrivate;

/*!
 * \class KPluginWidget
 * \inmodule KCMUtils
 *
 * \brief A widget that shows a list of available plugins
 * and allows to disable/enable them and open their configuration UI.
 *
 * Plugins that get added to the KPluginWidget need to define the \c X-KDE-ConfigModule property.
 * The value for this property is the namespace and file name of the KCM for the plugin.
 *
 * An example value is "kf6/krunner/kcms/kcm_krunner_charrunner",
 * "kf6/krunner/kcms" is the namespace and "kcm_krunner_charrunner"
 * the file name. The loaded KCMs don't need any embedded JSON metadata.
 *
 * \since 5.89
 */
class KCMUTILS_EXPORT KPluginWidget : public QWidget
{
    Q_OBJECT

public:
    /*!
     *
     */
    explicit KPluginWidget(QWidget *parent = nullptr);

    ~KPluginWidget();

    /*!
     * \brief Adds the \a plugins with the given \a categoryLabel to the widget.
     */
    void addPlugins(const QList<KPluginMetaData> &plugins, const QString &categoryLabel);

    /*!
     * \brief Sets the \a config object that will be used to store the enabled state of the plugins.
     *
     * When porting away from KPluginSelector, the "Plugins" group
     * from the config root should be used. For example:
     * \code
     * KSharedConfig::openConfig(QStringLiteral("krunnerrc"))->group("Plugins")
     * \endcode
     */
    void setConfig(const KConfigGroup &config);

    /*!
     * \brief Clears all the added plugins and any unsaved changes.
     */
    void clear();

    /*!
     * \brief Saves the changes to the config set by \l setConfig.
     */
    void save();

    /*!
     * \brief Loads the enabled state of the plugins from the config set
     * by setConfig() and clears any changes by the user.
     * \since 5.91
     */
    void load();

    /*!
     * \brief Resets the enabled state of the plugins to their defaults.
     * \sa KPluginMetaData::isEnabledByDefault
     */
    void defaults();

    /*!
     * \brief Returns \c true if the enabled state of each plugin is the same as that plugin's default state.
     */
    bool isDefault() const;

    /*!
     * \brief Returns true if the plugin selector has any changes that are not yet saved to configuration.
     * \sa save()
     */
    bool isSaveNeeded() const;

    /*!
     * \brief Sets the \a arguments with which the configuration modules will be initialized.
     */
    void setConfigurationArguments(const QVariantList &arguments);

    /*!
     * \brief Returns the configuration arguments that will be used.
     */
    QVariantList configurationArguments() const;

    /*!
     * \brief Shows the configuration dialog for the plugin \a pluginId if it's available.
     */
    void showConfiguration(const QString &pluginId);

    /*!
     * \brief Shows an indicator when a plugin status is different from default.
     *
     * \a isVisible Whether the indicator is visible.
     */
    void setDefaultsIndicatorsVisible(bool isVisible);

    /*!
     * \brief Add additional widgets to each row of the plugin selector.
     *
     * \a handler Returns the additional button that should be displayed in the row;
     * the handler can return a null pointer if no button should be displayed.
     */
    void setAdditionalButtonHandler(const std::function<QPushButton *(const KPluginMetaData &)> &handler);

Q_SIGNALS:
    /*!
     * \brief Emitted when any of the plugins are changed.
     *
     * \a pluginId The ID of the changed plugin.
     *
     * \a enabled Whether the given plugin is currently enabled.
     */
    void pluginEnabledChanged(const QString &pluginId, bool enabled);

    /*!
     * \brief Emitted when any of the plugins are changed.
     *
     * \a enabled Whether the given plugin is enabled.
     *
     * \sa isSaveNeeded()
     */
    void changed(bool enabled);

    /*!
     * \brief Emitted after the config of an embedded KCM has been saved.
     *
     * The \a pluginId is the name of the parent component
     * that needs to reload its config.
     */
    void pluginConfigSaved(const QString &pluginId);

    /*!
     * \brief Emitted after configuration is changed.
     *
     * \a isDefault Returns whether the configuration state is the default.
     */
    void defaulted(bool isDefault);

private:
    std::unique_ptr<KPluginWidgetPrivate> const d;
};

#endif
