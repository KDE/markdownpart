/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "markdownview.hpp"

// Qt
#include <QScrollBar>
#include <QTextCursor>
#include <QContextMenuEvent>


MarkdownView::MarkdownView(QTextDocument* document, QWidget* parent)
    : QTextBrowser(parent)
{
    setOpenLinks(false);

    setDocument(document);
}

bool MarkdownView::hasSelection() const
{
    return textCursor().hasSelection();
}

void MarkdownView::setScrollPosition(QPoint offset)
{
    horizontalScrollBar()->setValue(offset.x());
    verticalScrollBar()->setValue(offset.y());
}

QPoint MarkdownView::scrollPosition() const
{
    return {
        horizontalScrollBar()->value(),
        verticalScrollBar()->value()
    };
}

int MarkdownView::scrollPositionX() const
{
    return horizontalScrollBar()->value();
}

int MarkdownView::scrollPositionY() const
{
    return verticalScrollBar()->value();
}

void MarkdownView::contextMenuEvent(QContextMenuEvent* event)
{
    // default menu arguments
    bool hasSelection = false;
    const QUrl linkUrl(anchorAt(event->pos()));
    // linkText not straight to get, anchorAt uses ExactHit test, but cursorAt FuzzyHit, so this might not match

    if (!linkUrl.isValid()) {
        hasSelection = this->hasSelection();
    }

    Q_EMIT contextMenuRequested(event->globalPos(),
                                linkUrl,
                                hasSelection);

    event->accept();
}
