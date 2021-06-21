# Markdown Viewer KPart (QTextDocument-based)

## Introduction

This repository contains software for the rendered display of Markdown documents:

* a Markdown viewer [KParts](https://api.kde.org/frameworks/kparts/html/index.html) plugin, which allows KParts-using applications to display files in Markdown format in the target format

The software is mainly a wrapper around the classes [QTextDocument](https://doc.qt.io/qt-5/qtextdocument.html) and [QTextBrowser](https://doc.qt.io/qt-5/qtextbrowser.html) from Qt's QWidgets library. The Markdown support is thus completely driven by the abilities of those classes.

## Using

To use the MarkdownPart KParts plugin in a KParts-using applications, often you will need to configure that globally in the Plasma System Settings, and there in the "File Associations" page.
Select the MIME type "text/markdown" and in the "Embedding" tab in the "Service Preference Order" group make sure "Markdown View (markdownpart)" is on top of the list.

## Issues

Please report bugs and feature requests in the [KDE issue tracker](https://bugs.kde.org/enter_bug.cgi?product=markdownpart).
