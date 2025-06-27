/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// Widgets include.
#include "folder_chooser.hpp"

// Qt include.
#include <QBitmap>
#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QVector>

// C++ include.
#include <utility>

namespace MdShared
{

void createMask(QBitmap &m, QPainterPath &path, int width, int height)
{
    m.fill(Qt::color0);
    QPainter p(&m);
    p.setPen(Qt::color1);
    p.setBrush(Qt::color1);

    const auto h = height / 2;
    path.lineTo(height / 2, h);
    path.lineTo(0, h * 2);
    path.lineTo(height / 2 + width, h * 2);
    path.lineTo(height + width, h);
    path.lineTo(height / 2 + width, 0);
    path.closeSubpath();
    p.drawPath(path);
}

//
// FolderWidget
//

//! One folder.
class FolderWidget : public QWidget
{
    Q_OBJECT

signals:
    void clicked(int idx);

public:
    FolderWidget(const QString &folderName, int idx, QWidget *parent)
        : QWidget(parent)
        , m_folderName(folderName)
        , m_idx(idx)
    {
        QFontMetrics fm(font());
        m_height = fm.height();
        m_width = fm.horizontalAdvance(folderName);

        QBitmap m(desiredSize());
        QPainterPath path;

        createMask(m, path, m_width, m_height);
        m_path = path;
        setMask(m);
        setToolTip(tr("Choose working directory..."));
        setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        resize(desiredSize());
        setMinimumSize(desiredSize());
        setMaximumSize(desiredSize());
    }

    ~FolderWidget() override
    {
    }

    QSize sizeHint() const override
    {
        return {m_width + m_height + 2, m_height + 1};
    }

    int offset() const
    {
        return m_height / 2;
    }

    const QString &folderName() const
    {
        return m_folderName;
    }

public slots:
    void setSelected(bool on = true, bool notify = true)
    {
        m_selected = on;

        if (m_selected && notify) {
            emit clicked(m_idx);
        }

        update();
    }

protected:
    void paintEvent(QPaintEvent *e) override
    {
        QPainter p(this);
        p.setPen(m_selected ? palette().color(QPalette::HighlightedText) : palette().color(QPalette::WindowText));
        p.setBrush(m_selected ? palette().color(QPalette::Highlight) : palette().color(QPalette::Window));
        p.drawPath(m_path);
        p.drawText(m_height / 2 + 3, 0, m_width, m_height, 0, m_folderName);
    }

    void mousePressEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton) {
            m_pressed = true;
        }

        e->accept();
    }

    void mouseReleaseEvent(QMouseEvent *e) override
    {
        if (e->button() == Qt::LeftButton && m_pressed) {
            m_pressed = false;

            setSelected();
        }

        e->accept();
    }

private:
    QSize desiredSize() const
    {
        return {m_width + m_height + 2, m_height + 1};
    }

private:
    QString m_folderName;
    int m_idx = -1;
    int m_height = -1;
    int m_width = -1;
    QPainterPath m_path;
    bool m_selected = false;
    bool m_pressed = false;
}; // class FolderWidget

//
// FolderChooserPrivate
//

struct FolderChooserPrivate {
    FolderChooserPrivate(FolderChooser *parent)
        : m_q(parent)
    {
    }

    void init()
    {
        for (const auto &f : std::as_const(m_path)) {
            auto folder = new FolderWidget(f, m_folders.size(), m_q);
            folder->show();
            m_folders.push_back(folder);
            folder->move(m_width, 0);
            m_width += folder->sizeHint().width() - folder->offset() - 1;
            QObject::connect(folder, &FolderWidget::clicked, m_q, &FolderChooser::onClicked);
        }

        if (!m_folders.isEmpty()) {
            m_width += m_folders.front()->offset();

            QBitmap m(m_width, m_height);
            QPainterPath path;

            createMask(m, path, m_width - m_height + 1, m_height - 1);
            m_q->setMask(m);
        }
    }

    void deinit()
    {
        m_width = 0;

        for (const auto &w : std::as_const(m_folders)) {
            w->deleteLater();
        }

        m_folders.clear();
        m_path.clear();
    }

    //! Parent.
    FolderChooser *m_q = nullptr;
    //! Path.
    QStringList m_path;
    //! Folder widgets.
    QVector<FolderWidget*> m_folders;
    //! Width.
    int m_width = 0;
    //! Height.
    int m_height = 0;
    //! Mask.
    QBitmap m_mask;
    //! Current index.
    int m_idx = -1;
}; // struct FolderChooserPrivate

//
// FolderChooser
//

FolderChooser::FolderChooser(QWidget *parent)
    : QWidget(parent)
    , m_d(new FolderChooserPrivate(this))
{
    auto folder = new FolderWidget(QStringLiteral("/"), 0, this);
    m_d->m_height = folder->sizeHint().height();
    folder->deleteLater();
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

FolderChooser::~FolderChooser()
{
}

QSize FolderChooser::sizeHint() const
{
    return {m_d->m_width + 1, m_d->m_height};
}

QStringList FolderChooser::splitPath(const QString &path)
{
    auto tmp = path;
    tmp.replace(QLatin1Char('\\'), QLatin1Char('/'));
    auto splitted = tmp.split(QLatin1Char('/'));

    if (!splitted.isEmpty() && splitted.at(0).isEmpty()) {
        splitted[0] = QStringLiteral("/");
    }

    return splitted;
}

QString FolderChooser::currentPath() const
{
    QString path;

    for (int i = 0; i < m_d->m_folders.size(); ++i) {
        m_d->m_folders.at(i)->setSelected(i == m_d->m_idx, false);

        if (i <= m_d->m_idx) {
            path.append(m_d->m_folders.at(i)->folderName());

            if (m_d->m_folders.at(i)->folderName() != QStringLiteral("/")) {
                path.append(QStringLiteral("/"));
            }
        }
    }

    if (path.size() > 1) {
        path.removeLast();
    }

    return path;
}

void FolderChooser::setPath(const QString &path)
{
    m_d->deinit();
    m_d->m_path = splitPath(path);
    m_d->init();

    if (!m_d->m_folders.isEmpty()) {
        m_d->m_folders.back()->setSelected(true, false);
        m_d->m_idx = m_d->m_folders.size() - 1;
    } else {
        m_d->m_idx = -1;
    }

    resize(sizeHint());
    setMinimumSize(sizeHint());
    setMaximumSize(sizeHint());
}

void FolderChooser::setPopup(bool on)
{
    setWindowFlag(Qt::Popup, on);
}

void FolderChooser::emulateClick(int idx)
{
    onClicked(idx);
}

void FolderChooser::onClicked(int idx)
{
    m_d->m_idx = idx;

    const auto path = currentPath();

    if (!path.isEmpty()) {
        emit pathSelected(path);
    }
}

} /* namespace MdShared */

#include "folder_chooser.moc"
