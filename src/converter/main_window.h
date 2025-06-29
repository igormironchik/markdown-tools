/*
    SPDX-FileCopyrightText: 2024-2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-pdf include.
#include "ui_main_window.h"
#include "syntax.h"

// Qt include.
#include <QMainWindow>
#include <QScopedPointer>
#include <QThread>
#include <QWidget>
#include <QSettings>

// C++ include.
#include <memory>

//! Namespace for converter of Markdown to PDF.
namespace MdPdf
{

class Cfg;

//
// MainWidget
//

//! Main widget.
class MainWidget final : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent);
    ~MainWidget() override;

    //! Set Markdown file name.
    void setMarkdownFile(const QString &fileName);
    //! Save configuration.
    void saveCfg(QSettings &cfg) const;
    //! Apply configuration.
    void applyCfg(QSettings &cfg);

private slots:
    void changeLinkColor();
    void changeBorderColor();
    void selectMarkdown();
    void process();
    void codeFontSizeChanged(int i);
    void textFontSizeChanged(int i);
    void mmButtonToggled(bool on);
    void textFontChanged(const QFont &f);
    void codeFontChanged(const QFont &f);

private:
    void changeStateOfStartButton();

protected:
    void showEvent(QShowEvent *event) override;

private:
    QScopedPointer<Ui::MainWindow> m_ui;
    QThread *m_thread;
    bool m_textFontOk;
    bool m_codeFontOk;
    std::shared_ptr<Syntax> m_syntax;
    bool m_alreadyShown = false;

    Q_DISABLE_COPY(MainWidget)
}; // class MainWindow

//
// MainWindow
//

//! Main window.
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow() override = default;

    //! Set Markdown file name.
    void setMarkdownFile(const QString &fileName);

protected:
    void closeEvent(QCloseEvent *event) override;
    void showEvent(QShowEvent *event) override;

private slots:
    //! About.
    void about();
    //! About Qt.
    void aboutQt();
    //! Licenses.
    void licenses();
    //! Quit.
    void quit();

private:
    QString configFileName(bool inPlace) const;
    void readCfg();
    void saveCfg();

private:
    MainWidget *ui = nullptr;
    bool m_alreadyShown = false;
}; // class MainWindow

} /* namespace MdPdf */
