/*
 * Copyright (c) 2014-2017 Patrizio Bekerle -- http://www.bekerle.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 */

#pragma once

#include <QTextEdit>
#include <QEvent>
#include "markdownhighlighter.h"
#include "qtexteditsearchwidget.h"

class QMarkdownTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    enum AutoTextOption {
        None = 0x0000,

        // inserts closing characters for brackets and markdown characters
        BracketClosing = 0x0001,

        // removes matching brackets and markdown characters
        BracketRemoval = 0x0002
    };

    Q_DECLARE_FLAGS(AutoTextOptions, AutoTextOption)

    explicit QMarkdownTextEdit(QWidget *parent = 0);
    MarkdownHighlighter *highlighter();
    QTextEditSearchWidget *searchWidget();
    void setIgnoredClickUrlSchemata(QStringList ignoredUrlSchemata);
    virtual void openUrl(QString urlString);
    QString getMarkdownUrlAtPosition(QString text, int position);
    void initSearchFrame(QWidget *searchFrame);
    void setAutoTextOptions(AutoTextOptions options);
    void setHighlightingEnabled(bool enabled);
    static bool isValidUrl(QString urlString);

public slots:
    void duplicateText();
    void setText(const QString & text);
    void setHtml(const QString &text);
    void setPlainText(const QString & text);
    void adjustRightMargin();
    void hide();
    bool openLinkAtCursorPosition();
    bool handleBracketRemoval();

protected:
    MarkdownHighlighter *_highlighter;
    bool _highlightingEnabled;
    QStringList _ignoredClickUrlSchemata;
    QTextEditSearchWidget *_searchWidget;
    QWidget *_searchFrame;
    AutoTextOptions _autoTextOptions;
    QStringList _openingCharacters;
    QStringList _closingCharacters;

    bool eventFilter(QObject *obj, QEvent *event);
    bool increaseSelectedTextIndention(bool reverse);
    bool handleTabEntered(bool reverse);
    QMap<QString, QString> parseMarkdownUrlsFromText(QString text);
    bool handleReturnEntered();
    bool handleBracketClosing(QString openingCharacter,
                              QString closingCharacter = "");

signals:
    void urlClicked(QString url);
};
