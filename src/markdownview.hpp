/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef MARKDOWNBROWSER_HPP
#define MARKDOWNBROWSER_HPP

// Qt
#include <QTextBrowser>


class MarkdownView : public QTextBrowser
{
    Q_OBJECT

public:
    MarkdownView(QTextDocument* document, QWidget* parent);

public:
    bool hasSelection() const;

    void setScrollPosition(QPoint offset);
    QPoint scrollPosition() const;
    int scrollPositionX() const;
    int scrollPositionY() const;

Q_SIGNALS:
    void contextMenuRequested(QPoint globalPos, const QUrl& linkUrl,
                              bool hasSelection);

protected:
    void contextMenuEvent(QContextMenuEvent* event) override;
};

#endif
