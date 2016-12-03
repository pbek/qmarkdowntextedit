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
 * QTextEdit markdown highlighter
 */

#include <QTimer>
#include "markdownhighlighter.h"


/**
 * Markdown syntax highlighting
 *
 * markdown syntax:
 * http://daringfireball.net/projects/markdown/syntax
 *
 * @param parent
 * @return
 */
MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent)
        : QSyntaxHighlighter(parent) {
    _timer = new QTimer(this);
    QObject::connect(_timer, SIGNAL(timeout()),
                     this, SLOT(timerTick()));
    _timer->start(1000);

    // initialize the highlighting rules
    initHighlightingRules();

    // initialize the text formats
    initTextFormats();
}

/**
 * Does jobs every second
 */
void MarkdownHighlighter::timerTick() {
    // re-highlight all dirty blocks
    reHighlightDirtyBlocks();

    // emit a signal every second if there was some highlighting done
    if (_highlightingFinished) {
        _highlightingFinished = false;
        emit(highlightingFinished());
    }
}

/**
 * Re-highlights all dirty blocks
 */
void MarkdownHighlighter::reHighlightDirtyBlocks() {
    while (_dirtyTextBlocks.count() > 0) {
        QTextBlock block = _dirtyTextBlocks.at(0);
        rehighlightBlock(block);
        _dirtyTextBlocks.removeFirst();
    }
}

/**
 * Adds a dirty block to the list if it doesn't already exist
 *
 * @param block
 */
void MarkdownHighlighter::addDirtyBlock(QTextBlock block) {
    if (!_dirtyTextBlocks.contains(block)) {
        _dirtyTextBlocks.append(block);
    }
}

/**
 * Initializes the highlighting rules
 *
 * regexp tester:
 * https://regex101.com
 */
void MarkdownHighlighter::initHighlightingRules() {
    HighlightingRule rule;

    // highlight the reference of reference links
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("^\\[\\d+?\\]: .+://.+$");
    rule.state = HighlighterState::Comment;
    _highlightingRulesPre.append(rule);

    // highlight unordered lists
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("^\\s*[-*+]\\s");
    rule.state = HighlighterState::List;
    rule.useStateAsCurrentBlockState = true;
    _highlightingRulesPre.append(rule);

    // highlight ordered lists
    rule.pattern = QRegularExpression("^\\s*\\d\\.\\s");
    _highlightingRulesPre.append(rule);

    // highlight block quotes
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("^> ");
    rule.state = HighlighterState::BlockQuote;
    _highlightingRulesPre.append(rule);

    // highlight inline comments
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("<!\\-\\-.+?\\-\\->");
    rule.state = HighlighterState::Comment;
    _highlightingRulesPre.append(rule);

    // highlight horizontal rulers
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("^([*\\-_]\\s?){3,}$");
    rule.state = HighlighterState::HorizontalRuler;
    _highlightingRulesPre.append(rule);

    // highlight tables without starting |
    // we drop that for now, it's far too messy to deal with
//    rule = HighlightingRule();
//    rule.pattern = QRegularExpression("^.+? \\| .+? \\| .+$");
//    rule.state = HighlighterState::Table;
//    _highlightingRulesPre.append(rule);


    // highlight bold
    rule.pattern = QRegularExpression("\\B\\*{2}.+?\\*{2}\\B");
    rule.state = HighlighterState::Bold;
    _highlightingRulesAfter.append(rule);
    rule.pattern = QRegularExpression("(^|\\s)__.+?__($|\\s)");
    _highlightingRulesAfter.append(rule);

    // highlight italic
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("(^|\\s)\\*[^\\*]+\\*($|\\s)");
    rule.state = HighlighterState::Italic;
    _highlightingRulesAfter.append(rule);
    rule.pattern = QRegularExpression("(^|\\s)_[^_]+_($|\\s)");
    _highlightingRulesAfter.append(rule);

    // highlight urls
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("<.+?:\\/\\/.+?>");
    rule.state = HighlighterState::Link;
    _highlightingRulesAfter.append(rule);

    // highlight urls without any other markup
    rule.pattern = QRegularExpression("\\b\\w+?:\\/\\/[^\\s]+");
    _highlightingRulesAfter.append(rule);

    // highlight urls with title
    rule.pattern = QRegularExpression("\\[.+?\\]\\(.+?://.+?\\)");
    _highlightingRulesAfter.append(rule);

    // highlight email links
    rule.pattern = QRegularExpression("<.+?@.+?>");
    _highlightingRulesAfter.append(rule);

    // highlight reference links
    rule.pattern = QRegularExpression("\\[.+?\\]\\[\\d+\\]");
    _highlightingRulesAfter.append(rule);

    // highlight images
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("!\\[.*?\\]\\(.+?\\)");
    rule.state = HighlighterState::Image;
    _highlightingRulesAfter.append(rule);

    // highlight inline code
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("`.+?`");
    rule.state = HighlighterState::InlineCodeBlock;
    _highlightingRulesAfter.append(rule);

    // highlight code blocks with four spaces or tabs in front of them
    // and no list character after that
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("^((\\t)|( {4,})).+$");
    rule.state = HighlighterState::CodeBlock;
    rule.disableIfCurrentStateIsSet = true;
    _highlightingRulesAfter.append(rule);

    // highlight tables with starting |
    rule = HighlightingRule();
    rule.pattern = QRegularExpression("^\\|.+?\\|$");
    rule.state = HighlighterState::Table;
    _highlightingRulesAfter.append(rule);
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
    setCurrentBlockState(HighlighterState::NoState);
    currentBlock().setUserState(HighlighterState::NoState);
    highlightMarkdown(text);
    _highlightingFinished = true;
}

