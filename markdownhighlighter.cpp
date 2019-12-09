
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
#include <QTextDocument>
#include "markdownhighlighter.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <utility>
#include "qownlanguagedata.h"

QHash<QString, MarkdownHighlighter::HighlighterState> MarkdownHighlighter::langStringToEnum;

/**
 * Markdown syntax highlighting
 *
 * markdown syntax:
 * http://daringfireball.net/projects/markdown/syntax
 *
 * @param parent
 * @return
 */
MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent,
                                         HighlightingOptions highlightingOptions)
        : QSyntaxHighlighter(parent),
          _highlightingOptions(highlightingOptions) {
   // _highlightingOptions = highlightingOptions;
    _timer = new QTimer(this);
    QObject::connect(_timer, SIGNAL(timeout()),
                     this, SLOT(timerTick()));
    _timer->start(1000);

    // initialize the highlighting rules
    initHighlightingRules();

    // initialize the text formats
    initTextFormats();

    //initialize code langs
    initCodeLangs();
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
void MarkdownHighlighter::addDirtyBlock(const QTextBlock& block) {
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
        rule.pattern = QRegularExpression(QStringLiteral(R"(^\[.+?\]: \w+://.+$)"));
        _highlightingRulesPre.append(rule);
    }

    // highlight unordered lists
    {
        HighlightingRule rule(HighlighterState::List);
        rule.pattern = QRegularExpression(QStringLiteral("^\\s*[-*+]\\s"));
        rule.useStateAsCurrentBlockState = true;
        _highlightingRulesPre.append(rule);

        // highlight ordered lists
        rule.pattern = QRegularExpression(QStringLiteral(R"(^\s*\d+\.\s)"));
        _highlightingRulesPre.append(rule);
    }

    // highlight block quotes
    {
        HighlightingRule rule(HighlighterState::BlockQuote);
        rule.pattern = QRegularExpression(
                    _highlightingOptions.testFlag(
                        HighlightingOption::FullyHighlightedBlockQuote) ?
                        QStringLiteral("^\\s*(>\\s*.+)") : QStringLiteral("^\\s*(>\\s*)+"));
        _highlightingRulesPre.append(rule);
    }

    // highlight horizontal rulers
    {
        HighlightingRule rule(HighlighterState::HorizontalRuler);
        rule.pattern = QRegularExpression(QStringLiteral("^([*\\-_]\\s?){3,}$"));
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
                    QStringLiteral(R"((?:^|[^\*\b])(?:\*([^\* ][^\*]*?)\*)(?:[^\*\b]|$))"));
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);

        rule.pattern = QRegularExpression(QStringLiteral("\\b_([^_]+)_\\b"));
        _highlightingRulesAfter.append(rule);
    }

    {
        HighlightingRule rule(HighlighterState::Bold);
        // highlight bold
        rule.pattern = QRegularExpression(QStringLiteral(R"(\B\*{2}(.+?)\*{2}\B)"));
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);
        rule.pattern = QRegularExpression(QStringLiteral("\\b__(.+?)__\\b"));
        _highlightingRulesAfter.append(rule);
    }

    {
        HighlightingRule rule(HighlighterState::MaskedSyntax);
        // highlight strike through
        rule.pattern = QRegularExpression(QStringLiteral(R"(\~{2}(.+?)\~{2})"));
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);
    }

    // highlight urls
    {
        HighlightingRule rule(HighlighterState::Link);

        // highlight urls without any other markup
        rule.pattern = QRegularExpression(QStringLiteral(R"(\b\w+?:\/\/[^\s]+)"));
        rule.capturingGroup = 0;
        _highlightingRulesAfter.append(rule);

        // highlight urls with <> but without any . in it
        rule.pattern = QRegularExpression(QStringLiteral(R"(<(\w+?:\/\/[^\s]+)>)"));
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);

        // highlight links with <> that have a .in it
        //    rule.pattern = QRegularExpression("<(.+?:\\/\\/.+?)>");
        rule.pattern = QRegularExpression(QStringLiteral("<([^\\s`][^`]*?\\.[^`]*?[^\\s`])>"));
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);

        // highlight urls with title
        //    rule.pattern = QRegularExpression("\\[(.+?)\\]\\(.+?://.+?\\)");
        //    rule.pattern = QRegularExpression("\\[(.+?)\\]\\(.+\\)\\B");
        rule.pattern = QRegularExpression(QStringLiteral(R"(\[([^\[\]]+)\]\((\S+|.+?)\)\B)"));
        _highlightingRulesAfter.append(rule);

        // highlight urls with empty title
        //    rule.pattern = QRegularExpression("\\[\\]\\((.+?://.+?)\\)");
        rule.pattern = QRegularExpression(QStringLiteral(R"(\[\]\((.+?)\))"));
        _highlightingRulesAfter.append(rule);

        // highlight email links
        rule.pattern = QRegularExpression(QStringLiteral("<(.+?@.+?)>"));
        _highlightingRulesAfter.append(rule);

        // highlight reference links
        rule.pattern = QRegularExpression(QStringLiteral(R"(\[(.+?)\]\[.+?\])"));
        _highlightingRulesAfter.append(rule);
    }

    // Images
    {
        // highlight images with text
        HighlightingRule rule(HighlighterState::Image);
        rule.pattern = QRegularExpression(QStringLiteral(R"(!\[(.+?)\]\(.+?\))"));
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);

        // highlight images without text
        rule.pattern = QRegularExpression(QStringLiteral(R"(!\[\]\((.+?)\))"));
        _highlightingRulesAfter.append(rule);
    }

    // highlight images links
    {
//        HighlightingRule rule;
        HighlightingRule rule(HighlighterState::Link);
        rule.pattern = QRegularExpression(QStringLiteral(R"(\[!\[(.+?)\]\(.+?\)\]\(.+?\))"));
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);

        // highlight images links without text
        rule.pattern = QRegularExpression(QStringLiteral(R"(\[!\[\]\(.+?\)\]\((.+?)\))"));
        _highlightingRulesAfter.append(rule);
    }

    // highlight trailing spaces
    {
        HighlightingRule rule(HighlighterState::TrailingSpace);
        rule.pattern = QRegularExpression(QStringLiteral("( +)$"));
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);
    }

    // highlight inline code
    {
        HighlightingRule rule(HighlighterState::InlineCodeBlock);
//        HighlightingRule rule;
        rule.pattern = QRegularExpression(QStringLiteral("`(.+?)`"));
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);
    }

    // highlight code blocks with four spaces or tabs in front of them
    // and no list character after that
    {
        HighlightingRule rule(HighlighterState::CodeBlock);
//        HighlightingRule rule;
        rule.pattern = QRegularExpression(QStringLiteral("^((\\t)|( {4,})).+$"));
        rule.disableIfCurrentStateIsSet = true;
        _highlightingRulesAfter.append(rule);
    }

    // highlight inline comments
    {
        HighlightingRule rule(HighlighterState::Comment);
        rule.pattern = QRegularExpression(QStringLiteral(R"(<!\-\-(.+?)\-\->)"));
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);

        // highlight comments for Rmarkdown for academic papers
        rule.pattern = QRegularExpression(QStringLiteral(R"(^\[.+?\]: # \(.+?\)$)"));
        _highlightingRulesAfter.append(rule);
    }

    // highlight tables with starting |
    {
        HighlightingRule rule(HighlighterState::Table);
        rule.pattern = QRegularExpression(QStringLiteral("^\\|.+?\\|$"));
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

    /****************************************
     * Formats for syntax highlighting
     ***************************************/

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#F92672"));
    _formats[CodeKeyWord] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#E6DB74"));
    _formats[CodeString] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#75715E"));
    _formats[CodeComment] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#66D9EF"));
    _formats[CodeType] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#E69F66"));
    _formats[CodeOther] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#AE81FF"));
    _formats[CodeNumLiteral] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(Qt::green);
    _formats[CodeBuiltIn] = format;
}

