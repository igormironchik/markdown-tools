/*
    This file is part of the KDE libraries
    SPDX-FileCopyrightText: 2000 David Faure <faure@kde.org>
    SPDX-FileCopyrightText: 2000 Alexander Neundorf <neundorf@kde.org>
    SPDX-FileCopyrightText: 2010 Sebastian Trueg <trueg@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KEDITLISTWIDGET_H
#define KEDITLISTWIDGET_H

#include <kwidgetsaddons_export.h>

#include <QWidget>
#include <memory>

class QLineEdit;
class QComboBox;
class QListView;
class QPushButton;
class QItemSelection;

/*!
 * \class KEditListWidget
 * \inmodule KWidgetsAddons
 *
 * \brief An editable listbox.
 *
 * This class provides an editable listbox, this means
 * a listbox which is accompanied by a line edit to enter new
 * items into the listbox and pushbuttons to add and remove
 * items from the listbox and two buttons to move items up and down.
 *
 * \image keditlistbox.png "KEditListWidget"
 *
 * \since 4.6
 */
class KWIDGETSADDONS_EXPORT KEditListWidget : public QWidget
{
    Q_OBJECT

    /*!
     * \property KEditListWidget::buttons
     */
    Q_PROPERTY(Buttons buttons READ buttons WRITE setButtons)

    /*!
     * \property KEditListWidget::items
     */
    Q_PROPERTY(QStringList items READ items WRITE setItems NOTIFY changed USER true)

    /*!
     * \property KEditListWidget::checkAtEntering
     */
    Q_PROPERTY(bool checkAtEntering READ checkAtEntering WRITE setCheckAtEntering)

public:
    /*!
     * \class KEditListWidget::CustomEditor
     * \inheaderfile KEditListWidget
     *
     * \brief Custom editor class
     */
    class KWIDGETSADDONS_EXPORT CustomEditor
    {
    public:
        /*!
         *
         */
        CustomEditor();
        CustomEditor(QWidget *repWidget, QLineEdit *edit);
        CustomEditor(QComboBox *combo);
        virtual ~CustomEditor();

        /*!
         *
         */
        void setRepresentationWidget(QWidget *repWidget);

        /*!
         *
         */
        void setLineEdit(QLineEdit *edit);

        /*!
         *
         */
        virtual QWidget *representationWidget() const;

        /*!
         *
         */
        virtual QLineEdit *lineEdit() const;

    private:
        friend class KEditListWidgetCustomEditorPrivate;
        std::unique_ptr<class KEditListWidgetCustomEditorPrivate> const d;

        Q_DISABLE_COPY(CustomEditor)
    };

public:
    /*!
     * Enumeration of the buttons, the listbox offers. Specify them in the
     * constructor in the buttons parameter, or in setButtons.
     *
     * \value Add
     * \value Remove
     * \value UpDown
     * \value All
     */
    enum Button {
        Add = 0x0001,
        Remove = 0x0002,
        UpDown = 0x0004,
        All = Add | Remove | UpDown,
    };
    Q_DECLARE_FLAGS(Buttons, Button)
    Q_FLAG(Buttons)

    /*!
     * Create an editable listbox.
     */
    explicit KEditListWidget(QWidget *parent = nullptr);

    /*!
     * Constructor which allows to use a custom editing widget
     * instead of the standard QLineEdit widget. E.g. you can use a
     * KUrlRequester or a QComboBox as input widget. The custom
     * editor must consist of a lineedit and optionally another widget that
     * is used as representation. A QComboBox or a KUrlRequester have a
     * QLineEdit as child-widget for example, so the QComboBox is used as
     * the representation widget.
     *
     * \sa KUrlRequester::customEditor(), setCustomEditor
     */
    KEditListWidget(const CustomEditor &customEditor, QWidget *parent = nullptr, bool checkAtEntering = false, Buttons buttons = All);

    ~KEditListWidget() override;

