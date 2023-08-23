/*
    SPDX-FileCopyrightText: 2017, 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "markdownpartfactory.hpp"

// part
#include "markdownpart.hpp"
// KF
#include <KLocalizedString>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <KXmlGui5ConfigMigration>
#endif


MarkdownPartFactory::MarkdownPartFactory()
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    // copy KF5-time user toolbar configuration, keep old copy for users of KF5 plugin version
    KXmlGui5ConfigMigration::migrate(QStringLiteral("markdownpart"), {QStringLiteral("markdownpartui.rc")},
                                     KXmlGui5ConfigMigration::NoMigrationOptions);
#endif
}

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
