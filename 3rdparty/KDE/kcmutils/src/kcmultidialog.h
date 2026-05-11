/*
    SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
    SPDX-FileCopyrightText: 2003 Daniel Molkentin <molkentin@kde.org>
    SPDX-FileCopyrightText: 2003, 2006 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCMULTIDIALOG_H
#define KCMULTIDIALOG_H

#include <QScrollArea>
#include <QScrollBar>

#include <KPageDialog>
#include <KPluginMetaData>

#include "kcmutils_export.h"

class KCMultiDialogPrivate;

/*!
 * \brief A class that offers a KPageDialog containing config modules.
 * \inmodule KCMUtils
 */
class KCMUTILS_EXPORT KCMultiDialog : public KPageDialog
{
    Q_OBJECT

public:
    /*!
     * \brief Constructs a new KCMultiDialog.
     *
     * \a parent The parent widget.
     **/
    explicit KCMultiDialog(QWidget *parent = nullptr);

    ~KCMultiDialog() override;

    /*!
     * \brief Adds a module to the dialog. Its position will be determined based on the \c X-KDE-Weight value.
     *
     * \a metaData KPluginMetaData that will be used to load the plugin.
     *
     * \a args The arguments that should be given to the KCModule when it is created.
     */
    KPageWidgetItem *addModule(const KPluginMetaData &metaData, const QVariantList &args = {});

    /*!
     * \brief Removes all modules from the dialog.
     */
    void clear();

    /*!
     * \brief Whether to \a show or hide an indicator when settings have changed from their default value.
     *
     * \since 6.0
     */
    void setDefaultsIndicatorsVisible(bool show);

Q_SIGNALS:
    /*!
     * \brief Emitted after all KCModules have been told to save their configuration.
     *
     * The applyClicked and okClicked signals are emitted before the
     * configuration is saved.
     */
    void configCommitted();

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

protected Q_SLOTS:
    /*!
     * \brief This slot is called when the user presses the "Default" Button.
     *
     * You can reimplement it if needed.
     *
     * \note Make sure you call the original implementation.
     **/
    void slotDefaultClicked();

    /*!
     * \brief This slot is called when the user presses the "Reset" Button.
     * You can reimplement it if needed.
     *
     * \note Make sure you call the original implementation.
     */
    void slotUser1Clicked();

    /*!
     * \brief This slot is called when the user presses the "Apply" Button.
     * You can reimplement it if needed.
     *
     * \note Make sure you call the original implementation.
     **/
    void slotApplyClicked();

    /*!
     * \brief This slot is called when the user presses the "OK" Button.
     * You can reimplement it if needed.
     *
     * \note Make sure you call the original implementation.
     **/
    void slotOkClicked();

    /*!
     * \brief This slot is called when the user presses the "Help" Button.
     *
     * It reads the X-DocPath field of the currently selected KControl
     * module's .desktop file to find the path to the documentation,
     * which it then attempts to load.
     *
     * You can reimplement this slot if needed.
     *
     * \note Make sure you call the original implementation.
     **/
    void slotHelpClicked();

private:
    friend KCMultiDialogPrivate;
    const std::unique_ptr<KCMultiDialogPrivate> d;
    bool eventFilter(QObject *watched, QEvent *event) override;
};

class UnboundScrollArea : public QScrollArea
{
    Q_OBJECT
public:
    QSize sizeHint() const override
    {
        if (widget()) {
            // Try to avoid horizontal scrollbar, which just scrolls a scrollbar width.
            // We always need to reserve space for the vertical scroll bar,
            // because we can’t know here whether vertical scrolling will be used.
            QSize withScrollbar = widget()->sizeHint();
            withScrollbar.rwidth() += verticalScrollBar()->sizeHint().width() + 4;
            return withScrollbar;
        } else {
            return QScrollArea::sizeHint();
        }
    }

    explicit UnboundScrollArea(QWidget *w)
        : QScrollArea(w)
    {
    }
    ~UnboundScrollArea() override = default;
};

#endif
