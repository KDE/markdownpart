/*
    SPDX-FileCopyrightText: 2017, 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef MARKDOWNPART_HPP
#define MARKDOWNPART_HPP

// KF
#include <kparts_version.h>
#include <KParts/ReadOnlyPart>
// Qt
#include <QByteArray>
#include <QPoint>

class MarkdownBrowserExtension;
class MarkdownView;
class SearchToolBar;
#if KPARTS_VERSION >= QT_VERSION_CHECK(5, 77, 0)
class KPluginMetaData;
#else
class KAboutData;
#endif
class QTextDocument;


class MarkdownPart : public KParts::ReadOnlyPart
{
    Q_OBJECT

public:
    enum Modus {
        ReadOnlyModus = 0,
        BrowserViewModus = 1
    };

    /**
     * Default constructor, with arguments as expected by MarkdownPartFactory
     */
#if KPARTS_VERSION >= QT_VERSION_CHECK(5, 77, 0)
    MarkdownPart(QWidget* parentWidget, QObject* parent, const KPluginMetaData& metaData, Modus modus);
#else
    MarkdownPart(QWidget* parentWidget, QObject* parent, const KAboutData& aboutData, Modus modus);
#endif
    ~MarkdownPart() override;

public:
    MarkdownView* view() const;

    QAction* copySelectionAction() const;
    QAction* createCopyEmailAddressAction(QObject* parent, const QUrl& mailtoUrl);
    QAction* createCopyLinkUrlAction(QObject* parent, const QUrl& linkUrl);

    void copySelection();

protected: // KParts::ReadOnlyPart API
    bool openFile() override;

    bool doOpenStream(const QString& mimeType) override;
    bool doWriteStream(const QByteArray& data) override;
    bool doCloseStream() override;

    bool closeUrl() override;

private:
    void setupActions(Modus modus);
    void prepareViewStateRestoringOnReload();
    void restoreScrollPosition();

    void handleOpenUrlRequest(const QUrl& url);
    void handleContextMenuRequest(QPoint globalPos,
                                  const QUrl& linkUrl,
                                  bool hasSelection);
    void showHoveredLink(const QUrl& linkUrl);

    void selectAll();

private:
    QTextDocument* m_sourceDocument;
    MarkdownView* m_widget;
    SearchToolBar* m_searchToolBar;
    QAction* m_copySelectionAction;
    QAction* m_selectAllAction;
    QAction* m_searchAction;
    QAction* m_searchNextAction;
    QAction* m_searchPreviousAction;

    MarkdownBrowserExtension* const m_browserExtension;

    QByteArray m_streamedData;

    QUrl m_previousUrl;
    QPoint m_previousScrollPosition;
};

inline MarkdownView* MarkdownPart::view() const { return m_widget; }

#endif