/**
 * @brief initializes the langStringToEnum
 */
void MarkdownHighlighter::initCodeLangs()
{
    MarkdownHighlighter::langStringToEnum =
            QHash<QString, MarkdownHighlighter::HighlighterState> {
        {QLatin1String("bash"),        MarkdownHighlighter::CodeBash},
        {QLatin1String("c"),           MarkdownHighlighter::CodeC},
        {QLatin1String("cpp"),         MarkdownHighlighter::CodeCpp},
        {QLatin1String("cxx"),         MarkdownHighlighter::CodeCpp},
        {QLatin1String("c++"),         MarkdownHighlighter::CodeCpp},
        {QLatin1String("c#"),          MarkdownHighlighter::CodeCSharp},
        {QLatin1String("csharp"),      MarkdownHighlighter::CodeCSharp},
        {QLatin1String("go"),          MarkdownHighlighter::CodeCSharp},
        {QLatin1String("java"),        MarkdownHighlighter::CodeJava},
        {QLatin1String("javascript"),  MarkdownHighlighter::CodeJava},
        {QLatin1String("js"),          MarkdownHighlighter::CodeJs},
        {QLatin1String("php"),         MarkdownHighlighter::CodePHP},
        {QLatin1String("py"),          MarkdownHighlighter::CodePython},
        {QLatin1String("python"),      MarkdownHighlighter::CodePython},
        {QLatin1String("qml"),         MarkdownHighlighter::CodeQML},
        {QLatin1String("rust"),        MarkdownHighlighter::CodeRust},
        {QLatin1String("sh"),          MarkdownHighlighter::CodeBash},
        {QLatin1String("sql"),         MarkdownHighlighter::CodeSQL},
        {QLatin1String("v"),           MarkdownHighlighter::CodeV}
    };

}

