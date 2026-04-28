/*
    SPDX-FileCopyrightText: 1997 Bernd Johannes Wuebben <wuebben@kde.org>
    SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
    SPDX-FileCopyrightText: 1999 Mario Weilguni <mweilguni@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef K_FONT_CHOOSER_DIALOG_H
#define K_FONT_CHOOSER_DIALOG_H

#include <QDialog>
#include <QStringList>

#include <kfontchooser.h>

#include <memory>

class QFont;

class KFontChooserDialogPrivate;

/*!
 * \class KFontChooserDialog
 * \inmodule KWidgetsAddons
 *
 * \brief A font selection dialog.
 *
 * The KFontChooserDialog provides a dialog for interactive font selection.
 * It is basically a thin wrapper around the KFontChooser widget (the latter
 * can also be used standalone). In most cases, the simplest use of this
 * class is the static method KFontChooserDialog::getFont(), which shows
 * the dialog, allows the user to select a font, and returns when the
 * dialog is closed.
 *
 * Features offered by KFontChooserDialog/KFontChooser:
 * \list
 * \li The ability to set decimal font sizes (e.g. "12.1")
 * \li When selecting an initial font, if the styleName property of that font
 *   isn't set, the dialog will try and select the correct font style from the
 *   styles list
 * \li The ability to set multiple fonts at once, for an example of this functionality
 *   see "Adjust All Fonts" in the Fonts KCM in Systemsettings; and you can change
 *   the font family, style or size separately
 * \li Discarding the styleName property when closing the dialog for "Regular" font
 *   styles, since it doesn't make sense to set that property for such fonts and
 *   this allows setBold(true) to work correctly for more details see:
 *   https://bugreports.qt.io/browse/QTBUG-63792
 *   https://bugs.kde.org/show_bug.cgi?id=378523
 * \endlist
 *
 * Example, using the static getFont() method:
 *
 * \code
 *      QFont myFont;
 *      int result = KFontChooserDialog::getFont(myFont);
 *      if (result == QDialog::Accepted) {
 *            ...
 *      }
 * \endcode
 *
 * Another example, this time showing the dialog with show() (or open()):
 * \code
 *      KFontChooserDialog *fontDlg = new KFontChooserDialog(KFontChooser::NoDisplayFlags, this);
 *      // Delete the dialog when it's closed
 *      fontDlg->setAttribute(Qt::WA_DeleteOnClose);
 *
 *      connect(fontDlg, &QDialog::accepted, this, [fontDlg]() {
 *          // Get the selected font via fontDlg->font() and apply it, save it to
 *          // the settings... etc.
 *      });
 *
 *      fontDlg->show();
 * \endcode
 *
 * \image kfontchooserdialog.png "KFontChooserDialog"
 *
 * \since 5.69
 */
class KWIDGETSADDONS_EXPORT KFontChooserDialog : public QDialog
{
    Q_OBJECT

public:
    /*!
     * Constructs a font selection dialog.
     *
     * \a flags flags to define how the font chooser is displayed
     *
     * \a parent parent widget of the dialog, if any, the dialog will be centered relative to it
     */
    explicit KFontChooserDialog(const KFontChooser::DisplayFlags &flags = KFontChooser::NoDisplayFlags, QWidget *parent = nullptr);

    ~KFontChooserDialog() override;

    /*!
     * Sets the currently selected font in the dialog.
     *
     * \a font the font to select
     *
     * \a onlyFixed if \c true, the font list will show only fixed width (monospace)
     *        fonts, otherwise all available fonts are shown
     */
    void setFont(const QFont &font, bool onlyFixed = false);

    /*!
     * Returns the currently selected font in the dialog
     */
    QFont font() const;

    /*!
     * Creates a modal font dialog, lets the user choose a font, and returns when
     * the dialog is closed.
     *
     * \a theFont a reference to the font to write the chosen font into
     *
     * \a flags flags to define how the font chooser is displayed
     *
     * \a parent parent widget of the dialog, if any, the dialog will be centered relative to it
     *
     * Returns QDialog::result()
     */
    static int getFont(QFont &theFont, const KFontChooser::DisplayFlags &flags = KFontChooser::NoDisplayFlags, QWidget *parent = nullptr);

    /*!
     * Creates a modal font difference dialog, lets the user choose a selection
     * of changes that should be made to a set of fonts, and returns when the
     * dialog is closed. Useful for choosing slight adjustments to the font set
     * when the user would otherwise have to manually edit a number of fonts.
     *
     * \a theFont a reference to the font to write the chosen font into
     *
     * \a flags flags to define how the font chooser is displayed
     *
     * \a diffFlags a reference to the integer bitmask into which the chosen
     *        difference selection bitmask should be written.
     *        Check the bitmask afterwards like:
     *        \code
     *        if ( diffFlags & KFontChooser::FontDiffFamily )  {
     *            [...]
     *        }
     *        if ( diffFlags & KFontChooser::FontDiffStyle ) {
     *            [...]
     *        }
     *        if ( diffFlags & KFontChooser::FontDiffSize ) {
     *            [...]
     *        }
     *        \endcode
     *
     * \a parent parent widget of the dialog, if any, the dialog will be centered relative to it
     *
     * Returns QDialog::result()
     */
    static int getFontDiff(QFont &theFont,
                           KFontChooser::FontDiffFlags &diffFlags,
                           const KFontChooser::DisplayFlags &flags = KFontChooser::NoDisplayFlags,
                           QWidget *parent = nullptr);

Q_SIGNALS:
    /*!
     * Emitted whenever the currently selected font changes.
     * Connect to this to monitor the font as it is selected if you are
     * not running modal.
     */
    void fontSelected(const QFont &font);

private:
    std::unique_ptr<KFontChooserDialogPrivate> const d;

    Q_DISABLE_COPY(KFontChooserDialog)
};

#endif
