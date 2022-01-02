/*
    SPDX-FileCopyrightText: 2017, 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef MARKDOWNPARTFACTORY_HPP
#define MARKDOWNPARTFACTORY_HPP

// KF
#include <KPluginFactory>


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
};

#endif
