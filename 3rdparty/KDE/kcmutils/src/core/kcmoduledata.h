/*
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KCMODULEDATA_H
#define KCMODULEDATA_H

#include <QObject>
#include <QVariantList>
#include <kcmutilscore_export.h>
#include <memory>

class KCModuleDataPrivate;
class KCoreConfigSkeleton;

class QAction;

/*!
 * \class KCModuleData
 * \inmodule KCMUtilsCore
 * \brief A base class that offers information about a KCModule state.
 * \since 5.74
 */
class KCMUTILSCORE_EXPORT KCModuleData : public QObject
{
    Q_OBJECT
public:
    /*!
     *
     */
    explicit KCModuleData(QObject *parent = nullptr);
    ~KCModuleData() override;

    /*!
     * \brief Checks if the configuration is identical to the default one.
     *
     * Returns \c true if the module configuration is in the default state, \c false otherwise.
     */
    virtual bool isDefaults() const;

    /*!
     * \brief Reverts the module to default values and saves them.
     */
    virtual void revertToDefaults();

    /*!
     * \brief Returns whether this module matches a given \a query.
     *
     * The \a query is not expected to be a regex pattern but a full text search.
     */
    virtual bool matchesQuery(const QString &query) const;

    /*!
     * \brief An auxiliary action associated with this KCM.
     *
     * \since 6.18
     */
    QAction *auxiliaryAction() const;

    /**
     * \brief Whether this KCM is relevant in the current environment.
     *
     * The displaying application might not show this KCM if it is
     * not relevant. For example, a KCM for configuring touchscreens
     * could be hidden if no touchscreen is present.
     *
     * Default is true.
     *
     * \since 6.18
     */
    bool isRelevant() const;

Q_SIGNALS:
    /*!
     * \brief Emitted when KCModuleData is loaded.
     */
    void loaded();

    /*!
     * \internal
     * \brief Triggers the emit of \sa loaded() signal.
     *
     * This is the default behavior.
     *
     * To handle when loaded() is emitted in subclass,
     * disconnect this signal in the derived constructor.
     */
    void aboutToLoad(QPrivateSignal);

    /*!
     * \brief Emitted when the auxiliary action changes.
     *
     * \note The QAction can change its state (e.g. being enabled)
     * without the action object itself being replaced.
     *
     * \since 6.18
     */
    void auxiliaryActionChanged(QAction *action);

    /*!
     * \brief Emitted when the relevancy of this KCM changes.
     *
     * For example, after (un)plugging a relevant device.
     *
     * \since 6.18
     */
    void relevantChanged(bool relevant);

protected:
    /*!
     * \brief Sets an auxiliary action for this KCM.
     *
     * This can be displayed alongside this KCM in a list of settings
     * and provide quick access to a feature in the KCM without opening
     * it.
     *
     * For example, a "Pair new device" action or, in case of a checkable
     * action, a Switch to turn on/off a certain feature or device.
     */
    void setAuxiliaryAction(QAction *action);

    /*!
     * \brief Sets whether this KCM is relevant in the current environment.
     *
     * This can hide the KCM in a list of settings when it is currently
     * not relevant.
     *
     * For example to show touchscreen settings only when a touchscreen
     * is present. Default is true.
     */
    void setRelevant(bool relevant);

protected Q_SLOTS:
    /*!
     * \brief Allow to manually register a \a skeleton class.
     *
     * Used by a derived class when automatic discovery is not possible.
     */
    void registerSkeleton(KCoreConfigSkeleton *skeleton);

    /*!
     * \brief Automatically register child skeletons.
     *
     * Call it in your subclass constructor after skeleton creation
     */
    void autoRegisterSkeletons();

private:
    const std::unique_ptr<KCModuleDataPrivate> d;
    friend class KCModuleDataPrivate;
};

#endif
