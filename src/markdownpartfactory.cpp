/*
    SPDX-FileCopyrightText: 2017, 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "markdownpartfactory.hpp"

// part
#include "markdownpart.hpp"
#include "markdownpartconfig.hpp"
// KF
#include <KLocalizedString>


MarkdownPartFactory::MarkdownPartFactory()
    : m_aboutData(QStringLiteral("markdownpart"),
                  i18n("MarkdownPart"),
                  QStringLiteral(MARKDOWNPART_VERSION),
                  i18n("For the rendered display of Markdown files in a webpage-like layout"),
                  KAboutLicense::LGPL_V2_1,
                  i18n("Copyright 2020 Friedrich W. H. Kossebau"),
                  QString(),
                  QStringLiteral("https://commits.kde.org/markdownpart")
                 )
{
    m_aboutData.addAuthor(i18nc("@info:credit", "Friedrich W. H. Kossebau"),
                          i18nc("@info:credit", "Author"), QStringLiteral("kossebau@kde.org"));
}

MarkdownPartFactory::~MarkdownPartFactory() = default;

QObject* MarkdownPartFactory::create(const char* iface,
                                     QWidget* parentWidget, QObject* parent,
                                     const QVariantList& args, const QString& keyword)
{
    Q_UNUSED(keyword );

    const bool wantBrowserView = (args.contains(QStringLiteral("Browser/View")) ||
                                 (strcmp(iface, "Browser/View") == 0));
    const MarkdownPart::Modus modus =
        wantBrowserView ? MarkdownPart::BrowserViewModus :
        /* else */        MarkdownPart::ReadOnlyModus;

    return new MarkdownPart(parentWidget, parent, m_aboutData, modus);
}

