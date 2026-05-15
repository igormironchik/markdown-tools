/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: Mark Donohoe <donohoe@kde.org>
    SPDX-FileCopyrightText: 2001, 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KKEYSEQUENCEWIDGET_H
#define KKEYSEQUENCEWIDGET_H

#include <kxmlgui_export.h>

#include <KKeySequenceRecorder>

#include <QList>
#include <QPushButton>

class KKeySequenceWidgetPrivate;
class QAction;
class KActionCollection;

/*!
 * \class KKeySequenceWidget
 * \inmodule KXmlGui
 *
 * \brief A widget to input a QKeySequence.
 *
 * This widget lets the user choose a QKeySequence, which is usually used as a
 * shortcut key. The recording is initiated by calling captureKeySequence() or
 * by the user clicking on the widget.
 *
 * The widgets provides support for conflict handling.
 *
 * \image kkeysequencewidget.png "KKeySequenceWidget"
 *
 * \sa setCheckForConflictsAgainst()
 */
class KXMLGUI_EXPORT KKeySequenceWidget : public QWidget
{
    Q_OBJECT

    /*!
     * \property KKeySequenceWidget::keySequence
     */
    Q_PROPERTY(QKeySequence keySequence READ keySequence WRITE setKeySequence NOTIFY keySequenceChanged)

    /*!
     * \property KKeySequenceWidget::multiKeyShortcutsAllowed
     */
    Q_PROPERTY(bool multiKeyShortcutsAllowed READ multiKeyShortcutsAllowed WRITE setMultiKeyShortcutsAllowed)

    /*!
     * \property KKeySequenceWidget::checkForConflictsAgainst
     */
    Q_PROPERTY(ShortcutTypes checkForConflictsAgainst READ checkForConflictsAgainst WRITE setCheckForConflictsAgainst)

    /*!
     * \property KKeySequenceWidget::modifierlessAllowed
     * \deprecated[6.12]
     * Use the patterns property
     */
    Q_PROPERTY(bool modifierlessAllowed READ isModifierlessAllowed WRITE setModifierlessAllowed)

    /*!
     * \property KKeySequenceWidget::modifierOnlyAllowed
     */
    Q_PROPERTY(bool modifierOnlyAllowed READ modifierOnlyAllowed WRITE setModifierOnlyAllowed)

    /*!
     * \property KKeySequenceWidget::recording
     * Indicates whether a key sequence is currently being recorded.
     *
     * \since 6.12
     */
    Q_PROPERTY(bool recording READ isRecording NOTIFY recordingChanged)

    /*!
     * \property KKeySequenceWidget::patterns
     * Specifies the accepted shortcut formats.
     *
     * Default is KKeySequenceRecorder::ModifierAndKey.
     *
     * \since 6.12
     */
    Q_PROPERTY(KKeySequenceRecorder::Patterns patterns READ patterns WRITE setPatterns)

public:
    /*!
     * \enum KKeySequenceWidget::Validation
     *
     * An enum about validation when setting a key sequence.
     *
     * \sa setKeySequence()
     *
     * \value Validate
     *        Validate key sequence.
     * \value NoValidate
     *        Use key sequence without validation.
     */
    enum Validation {
        Validate = 0,
        NoValidate = 1,
    };

    /*!
     * \brief Constructs a widget used to input a QKeySequence.
     */
    explicit KKeySequenceWidget(QWidget *parent = nullptr);

    ~KKeySequenceWidget() override;

    /*!
     * \enum KKeySequenceWidget::ShortcutType
     *
     * \value None
     *        No checking for conflicts.
     * \value LocalShortcuts
     *        Check with local shortcuts.
     *        \sa setCheckActionCollections()
     * \value StandardShortcuts
     *        Check against standard shortcuts.
     *        \sa KStandardShortcut
     * \value GlobalShortcuts
     *        Check against global shortcuts.
     *        \sa KGlobalAccel
     */
    enum ShortcutType {
        None = 0x00,
        LocalShortcuts = 0x01,
        StandardShortcuts = 0x02,
        GlobalShortcuts = 0x04,
    };
    Q_DECLARE_FLAGS(ShortcutTypes, ShortcutType)
    Q_FLAG(ShortcutTypes)

