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

#include <QtGui>
#include <QFontDatabase>
#include "markdownhighlighter.h"


MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent)
        : QSyntaxHighlighter(parent) {
    HighlightingRule rule;

    // markdown syntax
    // http://daringfireball.net/projects/markdown/syntax

    // highlight bold
    rule.pattern = QRegularExpression("\\B\\*{2}.+?\\*{2}\\B");
    rule.state = HighlighterState::Bold;
    _highlightingRules.append(rule);

    // highlight italic
    rule.pattern = QRegularExpression("(^|\\s)\\*[^\\*]+\\*($|\\s)");
    rule.state = HighlighterState::Italic;
    _highlightingRules.append(rule);

    // highlight urls
    rule.pattern = QRegularExpression("<.+?://.+?>");
    rule.state = HighlighterState::Link;
    _highlightingRules.append(rule);

    // highlight urls with title
    rule.pattern = QRegularExpression("\\[.+?\\]\\(.+?://.+?\\)");
    rule.state = HighlighterState::Link;
    _highlightingRules.append(rule);

    // highlight email links
    rule.pattern = QRegularExpression("<.+?@.+?>");
    rule.state = HighlighterState::Link;
    _highlightingRules.append(rule);

    // highlight inline code
    rule.pattern = QRegularExpression("`.+?`");
    rule.state = HighlighterState::InlineCodeBlock;
    _highlightingRules.append(rule);

    // highlight unordered lists
    rule.pattern = QRegularExpression("^\\s*[-\\*\\+]\\s");
    rule.state = HighlighterState::List;
    _highlightingRules.append(rule);

    // highlight ordered lists
    rule.pattern = QRegularExpression("^\\s*\\d\\.\\s");
    rule.state = HighlighterState::List;
    _highlightingRules.append(rule);

    // highlight images
    rule.pattern = QRegularExpression("!\\[.+?\\]\\(.+?\\)");
    rule.state = HighlighterState::Image;
    _highlightingRules.append(rule);

    // highlight block quotes
    rule.pattern = QRegularExpression("^> ");
    rule.state = HighlighterState::BlockQuote;
    _highlightingRules.append(rule);

    // highlight inline comments
    rule.pattern = QRegularExpression("<!\\-\\-.+?\\-\\->");
    rule.state = HighlighterState::Comment;
    _highlightingRules.append(rule);

    // highlight horizontal rulers
    rule.pattern = QRegularExpression("^([*\\-]\\s?){3,}$");
    rule.state = HighlighterState::HorizontalRuler;
    _highlightingRules.append(rule);

    // highlight tables
    rule.pattern = QRegularExpression("^\\|.+?\\|$");
    rule.state = HighlighterState::Table;
    _highlightingRules.append(rule);


    // initialize the text formats
    initTextFormats();
}

/**
 * Initializes the text formats
 *
 * @param defaultFontSize
 */
void MarkdownHighlighter::initTextFormats(int defaultFontSize) {
    QTextCharFormat format;

    // set character formats for headlines
    format = QTextCharFormat();
    format.setForeground(QBrush(QColor(0, 49, 110)));
    format.setBackground(QBrush(QColor(230, 230, 240)));
    format.setFontWeight(QFont::Bold);
    format.setFontPointSize(defaultFontSize * 1.2);
    _formats[H1] = format;
    format.setFontPointSize(defaultFontSize * 1.1);
    _formats[H2] = format;
    format.setFontPointSize(defaultFontSize);
    _formats[H3] = format;
    _formats[H4] = format;
    _formats[H5] = format;
    _formats[H6] = format;

    // set character format for horizontal rulers
    format = QTextCharFormat();
    format.setForeground(QBrush(Qt::darkGray));
    format.setBackground(QBrush(Qt::lightGray));
    _formats[HorizontalRuler] = format;

    // set character format for lists
    format = QTextCharFormat();
    format.setForeground(QBrush(QColor(163, 0, 123)));
    _formats[List] = format;

    // set character format for links
    format = QTextCharFormat();
    format.setForeground(QBrush(QColor(255, 128, 0)));
    format.setBackground(QBrush(QColor(255, 233, 211)));
    _formats[Link] = format;

    // set character format for images
    format = QTextCharFormat();
    format.setForeground(QBrush(QColor(0, 191, 0)));
    format.setBackground(QBrush(QColor(228, 255, 228)));
    _formats[Image] = format;

    // set character format for code blocks
    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QBrush(Qt::darkGreen));
    format.setBackground(QBrush(QColor(217, 231, 217)));
    _formats[CodeBlock] = format;
    _formats[InlineCodeBlock] = format;

    // set character format for italic
    format = QTextCharFormat();
    format.setFontWeight(QFont::StyleItalic);
    format.setForeground(QBrush(QColor(0, 87, 174)));
    _formats[Italic] = format;

    // set character format for bold
    format = QTextCharFormat();
    format.setFontWeight(QFont::Bold);
    format.setForeground(QBrush(QColor(0, 66, 138)));
    _formats[Bold] = format;

    // set character format for comments
    format = QTextCharFormat();
    format.setForeground(QBrush(Qt::gray));
    _formats[Comment] = format;

    // set character format for tables
    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QBrush(QColor("#649449")));
    _formats[Table] = format;

    // set character format for block quotes
    format = QTextCharFormat();
    format.setForeground(QBrush(QColor(Qt::darkRed)));
    _formats[BlockQuote] = format;
}

