/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2020 Ahmad Samir <a.samirh78@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef KMESSAGEDIALOG_H
#define KMESSAGEDIALOG_H

#include <kwidgetsaddons_export.h>

#include <KStandardGuiItem>

#include <QDialog>
#include <QMessageBox>

#include <memory>

class KMessageDialogPrivate;
class KGuiItem;
class QWidget;

/*!
 * \class KMessageDialog
 * \inmodule KWidgetsAddons
 *
 * \brief KMessageDialog creates a message box similar to the ones you get from KMessageBox,
 * but that can be used asynchronously, i.e. you can show the dialog by using show()
 * or open().
 *
 * This class contructs a dialog similar to the dialogs the KMessageBox convenience functions
 * create. The main difference is that the KMessageBox methods typically use exec() to show
 * the dialogs; one of the main disadvantages of using exec(), is that it starts a nested
 * eventloop, which could lead to nasty crashes.
 *
 * Another difference is that the API is supposed to be slightly easier to use as it has
 * various methods to set up the dialog, e.g. setCaption(), setDetails() ...etc.
 *
 * By default, appropriate buttons based on the dialog type are added (since 5.85) (e.g. an
 * "OK" button is added to an Information dialog), you can set custom buttons by using the
 * setButtons() method.
 *
 * The QDialog::done() slot is called to set the result of the dialog, which will emit the
 * QDialog::finished() signal with that result. The result is one of the
 * KMessageDialog::ButtonType enum. This is useful as you can tell exactly which button
 * was clicked by the user. E.g.:
 * \list
 * \li the secondary action button having been clicked, in which case you may still want to save the status
 *   of the "Do not ask again" CheckBox
 * \li the "Cancel" button having been clicked, in which case you ideally will ignore the status
 *   of the "Do not ask again" CheckBox
 * \endlist
 *
 * For "warning" dialogs, i.e. dialogs with a potentially destructive action, the default
 * button is set to a button with the QDialogButtonBox::RejectRole. If the "Cancel" button
 * is used, it will be the default, otherwise the secondary action button.
 *
 * This class intends to be very flexible with the buttons that can be used, since you can
 * call the setButtons() method with a KGuiItem that has custom text/icon.
 *
 * Since Frameworks 5.97 a notification sound is played when the dialog opens like KMessageBox
 * does, this can be controlled using the setNotifyEnabled() method.
 *
 * Example:
 * \code
 * auto *dlg = new KMessageDialog(KMessageDialog::QuestionTwoActionsCancel,
 *                                QStringLiteral("Back or forward?"),
 *                                nullptr);
 *
 * dlg->setCaption(QStringLiteral("Window Title"));
 * dlg->setButtons(KStandardGuiItem::back(), KStandardGuiItem::forward(), KStandardGuiItem::cancel());
 * dlg->setListWidgetItems(QStringList{QStringLiteral("file1"), QStringLiteral("file2")});
 * dlg->setDetails(QStringLiteral("Some more details."));
 * dlg->setDontAskAgainText(QStringLiteral("Do not ask again"));
 * dlg->setDontAskAgainChecked(false);
 *
 *  // Delete the dialog when it's closed
 *  dlg->setAttribute(Qt::WA_DeleteOnClose);
 *  // Make the dialog window modal
 *  dlg->setWindowModality(Qt::WindowModal);
 *
 *  QObject::connect(dlg, &QDialog::finished, &app, [dlg](int result) {
 *      auto button = static_cast<KMessageDialog::ButtonType>(result);
 *      switch(button) {
 *      case KMessageDialog::PrimaryAction:
 *          // The user clicked the primary action, handle the result...
 *          // save the "do not ask again" box status...
 *          break;
 *      case KMessageDialog::SecondaryAction:
 *          // The user clicked the secondary action, handle the result...
 *          // save the "do not ask again" box status...
 *          break;
 *      case KMessageDialog::Cancel:
 *          // The user clicked cancel, reject the changes...
 *          break;
 *      default:
 *          break;
 *      }
 *  });
 *
 *  dlg->show();
 * \endcode
 *
 * \since 5.77
 */
