/*
    SPDX-FileCopyrightText: 2017, 2020 Friedrich W. H. Kossebau <kossebau@kde.org>

    SPDX-License-Identifier: LGPL-2.1-or-later
*/

#include "searchtoolbar.hpp"

// part
#include "ui_searchtoolbar.h"
// Qt
#include <QTextBrowser>
#include <QTextCursor>


SearchToolBar::SearchToolBar(QTextBrowser* markdownView, QWidget* parent)
    : QWidget(parent)
    , m_ui(new Ui::SearchToolBar)
    , m_markdownView(markdownView)
{
    m_ui->setupUi(this);

    connect(m_ui->hideButton, &QToolButton::clicked,
            this, &SearchToolBar::hide);
    connect(m_ui->searchTextEdit, &QLineEdit::textEdited,
            this, &SearchToolBar::searchIncrementally);
    connect(m_ui->matchCaseCheckButton, &QAbstractButton::toggled,
            this, &SearchToolBar::searchIncrementally);
    connect(m_ui->searchTextEdit, &QLineEdit::returnPressed,
            this, &SearchToolBar::searchNext);
    connect(m_ui->nextButton, &QToolButton::clicked,
            this, &SearchToolBar::searchNext);
    connect(m_ui->previousButton, &QToolButton::clicked,
            this, &SearchToolBar::searchPrevious);
    // TODO: disable next/previous buttons if no (more) search hits, color coding in text field
}

SearchToolBar::~SearchToolBar() = default;

void SearchToolBar::searchNext()
{
    const QString text = m_ui->searchTextEdit->text();
    if (text.isEmpty()) {
        startSearch();
        return;
    }

    QTextDocument::FindFlags findFlags = {};
    if (m_ui->matchCaseCheckButton->isChecked()) {
        findFlags |= QTextDocument::FindCaseSensitively;
    }

    m_markdownView->find(text, findFlags);

}

void SearchToolBar::searchPrevious()
{
    const QString text = m_ui->searchTextEdit->text();
    if (text.isEmpty()) {
        startSearch();
        return;
    }

    QTextDocument::FindFlags findFlags = {QTextDocument::FindBackward};
    if (m_ui->matchCaseCheckButton->isChecked()) {
        findFlags |= QTextDocument::FindCaseSensitively;
    }

    m_markdownView->find(text, findFlags);
}

void SearchToolBar::startSearch()
{
    // if there is seleted text, assume the user wants to search that selected text
    // otherwise reuse any old term
    const QString selectedText = m_markdownView->textCursor().selectedText();
    if (!selectedText.isEmpty()) {
        m_ui->searchTextEdit->setText(selectedText);
    }

    show();
    m_ui->searchTextEdit->selectAll();
    m_ui->searchTextEdit->setFocus();
}

void SearchToolBar::searchIncrementally()
{
    QTextDocument::FindFlags findFlags = {};

    if (m_ui->matchCaseCheckButton->isChecked()) {
        findFlags |= QTextDocument::FindCaseSensitively;
    }

    // calling with changed text with added or removed chars at end will result in current
    // selection kept, if also matching new text
    // behaviour on changed case sensitivity though is advancing to next match even if current
    // would be still matching. as there is no control about currently shown match, nothing
    // we can do about it. thankfully case sensitivity does not happen too often, so should
    // not be too grave UX
    m_markdownView->find(m_ui->searchTextEdit->text(), findFlags);
}

void SearchToolBar::hideEvent(QHideEvent* event)
{
    // finish search
    // passing emptry string to reset search
    m_markdownView->find(QString());

    QWidget::hideEvent(event);
}

#include "moc_searchtoolbar.cpp"
