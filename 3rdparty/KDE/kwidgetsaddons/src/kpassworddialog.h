/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2000 Dawit Alemayehu <adawit@kde.org>
    SPDX-FileCopyrightText: 2007 Olivier Goffart <ogoffart at kde.org>
    SPDX-FileCopyrightText: 2020 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KPASSWORDDIALOG_H
#define KPASSWORDDIALOG_H

#include <KPassword>
#include <QDialog>
#include <QDialogButtonBox>
#include <memory>

#include <kwidgetsaddons_export.h>

/*!
 * \class KPasswordDialog
 * \inmodule KWidgetsAddons
 *
 * \brief A dialog for requesting a password and optionally a login from the end user.
 *
 * \section1 Usage Example
 *
 * Requesting a simple password, asynchronous
 *
 * \code
 *  KPasswordDialog *dlg = new KPasswordDialog(parent);
 *  dlg->setPrompt(i18n("Enter a password"));
 *  connect(dlg, &KPasswordDialog::gotPassword,
 *          this, [this](const QString &password) { setPassword(password); });
 *  connect(dlg, &QDialog::rejected, this, [this]() { slotCancel(); });
 *  dlg->show();
 * \endcode
 *
 * Requesting a login and a password, synchronous
 *
 * \code
 *  KPasswordDialog dlg(parent, KPasswordDialog::ShowUsernameLine);
 *  dlg.setPrompt(i18n("Enter a login and a password"));
 *  if(!dlg.exec()) {
 *      return; //the user canceled
 *  }
 *  use(dlg.username(), dlg.password());
 * \endcode
 *
 * \image kpassworddialog.png "KPasswordDialog"
 */
class KWIDGETSADDONS_EXPORT KPasswordDialog : public QDialog
{
    Q_OBJECT

public:
    /*!
     * \value NoFlags
     * \value ShowKeepPassword If this flag is set, the "keep this password" checkbox will been shown, otherwise, it will not be shown and keepPassword will
     * have no effect.
     * \value ShowUsernameLine If this flag is set, there will be an additional line to let the user enter his login, otherwise, only the password line will be
     * shown.
     * \value UsernameReadOnly If this flag is set, the login lineedit will be in read only mode.
     * \value[since 4.1] ShowAnonymousLoginCheckBox If this flag is set, the Anonymous Login checkbox will be displayed
     * \value[since 4.1] ShowDomainLine If this flag is set, there will be an additional line to let the user enter the domain.
     * \value[since 4.1] DomainReadOnly If this flag is set, the domain lineedit will be in read only mode.
     */
    enum KPasswordDialogFlag {
        NoFlags = 0x00,
        ShowKeepPassword = 0x01,
        ShowUsernameLine = 0x02,
        UsernameReadOnly = 0x04,
        ShowAnonymousLoginCheckBox = 0x08,
        ShowDomainLine = 0x10,
        DomainReadOnly = 0x20,
    };
    Q_DECLARE_FLAGS(KPasswordDialogFlags, KPasswordDialogFlag)

    /*!
     * \value UnknownError
     * \value UsernameError A problem with the user name as entered
     * \value PasswordError Incorrect password
     * \value FatalError Error preventing further attempts, will result in disabling most of the interface
     * \value[since 4.1] DomainError A problem with the domain as entered
     */
    enum ErrorType {
        UnknownError = 0,
        UsernameError,
        PasswordError,
        FatalError,
        DomainError,
    };

    /*!
     * create a password dialog
     *
     * \a parent the parent widget
     *
     * \a flags a set of KPasswordDialogFlag flags
     */
    explicit KPasswordDialog(QWidget *parent = nullptr, const KPasswordDialogFlags &flags = KPasswordDialog::NoFlags);

    ~KPasswordDialog() override;

    /*!
     * Sets the prompt to show to the user.
     *
     * \a prompt instructional text to be shown.
     */
    void setPrompt(const QString &prompt);

    /*!
     * Returns the prompt
     */
    QString prompt() const;

    /*!
     * Set the icon that appears next to the prompt.
     * \since 5.63
     */
    void setIcon(const QIcon &icon);

    /*!
     * Returns the icon that appears next to the prompt.
     */
    QIcon icon() const;

    /*!
     * Adds a comment line to the dialog.
     *
     * This function allows you to add one additional comment
     * line to this widget.  Calling this function after a
     * comment has already been added will not have any effect.
     *
     * \a label label for comment (ex:"Command:")
     *
     * \a comment the actual comment text.
     */
    void addCommentLine(const QString &label, const QString &comment);

    /*!
     * Shows an error message in the dialog box. Prevents having to show a dialog-on-a-dialog.
     *
     * \a message the error message to show
     */
    void showErrorMessage(const QString &message, const ErrorType type = PasswordError);

    /*!
     * Returns the password entered by the user.
     */
    QString password() const;

    /*!
     * set the default username.
     */
    void setUsername(const QString &);

    /*!
     * Returns the username entered by the user.
     */
    QString username() const;

    /*!
     * set the default domain.
     * \since 4.1
     */
    void setDomain(const QString &);

    /*!
     * Returns the domain entered by the user.
     * \since 4.1
     */
    QString domain() const;

