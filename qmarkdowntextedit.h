/*
 * Copyright (C) 2016 Patrizio Bekerle -- http://www.bekerle.com
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
#include "highlighter.h"
#include "qtexteditsearchwidget.h"

class QMarkdownTextEdit : public QTextEdit
{
    Q_OBJECT

public:
    explicit QMarkdownTextEdit(QWidget *parent = 0);
    QMarkdownHighlighter *highlighter();
    QTextEditSearchWidget *searchWidget();
    void setIgnoredClickUrlSchemata(QStringList ignoredUrlSchemata);
    virtual void openUrl(QUrl url);
    QUrl getMarkdownUrlAtPosition(QString text, int position);
    void initSearchFrame(QWidget *searchFrame);

public slots:
    void duplicateText();
    void setText(const QString & text);
    void setHtml(const QString &text);
    void setPlainText(const QString & text);
    void adjustRightMargin();
    void hide();
    bool openLinkAtCursorPosition();

protected:
    QMarkdownHighlighter *_highlighter;
    QStringList _ignoredClickUrlSchemata;
    QTextEditSearchWidget *_searchWidget;
    QWidget *_searchFrame;
    bool eventFilter(QObject *obj, QEvent *event);
    bool increaseSelectedTextIndention(bool reverse);
    bool handleTabEntered(bool reverse);
    QMap<QString, QString> parseMarkdownUrlsFromText(QString text);
    bool handleReturnEntered();

signals:
    void urlClicked(QUrl url);
};
