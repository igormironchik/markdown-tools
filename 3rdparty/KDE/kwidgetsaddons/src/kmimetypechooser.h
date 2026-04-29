/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2001-2004 Anders Lund <anders@alweb.dk>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KMIMETYPE_CHOOSER_H
#define KMIMETYPE_CHOOSER_H

#include <QDialog>
#include <QWidget>
#include <kwidgetsaddons_export.h>
#include <memory>

/*!
 * \class KMimeTypeChooser
 * \inmodule KWidgetsAddons
 *
 * \brief This widget provides a checkable list of all available MIME types.
 *
 * Presented
 * as a treeview, with the MIME type comment and glob patterns as individual columns.
 *
 * It allows users to edit a MIME type by launching a MIME type editor (if it's
 * available on the system).
 *
 * When the user clicks the OK button, a list of the selected MIME type names
 * and associated glob patterns can be retrieved respectively; those lists can
 * be used to populate a text line edit, or set a configuration entry... etc.
 */
class KWIDGETSADDONS_EXPORT KMimeTypeChooser : public QWidget
{
    Q_OBJECT

public:
    /*!
     * Buttons and data for display.
     *
     * \value Comments Show the MIME type comment (e.g. "HTML Document") in a column
     * \value Patterns Show the MIME types glob patterns (e.g. "*.html;*.htm") in a column
     * \value EditButton Show the "Edit" button, allowing to edit the selected type
     */
    enum Visuals {
        Comments = 1,
        Patterns = 2,
        EditButton = 4
    };
    /*!
     * Create a new KMimeTypeChooser.
     *
     * \a text A plain text line to display above the list
     *
     * \a selectedMimeTypes A list of MIME type names, these will be initially selected
     *        in the list (provided they exist)
     *
     * \a visuals OR'd KMimetypeChooser::Visuals enum values to to set whether to
     *        show the MIME type comment and glob patterns columns and an Edit button,
     *        respectively, or not
     *
     * \a defaultGroup The group (e.g. "text") to expand in the treeview when no groups
     *        are selected. If not provided, no group is expanded by default.
     *        If \a groupsToShow is provided and it doesn't include \a defaultGroup, this
     *        parameter is ignored
     *
     * \a groupsToShow a list of MIME type groups to show. If empty, all groups are shown
     *
     * \a parent The parent widget to use
     */
    explicit KMimeTypeChooser(const QString &text = QString(),
                              const QStringList &selectedMimeTypes = QStringList(),
                              const QString &defaultGroup = QString(),
                              const QStringList &groupsToShow = QStringList(),
                              int visuals = Comments | Patterns | EditButton,
                              QWidget *parent = nullptr);
    ~KMimeTypeChooser() override;

    /*!
     * Returns a list of all selected MIME type names
     */
    QStringList mimeTypes() const;
    /*!
     * Returns a list of the filename glob patterns associated with all selected MIME types
     */
    QStringList patterns() const;

private:
    std::unique_ptr<class KMimeTypeChooserPrivate> const d;
};

/*!
 * \class KMimeTypeChooserDialog
 * \inmodule KWidgetsAddons
 * \inheaderfile KMimeTypeChooser
 *
 * \brief A dialog to select MIME types from the list of available ones on the system.
 *
 * This dialog embeds KMimeTypeChooser widget, presenting a checkable tree list of
 * MIME types, each with its associated icon, and optionally associated glob patterns
 * (displayed in a separate column); also an optional Edit button to launch a
 * MIME type editor to edit the selected MIME type.
 *
 * Here is an example, using the dialog to set the text of two line edits with the
 * list of MIME type names and glob patterns, respectively, of the MIME types that
 * the user has selected:
 *
 * \code
 *    QLineEdit *leMimetypes = new QLineEdit();
 *    QLineEdit *leGlobPatterns = new QLineEdit();
 *    [...]
 *    QString textLine = i18n("Select MIME types");
 *    QStringList mimeList = QStringList::split(QRegularExpression("\\s*;\\s*"), leMimetypes->text());
 *    KMimeTypeChooserDialog dlg(i18n("Select MIME Types"), textLine, mimeList, "text", this);
 *    if (dlg.exec() == QDialog::Accepted) {
 *        leMimetypes->setText(dlg.chooser()->mimeTypes().join(";"));
 *        leGlobPatterns->setText(dlg.chooser()->patterns().join(";"));
 *    }
 * \endcode
 *
 * \image kmimetypechooserdialog.png "KMimeTypeChooserDialog in action"
 */
class KWIDGETSADDONS_EXPORT KMimeTypeChooserDialog : public QDialog
{
    Q_OBJECT
public:
    /*!
     * Create a KMimeTypeChooser dialog.
     *
     * \a title The title of the dialog
     *
     * \a text A plain text line to display above the list
     *
     * \a selectedMimeTypes A list of MIME type names, these will be initially selected
     *        in the list, provided they exist
     *
     * \a visuals OR'd KMimetypeChooser::Visuals enum values to to set whether to
     *        show the MIME type comment and glob patterns columns and an Edit button,
     *        respectively, or not
     *
     * \a defaultGroup The group (e.g. "text") to expand in the treeview when no
     *        groups are selected. If not provided, no group is expanded by default
     *        If \a groupsToShow is provided and it doesn't include \a defaultGroup,
     *        this parameter is ignored
     *
     * \a groupsToShow A list of MIME type groups to show. If empty, all groups are shown
     *
     * \a parent The parent widget to use
     */
    explicit KMimeTypeChooserDialog(const QString &title = QString(),
                                    const QString &text = QString(),
                                    const QStringList &selectedMimeTypes = QStringList(),
                                    const QString &defaultGroup = QString(),
                                    const QStringList &groupsToShow = QStringList(),
                                    int visuals = KMimeTypeChooser::Comments | KMimeTypeChooser::Patterns | KMimeTypeChooser::EditButton,
                                    QWidget *parent = nullptr);

#if KWIDGETSADDONS_ENABLE_DEPRECATED_SINCE(6, 24)
    /*!
     * \overload
     * \deprecated[6.24]
     * use the other KMimeTypeChooserDialog constructor instead.
     */
    KWIDGETSADDONS_DEPRECATED_VERSION(6, 24, "Use the other KMimeTypeChooserDialog constructor instead")
    KMimeTypeChooserDialog(const QString &title,
                           const QString &text,
                           const QStringList &selectedMimeTypes,
                           const QString &defaultGroup,
                           QWidget *parent);
#endif

    ~KMimeTypeChooserDialog() override;

    /*!
     * Returns a pointer to the KMimeTypeChooser widget
     */
    KMimeTypeChooser *chooser();

    QSize sizeHint() const override;

private:
    std::unique_ptr<class KMimeTypeChooserDialogPrivate> const d;
};
#endif // _KMIMETYPE_CHOOSER_H_