class KWIDGETSADDONS_EXPORT KMessageDialog : public QDialog
{
    Q_OBJECT

public:
    /*!
     * Button types
     * \since 5.100
     * \value Ok Ok button
     * \value Cancel Cancel button
     * \value PrimaryAction Primary action button
     * \value SecondaryAction Secondary action button
     */
    enum ButtonType {
        Ok = 1,
        Cancel = 2,
        PrimaryAction = 3,
        SecondaryAction = 4,
    };

    /*!
     * \value[since 5.100] QuestionTwoActions Question dialog with two buttons
     * \value[since 5.100] QuestionTwoActionsCancel Question dialog with two buttons and Cancel
     * \value[since 5.100] WarningTwoActions Warning dialog with two buttons
     * \value[since 5.100] WarningTwoActionsCancel Warning dialog with two buttons and Cancel
     * \value WarningContinueCancel Warning dialog with Continue and Cancel
     * \value Information Information dialog
     * \value Error Error dialog
     */
    enum Type {
        QuestionTwoActions = 1,
        QuestionTwoActionsCancel = 2,
        WarningTwoActions = 3,
        WarningTwoActionsCancel = 4,
        WarningContinueCancel = 5,
        Information = 6,
        Error = 8,
    };

    /*!
     * Constructs a KMessageDialog.
     *
     * Buttons based on the dialog type are set by default in some cases,
     * using KStandardGuiItem instances. For the dialog types Information and Error
     * the button is set to KStandardGuiItem::ok(). For the type WarningContinueCancel
     * the buttons are set to  KStandardGuiItem::cont() & KStandardGuiItem::cancel().
     *
     * For the other Quesion* and Warning* types the buttons are to be set explicitly.
     *
     * \a type the dialog Type, one of KMessageDialog::Type enum
     *
     * \a text the text message that is going to be displayed in the dialog
     *
     * \a parent a QWidget* that will be set as the dialog parent
     */
    explicit KMessageDialog(KMessageDialog::Type type, const QString &text, QWidget *parent = nullptr);

    /*!
     * This constructor takes the window Id of the parent window, instead of a QWidget*.
     *
     * \a type the dialog Type, one of KMessageDialog::Type enum
     *
     * \a text the text message that is going to be displayed in the dialog
     *
     * \a parent_id the native parent's window system identifier
     */
    explicit KMessageDialog(KMessageDialog::Type type, const QString &text, WId parent_id);
    /*!
     * Destructor
     */
    ~KMessageDialog() override;

    /*!
     * This can be used to set the title of the dialog window. If you pass an
     * empty QString(), a generic title will be used depending on the dialog
     * Type. E.g. for KMessageDialog::WarningTwoActions, "Warning" will be used.
     */
    void setCaption(const QString &caption);

    /*!
     * This can be used to set an icon that will be shown next to the main
     * text message. If you pass a null QIcon() a generic icon based on the dialog
     * Type will be used. E.g. for KMessageDialog::QuestionTwoActions, QMessageBox::Question
     * will be used.
     */
    void setIcon(const QIcon &icon);

    /*!
     * This will add a QListWidget to the dialog and populate it with \a strlist.
     * If \a strlist is empty, the list widget will not be shown.
     */
    void setListWidgetItems(const QStringList &strlist);

    /*!
     * This will add a KCollapsibleGroupBox with a title "Details", as the class name
     * implies it is collapsible (and collapsed by default); you can use it to add a
     * more detailed explanation of what the dialog is trying to tell the user.
     *
     * If \a details is empty, the details widget will not be shown.
     */
    void setDetails(const QString &details);