/**
 * Sets the text formats
 *
 * @param formats
 */
void MarkdownHighlighter::setTextFormats(
        QHash<HighlighterState, QTextCharFormat> formats) {
    _formats = formats;
}

/**
 * Sets a text format
 *
 * @param formats
 */
void MarkdownHighlighter::setTextFormat(HighlighterState state,
                                        QTextCharFormat format) {
    _formats[state] = format;
}

/**
 * Does the markdown highlighting
 *
 * @param text
 */
void MarkdownHighlighter::highlightBlock(const QString &text) {
    setCurrentBlockState(-1);
    highlightMarkdown(text);
}

void MarkdownHighlighter::highlightMarkdown(QString text) {
    if (!text.isEmpty()) {
        highlightAdditionalRules(text);

        // needs to be called after the horizontal ruler highlighting
        highlightHeadline(text);
    }

    highlightCommentBlock(text);
    highlightCodeBlock(text);
}

/**
 * Highlight headlines
 *
 * @param text
 */
void MarkdownHighlighter::highlightHeadline(QString text) {
    QRegularExpression re("^(#+) .+?$");
    QRegularExpressionMatch match = re.match(text);

    // check for headline blocks with # in front of them
    if (match.hasMatch()) {
        int count = match.captured(1).count();

        // we just have H1 to H6
        count = qMin(count, 6);

        HighlighterState state = HighlighterState(
                HighlighterState::H1 + count - 1);

        setFormat(match.capturedStart(), match.capturedLength(),
                  _formats[state]);
        setCurrentBlockState(state);
        return;
    }

    // take care of ==== and ---- headlines
    QRegularExpression patternH1 = QRegularExpression("^=+$");
    QRegularExpression patternH2 = QRegularExpression("^\\-+$");

    // check for ===== after a headline and highlight as H1
    if (patternH1.match(text).hasMatch()) {
        QTextBlock previousBlock = currentBlock().previous();

        if (previousBlockState() == HighlighterState::H1 &&
                previousBlock.isValid()) {
//            rehighlightBlock(previousBlock);
//            previousBlock.set
//            rehighlight();

//            QTimer::singleShot(150, this, SLOT(rehighlight()));
            setFormat(0, text.length(), _formats[HighlighterState::H1]);
        }

        return;
    }

    // check for ----- after a headline and highlight as H2
    if (patternH2.match(text).hasMatch()) {
        QTextBlock previousBlock = currentBlock().previous();

        if (previousBlockState() == HighlighterState::H2 &&
                previousBlock.isValid()) {
            setFormat(0, text.length(), _formats[HighlighterState::H2]);
        }

        return;
    }

    QTextBlock nextBlock = currentBlock().next();
    QString nextBlockText = nextBlock.text();

    // highlight as H1 if next block is =====
    if (patternH1.match(nextBlockText).hasMatch() ||
            patternH2.match(nextBlockText).hasMatch()) {
        setFormat(0, text.length(), _formats[HighlighterState::H1]);
        setCurrentBlockState(HighlighterState::H1);
    }

    // highlight as H2 if next block is -----
    if (patternH2.match(nextBlockText).hasMatch()) {
        setFormat(0, text.length(), _formats[HighlighterState::H2]);
        setCurrentBlockState(HighlighterState::H2);
    }
}

/**
 * Highlight multiline code blocks
 *
 * @param text
 */
void MarkdownHighlighter::highlightCodeBlock(QString text) {
    bool highlight = false;

    QRegularExpression re("^```\\w*?$");
    QRegularExpressionMatch match = re.match(text);

    if (match.hasMatch()) {
        setCurrentBlockState(
                previousBlockState() == HighlighterState::CodeBlock ?
                HighlighterState::CodeBlockEnd : HighlighterState::CodeBlock);
        highlight = true;
    } else if (previousBlockState() == HighlighterState::CodeBlock) {
        setCurrentBlockState(HighlighterState::CodeBlock);
        highlight = true;
    }

    if (highlight) {
        setFormat(0, text.length(), _formats[HighlighterState::CodeBlock]);
    }
}

/**
 * Highlight multiline comments
 *
 * @param text
 */
void MarkdownHighlighter::highlightCommentBlock(QString text) {
    qDebug() << __func__ << " - 'previousBlockState()': "
             << previousBlockState();
    qDebug() << __func__ << " - 'text': " << text;

    bool highlight = false;
    QString startText = "<!--";
    QString endText = "-->";

    if ((text == startText) ||
            ((text != endText) &&
                    ((previousBlockState() == HighlighterState::Comment)))) {
        setCurrentBlockState(HighlighterState::Comment);
        highlight = true;
    } else if (text == endText) {
        highlight = true;
    }

    if (highlight) {
        setFormat(0, text.length(), _formats[HighlighterState::Comment]);
    }
}

/**
 * Highlights the rules from the _highlightingRules list
 *
 * @param text
 */
void MarkdownHighlighter::highlightAdditionalRules(QString text) {
    foreach(const HighlightingRule &rule, _highlightingRules) {
            QRegularExpression expression(rule.pattern);
            QRegularExpressionMatchIterator i = expression.globalMatch(text);

            while (i.hasNext()) {
                QRegularExpressionMatch match = i.next();
                setFormat(match.capturedStart(), match.capturedLength(),
                          _formats[rule.state]);
            }
        }
}