/**
 * Sets the text formats
 *
 * @param formats
 */
void MarkdownHighlighter::setTextFormats(
        QHash<HighlighterState, QTextCharFormat> formats) {
    _formats = std::move(formats);
}

/**
 * Sets a text format
 *
 * @param formats
 */
void MarkdownHighlighter::setTextFormat(HighlighterState state,
                                        QTextCharFormat format) {
    _formats[state] = std::move(format);
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

void MarkdownHighlighter::highlightMarkdown(const QString& text) {
    if (!text.isEmpty()) {
        highlightAdditionalRules(_highlightingRulesPre, text);

        // needs to be called after the horizontal ruler highlighting
        highlightHeadline(text);

        highlightAdditionalRules(_highlightingRulesAfter, text);
    }

    highlightCommentBlock(text);
    highlightCodeBlock(text);
    highlightFrontmatterBlock(text);
}

/**
 * Highlight headlines
 *
 * @param text
 */
void MarkdownHighlighter::highlightHeadline(const QString& text) {
    bool headingFound = text.startsWith(QLatin1String("# ")) ||
                        text.startsWith(QLatin1String("## ")) ||
                        text.startsWith(QLatin1String("### ")) ||
                        text.startsWith(QLatin1String("#### ")) ||
                        text.startsWith(QLatin1String("##### ")) ||
                        text.startsWith(QLatin1String("###### "));

    const QTextCharFormat &maskedFormat = _formats[HighlighterState::MaskedSyntax];

    if (headingFound) {
        int count = 0;
        int len = text.length() > 6 ? 6 : text.length();
        //check only first 6 chars of text
        for (int i = 0; i < len; ++i) {
            if (text.at(i) == QLatin1Char('#')) {
                ++count;
            }
        }

        const auto state = HighlighterState(HighlighterState::H1 + count - 1);

        QTextCharFormat &format = _formats[state];
        QTextCharFormat currentMaskedFormat = maskedFormat;

        // set the font size from the current rule's font format
        currentMaskedFormat.setFontPointSize(format.fontPointSize());

        // first highlight everything as MaskedSyntax
        setFormat(0, text.length(), currentMaskedFormat);

        //const int length = text.length() - count;
        // then highlight with the real format
        setFormat(0, text.length(), _formats[state]);

        // set a margin for the current block
        setCurrentBlockMargin(state);

        setCurrentBlockState(state);
        currentBlock().setUserState(state);
        return;
    }

    auto hasOnlyHeadChars = [](const QString &txt, const QChar c) -> bool {
        if (txt.isEmpty()) return false;
        for (int i = 0; i < txt.length(); ++i) {
            if (txt.at(i) != c)
                return false;
        }
        return true;
    };

    // take care of ==== and ---- headlines

    QTextBlock previousBlock = currentBlock().previous();
    const QString &previousText = previousBlock.text();

    const bool pattern1 = hasOnlyHeadChars(text, QLatin1Char('='));
    if (pattern1) {

        if (( (previousBlockState() == HighlighterState::H1) ||
               previousBlockState() == HighlighterState::NoState) &&
               previousText.length() > 0) {
            QTextCharFormat currentMaskedFormat = maskedFormat;
            // set the font size from the current rule's font format
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

    const bool pattern2 = hasOnlyHeadChars(text, QLatin1Char('-'));
    if (pattern2) {

        if (( (previousBlockState() == HighlighterState::H2) ||
               previousBlockState() == HighlighterState::NoState) &&
               previousText.length() > 0) {
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

    //check next block for ====
    QTextBlock nextBlock = currentBlock().next();
    const QString &nextBlockText = nextBlock.text();
    const bool nextHasEqualChars = hasOnlyHeadChars(nextBlockText, QLatin1Char('='));
    if (nextHasEqualChars) {
        setFormat(0, text.length(), _formats[HighlighterState::H1]);
        setCurrentBlockState(HighlighterState::H1);
        currentBlock().setUserState(HighlighterState::H1);
    }
    //check next block for ----
    const bool nextHasMinusChars = hasOnlyHeadChars(nextBlockText, QLatin1Char('-'));
    if (nextHasMinusChars) {
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
void MarkdownHighlighter::highlightCodeBlock(const QString& text) {

    if (text.startsWith(QLatin1String("```"))) {
        if (previousBlockState() != HighlighterState::CodeBlock &&
            previousBlockState() < HighlighterState::CodeCpp) {
            QString lang = text.mid(3, text.length());
            MarkdownHighlighter::HighlighterState progLang = langStringToEnum.value(lang);

            if (progLang >= HighlighterState::CodeCpp) {
                setCurrentBlockState(progLang);
            } else {
                previousBlockState() == HighlighterState::CodeBlock ?
                            setCurrentBlockState(CodeBlockEnd) : setCurrentBlockState(CodeBlock);
            }
        } else if (previousBlockState() == HighlighterState::CodeBlock ||
                   previousBlockState() >= HighlighterState::CodeCpp) {
            setCurrentBlockState(HighlighterState::CodeBlockEnd);
        }

        // set the font size from the current rule's font format
        QTextCharFormat &maskedFormat =
                _formats[HighlighterState::MaskedSyntax];
        maskedFormat.setFontPointSize(
                    _formats[HighlighterState::CodeBlock].fontPointSize());

        setFormat(0, text.length(), maskedFormat);
    } else if (previousBlockState() == HighlighterState::CodeBlock ||
               previousBlockState() >= HighlighterState::CodeCpp) {

        if (previousBlockState() >= HighlighterState::CodeCpp) {
            setCurrentBlockState(previousBlockState());
            highlightSyntax(text);
        } else {
            setFormat(0, text.length(), _formats[HighlighterState::CodeBlock]);
            setCurrentBlockState(HighlighterState::CodeBlock);
        }
    }
}



/**
 * @brief Does the code syntax highlighting
 * @param text
 */
void MarkdownHighlighter::highlightSyntax(const QString &text)
{
    if (text.isEmpty()) return;

    QChar comment;

    QMultiHash<QChar, QString> keywords;
    QMultiHash<QChar, QString> others;
    QMultiHash<QChar, QString> types;
    QMultiHash<QChar, QString> builtin;
    QMultiHash<QChar, QString> literals;

    QStringList wordList;

    switch (currentBlockState()) {
        case HighlighterState::CodeCpp :
            loadCppData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeJs :
            loadJSData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeC :
            loadCppData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeBash :
            loadShellData(types, keywords, builtin, literals, others);
            comment = QLatin1Char('#');
            break;
        case HighlighterState::CodePHP :
            loadPHPData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeQML :
            loadQMLData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodePython :
            loadPythonData(types, keywords, builtin, literals, others);
            comment = QLatin1Char('#');
            break;
        case HighlighterState::CodeRust :
            loadRustData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeJava :
            loadJavaData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeCSharp :
            loadCSharpData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeGo :
            loadGoData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeV :
            loadVData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeSQL :
            loadSQLData(types, keywords, builtin, literals, others);
            break;
    default:
        break;
    }

    // keep the default code block format
    QTextCharFormat f = _formats[CodeBlock];
    setFormat(0, text.length(), f);

    auto applyCodeFormat = [this, &wordList](int i, const QMultiHash<char, QLatin1String> &data,
                        const QString &text, const QTextCharFormat &fmt) -> int {
        // check if we are at the beginning OR if this is the start of a word
        // AND the current char is present in the data structure
        if ( ( i == 0 || !text[i-1].isLetter()) && data.contains(text[i].toLatin1())) {
            wordList = data.values(text[i].toLatin1());
            Q_FOREACH(const QString &word, wordList) {
                if (word == text.midRef(i, word.length())) {
                    //check if we are at the end of text OR if we have a complete word
                    if ( i + word.length() == text.length() ||
                         !text.at(i + word.length()).isLetter()) {
                        setFormat(i, word.length(), fmt);
                        i += word.length();
                    }
                }
            }
        }
        return i;
    };

    //prepare formats
    const QTextCharFormat &formatType = _formats[CodeType];
    const QTextCharFormat &formatKeyword = _formats[CodeKeyWord];
    const QTextCharFormat &formatComment = _formats[CodeComment];
    const QTextCharFormat &formatString = _formats[CodeString];
    const QTextCharFormat &formatNumLit = _formats[CodeNumLiteral];
    const QTextCharFormat &formatBuiltIn = _formats[CodeBuiltIn];
    const QTextCharFormat &formatOther = _formats[CodeOther];

    for (int i=0; i< text.length(); i++) {

        while (!text[i].isLetter()) {
            //inline comment
            if (text[i] == QLatin1Char('/')) {
                if((i+1) < text.length()){
                    if(text[i+1] == QLatin1Char('/')) {
                        setFormat(i, text.length(), formatComment);
                        return;
                    } else if(text[i+1] == QLatin1Char('*')) {
                        int next = text.indexOf(QLatin1String("*/"));
                        if (next == -1) {
                            setFormat(i, text.length(),  formatComment);
                            return;
                        } else {
                            next += 2;
                            setFormat(i, next - i,  formatComment);
                            i = next;
                            if (i >= text.length()) return;
                        }
                    }
                }
            } else if (text[i] == comment) {
                setFormat(i, text.length(), formatComment);
                return;
            //integer literal
            } else if (text[i].isNumber()) {
                int prevBound = text.lastIndexOf(QLatin1Char(' '), i);
                text[prevBound+1] == QLatin1Char(';') || text[prevBound+1] == QLatin1Char(',') ?
                ++prevBound : prevBound;
                int nextBoundary = text.indexOf(QLatin1Char(' '), i);
                text[nextBoundary-1] == QLatin1Char(';') || text[nextBoundary-1] == QLatin1Char(',') ?
                --nextBoundary : nextBoundary;
                prevBound = prevBound == -1 ? 0 : prevBound+1;
                nextBoundary = nextBoundary == -1 ? i : nextBoundary;
                bool allNum = true;
                for (int j = prevBound; j < nextBoundary; j++) {
                    if (text[j].isLetter()) {
                        //hex or decimal
                        if (text[j] == QLatin1Char('x'))
                            continue;
                        allNum = false;
                        break;
                    }
                }
                if (allNum) {
                    i = nextBoundary;
                    setFormat(prevBound, nextBoundary - prevBound, formatNumLit);
                } else {
                    i = nextBoundary;
                    setFormat(prevBound, nextBoundary - prevBound, _formats[CodeBlock]);
                }
            //string literal
            } else if (text[i] == QLatin1Char('\"')) {
                int pos = i;
                int cnt = 1;
                i++;
                //bound check
                if ( (i+1) >= text.length()) return;
                while (i < text.length()) {
                    if (text[i] == QLatin1Char('\"')) {
                        cnt++;
                        i++;
                        break;
                    }
                    i++; cnt++;
                    //bound check
                    if ( (i+1) >= text.length()) {
                        cnt++;
                        break;
                    }
                }
                setFormat(pos, cnt, formatString);
            }  else if (text[i] == QLatin1Char('\'')) {
                int pos = i;
                int cnt = 1;
                i++;
                //bound check
                if ( (i+1) >= text.length()) return;
                while (i < text.length()) {
                    if (text[i] == QLatin1Char('\'')) {
                        cnt++;
                        i++;
                        break;
                    }
                    //bound check
                    if ( (i+1) >= text.length()) {
                        cnt++;
                        break;
                    }
                    i++; cnt++;
                }
                setFormat(pos, cnt, formatString);
            }
            if (i+1 >= text.length()) return;
            i++;
        }

        i = applyCodeFormat(i, types, text, formatType);
        i = applyCodeFormat(i, keywords, text, formatKeyword);
        i = applyCodeFormat(i, literals, text, formatType);
        i = applyCodeFormat(i, builtin, text, formatBuiltIn);

        if (( i == 0 || !text[i-1].isLetter()) && others.contains(text[i])) {
            wordList = others.values(text[i]);
            Q_FOREACH(const QString &word, wordList) {
                if (word == text.midRef(i, word.length())) {
                    if ( i + word.length() == text.length() ||
                         !text.at(i + word.length()).isLetter()) {
                        currentBlockState() == HighlighterState::CodeCpp ?
                        setFormat(i-1, word.length()+1, formatOther) :
                                    setFormat(i, word.length(), formatOther);
                        i += word.length();
                    }
                }
            }
        }
    }
}

/**
 * Highlight multi-line frontmatter blocks
 *
 * @param text
 */
void MarkdownHighlighter::highlightFrontmatterBlock(const QString& text) {
    // return if there is no frontmatter in this document
    if (document()->firstBlock().text() != "---") {
        return;
    }

    if (text == "---") {
        bool foundEnd = previousBlockState() == HighlighterState::FrontmatterBlock;

        // return if the frontmatter block was already highlighted in previous blocks,
        // there just can be one frontmatter block
        if (!foundEnd && document()->firstBlock() != currentBlock()) {
            return;
        }

        setCurrentBlockState(foundEnd ? HighlighterState::FrontmatterBlockEnd : HighlighterState::FrontmatterBlock);

        QTextCharFormat &maskedFormat =
                _formats[HighlighterState::MaskedSyntax];
        setFormat(0, text.length(), maskedFormat);
    } else if (previousBlockState() == HighlighterState::FrontmatterBlock) {
        setCurrentBlockState(HighlighterState::FrontmatterBlock);
        setFormat(0, text.length(), _formats[HighlighterState::MaskedSyntax]);
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
    QString startText = QStringLiteral("<!--");
    QString endText = QStringLiteral("-->");

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
 * Format italics, bolds and links in headings(h1-h6)
 *
 * @param format The format that is being applied
 * @param match The regex match
 * @param capturedGroup The captured group
*/
void MarkdownHighlighter::setHeadingStyles(const QTextCharFormat &format,
                                           const QRegularExpressionMatch &match,
                                           const int capturedGroup) {
    QTextCharFormat f;
    int state = currentBlockState();
    if (state == HighlighterState::H1) f = _formats[H1];
    else if (state == HighlighterState::H2) f = _formats[H2];
    else if (state == HighlighterState::H3) f = _formats[H3];
    else if (state == HighlighterState::H4) f = _formats[H4];
    else if (state == HighlighterState::H5) f = _formats[H5];
    else f = _formats[H6];

    if (format == _formats[HighlighterState::Italic]) {
        f.setFontItalic(true);
        setFormat(match.capturedStart(capturedGroup),
                  match.capturedLength(capturedGroup),
                  f);
        return;
    } else if (format == _formats[HighlighterState::Bold]) {
        setFormat(match.capturedStart(capturedGroup),
                  match.capturedLength(capturedGroup),
                  f);
        return;
    }  else if (format == _formats[HighlighterState::Link]) {
        QTextCharFormat link = _formats[Link];
        link.setFontPointSize(f.fontPointSize());
        if (capturedGroup == 1) {
            setFormat(match.capturedStart(capturedGroup),
                  match.capturedLength(capturedGroup),
                  link);
        }
        return;
    }
/**
 * Waqar144
 * TODO: Test this again and make it work correctly
 * Q: Do we even need this in headings?
 */
//disabling these, as these work, but not as good I think.
//    else if (format == _formats[HighlighterState::InlineCodeBlock]) {
//        QTextCharFormat ff;
//        f.setFontPointSize(1.6);
//        f.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
//        f.setBackground(QColor(220, 220, 220));
//        setFormat(match.capturedStart(capturedGroup),
//                  match.capturedEnd(capturedGroup) - 18,
//                  f);
//        return;
//    }
}

/**
 * Highlights the rules from the _highlightingRules list
 *
 * @param text
 */
void MarkdownHighlighter::highlightAdditionalRules(
        const QVector<HighlightingRule> &rules, const QString& text) {
    const QTextCharFormat &maskedFormat = _formats[HighlighterState::MaskedSyntax];

    for(const HighlightingRule &rule : rules) {
            // continue if another current block state was already set if
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

                    if ((currentBlockState() == HighlighterState::H1 ||
                        currentBlockState() == HighlighterState::H2 ||
                        currentBlockState() == HighlighterState::H3 ||
                        currentBlockState() == HighlighterState::H4 ||
                        currentBlockState() == HighlighterState::H5 ||
                        currentBlockState() == HighlighterState::H6) &&
                        format != _formats[HighlighterState::InlineCodeBlock]) {
                        //setHeadingStyles(format, match, maskedGroup);

                    } else {

                        setFormat(match.capturedStart(maskedGroup),
                              match.capturedLength(maskedGroup),
                              currentMaskedFormat);
                    }
                }

                if ((currentBlockState() == HighlighterState::H1 ||
                    currentBlockState() == HighlighterState::H2 ||
                    currentBlockState() == HighlighterState::H3 ||
                    currentBlockState() == HighlighterState::H4 ||
                    currentBlockState() == HighlighterState::H5 ||
                    currentBlockState() == HighlighterState::H6) &&
                    format != _formats[HighlighterState::InlineCodeBlock]) {
                    setHeadingStyles(format, match, capturingGroup);

                } else {

                setFormat(match.capturedStart(capturingGroup),
                          match.capturedLength(capturingGroup),
                          format);
                }
            }
        }
}

void MarkdownHighlighter::setHighlightingOptions(const HighlightingOptions options) {
    _highlightingOptions = options;
}
