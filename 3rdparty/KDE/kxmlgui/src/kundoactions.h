/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2006 Peter Simonsson <peter.simonsson@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KUNDOACTIONS_H
#define KUNDOACTIONS_H

#include <kxmlgui_export.h>

#include <QString>

class KActionCollection;
class QAction;
class QUndoStack;

/*!
 * \brief Provides functions that creates undo/redo actions for a QUndoStack with KDE's default icons and shortcuts.
 *
 * \namespace KUndoActions
 * \inmodule KXmlGui
 *
 * See QUndoStack for more information.
 *
 * \since 5.0
 */
namespace KUndoActions
{
/*!
 * \brief Creates an redo action with the default shortcut and icon
 * and adds it to \a actionCollection.
 *
 * \a undoStack the QUndoStack the action triggers the redo on
 *
 * \a actionCollection the KActionCollection that should be the parent of the action
 *
 * \a actionName the created action's object name, empty string will set it to the KDE default
 *
 * Returns the created action.
 */
KXMLGUI_EXPORT QAction *createRedoAction(QUndoStack *undoStack, KActionCollection *actionCollection, const QString &actionName = QString());

/*!
 * \brief Creates an undo action with the default KDE shortcut and icon
 * and adds it to \a actionCollection.
 *
 * Returns the created action.
 *
 * \a undoStack The QUndoStack the action triggers the undo on.
 *
 * \a actionCollection The KActionCollection that should be the parent of the action.
 *
 * \a actionName The created action's object name, empty string
 * will set it to the KDE default.
 */
KXMLGUI_EXPORT QAction *createUndoAction(QUndoStack *undoStack, KActionCollection *actionCollection, const QString &actionName = QString());
}

#endif
