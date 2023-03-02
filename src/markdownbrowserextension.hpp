/*
    SPDX-FileCopyrightText: 2017, 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef MARKDOWNBROWSEREXTENSION_HPP
#define MARKDOWNBROWSEREXTENSION_HPP

// Qt
#include <QPoint>

// KF
#include <kparts_version.h>

#if KPARTS_VERSION >= QT_VERSION_CHECK(5, 240, 0)
#include <KParts/NavigationExtension>
#else
#include <KParts/BrowserExtension>
#endif

class MarkdownPart;
class KActionCollection;


#if KPARTS_VERSION >= QT_VERSION_CHECK(5, 240, 0)
class MarkdownBrowserExtension : public KParts::NavigationExtension
#else
class MarkdownBrowserExtension : public KParts::BrowserExtension
#endif
{
    Q_OBJECT

public:
    explicit MarkdownBrowserExtension(MarkdownPart* part);

public: // KParts::BrowserExtension API
    int xOffset() override;
    int yOffset() override;

public Q_SLOTS:
    void copy();

    void updateCopyAction(bool enabled);
    void requestOpenUrl(const QUrl& url);
    void requestOpenUrlNewWindow(const QUrl& url);
    void requestContextMenu(QPoint globalPos,
                            const QUrl& linkUrl,
                            bool hasSelection);

private:
    MarkdownPart* m_part;
    // needed to memory manage the context menu actions
    KActionCollection* m_contextMenuActionCollection;
};

#endif
