/*
 * Copyright (c) 2014-2019 Patrizio Bekerle -- http://www.bekerle.com
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
 * QPlainTextEdit markdown highlighter
 */

#include <QTimer>
#include <QDebug>
#include "markdownhighlighter.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>


/**
 * Markdown syntax highlighting
 *
 * markdown syntax:
 * http://daringfireball.net/projects/markdown/syntax
 *
 * @param parent
 * @return
 */
MarkdownHighlighter::MarkdownHighlighter(
        QTextDocument *parent, HighlightingOptions highlightingOptions)
        : QSyntaxHighlighter(parent) {
    _highlightingOptions = highlightingOptions;
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
    // qDebug() << "timerTick: " << this << ", " << this->parent()->parent()->parent()->objectName();

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
 * Clears the dirty blocks vector
 */
void MarkdownHighlighter::clearDirtyBlocks() {
    _dirtyTextBlocks.clear();
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
 *
 * other examples:
 * /usr/share/kde4/apps/katepart/syntax/markdown.xml
 */
void MarkdownHighlighter::initHighlightingRules() {
    // highlight the reference of reference links
    {
        HighlightingRule rule(HighlighterState::MaskedSyntax);
        rule.pattern = QRegularExpression("^\\[.+?\\]: \\w+://.+$");
        _highlightingRulesPre.append(rule);
    }

    // highlight unordered lists
    {
        HighlightingRule rule(HighlighterState::List);
        rule.pattern = QRegularExpression("^\\s*[-*+]\\s");
        rule.useStateAsCurrentBlockState = true;
        _highlightingRulesPre.append(rule);

        // highlight ordered lists
        rule.pattern = QRegularExpression("^\\s*\\d+\\.\\s");
        _highlightingRulesPre.append(rule);
    }

    // highlight block quotes
    {
        HighlightingRule rule(HighlighterState::BlockQuote);
        rule.pattern = QRegularExpression(
                    _highlightingOptions.testFlag(
                        HighlightingOption::FullyHighlightedBlockQuote) ?
                        "^\\s*(>\\s*.+)" : "^\\s*(>\\s*)+");
        _highlightingRulesPre.append(rule);
    }

    // highlight horizontal rulers
    {
        HighlightingRule rule(HighlighterState::HorizontalRuler);
        rule.pattern = QRegularExpression("^([*\\-_]\\s?){3,}$");
        _highlightingRulesPre.append(rule);
    }

    // highlight tables without starting |
    // we drop that for now, it's far too messy to deal with
//    rule = HighlightingRule();
//    rule.pattern = QRegularExpression("^.+? \\| .+? \\| .+$");
//    rule.state = HighlighterState::Table;
//    _highlightingRulesPre.append(rule);

    /*
     * highlight italic
     * this goes before bold so that bold can overwrite italic
     *
     * text to test:
     * **bold** normal **bold**
     * *start of line* normal
     * normal *end of line*
     * * list item *italic*
     */
    {
        HighlightingRule rule(HighlighterState::Italic);
        // we don't allow a space after the starting * to prevent problems with
        // unordered lists starting with a *
        rule.pattern = QRegularExpression(
                    "(?:^|[^\\*\\b])(?:\\*([^\\* ][^\\*]*?)\\*)(?:[^\\*\\b]|$)");
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);

        rule.pattern = QRegularExpression("\\b_([^_]+)_\\b");
        _highlightingRulesAfter.append(rule);
    }

    {
        HighlightingRule rule(HighlighterState::Bold);
        // highlight bold
        rule.pattern = QRegularExpression("\\B\\*{2}(.+?)\\*{2}\\B");
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);
        rule.pattern = QRegularExpression("\\b__(.+?)__\\b");
        _highlightingRulesAfter.append(rule);
    }

    // highlight urls
    {
        HighlightingRule rule(HighlighterState::Link);

        // highlight urls without any other markup
        rule.pattern = QRegularExpression("\\b\\w+?:\\/\\/[^\\s]+");
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);

        //    rule.pattern = QRegularExpression("<(.+?:\\/\\/.+?)>");
        rule.pattern = QRegularExpression("<([^\\s`][^`]*?[^\\s`])>");
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);

        // highlight urls with title
        //    rule.pattern = QRegularExpression("\\[(.+?)\\]\\(.+?://.+?\\)");
        //    rule.pattern = QRegularExpression("\\[(.+?)\\]\\(.+\\)\\B");
        rule.pattern = QRegularExpression("\\[([^\\[\\]]+)\\]\\((\\S+|.+?)\\)\\B");
        _highlightingRulesAfter.append(rule);

        // highlight urls with empty title
        //    rule.pattern = QRegularExpression("\\[\\]\\((.+?://.+?)\\)");
        rule.pattern = QRegularExpression("\\[\\]\\((.+?)\\)");
        _highlightingRulesAfter.append(rule);

        // highlight email links
        rule.pattern = QRegularExpression("<(.+?@.+?)>");
        _highlightingRulesAfter.append(rule);

        // highlight reference links
        rule.pattern = QRegularExpression("\\[(.+?)\\]\\s?\\[.+?\\]");
        _highlightingRulesAfter.append(rule);
    }

    // Images
    {
        // highlight images with text
        HighlightingRule rule(HighlighterState::Image);
        rule.pattern = QRegularExpression("!\\[(.+?)\\]\\(.+?\\)");
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);

        // highlight images without text
        rule.pattern = QRegularExpression("!\\[\\]\\((.+?)\\)");
        _highlightingRulesAfter.append(rule);
    }

    // highlight images links
    {
//        HighlightingRule rule;
        HighlightingRule rule(HighlighterState::Link);
        rule.pattern = QRegularExpression("\\[!\\[(.+?)\\]\\(.+?\\)\\]\\(.+?\\)");
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);

        // highlight images links without text
        rule.pattern = QRegularExpression("\\[!\\[\\]\\(.+?\\)\\]\\((.+?)\\)");
        _highlightingRulesAfter.append(rule);
    }

    // highlight inline code
    {
        HighlightingRule rule(HighlighterState::InlineCodeBlock);
//        HighlightingRule rule;
        rule.pattern = QRegularExpression("`(.+?)`");
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);
    }

    // highlight code blocks with four spaces or tabs in front of them
    // and no list character after that
    {
        HighlightingRule rule(HighlighterState::CodeBlock);
//        HighlightingRule rule;
        rule.pattern = QRegularExpression("^((\\t)|( {4,})).+$");
        rule.disableIfCurrentStateIsSet = true;
        _highlightingRulesAfter.append(rule);
    }

    // highlight inline comments
    {
        HighlightingRule rule(HighlighterState::Comment);
        rule.pattern = QRegularExpression("<!\\-\\-(.+?)\\-\\->");
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);

        // highlight comments for Rmarkdown for academic papers
        rule.pattern = QRegularExpression("^\\[.+?\\]: # \\(.+?\\)$");
        _highlightingRulesAfter.append(rule);
    }

    // highlight tables with starting |
    {
        HighlightingRule rule(HighlighterState::Table);
        rule.pattern = QRegularExpression("^\\|.+?\\|$");
        _highlightingRulesAfter.append(rule);
    }
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
    format.setFontWeight(QFont::Bold);
    format.setFontPointSize(defaultFontSize * 1.6);
    _formats[H1] = format;
    format.setFontPointSize(defaultFontSize * 1.5);
    _formats[H2] = format;
    format.setFontPointSize(defaultFontSize * 1.4);
    _formats[H3] = format;
    format.setFontPointSize(defaultFontSize * 1.3);
    _formats[H4] = format;
    format.setFontPointSize(defaultFontSize * 1.2);
    _formats[H5] = format;
    format.setFontPointSize(defaultFontSize * 1.1);
    _formats[H6] = format;
    format.setFontPointSize(defaultFontSize);

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
    format.setForeground(QBrush(QColor(0, 128, 255)));
    format.setFontUnderline(true);
    _formats[Link] = format;

    // set character format for images
    format = QTextCharFormat();
    format.setForeground(QBrush(QColor(0, 191, 0)));
    format.setBackground(QBrush(QColor(228, 255, 228)));
    _formats[Image] = format;

    // set character format for code blocks
    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setBackground(QColor(220, 220, 220));
    _formats[CodeBlock] = format;
    _formats[InlineCodeBlock] = format;

    // set character format for italic
    format = QTextCharFormat();
    format.setFontWeight(QFont::StyleItalic);
    format.setFontItalic(true);
    _formats[Italic] = format;

    // set character format for bold
    format = QTextCharFormat();
    format.setFontWeight(QFont::Bold);
    _formats[Bold] = format;

    // set character format for comments
    format = QTextCharFormat();
    format.setForeground(QBrush(Qt::gray));
    _formats[Comment] = format;

    // set character format for masked syntax
    format = QTextCharFormat();
    format.setForeground(QBrush("#cccccc"));
    _formats[MaskedSyntax] = format;

    // set character format for tables
    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QBrush(QColor("#649449")));
    _formats[Table] = format;

    // set character format for block quotes
    format = QTextCharFormat();
    format.setForeground(QBrush(QColor(Qt::darkRed)));
    _formats[BlockQuote] = format;

    format = QTextCharFormat();
    _formats[HeadlineEnd] = format;

    format = QTextCharFormat();
    _formats[NoState] = format;
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
    QRegularExpression regex("^(#+)\\s+(.+?)$");
    QRegularExpressionMatch match = regex.match(text);
    QTextCharFormat &maskedFormat = _formats[HighlighterState::MaskedSyntax];

    // check for headline blocks with # in front of them
    if (match.hasMatch()) {
        int count = match.captured(1).count();

        // we just have H1 to H6
        count = qMin(count, 6);

        HighlighterState state = HighlighterState(
                HighlighterState::H1 + count - 1);

        QTextCharFormat &format = _formats[state];
        QTextCharFormat currentMaskedFormat = maskedFormat;

        // set the font size from the current rule's font format
        currentMaskedFormat.setFontPointSize(format.fontPointSize());

        // first highlight everything as MaskedSyntax
        setFormat(match.capturedStart(), match.capturedLength(),
                  currentMaskedFormat);

        // then highlight with the real format
        setFormat(match.capturedStart(2), match.capturedLength(2),
                  _formats[state]);

        // set a margin for the current block
        setCurrentBlockMargin(state);

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
            // set the font size from the current rule's font format
            QTextCharFormat currentMaskedFormat = maskedFormat;
            currentMaskedFormat.setFontPointSize(
                    _formats[HighlighterState::H1].fontPointSize());

            setFormat(0, text.length(), currentMaskedFormat);
            setCurrentBlockState(HighlighterState::HeadlineEnd);
            previousBlock.setUserState(HighlighterState::H1);

            // set a margin for the current block
            setCurrentBlockMargin(HighlighterState::H1);

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
            // set the font size from the current rule's font format
            QTextCharFormat currentMaskedFormat = maskedFormat;
            currentMaskedFormat.setFontPointSize(
                    _formats[HighlighterState::H2].fontPointSize());

            setFormat(0, text.length(), currentMaskedFormat);
            setCurrentBlockState(HighlighterState::HeadlineEnd);
            previousBlock.setUserState(HighlighterState::H2);

            // set a margin for the current block
            setCurrentBlockMargin(HighlighterState::H2);

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
 * Sets a margin for the current block
 *
 * @param state
 */
void MarkdownHighlighter::setCurrentBlockMargin(
        MarkdownHighlighter::HighlighterState state) {
    // this is currently disabled because it causes multiple problems:
    // - it prevents "undo" in headlines
    //   https://github.com/pbek/QOwnNotes/issues/520
    // - invisible lines at the end of a note
    //   https://github.com/pbek/QOwnNotes/issues/667
    // - a crash when reaching the invisible lines when the current line is
    //   highlighted
    //   https://github.com/pbek/QOwnNotes/issues/701
    return;

    qreal margin;

    switch (state) {
        case HighlighterState::H1:
            margin = 5;
            break;
        case HighlighterState::H2:
        case HighlighterState::H3:
        case HighlighterState::H4:
        case HighlighterState::H5:
        case HighlighterState::H6:
            margin = 3;
            break;
        default:
            return;
    }

    QTextBlockFormat blockFormat = currentBlock().blockFormat();
    blockFormat.setTopMargin(2);
    blockFormat.setBottomMargin(margin);

    // this prevents "undo" in headlines!
    QTextCursor* myCursor = new QTextCursor(currentBlock());
    myCursor->setBlockFormat(blockFormat);
}

/**
 * Highlight multi-line code blocks
 *
 * @param text
 */
void MarkdownHighlighter::highlightCodeBlock(QString text) {
    QRegularExpression regex("^```\\w*?$");
    QRegularExpressionMatch match = regex.match(text);

    if (match.hasMatch()) {
        setCurrentBlockState(
                previousBlockState() == HighlighterState::CodeBlock ?
                HighlighterState::CodeBlockEnd : HighlighterState::CodeBlock);
        // set the font size from the current rule's font format
        QTextCharFormat &maskedFormat =
                _formats[HighlighterState::MaskedSyntax];
        maskedFormat.setFontPointSize(
                _formats[HighlighterState::CodeBlock].fontPointSize());

        setFormat(0, text.length(), maskedFormat);
    } else if (previousBlockState() == HighlighterState::CodeBlock) {
        setCurrentBlockState(HighlighterState::CodeBlock);
        setFormat(0, text.length(), _formats[HighlighterState::CodeBlock]);
    }
}

/**
 * Highlight multi-line comments
 *
 * @param text
 */
void MarkdownHighlighter::highlightCommentBlock(QString text) {
    bool highlight = false;
    text = text.trimmed();
    QString startText = "<!--";
    QString endText = "-->";

    // we will skip this case because that is an inline comment and causes
    // troubles here
    if (text.startsWith(startText) && text.contains(endText)) {
        return;
    }

    if (text.startsWith(startText) ||
            (!text.endsWith(endText) &&
                    (previousBlockState() == HighlighterState::Comment))) {
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
    QTextCharFormat &maskedFormat = _formats[HighlighterState::MaskedSyntax];

    for(const HighlightingRule &rule : rules) {
            // continue if an other current block state was already set if
            // disableIfCurrentStateIsSet is set
            if (rule.disableIfCurrentStateIsSet &&
                    (currentBlockState() != HighlighterState::NoState)) {
                continue;
            }

            QRegularExpression expression(rule.pattern);
            QRegularExpressionMatchIterator iterator = expression.globalMatch(text);
            int capturingGroup = rule.capturingGroup;
            int maskedGroup = rule.maskedGroup;
            QTextCharFormat &format = _formats[rule.state];

            // store the current block state if useStateAsCurrentBlockState
            // is set
            if (iterator.hasNext() && rule.useStateAsCurrentBlockState) {
                setCurrentBlockState(rule.state);
            }

            // find and format all occurrences
            while (iterator.hasNext()) {
                QRegularExpressionMatch match = iterator.next();

                // if there is a capturingGroup set then first highlight
                // everything as MaskedSyntax and highlight capturingGroup
                // with the real format
                if (capturingGroup > 0) {
                    QTextCharFormat currentMaskedFormat = maskedFormat;
                    // set the font size from the current rule's font format
                    if (format.fontPointSize() > 0) {
                        currentMaskedFormat.setFontPointSize(format.fontPointSize());
                    }

                    setFormat(match.capturedStart(maskedGroup),
                              match.capturedLength(maskedGroup),
                              currentMaskedFormat);
                }

                setFormat(match.capturedStart(capturingGroup),
                          match.capturedLength(capturingGroup),
                          format);
            }
        }
}

void MarkdownHighlighter::setHighlightingOptions(HighlightingOptions options) {
    _highlightingOptions = options;
}