    /*!
     * Returns a pointer to the embedded QListView.
     */
    QListView *listView() const;
    /*!
     * Returns a pointer to the embedded QLineEdit.
     */
    QLineEdit *lineEdit() const;
    /*!
     * Returns a pointer to the Add button
     */
    QPushButton *addButton() const;
    /*!
     * Returns a pointer to the Remove button
     */
    QPushButton *removeButton() const;
    /*!
     * Returns a pointer to the Up button
     */
    QPushButton *upButton() const;
    /*!
     * Returns a pointer to the Down button
     */
    QPushButton *downButton() const;

    /*!
     * Returns the count of elements in the list
     */
    int count() const;

    /*!
     * Inserts a \a list of elements from the \a index element
     * If \a index is negative, the elements will be appended
     */
    void insertStringList(const QStringList &list, int index = -1);

    /*!
     * Inserts a \a text element at the \a index position
     * If \a index is negative, the element will be appended
     */
    void insertItem(const QString &text, int index = -1);

    /*!
     * Clears both the listbox and the line edit.
     */
    void clear();

    /*!
     * Returns the text at the \a index position
     */
    QString text(int index) const;

    /*!
     * Returns the currently selected item
     */
    int currentItem() const;

    /*!
     * Returns the currently selected item's text
     */
    QString currentText() const;

    /*!
     * Returns a list with the text of all items in the listbox
     */
    QStringList items() const;

    /*!
     * Clears the listbox and sets the contents to \a items
     */
    void setItems(const QStringList &items);

    /*!
     * Returns which buttons are visible
     */
    Buttons buttons() const;

    /*!
     * Specifies which \a buttons are visible
     */
    void setButtons(Buttons buttons);

    /*!
     * If \a check is true, after every character you type
     * in the line edit KEditListWidget will enable or disable
     * the Add-button, depending whether the current content of the
     * line edit is already in the listbox. Maybe this can become a
     * performance hit with large lists on slow machines.
     *
     * If \a check is false,
     * it will be checked if you press the Add-button. It is not
     * possible to enter items twice into the listbox.
     * Default is false.
     */
    void setCheckAtEntering(bool check);

    /*!
     * Returns true if check at entering is enabled.
     */
    bool checkAtEntering();

    /*!
     * Allows to use a custom editing widget
     * instead of the standard QLineEdit widget. E.g. you can use a
     * KUrlRequester or a QComboBox as input widget. The custom
     * editor must consist of a lineedit and optionally another widget that
     * is used as representation. A QComboBox or a KUrlRequester have a
     * QLineEdit as child-widget for example, so the QComboBox is used as
     * the representation widget.
     */
    void setCustomEditor(const CustomEditor &editor);

    bool eventFilter(QObject *o, QEvent *e) override;

Q_SIGNALS:
    void changed();

    /*!
     * This signal is emitted when the user adds a new string to the list,
     * the parameter is the added string.
     */
    void added(const QString &text);

    /*!
     * This signal is emitted when the user removes a string from the list,
     * the parameter is the removed string.
     */
    void removed(const QString &text);

private Q_SLOTS:
    KWIDGETSADDONS_NO_EXPORT void moveItemUp();
    KWIDGETSADDONS_NO_EXPORT void moveItemDown();
    KWIDGETSADDONS_NO_EXPORT void addItem();
    KWIDGETSADDONS_NO_EXPORT void removeItem();
    KWIDGETSADDONS_NO_EXPORT void enableMoveButtons(const QModelIndex &, const QModelIndex &);
    KWIDGETSADDONS_NO_EXPORT void typedSomething(const QString &text);
    KWIDGETSADDONS_NO_EXPORT void slotSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

private:
    friend class KEditListWidgetPrivate;
    std::unique_ptr<class KEditListWidgetPrivate> const d;

    Q_DISABLE_COPY(KEditListWidget)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KEditListWidget::Buttons)

#endif
