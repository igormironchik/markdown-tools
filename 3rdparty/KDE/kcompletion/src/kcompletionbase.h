/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999, 2000 Carsten Pfeiffer <pfeiffer@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCOMPLETIONBASE_H
#define KCOMPLETIONBASE_H

#include <kcompletion.h>
#include <kcompletion_export.h>

#include <QMap>
#include <memory>

class KCompletionBasePrivate;

/*!
 * \class KCompletionBase
 * \inmodule KCompletion
 *
 * \brief An abstract base class for adding a completion feature
 * into widgets.
 *
 * This is a convenience class that provides the basic functions
 * needed to add text completion support into widgets. All that
 * is required is an implementation for the pure virtual function
 * setCompletedText(). Refer to KLineEdit or KComboBox
 * to see how easily such support can be added using this as a base
 * class.
 */

class KCOMPLETION_EXPORT KCompletionBase
{
public:
    Q_DECLARE_PRIVATE(KCompletionBase)
    /*!
     * Constants that represent the items whose shortcut
     * key binding is programmable. The default key bindings
     * for these items are defined in KStandardShortcut.
     *
     * \value TextCompletion Text completion (by default Ctrl-E).
     * \value PrevCompletionMatch Switch to previous completion (by default Ctrl-Up).
     * \value NextCompletionMatch Switch to next completion (by default Ctrl-Down).
     * \value SubstringCompletion Substring completion (by default Ctrl-T).
     */
    enum KeyBindingType {
        TextCompletion,
        PrevCompletionMatch,
        NextCompletionMatch,
        SubstringCompletion,
    };

    /*!
     * \typedef KCompletionBase::KeyBindingMap
     * Map for the key binding types mentioned above.
     */
    typedef QMap<KeyBindingType, QList<QKeySequence>> KeyBindingMap;

    /*!
     * Default constructor.
     */
    KCompletionBase();

    virtual ~KCompletionBase();

    /*!
     * Returns a pointer to the current completion object.
     *
     * If the completion object does not exist, it is automatically created and
     * by default handles all the completion signals internally unless handleSignals
     * is set to \c false. It is also automatically destroyed when the destructor
     * is called. You can change this default behavior using the
     * setAutoDeleteCompletionObject and setHandleSignals member
     * functions.
     *
     * See also compObj.
     *
     * \a handleSignals if true, handles completion signals internally.
     */
    KCompletion *completionObject(bool handleSignals = true);

    /*!
     * Sets up the completion object to be used.
     *
     * This method assigns the completion object and sets it up to automatically
     * handle the completion and rotation signals internally. You should use
     * this function if you want to share one completion object among your
     * widgets or need to use a customized completion object.
     *
     * The object assigned through this method is not deleted when this object's
     * destructor is invoked unless you explicitly call setAutoDeleteCompletionObject
     * after calling this method. Be sure to set the bool argument to false, if
     * you want to handle the completion signals yourself.
     *
     * \a completionObject a KCompletion or a derived child object.
     *
     * \a handleCompletionSignals if \c true, handles completion signals internally.
     */
    virtual void setCompletionObject(KCompletion *completionObject, bool handleSignals = true);

    /*!
     * Enables this object to handle completion and rotation
     * events internally.
     *
     * This function simply assigns a boolean value that
     * indicates whether it should handle rotation and
     * completion events or not. Note that this does not
     * stop the object from emitting signals when these
     * events occur.
     *
     * \a handle if true, it handles completion and rotation internally.
     */
    virtual void setHandleSignals(bool handle);

    /*!
     * Returns true if the completion object is deleted
     * upon this widget's destruction.
     *
     * See setCompletionObject() and enableCompletion()
     * for details.
     *
     * Returns \c true if the completion object will be deleted
     *              automatically
     */
    bool isCompletionObjectAutoDeleted() const;

    /*!
     * Sets the completion object when this widget's destructor
     * is called.
     *
     * If the argument is set to \c true, the completion object
     * is deleted when this widget's destructor is called.
     *
     * \a autoDelete if \c true, delete completion object on destruction.
     */
    void setAutoDeleteCompletionObject(bool autoDelete);

    /*!
     * Sets the widget's ability to emit text completion and
     * rotation signals.
     *
     * Invoking this function with \a enable set to \c false will
     * cause the completion and rotation signals not to be emitted.
     * However, unlike setting the completion object to \c nullptr
     * using setCompletionObject, disabling the emission of
     * the signals through this method does not affect the current
     * completion object.
     *
     * There is no need to invoke this function by default. When a
     * completion object is created through completionObject or
     * setCompletionObject, these signals are set to emit
     * automatically. Also note that disabling this signals will not
     * necessarily interfere with the objects' ability to handle these
     * events internally. See setHandleSignals.
     *
     * \a enable if false, disables the emission of completion and rotation signals.
     */
    void setEnableSignals(bool enable);

    /*!
     * Returns \c true if the object handles the signals.
     */
    bool handleSignals() const;

