/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Rafael Fernández López <ereslibre@kde.org>
    SPDX-FileCopyrightText: 2007 John Tapsell <tapsell@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KCATEGORIZEDSORTFILTERPROXYMODEL_H
#define KCATEGORIZEDSORTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>
#include <memory>

#include <kitemviews_export.h>
class KCategorizedSortFilterProxyModelPrivate;

class QItemSelection;

/*!
 * \class KCategorizedSortFilterProxyModel
 * \inmodule KItemViews
 *
 * \brief This class lets you categorize a view. It is meant to be used along with
 * KCategorizedView class.
 *
 * In general terms all you need to do is to reimplement subSortLessThan() and
 * compareCategories() methods. In order to make categorization work, you need
 * to also call setCategorizedModel() class to enable it, since the categorization
 * is disabled by default.
 *
 * \sa KCategorizedView
 */
class KITEMVIEWS_EXPORT KCategorizedSortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    /*!
     * \value CategoryDisplayRole This role is used for asking the category to a given index
     * \value CategorySortRole This role is used for sorting categories. You can return a string or a long long value. Strings will be sorted alphabetically
     * while long long will be sorted by their value. Please note that this value won't be shown on the view, is only for sorting purposes. What will be shown
     * as "Category" on the view will be asked with the role CategoryDisplayRole.
     */
    enum AdditionalRoles {
        // Note: use printf "0x%08X\n" $(($RANDOM*$RANDOM))
        // to define additional roles.
        CategoryDisplayRole = 0x17CE990A,
        CategorySortRole = 0x27857E60,
    };

    /*!
     *
     */
    KCategorizedSortFilterProxyModel(QObject *parent = nullptr);
    ~KCategorizedSortFilterProxyModel() override;

    /*!
     * Overridden from QSortFilterProxyModel. Sorts the source model using
     * \a column for the given \a order.
     */
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

    /*!
     * Returns whether the model is categorized or not. Disabled by default.
     */
    bool isCategorizedModel() const;

    /*!
     * Enables or disables the categorization feature.
     *
     * \a categorizedModel whether to enable or disable the categorization feature.
     */
    void setCategorizedModel(bool categorizedModel);

    /*!
     * Returns the column being used for sorting.
     */
    int sortColumn() const;

    /*!
     * Returns the sort order being used for sorting.
     */
    Qt::SortOrder sortOrder() const;

    /*!
     * Set if the sorting using CategorySortRole will use a natural comparison
     * in the case that strings were returned. If enabled, QCollator
     * will be used for sorting.
     *
     * \a sortCategoriesByNaturalComparison whether to sort using a natural comparison or not.
     */
    void setSortCategoriesByNaturalComparison(bool sortCategoriesByNaturalComparison);

    /*!
     * Returns whether it is being used a natural comparison for sorting. Enabled by default.
     */
    bool sortCategoriesByNaturalComparison() const;

protected:
    /*!
     * Overridden from QSortFilterProxyModel. If you are subclassing
     * KCategorizedSortFilterProxyModel, you will probably not need to reimplement this
     * method.
     *
     * It calls compareCategories() to sort by category.  If the both items are in the
     * same category (i.e. compareCategories returns 0), then subSortLessThan is called.
     *
     * Returns true if the item \a left is less than the item \a right when sorting.
     *
     * \warning You usually won't need to reimplement this method when subclassing
     *          from KCategorizedSortFilterProxyModel.
     */
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

    /*!
     * This method has a similar purpose as lessThan() has on QSortFilterProxyModel.
     * It is used for sorting items that are in the same category.
     *
     * Returns true if the item \a left is less than the item \a right when sorting.
     */
    virtual bool subSortLessThan(const QModelIndex &left, const QModelIndex &right) const;

    /*!
     * This method compares the category of the \a left index with the category
     * of the \a right index.
     *
     * Internally and if not reimplemented, this method will ask for \a left and
     * \a right models for role CategorySortRole. In order to correctly sort
     * categories, the data() method of the model should return a qlonglong (or numeric) value, or
     * a QString object. QString objects will be sorted with QString::localeAwareCompare if
     * sortCategoriesByNaturalComparison() is true.
     *
     * \note Please have present that:
     *       QString(QChar(QChar::ObjectReplacementCharacter)) >
     *       QString(QChar(QChar::ReplacementCharacter)) >
     *       [ all possible strings ] >
     *       QString();
     *
     *       This means that QString() will be sorted the first one, while
     *       QString(QChar(QChar::ObjectReplacementCharacter)) and
     *       QString(QChar(QChar::ReplacementCharacter)) will be sorted in last
     *       position.
     *
     * \warning Please note that data() method of the model should return always
     *          information of the same type. If you return a QString for an index,
     *          you should return always QStrings for all indexes for role CategorySortRole
     *          in order to correctly sort categories. You can't mix by returning
     *          a QString for one index, and a qlonglong for other.
     *
     * \note If you need a more complex layout, you will have to reimplement this
     *       method.
     *
     * Returns A negative value if the category of \a left should be placed before the
     *         category of \a right. 0 if \a left and \a right are on the same category, and
     *         a positive value if the category of \a left should be placed after the
     *         category of \a right.
     */
    virtual int compareCategories(const QModelIndex &left, const QModelIndex &right) const;

private:
    std::unique_ptr<KCategorizedSortFilterProxyModelPrivate> const d;
};

#endif // KCATEGORIZEDSORTFILTERPROXYMODEL_H
