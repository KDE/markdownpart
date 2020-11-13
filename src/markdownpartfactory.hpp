/*
    SPDX-FileCopyrightText: 2017, 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef MARKDOWNPARTFACTORY_HPP
#define MARKDOWNPARTFACTORY_HPP

// KF
#include <KPluginFactory>
#include <kparts_version.h>
#if KPARTS_VERSION < QT_VERSION_CHECK(5, 77, 0)
#include <KAboutData>
#endif


class MarkdownPartFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "markdownpart.json")
    Q_INTERFACES(KPluginFactory)

public:
    MarkdownPartFactory();
    ~MarkdownPartFactory() override;

public:
    QObject* create(const char* iface,
                    QWidget* parentWidget, QObject* parent,
                    const QVariantList& args, const QString& keyword) override;

#if KPARTS_VERSION < QT_VERSION_CHECK(5, 77, 0)
private:
    KAboutData m_aboutData;
#endif
};

#endif
