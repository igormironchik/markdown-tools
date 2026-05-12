/*
    SPDX-FileCopyrightText: 2001, 2002 Ellis Whitehead <ellis@kde.org>
    SPDX-FileCopyrightText: 2007 Andreas Hartmetz <ahartmetz@gmail.com>
    SPDX-FileCopyrightText: 2020 David Redondo <kde@david-redondo.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KKEYSEQUENCERECORDER_H
#define KKEYSEQUENCERECORDER_H

#include <kguiaddons_export.h>

#include <QKeySequence>
#include <QObject>
#include <QWindow>

#include <memory>

class KKeySequenceRecorderPrivate;

/*!
 * \class KKeySequenceRecorder
 * \inmodule KGuiAddons
 * \brief Record a QKeySequence by listening to key events in a window.
 *
 * After calling startRecording key events in the set window will be captured until a valid
 * QKeySequence has been recorded and gotKeySequence is emitted. See multiKeyShortcutsAllowed and
 * modifierlessAllowed for what constitutes a valid key sequence.
 *
 * During recording any shortcuts are inhibited and cannot be triggered. Either by using the
 * \l {https://cgit.freedesktop.org/wayland/wayland-protocols/tree/unstable/keyboard-shortcuts-inhibit/keyboard-shortcuts-inhibit-unstable-v1.xml}
 * {keyboard-shortcuts-inhibit protocol} on Wayland or grabbing the keyboard.
 *
 * For graphical elements that record key sequences and can optionally perform conflict checking
 * against existing shortcuts see KKeySequenceWidget and KeySequenceItem.
 *
 * Porting from KF5 to KF6:
 *
 * The class KeySequenceRecorder was renamed to KKeySequenceRecorder.
 *
 * \sa KKeySequenceWidget
 * \sa KeySequenceItem
 *
 * \since 6.0
 */
class KGUIADDONS_EXPORT KKeySequenceRecorder : public QObject
{
    Q_OBJECT

    /*!
     * \property KKeySequenceRecorder::isRecording
     *
     * Whether key events are currently recorded
     */
    Q_PROPERTY(bool isRecording READ isRecording NOTIFY recordingChanged)
    /*!
     * \property KKeySequenceRecorder::currentKeySequence
     *
     * The recorded key sequence.
     * After construction this is empty.
     *
     * During recording it is continuously updated with the newest user input.
     *
     * After recording it contains the last recorded QKeySequence
     */
    Q_PROPERTY(QKeySequence currentKeySequence READ currentKeySequence WRITE setCurrentKeySequence NOTIFY currentKeySequenceChanged)
    /*!
     * \property KKeySequenceRecorder::window
     *
     * The window in which the key events are happening that should be recorded
     */
    Q_PROPERTY(QWindow *window READ window WRITE setWindow NOTIFY windowChanged)
    /*!
     * \property KKeySequenceRecorder::modifierlessAllowed
     *
     * If key presses of "plain" keys without a modifier are considered to be a valid finished
     * key combination.
     * Plain keys  include letter and symbol keys and text editing keys (Return, Space, Tab,
     * Backspace, Delete). Other keys like F1, Cursor keys, Insert, PageDown will always work.
     *
     * By default this is \c false.
     *
     * \deprecated[6.12]
     *
     * Use the patterns property instead.
     */
    Q_PROPERTY(bool modifierlessAllowed READ modifierlessAllowed WRITE setModifierlessAllowed NOTIFY modifierlessAllowedChanged)
    /*!
     * \property KKeySequenceRecorder::multiKeyShortcutsAllowed
     *
     * Controls the amount of key combinations that are captured until recording stops and gotKeySequence
     * is emitted.
     * By default  this is \c true and "Emacs-style" key sequences are recorded. Recording does not
     * stop until four valid key combination have been recorded. Afterwards currentKeySequence().count()
     * will be 4.
     *
     * Otherwise only one key combination is recorded before gotKeySequence is emitted with a
     * QKeySequence with a count() of  1.
     * \sa QKeySequence
     */
    Q_PROPERTY(bool multiKeyShortcutsAllowed READ multiKeyShortcutsAllowed WRITE setMultiKeyShortcutsAllowed NOTIFY multiKeyShortcutsAllowedChanged)

    /*!
     * \property KKeySequenceRecorder::modifierOnlyAllowed
     *
     * It makes it acceptable for the key sequence to be just a modifier (e.g. Shift or Control)
     *
     * By default, if only a modifier is pressed and then released, the component will remain waiting for the sequence.
     * When enabled, it will take the modifier key as the key sequence.
     *
     * By default this is \c false.
     *
     * \deprecated[6.12]
     * Use the patterns property instead.
     */
    Q_PROPERTY(bool modifierOnlyAllowed READ modifierOnlyAllowed WRITE setModifierOnlyAllowed NOTIFY modifierOnlyAllowedChanged)

