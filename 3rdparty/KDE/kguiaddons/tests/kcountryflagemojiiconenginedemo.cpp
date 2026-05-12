// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
// SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

#include <QApplication>
#include <QLabel>
#include <QVBoxLayout>

#include <kcountryflagemojiiconengine.h>

using namespace Qt::Literals::StringLiterals;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    QWidget widget;
    widget.setWindowIcon(QIcon(new KCountryFlagEmojiIconEngine("UN")));
    widget.setLayout(new QHBoxLayout);

    {
        auto l = new QLabel(&widget);
        l->setMinimumSize(256, 256);
        l->setPixmap(QIcon(new KCountryFlagEmojiIconEngine("GB-SCT")).pixmap(256, 256));
        widget.layout()->addWidget(l);
    }

    {
        auto l = new QLabel(&widget);
        l->setMinimumSize(256, 256);
        l->setPixmap(QIcon(new KCountryFlagEmojiIconEngine("AT")).pixmap(256, 256));
        widget.layout()->addWidget(l);
    }

    {
        auto l = new QLabel(&widget);
        l->setMinimumSize(256, 256);
        l->setPixmap(QIcon(new KCountryFlagEmojiIconEngine("LATAM")).pixmap(256, 256));
        widget.layout()->addWidget(l);
    }

    widget.show();

    return app.exec();
}
