/*
    SPDX-FileCopyrightText: 2021 Nicolas Fella <nicolas.fella@gmx.de>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPLUGINMODEL_H
#define KPLUGINMODEL_H

#include "kcmutilscore_export.h"

#include <QAbstractListModel>
#include <QList>

#include <KPluginMetaData>
#include <memory>

class KConfigGroup;
class KPluginModelPrivate;

/*!
 * \class KPluginModel
 * \inmodule KCMUtilsCore
 * \brief A model that provides a list of available plugins and allows to disable/enable them.
 *
 * Plugins need to define the \c X-KDE-ConfigModule property for their config modules to be found.
 * The value for this property is the namespace and file name of the KCM for the plugin.
 *
 * An example value is "kf6/krunner/kcms/kcm_krunner_charrunner", "kf6/krunner/kcms" is the namespace
 * and "kcm_krunner_charrunner" the file name. The loaded KCMs don't need any embedded JSON metadata.
 *
 * \sa KPluginWidget
 *
 * \since 5.94
 */
class KCMUTILSCORE_EXPORT KPluginModel : public QAbstractListModel
{
    Q_OBJECT

public:
    /*!
     * \enum KPluginModel::Roles
     *
     * \value NameRole
     * \value IconRole
     * \value EnabledRole
     * \value DescriptionRole
     * \value IsChangeableRole
     * \value MetaDataRole
     * \value ConfigRole
     * \value IdRole
     * \value EnabledByDefaultRole
     * \value SortableRole
     */
    enum Roles {
        NameRole = Qt::DisplayRole,
        IconRole = Qt::DecorationRole,
        EnabledRole = Qt::CheckStateRole,
        DescriptionRole = Qt::UserRole + 1,
        IsChangeableRole,
        MetaDataRole,
        ConfigRole,
        IdRole,
        EnabledByDefaultRole,
        SortableRole,
    };

    /*!
     *
     */
    explicit KPluginModel(QObject *parent = nullptr);
    ~KPluginModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count, const QModelIndex &destinationParent, int destinationChild) override;

    /*!
     * \brief Appends \a plugins to the model.
     *
     * This will not replace existing entries.
     *
     * \a plugins The plugins to be added.
     *
     * \a categoryLabel A user-visible label for the section the plugins are added to.
     */
    void addPlugins(const QList<KPluginMetaData> &plugins, const QString &categoryLabel);

    /*!
     * \brief Adds \a plugins that should not be sorted automatically based on their name.
     *
     * This is useful in case your app has a custom sorting mechanism or implements reordering of plugins.
     *
     * \a plugins The plugins to be added.
     *
     * \a categoryLabel A user-visible label for the section the plugins are added to.
     *
     * \since 6.0
     */
    void addUnsortablePlugins(const QList<KPluginMetaData> &plugins, const QString &categoryLabel);

    /*!
     * \brief Removes a plugin with the specified \a data.
     * \since 6.0
     */
    void removePlugin(const KPluginMetaData &data);

    /*!
     * \brief Removes all plugins.
     */
    void clear();

    /*!
     * \brief Sets the KConfigGroup \a config that is used
     * to load/save the enabled state.
     */
    void setConfig(const KConfigGroup &config);

    /*!
     * \brief Saves the enabled state of the plugins to the config group set by \l setConfig.
     */
    void save();

    /*!
     * \brief Loads the enabled state of the plugins from the config group set by \l setConfig.
     */
    void load();

    /*!
     * \brief Resets the enabled state of the plugins to its defaults.
     */
    void defaults();

    /*!
     * \brief Returns whether there are unsaved changes to the enabled state of the plugins.
     */
    bool isSaveNeeded();

    /*!
     * \brief Returns the KPluginMetaData object of the plugin's config module with the specified \a pluginId.
     *
     * If no plugin is found or the plugin does not have a config, the resulting
     * KPluginMetaData object will be invalid.
     * \since 5.94
     */
    KPluginMetaData findConfigForPluginId(const QString &pluginId) const;

    /*!
     * \brief Emitted when the enabled state \a isDefaulted, that is, it matches the default changes.
     * \sa defaults
     */
    Q_SIGNAL void defaulted(bool isDefaulted);

    /*!
     * \brief Emitted when isSaveNeeded is changed.
     */
    Q_SIGNAL void isSaveNeededChanged();

private:
    const std::unique_ptr<KPluginModelPrivate> d;
    friend class KPluginProxyModel;
    QStringList getOrderedCategoryLabels();
};
#endif
