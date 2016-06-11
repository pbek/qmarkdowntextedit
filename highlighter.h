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
 * highlighter.cpp
 *
 * QTextEdit highlighter
 */


#ifndef HIGHLIGHTER_H
#define HIGHLIGHTER_H

#include <QTextCharFormat>
#include <QThread>

extern "C" {
#include "lib/peg-markdown-highlight/pmh_parser.h"
}

QT_BEGIN_NAMESPACE
class QTextDocument;

QT_END_NAMESPACE

class WorkerThread : public QThread {
public:
    ~WorkerThread();

    void run();

    char *content;
    pmh_element **result;
};

struct HighlightingStyle {
    pmh_element_type type;
    QTextCharFormat format;
};


class QMarkdownHighlighter : public QObject {
Q_OBJECT

public:
    QMarkdownHighlighter(QTextDocument *parent = 0, int waitInterval = 200);

    void setStyles(QVector<HighlightingStyle> &styles);

    void parse();

    void setDefaultStyles(int defaultFontSize = 12);

protected:

private slots:

    void handleContentsChange(int position, int charsRemoved, int charsAdded);

    void threadFinished();

    void timerTimeout();

signals:
    void parsingFinished();

private:
    QTimer *timer;
    QTextDocument *document;
    WorkerThread *workerThread;
    bool parsePending;
    pmh_element **cached_elements;
    QVector<HighlightingStyle> *highlightingStyles;
    bool _highlightingEnabled;

    void clearFormatting();

    void highlight();
};

#endif