void MarkdownHighlighter::highlightMarkdown(QString text) {
    if (!text.isEmpty()) {
        highlightAdditionalRules(_highlightingRulesPre, text);

        // needs to be called after the horizontal ruler highlighting
        highlightHeadline(text);

        highlightAdditionalRules(_highlightingRulesAfter, text);
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
        currentBlock().setUserState(state);
        return;
    }

    // take care of ==== and ---- headlines
    QRegularExpression patternH1 = QRegularExpression("^=+$");
    QRegularExpression patternH2 = QRegularExpression("^-+$");
    QTextBlock previousBlock = currentBlock().previous();
    QString previousText = previousBlock.text();
    previousText.trimmed().remove(QRegularExpression("[=-]"));

    // check for ===== after a headline text and highlight as H1
    if (patternH1.match(text).hasMatch()) {
        if (((previousBlockState() == HighlighterState::H1) ||
                (previousBlockState() == HighlighterState::NoState)) &&
                (previousText.length() > 0)) {
            setFormat(0, text.length(), _formats[HighlighterState::H1]);
            setCurrentBlockState(HighlighterState::HeadlineEnd);
            previousBlock.setUserState(HighlighterState::H1);

            // we want to re-highlight the previous block
            // this must not done directly, but with a queue, otherwise it
            // will crash
            // setting the character format of the previous text, because this
            // causes text to be formatted the same way when writing after
            // the text
            addDirtyBlock(previousBlock);
        }

        return;
    }

    // check for ----- after a headline text and highlight as H2
    if (patternH2.match(text).hasMatch()) {
        if (((previousBlockState() == HighlighterState::H2) ||
             (previousBlockState() == HighlighterState::NoState)) &&
            (previousText.length() > 0)) {
            setFormat(0, text.length(), _formats[HighlighterState::H2]);
            setCurrentBlockState(HighlighterState::HeadlineEnd);
            previousBlock.setUserState(HighlighterState::H2);

            // we want to re-highlight the previous block
            addDirtyBlock(previousBlock);
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
        currentBlock().setUserState(HighlighterState::H1);
    }

    // highlight as H2 if next block is -----
    if (patternH2.match(nextBlockText).hasMatch()) {
        setFormat(0, text.length(), _formats[HighlighterState::H2]);
        setCurrentBlockState(HighlighterState::H2);
        currentBlock().setUserState(HighlighterState::H2);
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
    bool highlight = false;
    text.trimmed();
    QString startText = "<!--";
    QString endText = "-->";

    if ((text.startsWith(startText)) ||
            ((!text.endsWith(endText)) &&
                    ((previousBlockState() == HighlighterState::Comment)))) {
        setCurrentBlockState(HighlighterState::Comment);
        highlight = true;
    } else if (text.endsWith(endText)) {
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
void MarkdownHighlighter::highlightAdditionalRules(
        QVector<HighlightingRule> &rules, QString text) {
    foreach(const HighlightingRule &rule, rules) {
            // continue if an other current block state was already set if
            // disableIfCurrentStateIsSet is set
            if (rule.disableIfCurrentStateIsSet &&
                    (currentBlockState() != HighlighterState::NoState)) {
                continue;
            }

            QRegularExpression expression(rule.pattern);
            QRegularExpressionMatchIterator i = expression.globalMatch(text);

            // store the current block state if useStateAsCurrentBlockState
            // is set
            if (i.hasNext() && rule.useStateAsCurrentBlockState) {
                setCurrentBlockState(rule.state);
            }

            // find and format all occurrences
            while (i.hasNext()) {
                QRegularExpressionMatch match = i.next();
                setFormat(match.capturedStart(), match.capturedLength(),
                          _formats[rule.state]);
            }
        }
}