    /*!
     * Returns \c true if the object emits the signals.
     */
    bool emitSignals() const;

    /*!
     * Sets whether the object emits rotation signals.
     *
     * \a emitRotationSignals if \c false, disables the emission of rotation signals.
     */
    void setEmitSignals(bool emitRotationSignals);

    /*!
     * Sets the type of completion to be used.
     *
     * \a mode Completion type
     */
    virtual void setCompletionMode(KCompletion::CompletionMode mode);

    /*!
     * Returns the current completion mode.
     */
    KCompletion::CompletionMode completionMode() const;

    /*!
     * Sets the key binding to be used for manual text
     * completion, text rotation in a history list as
     * well as a completion list.
     *
     *
     * When the keys set by this function are pressed, a
     * signal defined by the inheriting widget will be activated.
     * If the default value or 0 is specified by the second
     * parameter, then the key binding as defined in the global
     * setting should be used. This method returns false
     * when \a key is negative or the supplied key binding conflicts
     * with another one set for another feature.
     *
     * \note To use a modifier key (Shift, Ctrl, Alt) as part of
     * the key binding simply \a sum up the values of the
     * modifier and the actual key. For example, to use CTRL+E, supply
     * \c {"Qt::CtrlButton | Qt::Key_E"} as the second argument to this
     * function.
     *
     * \a item the feature whose key binding needs to be set:
     * \list
     * \li TextCompletion the manual completion key binding.
     * \li PrevCompletionMatch the previous match key for multiple completion.
     * \li NextCompletionMatch the next match key for for multiple completion.
     * \li SubstringCompletion the key for substring completion
     * \endlist
     *
     * \a key key binding used to rotate down in a list.
     *
     * Returns \c true if key binding is successfully set.
     * \sa keyBinding
     */
    bool setKeyBinding(KeyBindingType item, const QList<QKeySequence> &key);

    /*!
     * Returns the key binding used for the specified item.
     *
     * This method returns the key binding used to activate
     * the feature given by \a item. If the binding
     * contains modifier key(s), the sum of the modifier key
     * and the actual key code is returned.
     *
     * \a item the item to check
     *
     * Returns the key binding used for the feature given by \a item.
     *
     * \sa setKeyBinding
     *
     * \since 5.0
     */
    QList<QKeySequence> keyBinding(KeyBindingType item) const;

    /*!
     * Sets this object to use global values for key bindings.
     *
     * This method changes the values of the key bindings for
     * rotation and completion features to the default values
     * provided in KStandardShortcut.
     *
     * \note By default, inheriting widgets should use the
     * global key bindings so that there is no need to
     * call this method.
     */
    void useGlobalKeyBindings();

    /*!
     * A pure virtual function that must be implemented by
     * all inheriting classes.
     *
     * This function is intended to allow external completion
     * implementations to set completed text appropriately. It
     * is mostly relevant when the completion mode is set to
     * CompletionAuto and CompletionManual modes. See
     * KCompletionBase::setCompletedText.
     * Does nothing in CompletionPopup mode, as all available
     * matches will be shown in the popup.
     *
     * \a text the completed text to be set in the widget.
     */
    virtual void setCompletedText(const QString &text) = 0;

    /*!
     * A pure virtual function that must be implemented by
     * all inheriting classes.
     *
     * \a items the list of completed items
     *
     * \a autoSuggest if \c true, the first element of \a items
     *        is automatically completed (i.e. preselected).
     */
    virtual void setCompletedItems(const QStringList &items, bool autoSuggest = true) = 0;

    /*!
     * Returns a pointer to the completion object.
     *
     * This method is only different from completionObject()
     * in that it does not create a new KCompletion object even if
     * the internal pointer is \c nullptr. Use this method to get the
     * pointer to a completion object when inheriting so that you
     * will not inadvertently create it.
     *
     * Returns the completion object or \c nullptr if one does not exist.
     */
    KCompletion *compObj() const;

protected:
    /*!
     * Returns a key binding map.
     *
     * This method is the same as getKeyBinding(), except that it
     * returns the whole keymap containing the key bindings.
     *
     * Returns the key binding used for the feature given by \a item.
     * \since 5.0
     */
    KeyBindingMap keyBindingMap() const;

    /*!
     * Sets the keymap.
     *
     * \a keyBindingMap
     */
    void setKeyBindingMap(KeyBindingMap keyBindingMap);

    /*!
     * Sets or removes the delegation object. If a delegation object is
     * set, all function calls will be forwarded to the delegation object.
     * \a delegate the delegation object, or \c nullptr to remove it
     */
    void setDelegate(KCompletionBase *delegate);

    /*!
     * Returns the delegation object, or \c nullptr if there is none
     * \sa setDelegate()
     */
    KCompletionBase *delegate() const;

    virtual void virtual_hook(int id, void *data);

private:
    Q_DISABLE_COPY(KCompletionBase)
    std::unique_ptr<KCompletionBasePrivate> const d_ptr;
};

#endif // KCOMPLETIONBASE_H