    /*!
     * This will add a "Do not ask again" checkbox to the dialog with the text
     * from \a dontAskAgainText. You can set the initial status of the checkbox
     * by using setDontAskAgainChecked().
     *
     * If \a dontAskAgainText is empty, no checkbox will be shown.
     *
     * Typical usage of this checkbox is for recurring questions, e.g. showing
     * a dialog to confirm moving files/directories to trash, the user can then
     * set the checkbox so as not to be asked about that again.
     *
     * You can get the state of the checkbox by using isDontAskAgainChecked().
     */
    void setDontAskAgainText(const QString &dontAskAgainText);

    /*!
     * This can be used to set the initial status of the "Do not ask again" checkbox,
     * checked or unchecked, by setting \a isChecked to \c true or \c false
     * respectively.
     *
     * You need to call setDontAskAgainText() first to actually show a checkbox in
     * the dialog, otherwise calling this function will have no effect.
     */
    void setDontAskAgainChecked(bool isChecked);

    /*!
     * This can be used to query the status of the "Do not ask again" checkbox;
     * returns \c true if the box is checked and \c false otherwise.
     *
     * \note This method will return \c false if a checkbox widget isn't shown in
     * the dialog. The dialog will not show a checkbox if setDontAskAgainText() was
     * not used previously to add a checkbox to begin with.
     */
    bool isDontAskAgainChecked() const;

    /*!
     * Sets the text labels in the dialog to either allow opening external links or not.
     */
    void setOpenExternalLinks(bool isAllowed);

    /*!
     * Whether a KNotification is emitted when the dialog is shown.
     *
     * This typically plays a notification sound. Default is true.
     *
     * \since 5.97
     * \sa KMessageBox::Notify
     */
    bool isNotifyEnabled() const;

    /*!
     * Whether to emit a KNotification when the dialog is shown.
     *
     * This typically plays a notification sound.
     *
     * \since 5.97
     * \sa KMessageBox::Notify
     */
    void setNotifyEnabled(bool enable);

    /*!
     * Sets the buttons in the buttom box.
     *
     * Using this method, you can customize the behavior based on your use-case, by
     * using a KGuiItem to get a button with custom text and icon.
     *
     * Since 5.85 buttons based on the dialog type are added by default (see
     * KMessageDialog(KMessageDialog::Type, const QString &, QWidget *) for details).
     * Before, this method had to be called explicitly to have any buttons added to the dialog.
     *
     * \note For dialog types Information and Error only one button
     *   (KStandardGuiItem::ok()) is added to the dialog.
     *
     * \a primaryAction the action for the primary button.
     *                      Reported in the result for dialog types Information and Error
     *                      as KMessageDialog::Ok enum value, otherwise as KMessageDialog::PrimaryAction.
     *
     * \a secondaryAction the action for the secondary button.
     *                        Reported in the result as KMessageDialog::SecondaryAction enum value.
     *                        Ignored with all dialog types without a "secondary" action.
     *
     * \a cancelAction the action for the cancel button.
     *                     Reported in the result as KMessageDialog::Cancel enum value.
     *                     Ignored with all dialog types without a Cancel button.
     */
    void setButtons(const KGuiItem &primaryAction = KGuiItem(), const KGuiItem &secondaryAction = KGuiItem(), const KGuiItem &cancelAction = KGuiItem());

    /*!
     * Manually play the notification sound
     *
     * When implementing your entirely own message box, not using KMessageDialog,
     * you can call this function to play the relevant notification sound (if enabled).
     *
     * \note You don't need to call this when using KMessageDialog, it plays the sound automatically.
     *
     * \a type The message box type
     *
     * \a text The message box contents, for accessibility purposes.
     *
     * \a dialog The dialog that was displayed
     *
     * \since 6.0
     * \sa setNotifyEnabled
     */
    static void beep(KMessageDialog::Type type, const QString &text = QString(), QWidget *dialog = nullptr);

protected:
    void showEvent(QShowEvent *event) override;

private:
    std::unique_ptr<KMessageDialogPrivate> const d;
};

#endif // KMESSAGEDIALOG_H
