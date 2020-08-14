/*
    SPDX-FileCopyrightText: 2017, 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#ifndef SEARCHTOOLBAR_HPP
#define SEARCHTOOLBAR_HPP

// Qt
#include <QWidget>
#include <QScopedPointer>

namespace Ui {
class SearchToolBar;
}
class QTextBrowser;


class SearchToolBar : public QWidget
{
    Q_OBJECT

public:
    explicit SearchToolBar(QTextBrowser* markdownView, QWidget* parent = nullptr);
    ~SearchToolBar() override;

    void hideEvent(QHideEvent* event) override;

public Q_SLOTS:
    void startSearch();
    void searchNext();
    void searchPrevious();

private Q_SLOTS:
    void searchIncrementally();

private:
    QScopedPointer<Ui::SearchToolBar> const m_ui;
    QTextBrowser* const m_markdownView;
};

#endif
