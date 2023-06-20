/*
    SPDX-FileCopyrightText: 2017, 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "markdownpartfactory.hpp"

// part
#include "markdownpart.hpp"
// KF
#include <KLocalizedString>


MarkdownPartFactory::MarkdownPartFactory() = default;

MarkdownPartFactory::~MarkdownPartFactory() = default;

QObject* MarkdownPartFactory::create(const char* iface,
                                     QWidget* parentWidget, QObject* parent,
#if KCOREADDONS_VERSION >= QT_VERSION_CHECK(5, 240, 0)
                                     const QVariantList& args)
{
#else
                                     const QVariantList& args, const QString& keyword)
{
    Q_UNUSED(keyword );
#endif

    const bool wantBrowserView = (args.contains(QStringLiteral("Browser/View")) ||
                                 (strcmp(iface, "Browser/View") == 0));
    const MarkdownPart::Modus modus =
        wantBrowserView ? MarkdownPart::BrowserViewModus :
        /* else */        MarkdownPart::ReadOnlyModus;

    return new MarkdownPart(parentWidget, parent, metaData(), modus);
}

#include "moc_markdownpartfactory.cpp"
