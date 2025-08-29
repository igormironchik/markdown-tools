/*
    SPDX-FileCopyrightText: 2025 Igor Mironchik <igor.mironchik@gmail.com>
    SPDX-License-Identifier: GPL-3.0-or-later
*/

// md-pdf include.
#include "ui_main_window.h"

// shared include.
#include "plugins_page.h"
#include "syntax.h"

// Qt include.
#include <QMainWindow>
#include <QScopedPointer>
#include <QSettings>
#include <QThread>
#include <QWidget>

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
    //! \return Plugins configuration.
    const MdShared::PluginsCfg &pluginsCfg() const;
    //! Set plugins configuration.
    void setPluginsCfg(const MdShared::PluginsCfg &cfg);
    //! \return Mark color.
    const QColor &markColor() const;
    //! Set mark color.
    void setMarkColor(const QColor &c);

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
    std::shared_ptr<MdShared::Syntax> m_syntax;
    bool m_alreadyShown = false;
    MdShared::PluginsCfg m_pluginsCfg;
    QColor m_markColor;

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
    //! Settings.
    void settings();

private:
    QString configFileName(bool inPlace) const;
    void readCfg();
    void saveCfg();

private:
    MainWidget *ui = nullptr;
    bool m_alreadyShown = false;
}; // class MainWindow

} /* namespace MdPdf */