    /*!
     * set anonymous mode (all other fields will be grayed out)
     * \since 4.1
     */
    void setAnonymousMode(bool anonymous);

    /*!
     * Returns whether anonymous mode has been selected.
     * \since 4.1
     */
    bool anonymousMode() const;

    /*!
     * Determines whether supplied authorization should
     * persist even after the application has been closed.
     *
     * this is set with the check password checkbox is the ShowKeepCheckBox flag
     * is set in the constructor, if it is not set, this function return false
     *
     * Returns whether to keep the password
     */
    bool keepPassword() const;

    /*!
     * Check or uncheck the "keep password" checkbox.
     * This can be used to check it before showing the dialog, to tell
     * the user that the password is stored already (e.g. in the wallet).
     * enableKeep must have been set to true in the constructor.
     *
     * has only effect if ShowKeepCheckBox is set in the constructor
     */
    void setKeepPassword(bool b);

    /*!
     * Sets the username field read-only and sets the
     * focus to the password field.
     *
     * this can also be set by passing UsernameReadOnly as flag in the constructor
     *
     * \a readOnly true to set the user field to read-only
     */
    void setUsernameReadOnly(bool readOnly);

    /*!
     * Presets the password.
     *
     * If the password is not empty, the ability to show the password will not be available.
     *
     * \a password the password to set
     */
    void setPassword(const QString &password);

    /*!
     * Presets a number of login+password pairs that the user can choose from.
     *
     * The passwords can be empty if you simply want to offer usernames to choose from.
     *
     * This require the flag ShowUsernameLine to be set in the constructoe, and not the flag UsernameReadOnly
     *
     * \a knownLogins map of known logins: the keys are usernames, the values are passwords.
     */
    void setKnownLogins(const QMap<QString, QString> &knownLogins);

    void accept() override;

    /*!
     * Returns the button box used in the dialog.
     * This can be used to add new buttons.
     *
     * \since 5.0
     */
    QDialogButtonBox *buttonBox() const;

    /*!
     * Sets contextual help for the username input field. This displays a
     * somewhat visual hint in the UI giving very visible access to a whats-this
     * style input description for the user name line. This is particularly useful
     * when the user name may require or support special input syntax.
     * For example windows-like auth dialogs supports multiple different logon
     * name syntax.
     * \since 5.76
     */
    void setUsernameContextHelp(const QString &help);

#if KWIDGETSADDONS_ENABLE_DEPRECATED_SINCE(6, 0)
    /*!
     * Whether to show the visibility trailing action in the line edit.
     * Default is @c true. This can be used to honor the lineedit_reveal_password
     * kiosk key, for example:
     * \code
     * dlg->setRevealPasswordAvailable(KAuthorized::authorize(QStringLiteral("lineedit_reveal_password")));
     * \endcode
     * \deprecated[6.0]
     * \since 5.84
     */
    [[deprecated("Use setRevealPasswordMode instead.")]] void setRevealPasswordAvailable(bool reveal);

    /*!
     * Whether the visibility trailing action in the line edit is visible.
     * \since 5.84
     * \deprecated[6.0]
     */
    [[deprecated("Use revealPasswordMode instead.")]] bool isRevealPasswordAvailable() const;
#endif

    /*!
     * Return when the reveal password button is visible.
     * \since 6.0
     */
    KPassword::RevealMode revealPasswordMode() const;

    /*!
     * Set when the reveal password button will be visible.
     *
     * The default is RevealMode::OnlyNew and the reveal password button will
     * only be visible when entering a new password.
     *
     * This can be used to honor the lineedit_reveal_password kiosk key, for example:
     *
     * \code
     * if (KAuthorized::authorize(QStringLiteral("lineedit_reveal_password"))) {
     *     passwordDialog.setRevealPasswordMode(KPassword::RevealMode::OnlyNew);
     * } else {
     *     passwordDialog.setRevealPasswordMode(KPassword::RevealMode::Never);
     * }
     * \endcode
     * \since 6.0
     */
    void setRevealPasswordMode(KPassword::RevealMode revealPasswordMode);

Q_SIGNALS:
    /*!
     * emitted when the dialog has been accepted
     *
     * \a password  the entered password
     *
     * \a keep true if the "remember password" checkbox was checked, false otherwise.  false if ShowKeepPassword was not set in the constructor
     */
    void gotPassword(const QString &password, bool keep);

    /*!
     * emitted when the dialog has been accepted, and ShowUsernameLine was set on the constructor
     *
     * \a username the entered username
     *
     * \a password  the entered password
     *
     * \a keep true if the "remember password" checkbox was checked, false otherwise.  false if ShowKeepPassword was not set in the constructor
     */
    void gotUsernameAndPassword(const QString &username, const QString &password, bool keep);

protected:
    /*!
     * Virtual function that can be overridden to provide password
     * checking in derived classes. It should return \c true if the
     * password is valid, \c false otherwise.
     */
    virtual bool checkPassword();

private:
    friend class KPasswordDialogPrivate;
    std::unique_ptr<class KPasswordDialogPrivate> const d;

    Q_DISABLE_COPY(KPasswordDialog)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KPasswordDialog::KPasswordDialogFlags)

#endif