    /*!
     * \property KKeySequenceRecorder::patterns
     *
     * Specifies what components the recorded shortcut must have, for example whether the shortcut
     * must contain only modifier keys (Modifier) or modifiers keys and a normal key (ModifierAndKey).
     *
     * The patterns property can contain one or more recording patterns. For example, if the recorder
     * accepts both normal and modifier only shortcuts, e.g. Modifier | ModifierAndKey.
     *
     * By default this is ModifierAndKey.
     *
     * \since 6.12
     */
    Q_PROPERTY(Patterns patterns READ patterns WRITE setPatterns NOTIFY patternsChanged)

public:
    /*!
     * The Pattern type specifies what components the recorded shortcut must have, e.g. modifiers or just a key.
     *
     * \value Modifier The recorded shortcut must contain one or more modifier keys (Meta, Shift, Ctrl, or Alt).
     * \value Key The recorded shortcut must contain only one regular key, e.g. "A".
     * \value ModifierAndKey The recorded shortcut must contain one or more modifier keys followed by a regular key,
     *  e.g. Meta+A.
     * Note: with this pattern, special keys like "Insert" without any pressed modifier will be
     * captured too. Please do not rely on this behavior and instead use Key | ModifierAndKey
     * explicitly. The future versions of KKeySequenceRecorder are expected not to record special
     * keys in the ModifierAndKey mode.
     */
    enum Pattern {
        Modifier = 0x1,
        Key = 0x2,
        ModifierAndKey = 0x4,
    };
    Q_DECLARE_FLAGS(Patterns, Pattern)
    Q_FLAG(Patterns)

    /*!
     * Constructor.
     *
     * \a window The window whose key events will be recorded.
     *
     * \sa window
     */
    explicit KKeySequenceRecorder(QWindow *window, QObject *parent = nullptr);
    ~KKeySequenceRecorder() override;

    /*!
     * Start recording.
     * Calling startRecording when window() is \c nullptr has no effect.
     */
    Q_INVOKABLE void startRecording();

    bool isRecording() const;

    QKeySequence currentKeySequence() const;
    void setCurrentKeySequence(const QKeySequence &sequence);

    QWindow *window() const;
    void setWindow(QWindow *window);

    bool multiKeyShortcutsAllowed() const;
    void setMultiKeyShortcutsAllowed(bool allowed);

#if KGUIADDONS_ENABLE_DEPRECATED_SINCE(6, 12)
    /*!
     * \deprecated[6.12]
     * Use setPatterns() instead
     */
    KGUIADDONS_DEPRECATED_VERSION(6, 12, "use setPatterns() instead") void setModifierlessAllowed(bool allowed);

    /*!
     * \deprecated[6.12]
     * Use patterns() instead
     */
    KGUIADDONS_DEPRECATED_VERSION(6, 12, "use patterns() instead") bool modifierlessAllowed() const;

    /*!
     * \deprecated[6.12]
     * Use setPatterns() instead
     */
    KGUIADDONS_DEPRECATED_VERSION(6, 12, "use setPatterns() instead") void setModifierOnlyAllowed(bool allowed);

    /*!
     * \deprecated[6.12]
     * Use patterns() instead
     */
    KGUIADDONS_DEPRECATED_VERSION(6, 12, "use patterns() instead") bool modifierOnlyAllowed() const;
#endif

    void setPatterns(Patterns patterns);
    Patterns patterns() const;

public Q_SLOTS:
    /*!
     * Stops the recording session
     */
    void cancelRecording();

Q_SIGNALS:
    /*!
     * This signal is emitted when a key sequence has been recorded.
     *
     * Compared to currentKeySequenceChanged and currentKeySequence this is signal is not emitted
     * continuously during recording but only after recording has finished.
     */
    void gotKeySequence(const QKeySequence &keySequence);

    void recordingChanged();
    void windowChanged();
    void currentKeySequenceChanged();
    void multiKeyShortcutsAllowedChanged();
#if KGUIADDONS_ENABLE_DEPRECATED_SINCE(6, 12)
    KGUIADDONS_DEPRECATED_VERSION(6, 12, "use patternsChanged() instead") void modifierlessAllowedChanged();
    KGUIADDONS_DEPRECATED_VERSION(6, 12, "use patternsChanged() instead") void modifierOnlyAllowedChanged();
#endif
    void patternsChanged();

private:
    friend class KKeySequenceRecorderPrivate;
    std::unique_ptr<KKeySequenceRecorderPrivate> const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KKeySequenceRecorder::Patterns)

#endif
