/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// Qt include.
#include <QAbstractItemModel>
#include <QRect>
#include <QScopedPointer>

namespace MdEditor
{

//
// StringData
//

struct UnitData {
    QString m_text;
    bool m_code = false;
    bool m_isRightToLeft = false;
};

struct StringData {
    StringData(const QString &t, bool c, bool rtl);

    UnitData m_data;
    QVector<QPair<QString, bool>> m_splittedText;
    QVector<QRect> m_backgroundRects;
    QVector<QPair<QRect, UnitData>> m_textRects;
}; // struct StringData

using StringDataVec = QVector<StringData>;

//
// TocData
//

struct TocData {
    TocData(const StringDataVec &t, long long int l, int v, const QString &id, TocData *p = nullptr);

    QString concatenatedText() const;

    StringDataVec m_text;
    QString m_id;
    long long int m_line = -1;
    int m_level = -1;
    TocData *m_parent = nullptr;
    std::vector<std::shared_ptr<TocData>> m_children;
}; // struct TocData

//
// TocModel
//

struct TocModelPrivate;

//! Model for TOC.
class TocModel final : public QAbstractItemModel
{
public:
    explicit TocModel(QObject *parent);
    ~TocModel() override;

    //! Add top-level item.
    void addTopLevelItem(const StringDataVec &text, long long int line, int level,
                         const QString &label);
    //! Add child item.
    void addChildItem(const QModelIndex &parent, const StringDataVec &text, long long int line, int level,
                      const QString &label);
    //! Clear.
    void clear();
    //! \return Level.
    int level(const QModelIndex &index) const;
    //! \return Line number.
    int lineNumber(const QModelIndex &index) const;
    //! \return String data.
    StringDataVec &stringData(const QModelIndex &index) const;

    //! \return Count of the rows.
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    //! \return Count of the columns.
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    //! \return Data by the given index and role.
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    //! Set data.
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    //! \return Flags.
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    //! \return Header data.
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    //! \return Index.
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    //! \return Parent index.
    QModelIndex parent(const QModelIndex &index) const override;

private:
    QScopedPointer<TocModelPrivate> m_d;

    Q_DISABLE_COPY(TocModel)
}; // class TocModel

} /* namespace MdEditor */
