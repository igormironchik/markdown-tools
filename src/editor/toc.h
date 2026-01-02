/*
    SPDX-FileCopyrightText: 2026 Igor Mironchik <igor.mironchik@gmail.com>
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

//! ToC tree view item data.
struct UnitData {
    //! Text.
    QString m_text;
    //! Is it a code?
    bool m_code = false;
    //! Is RTL?
    bool m_isRightToLeft = false;
};

//! ToC tree view full data.
struct StringData {
    StringData(const QString &t,
               bool c,
               bool rtl);

    //! Non-splitted text.
    UnitData m_data;
    //! Already splitted text.
    QVector<QPair<QString, bool>> m_splittedText;
    //! Rectangles for code.
    QVector<QRect> m_backgroundRects;
    //! Text rects with corresponding data.
    QVector<QPair<QRect, UnitData>> m_textRects;
}; // struct StringData

//! Vector of string data for ToC tree view item.
using StringDataVec = QVector<StringData>;

//
// TocData
//

//! Data for ToC item in the model.
struct TocData {
    TocData(const StringDataVec &t,
            long long int l,
            int v,
            const QString &id,
            TocData *p = nullptr);

    //! \return COncatenated text.
    QString concatenatedText() const;

    //! Text data.
    StringDataVec m_text;
    //! ID of item.
    QString m_id;
    //! Line number.
    long long int m_line = -1;
    //! Heading level.
    int m_level = -1;
    //! Pointer to parent.
    TocData *m_parent = nullptr;
    //! Children.
    std::vector<std::shared_ptr<TocData>> m_children;
}; // struct TocData

//
// TocModel
//

struct TocModelPrivate;

//! Model for ToC.
class TocModel final : public QAbstractItemModel
{
public:
    explicit TocModel(QObject *parent);
    ~TocModel() override;

    //! Add top-level item.
    void addTopLevelItem(const StringDataVec &text,
                         long long int line,
                         int level,
                         const QString &label);
    //! Add child item.
    void addChildItem(const QModelIndex &parent,
                      const StringDataVec &text,
                      long long int line,
                      int level,
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
    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;
    //! Set data.
    bool setData(const QModelIndex &index,
                 const QVariant &value,
                 int role = Qt::EditRole) override;
    //! \return Flags.
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    //! \return Header data.
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    //! \return Index.
    QModelIndex index(int row,
                      int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    //! \return Parent index.
    QModelIndex parent(const QModelIndex &index) const override;

private:
    QScopedPointer<TocModelPrivate> m_d;

    Q_DISABLE_COPY(TocModel)
}; // class TocModel

} /* namespace MdEditor */
