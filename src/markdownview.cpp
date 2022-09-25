/*
    SPDX-FileCopyrightText: 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "markdownview.hpp"

// Qt
#include <QScrollBar>
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
    // Compare KWebKitPart's WebView::contextMenuEvent & WebEnginePart's WebEngineView::contextMenuEvent
    // for the patterns used to fill the menu.
    // QTextBrowser at of Qt 5.15 provides less data though, so for now this is reduced variant.

    // trying to get linkText skipped, because not reliable to get:
    // anchorAt uses ExactHit test, but cursorAt FuzzyHit, so this might not match

    const QUrl linkUrl(anchorAt(event->pos()));

    // only report any selection if this is not a context menu for a link
    const bool hasSelection = !linkUrl.isValid() && this->hasSelection();

    Q_EMIT contextMenuRequested(event->globalPos(),
                                linkUrl,
                                hasSelection);

    event->accept();
}