    /*!
     * \brief Configure if the widget should check for conflicts with existing
     * shortcut \a types.
     *
     * When capturing a key sequence for local shortcuts you should check
     * against GlobalShortcuts and your other local shortcuts. This is the
     * default.
     *
     * You have to provide the local actions to check against with
     * setCheckActionCollections().
     *
     * When capturing a key sequence for a global shortcut you should
     * check against StandardShortcuts, GlobalShortcuts and your local
     * shortcuts.
     *
     * There are two ways to react to a user agreeing to steal a shortcut:
     *
     * 1. Listen to the stealShortcut() signal and steal the shortcuts
     * manually. It's your responsibility to save that change later when
     * you think it is appropriate.
     *
     * 2. Call applyStealShortcut() and KKeySequenceWidget will steal the
     * shortcut. This will save the actionCollections the shortcut is part
     * of so make sure it doesn't inadvertly save some unwanted changes
     * too. Read its documentation for some limitation when handling
     * global shortcuts.
     *
     * If you want to do the conflict checking yourself here are some code
     * snippets for global ...
     *
     * \code
     * QStringList conflicting = KGlobalAccel::findActionNameSystemwide(keySequence);
     * if (!conflicting.isEmpty()) {
     *     // Inform and ask the user about the conflict and reassigning
     *     // the keys sequence
     *     if (!KGlobalAccel::promptStealShortcutSystemwide(q, conflicting, keySequence)) {
     *         return true;
     *     }
     *     KGlobalAccel::stealShortcutSystemwide(keySequence);
     * }
     * \endcode
     *
     * ...  and standard shortcuts
     *
     * \code
     * KStandardShortcut::StandardShortcut ssc = KStandardShortcut::find(keySequence);
     * if (ssc != KStandardShortcut::AccelNone) {
     *     // We have a conflict
     * }
     * \endcode
     *
     * \since 4.2
     */
    void setCheckForConflictsAgainst(ShortcutTypes types);

    /*!
     * \brief The shortcut types we check for conflicts.
     *
     * \sa setCheckForConflictsAgainst()
     * \since 4.2
     */
    ShortcutTypes checkForConflictsAgainst() const;

    /*!
     * \brief Sets whether to allow for multikey shortcuts.
     */
    void setMultiKeyShortcutsAllowed(bool);

    /*!
     * \brief Returns whether multikey shortcuts are allowed.
     */
    bool multiKeyShortcutsAllowed() const;

#if KXMLGUI_ENABLE_DEPRECATED_SINCE(6, 12)
    /*!
     * \brief Sets whether to \a allow "plain" keys without modifiers
     * (like Ctrl, Alt, Meta).
     *
     * This only applies to user input, not to setKeySequence().
     *
     * Plain keys by our definition include letter and symbol keys and
     * text editing keys (Return, Space, Tab, Backspace, Delete).
     *
     * "Special" keys like F1, Cursor keys, Insert, PageDown will always work.
     *
     * \deprecated[6.12]
     * Use setPatterns() instead
     */
    KXMLGUI_DEPRECATED_VERSION(6, 12, "Use setPatterns()") void setModifierlessAllowed(bool allow);

    /*!
     * \sa setModifierlessAllowed()
     *
     * \deprecated[6.12]
     *  Use patterns() instead
     * \brief Returns whether "plain" keys without modifiers
     * (like Ctrl, Alt, Meta) are allowed.
     * \sa setModifierlessAllowed()
     */
    KXMLGUI_DEPRECATED_VERSION(6, 12, "Use patterns()") bool isModifierlessAllowed();

    /*!
     * Whether to allow modifier-only key sequences.
     * \since 6.1
     * \deprecated[6.12]
     * Use setPatterns() instead
     */
    KXMLGUI_DEPRECATED_VERSION(6, 12, "Use setPatterns()") void setModifierOnlyAllowed(bool allow);

    /*!
     * \deprecated[6.12] Use patterns() instead
     */
    KXMLGUI_DEPRECATED_VERSION(6, 12, "Use patterns()") bool modifierOnlyAllowed() const;
#endif

    /*!
     * \brief Sets whether to \a show a small button to set an empty key sequence
     * next to the main input widget.
     *
     * The default is to show the clear button.
     */
    void setClearButtonShown(bool show);

