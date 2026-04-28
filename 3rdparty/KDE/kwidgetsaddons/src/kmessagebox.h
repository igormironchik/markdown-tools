/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 1999 Waldo Bastian <bastian@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

// krazy:excludeall=dpointer

#ifndef KMESSAGEBOX_H
#define KMESSAGEBOX_H

#include <QDialogButtonBox>
#include <QMessageBox>
#include <QStringList>

#include <kguiitem.h>
#include <kstandardguiitem.h>

#include <kwidgetsaddons_export.h>

class KMessageBoxDontAskAgainInterface;
class KMessageBoxNotifyInterface;
class QDialog;
class QDialogButtonBox;
class QWidget;
class KConfig;

/*!
 * \namespace KMessageBox
 * \inmodule KWidgetsAddons
 *
 * \brief Easy message dialog box.
 *
 * Provides convenience functions for some i18n'ed standard dialogs,
 * as well as audible notification via KNotification
 *
 * The text in message boxes is wrapped automatically. The text may either
 * be plaintext or richtext. If the text is plaintext, a newline-character
 * may be used to indicate the end of a paragraph.
 *
 * \image kmessagebox.png "An information dialog box"
 */
namespace KMessageBox
{
/*!
 * Button types.
 *
 * \value Ok Ok button
 * \value Cancel Cancel button
 * \value[since 5.100] PrimaryAction Primary action button
 * \value[since 5.100] SecondaryAction Secondary action button
 * \value Continue Continue button
 */
enum ButtonCode {
    Ok = 1,
    Cancel = 2,
    PrimaryAction = 3,
    SecondaryAction = 4,
    Continue = 5,
};

/*!
 *
 * \value[since 5.100] QuestionTwoActions Question dialog with two buttons
 * \value[since 5.100] WarningTwoActions Warning dialog with two buttons
 * \value WarningContinueCancel Warning dialog with Continue and Cancel
 * \value[since 5.100] WarningTwoActionsCancel Warning dialog with two buttons and Cancel
 * \value Information Information dialog
 * \value Error Error dialog
 * \value[since 5.100] QuestionTwoActionsCancel Question dialog with two buttons and Cancel
 */
enum DialogType {
    QuestionTwoActions = 1,
    WarningTwoActions = 2,
    WarningContinueCancel = 3,
    WarningTwoActionsCancel = 4,
    Information = 5,
    // Reserved for: SSLMessageBox = 6
    Error = 8,
    QuestionTwoActionsCancel = 9,
};

/*!
 * \value Notify Emit a KNotify event
 * \value AllowLink The message may contain links.
 * \value Dangerous The action to be confirmed by the dialog is a potentially destructive one. The default button will be set to Cancel or SecondaryAction,
 * depending on which is available.
 * \value NoExec Do not call exec() in createKMessageBox()
 * \value WindowModal The window is to be modal relative to its parent. By default, it is application modal.
 * \value[since 6.9] PlainText The label content should be considered as plain text. This should be used when the text comes from untrusted user input.
 */
enum Option {
    Notify = 1,
    AllowLink = 2,
    Dangerous = 4,
    NoExec = 16,
    WindowModal = 32,
    PlainText = 64,
};

Q_DECLARE_FLAGS(Options, Option)

// This declaration must be defined before first Option is used in method signatures
Q_DECLARE_OPERATORS_FOR_FLAGS(Options)

/*!
 * Display a "question" dialog with two action buttons.
 *
 * To be used for questions like "Do you want to save the message for later or discard it?".
 *
 * The default button is the primary button. Pressing "Esc" triggers the secondary button.
 *
 * \a parent  the parent widget
 *
 * \a text    the message string
 *
 * \a title   the message box title. If an empty string, defaults to i18n("Question").
 *
 * \a primaryAction the action for the primary button
 *
 * \a secondaryAction the action for the secondary button
 *
 * \a dontAskAgainName If not an empty string, a checkbox is added with which
 *                further confirmation can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 *                If \a dontAskAgainName starts with a ':' then the setting
 *                is stored in the global config file.
 *
 * \a options see Option
 *
 * Returns PrimaryAction if the primary button is triggered, SecondaryAction
 *          if the secondary button is triggered.
 *
 * \since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode questionTwoActions(QWidget *parent,
                              const QString &text,
                              const QString &title,
                              const KGuiItem &primaryAction,
                              const KGuiItem &secondaryAction,
                              const QString &dontAskAgainName = QString(),
                              Options options = Notify);

/*!
 * Display a "question" dialog with two action buttons and a cancel button.
 *
 * To be used for questions like "Do you want to save the message for later or discard it?".
 *
 * The default button is the primary button. Pressing "Esc" triggers the cancel button.
 *
 * \a parent  the parent widget
 *
 * \a text    the message string
 *
 * \a title   the message box title. If an empty string, defaults to i18n("Question").
 *
 * \a primaryAction the action for the primary button
 *
 * \a secondaryAction the action for the secondary button
 *
 * \a cancelAction the action for the cancel button
 *
 * \a dontAskAgainName If not an empty string, a checkbox is added with which
 *                further confirmation can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 *                If \a dontAskAgainName starts with a ':' then the setting
 *                is stored in the global config file.
 *
 * \a options  see Option
 *
 * Returns PrimaryAction if the primary button is triggered, SecondaryAction
 *          if the secondary button is triggered. Cancel if the cancel button is triggered.
 *
 * \since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode questionTwoActionsCancel(QWidget *parent,
                                    const QString &text,
                                    const QString &title,
                                    const KGuiItem &primaryAction,
                                    const KGuiItem &secondaryAction,
                                    const KGuiItem &cancelAction = KStandardGuiItem::cancel(),
                                    const QString &dontAskAgainName = QString(),
                                    Options options = Notify);

/*!
 * Display a "question" dialog with a listbox to show information to the user
 * and two action buttons.
 *
 * To be used for questions like "Do you really want to delete these files?"
 * and show the user exactly which files are going to be deleted in case.
 *
 * The default button is the primary button. Pressing "Esc" triggers the secondary button.
 *
 * \a parent  the parent widget
 *
 * \a text    the message string
 *
 * \a strlist List of strings to be written in the listbox. If the list is
 *                empty, it doesn't show any listbox, working as questionTwoActions().
 *
 * \a title   the message box title. If an empty string, defaults to i18n("Question").
 *
 * \a primaryAction the action for the primary button
 *
 * \a secondaryAction the action for the secondary button
 *
 * \a dontAskAgainName If not an empty string, a checkbox is added with which
 *                further confirmation can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 *                If \a dontAskAgainName starts with a ':' then the setting
 *                is stored in the global config file.
 * \a options  see Option
 *
 * Returns PrimaryAction if the primary button is triggered, SecondaryAction
 *          if the secondary button is triggered.
 *
 * \since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode questionTwoActionsList(QWidget *parent,
                                  const QString &text,
                                  const QStringList &strlist,
                                  const QString &title,
                                  const KGuiItem &primaryAction,
                                  const KGuiItem &secondaryAction,
                                  const QString &dontAskAgainName = QString(),
                                  Options options = Notify);

/*!
 * Display a "warning" dialog with two action buttons.
 *
 * To be used for questions like "Shall I update your configuration?".
 * The text should explain the implication of both actions.
 *
 * The default button is the secondary button. Pressing "Esc" triggers the secondary button.
 *
 * \a parent  the parent widget
 *
 * \a text    the message string
 *
 * \a title   the message box title. If an empty string, defaults to i18n("Warning").
 *
 * \a primaryAction the action for the primary button
 *
 * \a secondaryAction the action for the secondary button
 *
 * \a dontAskAgainName If not an empty string, a checkbox is added with which
 *                further confirmation can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 *                If \a dontAskAgainName starts with a ':' then the setting
 *                is stored in the global config file.
 *
 * \a options see Options
 *
 * Returns \c PrimaryAction if the primary button is triggered, \c SecondaryAction
 *          if the secondary button is triggered.
 *
 * \since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode warningTwoActions(QWidget *parent,
                             const QString &text,
                             const QString &title,
                             const KGuiItem &primaryAction,
                             const KGuiItem &secondaryAction,
                             const QString &dontAskAgainName = QString(),
                             Options options = Options(Notify | Dangerous));

/*!
 * Display a "warning" dialog with a listbox to show information to the user
 * and two action buttons.
 *
 * To be used for questions like "Shall I update your configuration?".
 * The text should explain the implication of both actions.
 *
 * The default button is the secondary button. Pressing "Esc" triggers the secondary button.
 *
 * \a parent  the parent widget
 *
 * \a text    the message string
 *
 * \a strlist List of strings to be written in the listbox. If the list is
 *                empty, it doesn't show any listbox, working as warningTwoActions.
 *
 * \a title   the message box title. If an empty string, defaults to i18n("Warning").
 *
 * \a primaryAction the action for the primary button
 *
 * \a secondaryAction the action for the secondary button
 *
 * \a dontAskAgainName If not an empty string, a checkbox is added with which
 *                further confirmation can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 *                If \a dontAskAgainName starts with a ':' then the setting
 *                is stored in the global config file.
 *
 * \a options see Options
 *
 * Returns PrimaryAction if the primary button is triggered, SecondaryAction
 *          if the secondary button is triggered.
 *
 * \since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode warningTwoActionsList(QWidget *parent,
                                 const QString &text,
                                 const QStringList &strlist,
                                 const QString &title,
                                 const KGuiItem &primaryAction,
                                 const KGuiItem &secondaryAction,
                                 const QString &dontAskAgainName = QString(),
                                 Options options = Options(Notify | Dangerous));

/*!
 * Display a "warning" dialog.
 *
 * \a parent  Parent widget.
 *
 * \a text    Message string.
 *
 * \a title   Message box title. The application name is added to
 *                the title. The default title is i18n("Warning").
 *
 * \a buttonContinue The text for the first button.
 *                       The default is KStandardGuiItem::cont().
 *
 * \a buttonCancel The text for the second button.
 *                     The default is KStandardGuiItem::cancel().
 *
 * \a dontAskAgainName If provided, a checkbox is added with which
 *                further confirmation can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 *                If \a dontAskAgainName starts with a ':' then the setting
 *                is stored in the global config file.
 *
 * \a options  see Options
 *
 * Continue is returned if the Continue-button is pressed.
 *
 * Cancel is returned if the Cancel-button is pressed.
 *
 * To be used for questions like "You are about to Print. Are you sure?"
 * the continueButton should then be labeled "Print".
 *
 * The default button is buttonContinue. Pressing "Esc" selects "Cancel".
 */
KWIDGETSADDONS_EXPORT ButtonCode warningContinueCancel(QWidget *parent,
                                                       const QString &text,
                                                       const QString &title = QString(),
                                                       const KGuiItem &buttonContinue = KStandardGuiItem::cont(),
                                                       const KGuiItem &buttonCancel = KStandardGuiItem::cancel(),
                                                       const QString &dontAskAgainName = QString(),
                                                       Options options = Notify);

/*!
 * Display a "warning" dialog with a collapsible "Details" section.
 *
 * \since 5.61
 */
KWIDGETSADDONS_EXPORT ButtonCode warningContinueCancelDetailed(QWidget *parent,
                                                               const QString &text,
                                                               const QString &title = QString(),
                                                               const KGuiItem &buttonContinue = KStandardGuiItem::cont(),
                                                               const KGuiItem &buttonCancel = KStandardGuiItem::cancel(),
                                                               const QString &dontAskAgainName = QString(),
                                                               Options options = Notify,
                                                               const QString &details = QString());

/*!
 * Display a "warning" dialog with a listbox to show information to the user.
 *
 * \a parent  Parent widget.
 *
 * \a text    Message string.
 *
 * \a strlist List of strings to be written in the listbox. If the
 *                list is empty, it doesn't show any listbox, working
 *                as warningContinueCancel.
 *
 * \a title   Message box title. The application name is added to
 *                the title. The default title is i18n("Warning").
 *
 * \a buttonContinue The text for the first button.
 *                       The default is KStandardGuiItem::cont().
 *
 * \a buttonCancel The text for the second button.
 *                     The default is KStandardGuiItem::cancel().
 *
 * \a dontAskAgainName If provided, a checkbox is added with which
 *                further confirmation can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 *                If \a dontAskAgainName starts with a ':' then the setting
 *                is stored in the global config file.
 *
 * \a options  see Options
 *
 * Continue is returned if the Continue-button is pressed.
 *
 * Cancel is returned if the Cancel-button is pressed.
 *
 * To be used for questions like "You are about to Print. Are you sure?"
 * the continueButton should then be labeled "Print".
 *
 * The default button is buttonContinue. Pressing "Esc" selects "Cancel".
 */
KWIDGETSADDONS_EXPORT ButtonCode warningContinueCancelList(QWidget *parent,
                                                           const QString &text,
                                                           const QStringList &strlist,
                                                           const QString &title = QString(),
                                                           const KGuiItem &buttonContinue = KStandardGuiItem::cont(),
                                                           const KGuiItem &buttonCancel = KStandardGuiItem::cancel(),
                                                           const QString &dontAskAgainName = QString(),
                                                           Options options = Notify);

/*!
 * Display a "warning" dialog with two action buttons and a cancel button.
 *
 * To be used for questions like "Shall I update your configuration?".
 * The text should explain the implication of both actions.
 *
 * The default button is the cancel button. Pressing "Esc" triggers the cancel button.
 *
 * \a parent  the parent widget
 *
 * \a text    the message string
 *
 * \a title   the message box title. If an empty string, defaults to i18n("Warning").
 *
 * \a primaryAction the action for the primary button
 *
 * \a secondaryAction the action for the secondary button
 *
 * \a cancelAction the action for the cancel button
 *
 * \a dontAskAgainName If not an empty string, a checkbox is added with which
 *                further confirmation can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 *                If \a dontAskAgainName starts with a ':' then the setting
 *                is stored in the global config file.
 *
 * \a options see Options
 *
 * Returns PrimaryAction if the primary button is triggered, SecondaryAction
 *          if the secondary button is triggered. Cancel if the cancel button is triggered.
 *
 * \since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode warningTwoActionsCancel(QWidget *parent,
                                   const QString &text,
                                   const QString &title,
                                   const KGuiItem &primaryAction,
                                   const KGuiItem &secondaryAction,
                                   const KGuiItem &cancelAction = KStandardGuiItem::cancel(),
                                   const QString &dontAskAgainName = QString(),
                                   Options options = Options(Notify | Dangerous));

/*!
 * Display a "warning" dialog with a listbox to show information
 * to the user, two action buttons and a cancel button.
 *
 * To be used for questions like "Shall I update your configuration?".
 * The text should explain the implication of both actions.
 *
 * The default button is the cancel button. Pressing "Esc" triggers the cancel button.
 *
 * \a parent  the parent widget
 *
 * \a text    the message string
 *
 * \a strlist a List of strings to be written in the listbox. If the
 *                list is empty, it doesn't show any listbox, working
 *                as warningTwoActionsCancel().
 *
 * \a title   the message box title. If an empty string, defaults to i18n("Warning").
 *
 * \a primaryAction the action for the primary button
 *
 * \a secondaryAction the action for the secondary button
 *
 * \a cancelAction the action for the cancel button
 *
 * \a dontAskAgainName If not an empty string, a checkbox is added with which
 *                further confirmation can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 *                If \a dontAskAgainName starts with a ':' then the setting
 *                is stored in the global config file.
 *
 * \a options see Options
 *
 * Returns PrimaryAction if the primary button is triggered, SecondaryAction
 *          if the secondary button is triggered. Cancel if the cancel button is triggered.
 *
 * \since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode warningTwoActionsCancelList(QWidget *parent,
                                       const QString &text,
                                       const QStringList &strlist,
                                       const QString &title,
                                       const KGuiItem &primaryAction,
                                       const KGuiItem &secondaryAction,
                                       const KGuiItem &cancelAction = KStandardGuiItem::cancel(),
                                       const QString &dontAskAgainName = QString(),
                                       Options options = Options(Notify | Dangerous));

/*!
 * Display an "Error" dialog.
 *
 * \a parent Parent widget.
 *
 * \a text Message string.
 *
 * \a title Message box title. The application name is added to
 *                the title. The default title is i18n("Error").
 *
 * \a options see Options
 *
 * Your program messed up and now it's time to inform the user.
 * To be used for important things like "Sorry, I deleted your hard disk."
 *
 * The default button is "&OK". Pressing "Esc" selects the OK-button.
 *
 * \note The OK button will always have the i18n'ed text '&OK'.
 */
KWIDGETSADDONS_EXPORT void error(QWidget *parent, const QString &text, const QString &title = QString(), Options options = Notify);

/*!
 * Display an "Error" dialog.
 *
 * \a parent  Parent widget.
 *
 * \a text    Message string.
 *
 * \a title   Message box title. The application name is added to
 *                the title. The default title is i18n("Error").
 *
 * \a buttonOk The text for the only button.
 *                 The default is KStandardGuiItem::ok().
 *
 * \a options  see Options
 *
 * There is only one button, therefore it's the default button, and pressing "Esc" selects it as well.
 *
 * \since 5.97
 */
KWIDGETSADDONS_EXPORT
void error(QWidget *parent,
           const QString &text,
           const QString &title /*= QString()*/,
           const KGuiItem &buttonOk /*= KStandardGuiItem::ok()*/,
           Options options = Notify); // TODO KF6 merge with previous overload

/*!
 * Display an "Error" dialog with a listbox.
 *
 * \a parent  Parent widget.
 *
 * \a text    Message string.
 *
 * \a strlist List of strings to be written in the listbox. If the
 *                list is empty, it doesn't show any listbox, working
 *                as error().
 *
 * \a title   Message box title. The application name is added to
 *                the title. The default title is i18n("Error").
 *
 * \a options  see Options
 *
 * Your program messed up and now it's time to inform the user.
 * To be used for important things like "Sorry, I deleted your hard disk."
 *
 * The default button is "&OK". Pressing "Esc" selects the OK-button.
 *
 * \note The OK button will always have the i18n'ed text '&OK'.
 */
KWIDGETSADDONS_EXPORT void
errorList(QWidget *parent, const QString &text, const QStringList &strlist, const QString &title = QString(), Options options = Notify);

/*!
 * Displays an "Error" dialog with a "Details >>" button.
 *
 * \a parent Parent widget.
 *
 * \a text Message string.
 *
 * \a details Detailed message string.
 *
 * \a title Message box title. The application name is added to
 *                the title. The default title is i18n("Error").
 *
 * \a options see Options
 *
 * Your program messed up and now it's time to inform the user.
 * To be used for important things like "Sorry, I deleted your hard disk."
 *
 * The \a details message can contain additional information about
 * the problem and can be shown on request to advanced/interested users.
 *
 * The default button is "&OK". Pressing "Esc" selects the OK-button.
 *
 * \note The OK button will always have the i18n'ed text '&OK'.
 */
KWIDGETSADDONS_EXPORT void
detailedError(QWidget *parent, const QString &text, const QString &details, const QString &title = QString(), Options options = Notify);

/*!
 * Displays an "Error" dialog with a "Details >>" button.
 *
 * \a parent Parent widget.
 *
 * \a text Message string.
 *
 * \a details Detailed message string.
 *
 * \a title Message box title. The application name is added to
 *                the title. The default title is i18n("Error").
 *
 * \a buttonOk The text for the only button.
 *                 The default is KStandardGuiItem::ok().
 *
 * \a options see Options
 *
 * Your program messed up and now it's time to inform the user.
 * To be used for important things like "Sorry, I deleted your hard disk."
 *
 * The \a details message can contain additional information about
 * the problem and can be shown on request to advanced/interested users.
 *
 * There is only one button, therefore it's the default button, and pressing "Esc" selects it as well.
 *
 * \since 5.97
 */
KWIDGETSADDONS_EXPORT
void detailedError(QWidget *parent,
                   const QString &text,
                   const QString &details,
                   const QString &title /*= QString()*/,
                   const KGuiItem &buttonOk /*= KStandardGuiItem::ok()*/,
                   Options options = Notify); // TODO KF6 merge with previous overload

/*!
 * Display an "Information" dialog.
 *
 * \a parent Parent widget.
 *
 * \a text Message string.
 *
 * \a title Message box title. The application name is added to
 *                the title. The default title is i18n("Information").
 *
 * \a dontShowAgainName If provided, a checkbox is added with which
 *                further notifications can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 *
 * \a options see Options
 *
 * Your program wants to tell the user something.
 * To be used for things like:
 * "Your bookmarks have been rearranged."
 *
 * The default button is "&OK". Pressing "Esc" selects the OK-button.
 *
 * \note The OK button will always have the i18n'ed text '&OK'.
 */
KWIDGETSADDONS_EXPORT void
information(QWidget *parent, const QString &text, const QString &title = QString(), const QString &dontShowAgainName = QString(), Options options = Notify);

/*!
 * Display an "Information" dialog with a listbox.
 *
 * \a parent  Parent widget.
 *
 * \a text    Message string.
 *
 * \a strlist List of strings to be written in the listbox. If the
 *                list is empty, it doesn't show any listbox, working
 *                as information.
 *
 * \a title   Message box title. The application name is added to
 *                the title. The default title is i18n("Information").
 *
 * \a dontShowAgainName If provided, a checkbox is added with which
 *                further notifications can be turned off.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *                The setting is stored in the "Notification Messages" group.
 *
 * \a options  see Options
 *
 *
 * Your program wants to tell the user something.
 * To be used for things like:
 * "The following bookmarks have been rearranged:"
 *
 * The default button is "&OK". Pressing "Esc" selects the OK-button.
 *
 * \note The OK button will always have the i18n'ed text '&OK'.
 */
KWIDGETSADDONS_EXPORT void informationList(QWidget *parent,
                                           const QString &text,
                                           const QStringList &strlist,
                                           const QString &title = QString(),
                                           const QString &dontShowAgainName = QString(),
                                           Options options = Notify);

/*!
 * Enable all messages which have been turned off with the
 *
 * \a dontShowAgainName feature.
 */
KWIDGETSADDONS_EXPORT void enableAllMessages();

/*!
 * Re-enable a specific \a dontShowAgainName messages that had
 * previously been turned off.
 *
 * \sa saveDontShowAgainTwoActions()
 * \sa saveDontShowAgainContinue()
 */
KWIDGETSADDONS_EXPORT void enableMessage(const QString &dontShowAgainName);

/*!
 * Alternate method to show a messagebox:
 *
 * \a parent Parent widget.
 *
 * \a type type of message box: QuestionTwoActions, WarningTwoActions, WarningContinueCancel...
 *
 * \a text Message string.
 *
 * \a title Message box title.
 *
 * \a primaryAction The KGuiItem for the first button.
 *
 * \a secondaryAction The KGuiItem for the second button.
 *
 * \a cancelAction The text for the third button.
 *                     The default is KStandardGuiItem::cancel().
 *
 * \a dontShowAskAgainName If provided, a checkbox is added with which
 *                further questions/information can be turned off. If turned off
 *                all questions will be automatically answered with the
 *                last answer (either PrimaryAction or SecondaryAction),
 *                if the message box needs an answer.
 *                The string is used to lookup and store the setting
 *                in the applications config file.
 *
 * \a options see Options
 *
 * Note: for ContinueCancel, primaryAction is the continue button and secondaryAction is unused.
 *       and for Information, none is used.
 *
 * Returns a button code, as defined in KMessageBox.
 */
KWIDGETSADDONS_EXPORT
ButtonCode messageBox(QWidget *parent,
                      DialogType type,
                      const QString &text,
                      const QString &title,
                      const KGuiItem &primaryAction,
                      const KGuiItem &secondaryAction,
                      const KGuiItem &cancelAction = KStandardGuiItem::cancel(),
                      const QString &dontShowAskAgainName = QString(),
                      Options options = Notify);

/*!
 * \a dontShowAgainName the name that identifies the message box.
 *                          If empty, \c true is always returned.
 *
 * \a result reference to a variable to be set to the choice (\c PrimaryAction or \c SecondaryAction)
 *               that was chosen the last time the message box was shown.
 *               Only meaningful if the message box should not be shown.
 *
 * Returns \c true if the corresponding two actions message box should be shown, \c false otherwise.
 *
 * \since 5.100
 */
KWIDGETSADDONS_EXPORT
bool shouldBeShownTwoActions(const QString &dontShowAgainName, ButtonCode &result);

/*!
 * Returns \c true if the corresponding continue/cancel message box should be
 * shown.
 *
 * \a dontShowAgainName the name that identify the message box. If
 * empty, true is always returned.
 */
KWIDGETSADDONS_EXPORT bool shouldBeShownContinue(const QString &dontShowAgainName);

/*!
 * Save the fact that a two actions message box should not be shown again.
 *
 * \a dontShowAgainName the name that identifies the message box.
 *                          If empty, this method does nothing.
 *
 * \a result the value (\c PrimaryAction or \c SecondaryAction) that should be used
 *               as the result for the message box.
 *
 * \since 5.100
 */
KWIDGETSADDONS_EXPORT
void saveDontShowAgainTwoActions(const QString &dontShowAgainName, ButtonCode result);

/*!
 * Save the fact that the continue/cancel message box should not be shown
 * again.
 *
 * \a dontShowAgainName the name that identify the message box. If
 * empty, this method does nothing.
 */
KWIDGETSADDONS_EXPORT void saveDontShowAgainContinue(const QString &dontShowAgainName);

/*!
 * Use \a cfg for all settings related to the dontShowAgainName feature.
 *
 * If \a cfg is 0 (default) KGlobal::config() will be used.
 */
KWIDGETSADDONS_EXPORT void setDontShowAgainConfig(KConfig *cfg);

/*!
 * Use \a dontAskAgainInterface for all settings related to the dontShowAgain feature.
 *
 * This method does not take ownership of \a dontAskAgainInterface.
 *
 * \since 5.0
 */
KWIDGETSADDONS_EXPORT void setDontShowAgainInterface(KMessageBoxDontAskAgainInterface *dontAskAgainInterface);

/*!
 * Use \a notifyInterface to send notifications.
 *
 * This method does not take ownership of \a notifyInterface.
 *
 * \since 5.0
 */
KWIDGETSADDONS_EXPORT void setNotifyInterface(KMessageBoxNotifyInterface *notifyInterface);

/*!
 * Create content and layout of a standard dialog
 *
 * \a dialog The parent dialog base
 *
 * \a buttons a QDialogButtonBox instance. This function will take care of connecting to it.
 *
 * \a icon Which predefined icon the message box shall show.
 *
 * \a text Message string.
 *
 * \a strlist List of strings to be written in the listbox.
 *                If the list is empty, it doesn't show any listbox
 *
 * \a ask The text of the checkbox. If empty none will be shown.
 *
 * \a checkboxReturn The result of the checkbox. If it's initially
 *                true then the checkbox will be checked by default.
 *                May be a null pointer. Incompatible with NoExec.
 *
 * \a options see Options
 *
 * \a details Detailed message string.
 *
 * Returns a QDialogButtonBox::StandardButton button code, not a KMessageBox
 *         button code, based on the buttonmask given to the constructor of
 *         the \a dialog (ie. will return QDialogButtonBox::Yes instead of
 *         KMessageBox::PrimaryAction). Will return QDialogButtonBox::NoButton if the
 *         message box is queued for display instead of exec()ed immediately
 *         or if the option NoExec is set.
 *
 * \note   Unless NoExec is used,
 *         the \a dialog that is passed in is deleted by this
 *         function. Do not delete it yourself.
 */
KWIDGETSADDONS_EXPORT QDialogButtonBox::StandardButton createKMessageBox(QDialog *dialog,
                                                                         QDialogButtonBox *buttons,
                                                                         QMessageBox::Icon icon, // krazy:exclude=qclasses
                                                                         const QString &text,
                                                                         const QStringList &strlist,
                                                                         const QString &ask,
                                                                         bool *checkboxReturn,
                                                                         Options options,
                                                                         const QString &details = QString());

/*!
 * Create content and layout of a standard dialog
 *
 * \a dialog  The parent dialog base
 *
 * \a buttons a QDialogButtonBox instance. This function will take care of connecting to it.
 *
 * \a icon    A QPixmap containing the icon to be displayed in the
 *                dialog next to the text.
 *
 * \a text    Message string.
 *
 * \a strlist List of strings to be written in the listbox.
 *                If the list is empty, it doesn't show any listbox
 *
 * \a ask     The text of the checkbox. If empty none will be shown.
 *
 * \a checkboxReturn The result of the checkbox. If it's initially
 *                true then the checkbox will be checked by default.
 *                May be a null pointer. Incompatible with NoExec.
 *
 * \a options  see Options
 *
 * \a details Detailed message string.
 *
 * \a notifyType The type of notification to send when this message
 *                is presentend.
 *
 * Returns a QDialogButtonBox::StandardButton button code, not a KMessageBox
 *         button code, based on the buttonmask given to the constructor of
 *         the \a dialog (ie. will return QDialogButtonBox::Yes instead of
 *         KMessageBox::PrimaryAction). Will return QDialogButtonBox::NoButton if the
 *         message box is queued for display instead of exec()ed immediately
 *         or if the option NoExec is set.
 *
 * \note Unless NoExec is used,
 *         the \a dialog that is passed in is deleted by this
 *         function. Do not delete it yourself.
 */
KWIDGETSADDONS_EXPORT QDialogButtonBox::StandardButton createKMessageBox(QDialog *dialog,
                                                                         QDialogButtonBox *buttons,
                                                                         const QIcon &icon,
                                                                         const QString &text,
                                                                         const QStringList &strlist,
                                                                         const QString &ask,
                                                                         bool *checkboxReturn,
                                                                         Options options,
                                                                         const QString &details = QString(),
                                                                         QMessageBox::Icon notifyType = QMessageBox::Information); // krazy:exclude=qclasses

/*!
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 *
 * \sa questionTwoActions()
 * \since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode questionTwoActionsWId(WId parent_id,
                                 const QString &text,
                                 const QString &title,
                                 const KGuiItem &primaryAction,
                                 const KGuiItem &secondaryAction,
                                 const QString &dontAskAgainName = QString(),
                                 Options options = Notify);

/*!
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 *
 * \sa questionTwoActionsCancel()
 * \since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode questionTwoActionsCancelWId(WId parent_id,
                                       const QString &text,
                                       const QString &title,
                                       const KGuiItem &primaryAction,
                                       const KGuiItem &secondaryAction,
                                       const KGuiItem &cancelAction = KStandardGuiItem::cancel(),
                                       const QString &dontAskAgainName = QString(),
                                       Options options = Notify);

/*!
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 *
 * \sa questionTwoActionsList()
 * \since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode questionTwoActionsListWId(WId parent_id,
                                     const QString &text,
                                     const QStringList &strlist,
                                     const QString &title,
                                     const KGuiItem &primaryAction,
                                     const KGuiItem &secondaryAction,
                                     const QString &dontAskAgainName = QString(),
                                     Options options = Notify);

/*!
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 *
 * \sa warningTwoActions()
 * \since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode warningTwoActionsWId(WId parent_id,
                                const QString &text,
                                const QString &title,
                                const KGuiItem &primaryAction,
                                const KGuiItem &secondaryAction,
                                const QString &dontAskAgainName = QString(),
                                Options options = Options(Notify | Dangerous));

/*!
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 *
 * \sa warningTwoActionsList()
 * \since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode warningTwoActionsListWId(WId parent_id,
                                    const QString &text,
                                    const QStringList &strlist,
                                    const QString &title,
                                    const KGuiItem &primaryAction,
                                    const KGuiItem &secondaryAction,
                                    const QString &dontAskAgainName = QString(),
                                    Options options = Options(Notify | Dangerous));

/*!
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 */
KWIDGETSADDONS_EXPORT ButtonCode warningContinueCancelWId(WId parent_id,
                                                          const QString &text,
                                                          const QString &title = QString(),
                                                          const KGuiItem &buttonContinue = KStandardGuiItem::cont(),
                                                          const KGuiItem &buttonCancel = KStandardGuiItem::cancel(),
                                                          const QString &dontAskAgainName = QString(),
                                                          Options options = Notify);

/*!
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 */
KWIDGETSADDONS_EXPORT ButtonCode warningContinueCancelListWId(WId parent_id,
                                                              const QString &text,
                                                              const QStringList &strlist,
                                                              const QString &title = QString(),
                                                              const KGuiItem &buttonContinue = KStandardGuiItem::cont(),
                                                              const KGuiItem &buttonCancel = KStandardGuiItem::cancel(),
                                                              const QString &dontAskAgainName = QString(),
                                                              Options options = Notify);

/*!
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 *
 * \sa warningTwoActionsCancel()
 * \since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode warningTwoActionsCancelWId(WId parent_id,
                                      const QString &text,
                                      const QString &title,
                                      const KGuiItem &primaryAction,
                                      const KGuiItem &secondaryAction,
                                      const KGuiItem &cancelAction = KStandardGuiItem::cancel(),
                                      const QString &dontAskAgainName = QString(),
                                      Options options = Options(Notify | Dangerous));

/*!
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 *
 * \sa warningTwoActionsCancelList()
 * \since 5.100
 */
KWIDGETSADDONS_EXPORT
ButtonCode warningTwoActionsCancelListWId(WId parent_id,
                                          const QString &text,
                                          const QStringList &strlist,
                                          const QString &title,
                                          const KGuiItem &primaryAction,
                                          const KGuiItem &secondaryAction,
                                          const KGuiItem &cancelAction = KStandardGuiItem::cancel(),
                                          const QString &dontAskAgainName = QString(),
                                          Options options = Options(Notify | Dangerous));

/*!
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 */
KWIDGETSADDONS_EXPORT void errorWId(WId parent_id, const QString &text, const QString &title = QString(), Options options = Notify);

/*!
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 */
KWIDGETSADDONS_EXPORT void
errorListWId(WId parent_id, const QString &text, const QStringList &strlist, const QString &title = QString(), Options options = Notify);

/*!
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 */
KWIDGETSADDONS_EXPORT void
detailedErrorWId(WId parent_id, const QString &text, const QString &details, const QString &title = QString(), Options options = Notify);

/*!
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 * \since 5.97
 */
KWIDGETSADDONS_EXPORT
void detailedErrorWId(WId parent_id,
                      const QString &text,
                      const QString &details,
                      const QString &title /*= QString()*/,
                      const KGuiItem &buttonOk /*= KStandardGuiItem::ok()*/,
                      Options options = Notify); // TODO KF6 merge with previous overload

/*!
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 */
KWIDGETSADDONS_EXPORT void
informationWId(WId parent_id, const QString &text, const QString &title = QString(), const QString &dontShowAgainName = QString(), Options options = Notify);

/*!
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 */
KWIDGETSADDONS_EXPORT void informationListWId(WId parent_id,
                                              const QString &text,
                                              const QStringList &strlist,
                                              const QString &title = QString(),
                                              const QString &dontShowAgainName = QString(),
                                              Options options = Notify);

/*!
 * This function accepts the window id of the parent window, instead
 * of QWidget*. It should be used only when necessary.
 */
KWIDGETSADDONS_EXPORT
ButtonCode messageBoxWId(WId parent_id,
                         DialogType type,
                         const QString &text,
                         const QString &title,
                         const KGuiItem &primaryAction,
                         const KGuiItem &secondaryAction,
                         const KGuiItem &cancelAction = KStandardGuiItem::cancel(),
                         const QString &dontShowAskAgainName = QString(),
                         Options options = Notify);
}

#endif
