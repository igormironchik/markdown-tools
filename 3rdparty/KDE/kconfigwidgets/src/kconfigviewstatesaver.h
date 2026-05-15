
#ifndef KCONFIGVIEWSTATESAVER_H
#define KCONFIGVIEWSTATESAVER_H

#include <KViewStateSerializer>

#include "kconfigwidgets_export.h"

class KConfigGroup;

/*!
 * \class KConfigViewStateSaver
 * \inmodule KConfigWidgets
 *
 * \brief Base class for saving and restoring state in QTreeViews and QItemSelectionModels using KConfig as storage.
 */
class KCONFIGWIDGETS_EXPORT KConfigViewStateSaver : public KViewStateSerializer
{
    Q_OBJECT
public:
    explicit KConfigViewStateSaver(QObject *parent = nullptr);

    /*!
      Saves the state to the \a configGroup
    */
    void saveState(KConfigGroup &configGroup);

    /*!
      Restores the state from the \a configGroup
    */
    void restoreState(const KConfigGroup &configGroup);
};

#endif