    /*!
     * \brief Returns whether the key sequence \a seq is available to grab.
     *
     * The sequence is checked under the same rules as if it has been typed by
     * the user. This method is useful if you get key sequences from another
     * input source and want to check if it is save to set them.
     *
     * \since 4.2
     */
    bool isKeySequenceAvailable(const QKeySequence &seq) const;

    /*!
     * \brief Returns the currently selected key sequence.
     * \since 5.65
     */
    QKeySequence keySequence() const;

    /*!
     * \brief Sets a list of \a actionCollections to check against for conflictuous shortcut.
     *
     *
     * If a QAction with a conflicting shortcut is found inside this list and
     * its shortcut can be configured (KActionCollection::isShortcutConfigurable()
     * returns true) the user will be prompted whether to steal the shortcut
     * from this action.
     *
     * \sa setCheckForConflictsAgainst()
     * \since 4.1
     */
    void setCheckActionCollections(const QList<KActionCollection *> &actionCollections);

    /*!
     * \brief Sets the \a componentName for the component using this widget.
     *
     * If the component using this widget supports shortcuts contexts,
     * it must set its component name so we can check conflicts correctly.
     */
    void setComponentName(const QString &componentName);

    /*!
     * Returns \c true if a key sequence is currently being recorded; otherwise returns \c false.
     *
     * \since 6.12
     */
    bool isRecording() const;

    /*!
     * Sets the accepted shortcut patterns to \a patterns. A shortcut pattern specifies what
     * components the recoreded shortcut must have, e.g. whether it should include modifier keys, etc.
     *
     * \since 6.12
     */
    void setPatterns(KKeySequenceRecorder::Patterns patterns);

    /*!
     * Returns the accepted shortcut patterns.
     *
     * Default is Modifier | Key
     *
     * \since 6.12
     */
    KKeySequenceRecorder::Patterns patterns() const;

Q_SIGNALS:

    /*!
     * \brief This signal is emitted when the current key sequence \a seq has changed,
     * be it by user input or programmatically.
     * \since 5.65
     */
    void keySequenceChanged(const QKeySequence &seq);

    /*!
     * \brief This signal is emitted after the user agreed to steal a shortcut
     * sequence \a seq from an \a action.
     *
     * This is only done for local shortcuts. So you can be sure
     * \a action is one of the actions you provided with setCheckActionList() or
     * setCheckActionCollections().
     *
     * If you listen to that signal and don't call applyStealShortcut() you
     * are supposed to steal the shortcut and save this change.
     */
    void stealShortcut(const QKeySequence &seq, QAction *action);

    /*!
     * This signal is emitted when the user begins or finishes recording a key sequence. It
     * is not emitted when the current key sequence is changed using setKeySequence().
     *
     * \since 6.12
     */
    void recordingChanged();

public Q_SLOTS:

    /*!
     * \brief Capture a shortcut from the keyboard.
     *
     * This call will only return once a key sequence has been captured
     * or input was aborted.
     *
     * If a key sequence was input, keySequenceChanged() will be emitted.
     *
     * \sa setModifierlessAllowed()
     */
    void captureKeySequence();

    /*!
     * \brief Sets the key sequence \a seq.
     *
     * If \a val == Validate, and the call is actually changing the key sequence,
     * conflictuous shortcut will be checked.
     *
     * \since 5.65
     */
    void setKeySequence(const QKeySequence &seq, Validation val = NoValidate);

    /*!
     * \brief Clears the key sequence.
     */
    void clearKeySequence();

    /*!
     * \brief Actually remove the shortcut that the user wanted to steal, from the
     * action that was using it.
     *
     * This only applies to actions provided to us
     * by setCheckActionCollections() and setCheckActionList().
     *
     * Global and Standard Shortcuts have to be stolen immediately when the
     * user gives their consent (technical reasons). That means those changes
     * will be active even if you never call applyStealShortcut().
     *
     * To be called before you apply your changes. No local shortcuts are
     * stolen until this function is called.
     */
    void applyStealShortcut();

private:
    friend class KKeySequenceWidgetPrivate;
    KKeySequenceWidgetPrivate *const d;

    bool event(QEvent *ev) override;

    Q_DISABLE_COPY(KKeySequenceWidget)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KKeySequenceWidget::ShortcutTypes)

#endif // KKEYSEQUENCEWIDGET_H
