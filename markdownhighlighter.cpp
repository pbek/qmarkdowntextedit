
/*
 * Copyright (c) 2014-2020 Patrizio Bekerle -- <patrizio@bekerle.com>
 * Copyright (c) 2019-2020 Waqar Ahmed      -- <waqar.17a@gmail.com>
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

QHash<QString, MarkdownHighlighter::HighlighterState> MarkdownHighlighter::_langStringToEnum;

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
        emit highlightingFinished();
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
        rule.shouldContain[0] = QStringLiteral("://");
        _highlightingRulesPre.append(rule);
    }

    // highlight lists
    {
        // highlight unordered lists
        HighlightingRule rule(HighlighterState::List);
        rule.pattern = QRegularExpression(QStringLiteral("^\\s*[-*+]\\s"));
        rule.shouldContain[0] = QStringLiteral("- ");
        rule.shouldContain[1] = QStringLiteral("* ");
        rule.shouldContain[2] = QStringLiteral("+ ");
        rule.useStateAsCurrentBlockState = true;
        _highlightingRulesPre.append(rule);

        // highlight ordered lists
        rule.pattern = QRegularExpression(QStringLiteral(R"(^\s*\d+\.\s)"));
        rule.shouldContain[0] = QStringLiteral(". ");
        _highlightingRulesPre.append(rule);
    }

    // highlight checked checkboxes
    {
        HighlightingRule rule(HighlighterState::CheckBoxChecked);
        rule.pattern = QRegularExpression(R"(^\s*[+|\-|\*] (\[x\])(\s+))");
        rule.shouldContain[0] = QStringLiteral("- [x]");
        rule.shouldContain[1] = QStringLiteral("* [x]");
        rule.shouldContain[2] = QStringLiteral("+ [x]");
        rule.capturingGroup = 1;
        _highlightingRulesPre.append(rule);
    }

    // highlight unchecked checkboxes
    {
        HighlightingRule rule(HighlighterState::CheckBoxUnChecked);
        rule.pattern = QRegularExpression(R"(^\s*[+|\-|\*] (\[( |)\])(\s+))");
        rule.shouldContain[0] = QStringLiteral("- [");
        rule.shouldContain[1] = QStringLiteral("* [");
        rule.shouldContain[2] = QStringLiteral("+ [");
        rule.capturingGroup = 1;
        _highlightingRulesPre.append(rule);
    }

    // highlight block quotes
    {
        HighlightingRule rule(HighlighterState::BlockQuote);
        rule.pattern = QRegularExpression(
                    _highlightingOptions.testFlag(
                        HighlightingOption::FullyHighlightedBlockQuote) ?
                        QStringLiteral("^\\s*(>\\s*.+)") : QStringLiteral("^\\s*(>\\s*)+"));
        rule.shouldContain[0] = QStringLiteral("> ");
        _highlightingRulesPre.append(rule);
    }

    // highlight horizontal rulers
    {
        HighlightingRule rule(HighlighterState::HorizontalRuler);
        rule.pattern = QRegularExpression(QStringLiteral("^([*\\-_]\\s?){3,}$"));
        rule.shouldContain[0] = QStringLiteral("---");
        rule.shouldContain[1] = QStringLiteral("***");
        rule.shouldContain[2] = QStringLiteral("+++");
        _highlightingRulesPre.append(rule);
    }

    // highlight tables without starting |
    // we drop that for now, it's far too messy to deal with
//    rule = HighlightingRule();
//    rule.pattern = QRegularExpression("^.+? \\| .+? \\| .+$");
//    rule.state = HighlighterState::Table;
//    _highlightingRulesPre.append(rule);

    // highlight urls
    {
        HighlightingRule rule(HighlighterState::Link);

        // highlight urls without any other markup
        rule.pattern = QRegularExpression(QStringLiteral(R"(\b\w+?:\/\/[^\s>]+)"));
        rule.capturingGroup = 0;
        rule.shouldContain[0] = QStringLiteral("://");
        _highlightingRulesAfter.append(rule);

        // highlight urls with <> but without any . in it
        rule.pattern = QRegularExpression(QStringLiteral(R"(<(\w+?:\/\/[^\s]+)>)"));
        rule.capturingGroup = 1;
        rule.shouldContain[0] = QStringLiteral("://");
        _highlightingRulesAfter.append(rule);

        // highlight links with <> that have a .in it
        //    rule.pattern = QRegularExpression("<(.+?:\\/\\/.+?)>");
        rule.pattern = QRegularExpression(QStringLiteral("<([^\\s`][^`]*?\\.[^`]*?[^\\s`])>"));
        rule.capturingGroup = 1;
        rule.shouldContain[0] = QStringLiteral("<");
        _highlightingRulesAfter.append(rule);

        // highlight urls with title
        //    rule.pattern = QRegularExpression("\\[(.+?)\\]\\(.+?://.+?\\)");
        //    rule.pattern = QRegularExpression("\\[(.+?)\\]\\(.+\\)\\B");
        rule.pattern = QRegularExpression(QStringLiteral(R"(\[([^\[\]]+)\]\((\S+|.+?)\)\B)"));
        rule.shouldContain[0] = QStringLiteral("](");
        _highlightingRulesAfter.append(rule);

        // highlight urls with empty title
        //    rule.pattern = QRegularExpression("\\[\\]\\((.+?://.+?)\\)");
        rule.pattern = QRegularExpression(QStringLiteral(R"(\[\]\((.+?)\))"));
        rule.shouldContain[0] = QStringLiteral("[](");
        _highlightingRulesAfter.append(rule);

        // highlight email links
        rule.pattern = QRegularExpression(QStringLiteral("<(.+?@.+?)>"));
        rule.shouldContain[0] = QStringLiteral("@");
        _highlightingRulesAfter.append(rule);

        // highlight reference links
        rule.pattern = QRegularExpression(QStringLiteral(R"(\[(.+?)\]\[.+?\])"));
        rule.shouldContain[0] = QStringLiteral("[");
        _highlightingRulesAfter.append(rule);
    }

    // Images
    {
        // highlight images with text
        HighlightingRule rule(HighlighterState::Image);
        rule.pattern = QRegularExpression(QStringLiteral(R"(!\[(.+?)\]\(.+?\))"));
        rule.shouldContain[0] = QStringLiteral("![");
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);

        // highlight images without text
        rule.pattern = QRegularExpression(QStringLiteral(R"(!\[\]\((.+?)\))"));
        rule.shouldContain[0] = QStringLiteral("![]");
        _highlightingRulesAfter.append(rule);
    }

    // highlight images links
    {
        HighlightingRule rule(HighlighterState::Link);
        rule.pattern = QRegularExpression(QStringLiteral(R"(\[!\[(.+?)\]\(.+?\)\]\(.+?\))"));
        rule.shouldContain[0] = QStringLiteral("[![");
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);

        // highlight images links without text
        rule.pattern = QRegularExpression(QStringLiteral(R"(\[!\[\]\(.+?\)\]\((.+?)\))"));
        rule.shouldContain[0] = QStringLiteral("[![](");
        _highlightingRulesAfter.append(rule);
    }

    // highlight trailing spaces
    {
        HighlightingRule rule(HighlighterState::TrailingSpace);
        rule.pattern = QRegularExpression(QStringLiteral("( +)$"));
        rule.shouldContain[0] = QString(" \0"); //waqar144: dont use QStringLiteral here.
        rule.capturingGroup = 1;
        _highlightingRulesAfter.append(rule);
    }

    // highlight code blocks with four spaces or tabs in front of them
    // and no list character after that
    {
        HighlightingRule rule(HighlighterState::CodeBlock);
        rule.pattern = QRegularExpression(QStringLiteral("^((\\t)|( {4,})).+$"));
        rule.shouldContain[0] = QChar('\t');
        rule.shouldContain[1] = QStringLiteral("    ");
        rule.disableIfCurrentStateIsSet = true;
        _highlightingRulesAfter.append(rule);
    }

    // highlight inline comments
    {
        HighlightingRule rule(HighlighterState::Comment);
//        rule.pattern = QRegularExpression(QStringLiteral(R"(<!\-\-(.+?)\-\->)"));
//        rule.shouldContain[0] = QStringLiteral("<!--");
//        rule.capturingGroup = 1;
//        _highlightingRulesAfter.append(rule);

        // highlight comments for Rmarkdown for academic papers
        rule.pattern = QRegularExpression(QStringLiteral(R"(^\[.+?\]: # \(.+?\)$)"));
        rule.shouldContain[0] = QStringLiteral("]: # (");
        _highlightingRulesAfter.append(rule);
    }

    // highlight tables with starting |
    {
        HighlightingRule rule(HighlighterState::Table);
        rule.shouldContain[0] = QStringLiteral("|");
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
    format.setForeground(QBrush(QColor(2, 69, 150)));
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
    //format.setBackground(QColor(220, 220, 220));
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
    format.setForeground(QColor("#a39b4e"));
    _formats[CodeString] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#75715E"));
    _formats[CodeComment] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#54aebf"));
    _formats[CodeType] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#db8744"));
    _formats[CodeOther] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#AE81FF"));
    _formats[CodeNumLiteral] = format;

    format = QTextCharFormat();
    format.setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    format.setForeground(QColor("#018a0f"));
    _formats[CodeBuiltIn] = format;
}

/**
 * @brief initializes the langStringToEnum
 */
void MarkdownHighlighter::initCodeLangs()
{
    MarkdownHighlighter::_langStringToEnum =
            QHash<QString, MarkdownHighlighter::HighlighterState> {
        {QLatin1String("bash"),        MarkdownHighlighter::CodeBash},
        {QLatin1String("c"),           MarkdownHighlighter::CodeC},
        {QLatin1String("cpp"),         MarkdownHighlighter::CodeCpp},
        {QLatin1String("cxx"),         MarkdownHighlighter::CodeCpp},
        {QLatin1String("c++"),         MarkdownHighlighter::CodeCpp},
        {QLatin1String("c#"),          MarkdownHighlighter::CodeCSharp},
        {QLatin1String("csharp"),      MarkdownHighlighter::CodeCSharp},
        {QLatin1String("css"),         MarkdownHighlighter::CodeCSS},
        {QLatin1String("go"),          MarkdownHighlighter::CodeCSharp},
        {QLatin1String("html"),        MarkdownHighlighter::CodeXML},
        {QLatin1String("ini"),         MarkdownHighlighter::CodeINI},
        {QLatin1String("java"),        MarkdownHighlighter::CodeJava},
        {QLatin1String("javascript"),  MarkdownHighlighter::CodeJava},
        {QLatin1String("js"),          MarkdownHighlighter::CodeJs},
        {QLatin1String("json"),        MarkdownHighlighter::CodeJSON},
        {QLatin1String("php"),         MarkdownHighlighter::CodePHP},
        {QLatin1String("py"),          MarkdownHighlighter::CodePython},
        {QLatin1String("python"),      MarkdownHighlighter::CodePython},
        {QLatin1String("qml"),         MarkdownHighlighter::CodeQML},
        {QLatin1String("rust"),        MarkdownHighlighter::CodeRust},
        {QLatin1String("sh"),          MarkdownHighlighter::CodeBash},
        {QLatin1String("sql"),         MarkdownHighlighter::CodeSQL},
        {QLatin1String("taggerscript"),MarkdownHighlighter::CodeTaggerScript},
        {QLatin1String("ts"),          MarkdownHighlighter::CodeTypeScript},
        {QLatin1String("typescript"),  MarkdownHighlighter::CodeTypeScript},
        {QLatin1String("v"),           MarkdownHighlighter::CodeV},
        {QLatin1String("vex"),         MarkdownHighlighter::CodeVex},
        {QLatin1String("xml"),         MarkdownHighlighter::CodeXML},
        {QLatin1String("yml"),         MarkdownHighlighter::CodeYAML},
        {QLatin1String("yaml"),        MarkdownHighlighter::CodeYAML}
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
    if (currentBlockState() == HeadlineEnd) {
        currentBlock().previous().setUserState(NoState);
        addDirtyBlock(currentBlock().previous());
    }
    setCurrentBlockState(HighlighterState::NoState);
    currentBlock().setUserState(HighlighterState::NoState);

    highlightMarkdown(text);
    _highlightingFinished = true;
}

void MarkdownHighlighter::highlightMarkdown(const QString& text) {
    const bool isBlockCodeBlock = isCodeBlock(previousBlockState()) ||
                            text.startsWith(QLatin1String("```")) ||
                            text.startsWith(QLatin1String("~~~"));

    if (!text.isEmpty() && !isBlockCodeBlock) {
        highlightAdditionalRules(_highlightingRulesPre, text);

        // needs to be called after the horizontal ruler highlighting
        highlightHeadline(text);

        highlightAdditionalRules(_highlightingRulesAfter, text);

        highlightInlineRules(text);
    }

    highlightCommentBlock(text);
    if (isBlockCodeBlock)
        highlightCodeFence(text);
    highlightFrontmatterBlock(text);
}

/**
 * Highlight headlines
 *
 * @param text
 */
void MarkdownHighlighter::highlightHeadline(const QString& text) {
    bool headingFound = text.at(0) == QChar('#');
    int headingLevel = 0;

    if (headingFound) {
        int i = 1;
        if (i >= text.length())
            return;
        while(i < text.length() && text.at(i) == QChar('#') && i < 6)
            ++i;

        if (i < text.length() && text.at(i) == QChar(' '))
            headingLevel = i;
    }

    if (headingLevel > 0) {
        const auto state = HighlighterState(HighlighterState::H1 + headingLevel - 1);

        setFormat(0, text.length(), _formats[state]);

        // set a margin for the current block
        setCurrentBlockMargin(state);

        setCurrentBlockState(state);
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
    const bool pattern1 = hasOnlyHeadChars(text, QLatin1Char('='));
    if (pattern1) {
        highlightSubHeadline(text, H1);
        return;
    }

    const bool pattern2 = hasOnlyHeadChars(text, QLatin1Char('-'));
    if (pattern2) {
        highlightSubHeadline(text, H2);
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

void MarkdownHighlighter::highlightSubHeadline(const QString &text, HighlighterState state)
{
    const QTextCharFormat &maskedFormat = _formats[HighlighterState::MaskedSyntax];
    QTextBlock previousBlock = currentBlock().previous();
    bool prevEmpty = previousBlock.text().isEmpty();

    if (prevEmpty)
        return;

    //we check for both H1/H2 so that if the user changes his mind, and changes === to ---, changes
    //be reflected immediately
    if (previousBlockState() == H1 || previousBlockState() == H2 || previousBlockState() == NoState) {
        QTextCharFormat currentMaskedFormat = maskedFormat;
        // set the font size from the current rule's font format
        currentMaskedFormat.setFontPointSize(_formats[state].fontPointSize());

        setFormat(0, text.length(), currentMaskedFormat);
        setCurrentBlockState(HeadlineEnd);

        // set a margin for the current block
        setCurrentBlockMargin(state);

        // we want to re-highlight the previous block
        // this must not done directly, but with a queue, otherwise it
        // will crash
        // setting the character format of the previous text, because this
        // causes text to be formatted the same way when writing after
        // the text
        if (previousBlockState() != state) {
            addDirtyBlock(previousBlock);
            previousBlock.setUserState(state);
        }
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

void MarkdownHighlighter::highlightCodeFence(const QString &text) {
    //already in tilde block
    if ((previousBlockState() == CodeBlockTilde || previousBlockState() == CodeBlockTildeComment ||
         previousBlockState() >= CodeCpp + tildeOffset )) {
         highlightCodeBlock(text, QStringLiteral("~~~"));
    //start of a tilde block
    } else if (previousBlockState() == NoState && text.startsWith(QLatin1String("~~~"))) {
         highlightCodeBlock(text, QStringLiteral("~~~"));
    } else {
        //back tick block
        highlightCodeBlock(text);
    }
}

/**
 * Highlight multi-line code blocks
 *
 * @param text
 */
void MarkdownHighlighter::highlightCodeBlock(const QString& text, const QString &opener) {

    if (text.startsWith(opener)) {

        //if someone decides to put these on the same line
        //interpret it as inline code, not code block
        if (text.endsWith(QLatin1String("```")) && text.length() > 3) {
            setFormat(3, text.length() - 3, _formats[HighlighterState::InlineCodeBlock]);
            setFormat(0, 3, _formats[HighlighterState::MaskedSyntax]);
            setFormat(text.length() - 3, 3, _formats[HighlighterState::MaskedSyntax]);
            return;
        }
        if ((previousBlockState() != CodeBlock && previousBlockState() != CodeBlockTilde) &&
            (previousBlockState() != CodeBlockComment && previousBlockState() != CodeBlockTildeComment) &&
            previousBlockState() < CodeCpp) {
            const QString &lang = text.mid(3, text.length()).toLower();
            HighlighterState progLang = _langStringToEnum.value(lang);

            if (progLang >= CodeCpp) {
                const int state = text.startsWith(QLatin1String("```")) ? progLang : progLang + tildeOffset;
                setCurrentBlockState(state);
            } else {
                const int state = opener == QLatin1String("```") ? CodeBlock : CodeBlockTilde;
                setCurrentBlockState(state);
            }
        } else if (isCodeBlock(previousBlockState())) {
            const int state = opener == QLatin1String("```") ? CodeBlockEnd : CodeBlockTildeEnd;
            setCurrentBlockState(state);
        }

        // set the font size from the current rule's font format
        QTextCharFormat &maskedFormat = _formats[MaskedSyntax];
        maskedFormat.setFontPointSize(_formats[CodeBlock].fontPointSize());

        setFormat(0, text.length(), maskedFormat);
    } else if (isCodeBlock(previousBlockState())) {
        setCurrentBlockState(previousBlockState());
        highlightSyntax(text);
    }
}



/**
 * @brief Does the code syntax highlighting
 * @param text
 */
void MarkdownHighlighter::highlightSyntax(const QString &text)
{
    if (text.isEmpty()) return;

    const auto textLen = text.length();

    QChar comment;
    bool isCSS = false;
    bool isYAML = false;


    QMultiHash<char, QLatin1String> keywords{};
    QMultiHash<char, QLatin1String> others{};
    QMultiHash<char, QLatin1String> types{};
    QMultiHash<char, QLatin1String> builtin{};
    QMultiHash<char, QLatin1String> literals{};

    QList<QLatin1String> wordList;

    switch (currentBlockState()) {
        case HighlighterState::CodeCpp:
        case HighlighterState::CodeCpp + tildeOffset:
        case HighlighterState::CodeCppComment:
        case HighlighterState::CodeCppComment + tildeOffset:
            loadCppData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeJs :
        case HighlighterState::CodeJs + tildeOffset:
        case HighlighterState::CodeJsComment :
        case HighlighterState::CodeJsComment + tildeOffset:
            loadJSData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeC :
        case HighlighterState::CodeC + tildeOffset:
        case HighlighterState::CodeCComment :
        case HighlighterState::CodeCComment + tildeOffset:
            loadCppData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeBash :
        case HighlighterState::CodeBash + tildeOffset:
            loadShellData(types, keywords, builtin, literals, others);
            comment = QLatin1Char('#');
            break;
        case HighlighterState::CodePHP :
        case HighlighterState::CodePHP + tildeOffset:
        case HighlighterState::CodePHPComment :
        case HighlighterState::CodePHPComment + tildeOffset:
            loadPHPData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeQML :
        case HighlighterState::CodeQML + tildeOffset:
        case HighlighterState::CodeQMLComment :
        case HighlighterState::CodeQMLComment + tildeOffset:
            loadQMLData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodePython :
        case HighlighterState::CodePython + tildeOffset:
            loadPythonData(types, keywords, builtin, literals, others);
            comment = QLatin1Char('#');
            break;
        case HighlighterState::CodeRust :
        case HighlighterState::CodeRust + tildeOffset:
        case HighlighterState::CodeRustComment :
        case HighlighterState::CodeRustComment + tildeOffset:
            loadRustData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeJava :
        case HighlighterState::CodeJava + tildeOffset:
        case HighlighterState::CodeJavaComment :
        case HighlighterState::CodeJavaComment + tildeOffset:
            loadJavaData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeCSharp :
        case HighlighterState::CodeCSharp + tildeOffset:
        case HighlighterState::CodeCSharpComment :
        case HighlighterState::CodeCSharpComment + tildeOffset:
            loadCSharpData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeGo :
        case HighlighterState::CodeGo + tildeOffset:
        case HighlighterState::CodeGoComment :
        case HighlighterState::CodeGoComment + tildeOffset:
            loadGoData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeV :
        case HighlighterState::CodeV + tildeOffset:
        case HighlighterState::CodeVComment :
        case HighlighterState::CodeVComment + tildeOffset:
            loadVData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeSQL :
        case HighlighterState::CodeSQL + tildeOffset:
            loadSQLData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeJSON :
        case HighlighterState::CodeJSON + tildeOffset:
            loadJSONData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeXML :
        case HighlighterState::CodeXML + tildeOffset:
            xmlHighlighter(text);
            return;
        case HighlighterState::CodeCSS :
        case HighlighterState::CodeCSS + tildeOffset:
        case HighlighterState::CodeCSSComment :
        case HighlighterState::CodeCSSComment + tildeOffset:
            isCSS = true;
            loadCSSData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeTypeScript:
        case HighlighterState::CodeTypeScript + tildeOffset:
        case HighlighterState::CodeTypeScriptComment:
        case HighlighterState::CodeTypeScriptComment + tildeOffset:
            loadTypescriptData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeYAML:
        case HighlighterState::CodeYAML + tildeOffset:
            isYAML = true;
            comment = QLatin1Char('#');
            loadYAMLData(types, keywords, builtin, literals, others);
            break;
        case HighlighterState::CodeINI:
        case HighlighterState::CodeINI + tildeOffset:
            iniHighlighter(text);
            return;
        case HighlighterState::CodeTaggerScript:
        case HighlighterState::CodeTaggerScript + tildeOffset:
            taggerScriptHighlighter(text);
            return;
        case HighlighterState::CodeVex:
        case HighlighterState::CodeVex + tildeOffset:
        case HighlighterState::CodeVexComment:
        case HighlighterState::CodeVexComment + tildeOffset:
            loadVEXData(types, keywords, builtin, literals, others);
            break;
        default:
            comment = QChar('\a');
            break;
        }

    // keep the default code block format
    // this statement is very slow
    // TODO: do this formatting when necessary instead of
    // applying it to the whole block in the beginning
    setFormat(0, textLen, _formats[CodeBlock]);

    auto applyCodeFormat = [this, &wordList](int i, const QMultiHash<char, QLatin1String> &data,
                        const QString &text, const QTextCharFormat &fmt) -> int {
        // check if we are at the beginning OR if this is the start of a word
        // AND the current char is present in the data structure
        if ( ( i == 0 || !text[i-1].isLetter()) && data.contains(text[i].toLatin1())) {
            wordList = data.values(text[i].toLatin1());
#if QT_VERSION >= 0x050700
            for(const QLatin1String &word : qAsConst(wordList)) {
#else
            for(const QLatin1String &word : wordList) {
#endif
                if (word == text.midRef(i, word.size())) {
                    //check if we are at the end of text OR if we have a complete word
                    if ( i + word.size() == text.length() ||
                         !text.at(i + word.size()).isLetter()) {
                        setFormat(i, word.size(), fmt);
                        i += word.size();
                    }
                }
            }
        }
        return i;
    };

    const QTextCharFormat &formatType = _formats[CodeType];
    const QTextCharFormat &formatKeyword = _formats[CodeKeyWord];
    const QTextCharFormat &formatComment = _formats[CodeComment];
    const QTextCharFormat &formatNumLit = _formats[CodeNumLiteral];
    const QTextCharFormat &formatBuiltIn = _formats[CodeBuiltIn];
    const QTextCharFormat &formatOther = _formats[CodeOther];

    for (int i=0; i< textLen; ++i) {

        if (currentBlockState() % 2 != 0) goto Comment;

        while (i < textLen && !text[i].isLetter()) {
            if (text[i].isSpace()) {
                ++i;
                //make sure we don't cross the bound
                if (i == textLen) break;
                if (text[i].isLetter()) break;
                else continue;
            }
            //inline comment
            if (comment.isNull() && text[i] == QLatin1Char('/')) {
                if((i+1) < textLen){
                    if(text[i+1] == QLatin1Char('/')) {
                        setFormat(i, textLen, formatComment);
                        return;
                    } else if(text[i+1] == QLatin1Char('*')) {
                        Comment:
                        int next = text.indexOf(QLatin1String("*/"), i);
                        if (next == -1) {
                            //we didn't find a comment end.
                            //Check if we are already in a comment block
                            if (currentBlockState() % 2 == 0)
                                setCurrentBlockState(currentBlockState() + 1);
                            setFormat(i, textLen,  formatComment);
                            return;
                        } else {
                            //we found a comment end
                            //mark this block as code if it was previously comment
                            //first check if the comment ended on the same line
                            //if modulo 2 is not equal to zero, it means we are in a comment
                            //-1 will set this block's state as language
                            if (currentBlockState() % 2 != 0) {
                                setCurrentBlockState(currentBlockState() - 1);
                            }
                            next += 2;
                            setFormat(i, next - i,  formatComment);
                            i = next;
                            if (i >= textLen) return;
                        }
                    }
                }
            } else if (text[i] == comment) {
                setFormat(i, textLen, formatComment);
                i = textLen;
                break;
            //integer literal
            } else if (text[i].isNumber()) {
               i = highlightNumericLiterals(text, i);
            //string literals
            } else if (text[i] == QLatin1Char('\"')) {
               i = highlightStringLiterals('\"', text, i);
            }  else if (text[i] == QLatin1Char('\'')) {
               i = highlightStringLiterals('\'', text, i);
            }
            if (i >= textLen) {
                break;
            }
            ++i;
        }

        int pos = i;

        if (i == textLen || !text[i].isLetter()) continue;

        /* Highlight Types */
        i = applyCodeFormat(i, types, text, formatType);
        /************************************************
         next letter is usually a space, in that case
         going forward is useless, so continue;
         We can ++i here and go to the beginning of the next word
         so that the next formatter can check for a match but this will
         cause problems in case the next word is also of 'Type' or the current
         type(keyword/builtin). We can work around it and reset the value of i
         in the beginning of the loop to the word's first letter but its not
         as efficient.
         ************************************************/
        if (i == textLen || !text[i].isLetter()) continue;

        /* Highlight Keywords */
        i = applyCodeFormat(i, keywords, text, formatKeyword);
        if (i == textLen || !text[i].isLetter()) continue;

        /* Highlight Literals (true/false/NULL,nullptr) */
        i = applyCodeFormat(i, literals, text, formatNumLit);
        if (i == textLen || !text[i].isLetter()) continue;

        /* Highlight Builtin library stuff */
        i = applyCodeFormat(i, builtin, text, formatBuiltIn);
        if (i == textLen || !text[i].isLetter()) continue;

        /* Highlight other stuff (preprocessor etc.) */
        if (( i == 0 || !text[i-1].isLetter()) && others.contains(text[i].toLatin1())) {
            wordList = others.values(text[i].toLatin1());
#if QT_VERSION >= 0x050700
            for(const QLatin1String &word : qAsConst(wordList)) {
#else
            for(const QLatin1String &word : wordList) {
#endif
                if (word == text.midRef(i, word.size()).toLatin1()) {
                    if ( i + word.size() == textLen ||
                         !text.at(i + word.size()).isLetter()) {
                        //for C/C++ we do -1 to highlight the '#' in preprocessor
                        currentBlockState() == CodeCpp || currentBlockState() == CodeC ?
                        setFormat(i-1, word.size()+1, formatOther) :
                                    setFormat(i, word.size(), formatOther);
                        i += word.size();
                    }
                }
            }
        }

        //we were unable to find any match, lets skip this word
        if (pos == i) {
            int cnt = i;
            while (cnt < textLen) {
                if (!text[cnt].isLetter()) break;
                ++cnt;
            }
            i = cnt;
        }
    }

    /***********************
    **** POST PROCESSORS ***
    ***********************/

    if (isCSS) cssHighlighter(text);
    if (isYAML) ymlHighlighter(text);
}

/**
 * @brief Highlight string literals in code
 * @param strType str type i.e., ' or "
 * @param text the text being scanned
 * @param i pos of i in loop
 * @return pos of i after the string
 */
int MarkdownHighlighter::highlightStringLiterals(QChar strType, const QString &text, int i) {
    setFormat(i, 1,  _formats[CodeString]);
    ++i;

    while (i < text.length()) {
        //look for string end
        //make sure it's not an escape seq
        if (text.at(i) == strType && text.at(i-1) != QLatin1Char('\\')) {
            setFormat(i, 1,  _formats[CodeString]);
            ++i;
            break;
        }
        //look for escape sequence
        if (text.at(i) == QLatin1Char('\\') && (i+1) < text.length()) {
            int len = 0;
            switch(text.at(i+1).toLatin1()) {
            case 'a':
            case 'b':
            case 'e':
            case 'f':
            case 'n':
            case 'r':
            case 't':
            case 'v':
            case '\'':
            case '"':
            case '\\':
            case '\?':
                //2 because we have to highlight \ as well as the following char
                len = 2;
                break;
            //octal esc sequence \123
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            {
                if (i + 4 <= text.length()) {
                    bool isCurrentOctal = true;
                    if (!isOctal(text.at(i+2).toLatin1())) {
                        isCurrentOctal = false;
                        break;
                    }
                    if (!isOctal(text.at(i+3).toLatin1())) {
                        isCurrentOctal = false;
                        break;
                    }
                    len = isCurrentOctal ? 4 : 0;
                }
                break;
            }
            //hex numbers \xFA
            case 'x':
            {
                if (i + 3 <= text.length()) {
                    bool isCurrentHex = true;
                    if (!isHex(text.at(i+2).toLatin1())) {
                        isCurrentHex = false;
                        break;
                    }
                    if (!isHex(text.at(i+3).toLatin1())) {
                        isCurrentHex = false;
                        break;
                    }
                    len = isCurrentHex ? 4 : 0;
                }
                break;
            }
            //TODO: implement unicode code point escaping
            default:
                break;
            }

            //if len is zero, that means this wasn't an esc seq
            //increment i so that we skip this backslash
            if (len == 0) {
                setFormat(i, 1,  _formats[CodeString]);
                ++i;
                continue;
            }

            setFormat(i, len, _formats[CodeNumLiteral]);
            i += len;
            continue;
        }
        setFormat(i, 1,  _formats[CodeString]);
        ++i;
    }
    return i;
}

/**
 * @brief Highlight numeric literals in code
 * @param text the text being scanned
 * @param i pos of i in loop
 * @return pos of i after the number
 *
 * @details it doesn't highlight the following yet:
 *  - 1000'0000
 */
int MarkdownHighlighter::highlightNumericLiterals(const QString &text, int i)
{
    bool isPreAllowed = false;
    if (i == 0) isPreAllowed = true;
    else {
        //these values are allowed before a number
        switch(text.at(i - 1).toLatin1()) {
        //css number
        case ':':
            if (currentBlockState() == CodeCSS)
                isPreAllowed = true;
            break;
        case '[':
        case '(':
        case '{':
        case ' ':
        case ',':
        case '=':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '<':
        case '>':
            isPreAllowed = true;
            break;
        }
    }

    if (!isPreAllowed) return i;

    int start = i;

    if ((i+1) >= text.length()) {
        setFormat(i, 1, _formats[CodeNumLiteral]);
        return ++i;
    }

    ++i;
    //hex numbers highlighting (only if there's a preceding zero)
    if (text.at(i) == QChar('x') && text.at(i - 1) == QChar('0'))
        ++i;

    while (i < text.length()) {
        if (!text.at(i).isNumber() && text.at(i) != QChar('.') &&
             text.at(i) != QChar('e')) //exponent
            break;
        ++i;
    }

    bool isPostAllowed = false;
    if (i == text.length()) {
        //cant have e at the end
        if (text.at(i - 1) != QChar('e'))
            isPostAllowed = true;
    } else {
        //these values are allowed after a number
        switch(text.at(i).toLatin1()) {
        case ']':
        case ')':
        case '}':
        case ' ':
        case ',':
        case '=':
        case '+':
        case '-':
        case '*':
        case '/':
        case '%':
        case '>':
        case '<':
        case ';':
            isPostAllowed = true;
            break;
        // for 100u, 1.0F
        case 'p':
            if (currentBlockState() == CodeCSS)
                if (i + 1 < text.length() && text.at(i+1) == QChar('x')) {
                    if (i + 2 == text.length() || !text.at(i+2).isLetterOrNumber())
                    isPostAllowed = true;
                }
            break;
        case 'e':
            if (currentBlockState() == CodeCSS)
                if (i + 1 < text.length() && text.at(i+1) == QChar('m')) {
                    if (i + 2 == text.length() || !text.at(i+2).isLetterOrNumber())
                    isPostAllowed = true;
                }
            break;
        case 'u':
        case 'l':
        case 'f':
        case 'U':
        case 'L':
        case 'F':
            if (i + 1 == text.length() || !text.at(i+1).isLetterOrNumber()) {
                isPostAllowed = true;
                ++i;
            }
            break;
        }
    }
    if (isPostAllowed) {
        int end = i;
        setFormat(start, end - start, _formats[CodeNumLiteral]);
    }
    //decrement so that the index is at the last number, not after it
    return --i;
}

/**
 * @brief The Tagger Script highlighter
 * @param text
 * @details his function is responsible for taggerscript highlighting.
 * It highlights anything between a (inclusive) '&' and a (exclusive) '(' as a function.
 * An exception is the '$noop()'function, which get highlighted as a comment.
 *
 * It has basic error detection when there is an unlcosed %Metadata Variable%
 */
void MarkdownHighlighter::taggerScriptHighlighter(const QString &text) {
    if (text.isEmpty()) return;
    const auto textLen = text.length();

    for (int i = 0; i < textLen; ++i) {

        //highlight functions, unless it's a comment function
        if (text.at(i) == QChar('$') && text.midRef(i, 5) != QLatin1String("$noop") ) {
            int next = text.indexOf(QChar('('), i);
            if (next == -1) break;
            setFormat(i, next-i, _formats[CodeKeyWord]);
            i = next;
        }

        //highlight variables
        if (text.at(i) == QChar('%')) {
            int next = text.indexOf(QChar('%'), i+1);
            int start = i;
            i++;
            if (next != -1){
                setFormat(start, next-start+1, _formats[CodeType]);
                i = next;
            }else{
                // error highlighting
                QTextCharFormat errorFormat = _formats[NoState];
                errorFormat.setUnderlineColor(Qt::red);
                errorFormat.setUnderlineStyle(QTextCharFormat::WaveUnderline);
                setFormat(start, 1, errorFormat);
            }
        }

        //highlight comments
        if (text.midRef(i, 5) == QLatin1String("$noop")) {
            int next = text.indexOf(QChar(')'), i);
            if (next == -1) break;
            setFormat(i, next-i+1, _formats[CodeComment]);
            i = next;
        }

        //highlight escape chars
        if (text.at(i) == QChar('\\')) {
            setFormat(i,2, _formats[CodeOther]);
            i++;
        }
    }
}

/**
 * @brief The YAML highlighter
 * @param text
 * @details This function post processes a line after the main syntax
 * highlighter has run for additional highlighting. It does these things
 *
 * If the current line is a comment, skip it
 *
 * Highlight all the words that have a colon after them as 'keyword' except:
 * If the word is a string, skip it.
 * If the colon is in between a path, skip it (C:\)
 *
 * Once the colon is found, the function will skip every character except 'h'
 *
 * If an h letter is found, check the next 4/5 letters for http/https and
 * highlight them as a link (underlined)
 */
void MarkdownHighlighter::ymlHighlighter(const QString &text) {
    if (text.isEmpty()) return;
    const auto textLen = text.length();
    bool colonNotFound = false;

    //if this is a comment don't do anything and just return
    if (text.trimmed().at(0) == QChar('#')) return;

    for (int i = 0; i < textLen; ++i) {
        if (!text.at(i).isLetter()) continue;

        if (colonNotFound && text.at(i) != QChar('h')) continue;

        //we found a string literal, skip it
        if (i != 0 && text.at(i-1) == QChar('"')) {
            int next = text.indexOf(QChar('"'), i);
            if (next == -1) break;
            i = next;
            continue;
        }

        if (i != 0 && text.at(i-1) == QChar('\'')) {
            int next = text.indexOf(QChar('\''), i);
            if (next == -1) break;
            i = next;
            continue;
        }


        int colon = text.indexOf(QChar(':'), i);

        //if colon isn't found, we set this true
        if (colon == -1) colonNotFound = true;

        if (!colonNotFound) {
            //if the line ends here, format and return
            if (colon+1 == textLen) {
                setFormat(i, colon - i, _formats[CodeKeyWord]);
                return;
            } else {
                //colon is found, check if it isn't some path or something else
                if (!(text.at(colon+1) == QChar('\\') && text.at(colon+1) == QChar('/'))) {
                    setFormat(i, colon - i, _formats[CodeKeyWord]);
                }
            }
        }

        //underlined links
        if (text.at(i) == QChar('h')) {
            if (text.midRef(i, 5) == QLatin1String("https") ||
                    text.midRef(i, 4) == QLatin1String("http")) {
                int space = text.indexOf(QChar(' '), i);
                if (space == -1) space = textLen;
                QTextCharFormat f = _formats[CodeString];
                f.setUnderlineStyle(QTextCharFormat::SingleUnderline);
                setFormat(i, space - i, f);
                i = space;
            }
        }
    }
}

/**
 * @brief The INI highlighter
 * @param text The text being highlighted
 * @details This function is responsible for ini highlighting.
 * It has basic error detection when
 * (1) You opened a section but didn't close with bracket e.g [Section
 * (2) You wrote an option but it didn't have an '='
 * Such errors will be marked with a dotted red underline
 *
 * It has comment highlighting support. Everything after a ';' will
 * be highlighted till the end of the line.
 *
 * An option value pair will be highlighted regardless of space. Example:
 * Option 1 = value
 * In this, 'Option 1' will be highlighted completely and not just '1'.
 * I am not sure about its correctness but for now its like this.
 *
 * The loop is unrolled frequently upon a match. Before adding anything
 * new be sure to test in debug mode and apply bound checking as required.
 */
void MarkdownHighlighter::iniHighlighter(const QString &text) {
    if (text.isEmpty()) return;
    const auto textLen = text.length();

    for (int i = 0; i < textLen; ++i) {
        //start of a [section]
        if (text.at(i) == QChar('[')) {
            QTextCharFormat sectionFormat = _formats[CodeType];
            int sectionEnd = text.indexOf(QChar(']'), i);
            //if an end bracket isn't found, we apply red underline to show error
            if (sectionEnd == -1) {
                sectionFormat.setUnderlineStyle(QTextCharFormat::DotLine);
                sectionFormat.setUnderlineColor(Qt::red);
                sectionEnd = textLen;
            }
            sectionEnd++;
            setFormat(i, sectionEnd - i, sectionFormat);
            i = sectionEnd;
            if (i >= textLen) break;
        }

        //comment ';'
        else if (text.at(i) == QChar(';')) {
            setFormat(i, textLen - i, _formats[CodeComment]);
            i = textLen;
            break;
        }

        //key-val
        else if (text.at(i).isLetter()) {
            QTextCharFormat format = _formats[CodeKeyWord];
            int equalsPos = text.indexOf(QChar('='), i);
            if (equalsPos == -1) {
                format.setUnderlineColor(Qt::red);
                format.setUnderlineStyle(QTextCharFormat::DotLine);
                equalsPos = textLen;
            }
            setFormat(i, equalsPos - i, format);
            i = equalsPos - 1;
            if (i >= textLen) break;
        }
        //skip everything after '=' (except comment)
        else if (text.at(i) == QChar('=')) {
            int findComment = text.indexOf(QChar(';'), i);
            if (findComment == -1) break;
            i = findComment - 1;
        }
    }
}

void MarkdownHighlighter::cssHighlighter(const QString &text)
{
    if (text.isEmpty()) return;
    const auto textLen = text.length();
    for (int i = 0; i<textLen; ++i) {
        if (text[i] == QLatin1Char('.') || text[i] == QLatin1Char('#')) {
            if (i+1 >= textLen) return;
            if (text[i + 1].isSpace() || text[i+1].isNumber()) continue;
            int space = text.indexOf(QLatin1Char(' '), i);
            if (space < 0) {
                space = text.indexOf(QChar('{'));
                if (space < 0) {
                    space = textLen;
                }
            }
            setFormat(i, space - i, _formats[CodeKeyWord]);
            i = space;
        } else if (text[i] == QLatin1Char('c')) {
            if (text.midRef(i, 5) == QLatin1String("color")) {
                i += 5;
                int colon = text.indexOf(QLatin1Char(':'), i);
                if (colon < 0) continue;
                i = colon;
                i++;
                while(i < textLen) {
                    if (!text[i].isSpace()) break;
                    i++;
                }
                int semicolon = text.indexOf(QLatin1Char(';'));
                if (semicolon < 0) semicolon = textLen;
                QString color = text.mid(i, semicolon-i);
                QTextCharFormat f = _formats[CodeBlock];
                QColor c(color);
                if (color.startsWith(QLatin1String("rgb"))) {
                    int t = text.indexOf(QChar('('), i);
                    int rPos = text.indexOf(QChar(','), t);
                    int gPos = text.indexOf(QChar(','), rPos+1);
                    int bPos = text.indexOf(QChar(')'), gPos);
                    if (rPos > -1 && gPos > -1 && bPos > -1) {
                        const QStringRef r = text.midRef(t+1, rPos - (t+1));
                        const QStringRef g = text.midRef(rPos+1, gPos - (rPos + 1));
                        const QStringRef b = text.midRef(gPos+1, bPos - (gPos+1));
                        c.setRgb(r.toInt(), g.toInt(), b.toInt());
                    } else {
                        c = _formats[HighlighterState::NoState].background().color();
                    }
                }

                if (!c.isValid()) {
                    continue;
                }

                int lightness{};
                QColor foreground;
                //really dark
                if (c.lightness() <= 20) {
                    foreground = Qt::white;
                } else if (c.lightness() > 20 && c.lightness() <= 51){
                    foreground = QColor("#ccc");
                } else if (c.lightness() > 51 && c.lightness() <= 78){
                    foreground = QColor("#bbb");
                } else if (c.lightness() > 78 && c.lightness() <= 110){
                    foreground = QColor("#bbb");
                } else if (c.lightness() > 127) {
                    lightness = c.lightness() + 100;
                    foreground = c.darker(lightness);
                }
                else {
                    lightness = c.lightness() + 100;
                    foreground = c.lighter(lightness);
                }

                f.setBackground(c);
                f.setForeground(foreground);
                setFormat(i, semicolon - i, QTextCharFormat()); //clear prev format
                setFormat(i, semicolon - i, f);
                i = semicolon;
            }
        }
    }
}


void MarkdownHighlighter::xmlHighlighter(const QString &text) {
    if (text.isEmpty()) return;
    const auto textLen = text.length();

    setFormat(0, textLen, _formats[CodeBlock]);

    for (int i = 0; i < textLen; ++i) {
        if (i + 1 < textLen && text[i] == QLatin1Char('<') && text[i+1] != QLatin1Char('!')) {

            int found = text.indexOf(QLatin1Char('>'), i);
            if (found > 0) {
                ++i;
                if (text[i] == QLatin1Char('/')) ++i;
                setFormat(i, found - i, _formats[CodeKeyWord]);
            }
        }

        if (text[i] == QLatin1Char('=')) {
            int lastSpace = text.lastIndexOf(QLatin1Char(' '), i);
            if (lastSpace == i-1) lastSpace = text.lastIndexOf(QLatin1Char(' '), i-2);
            if (lastSpace > 0) {
                setFormat(lastSpace, i - lastSpace, _formats[CodeBuiltIn]);
            }
        }

        if (text[i] == QLatin1Char('\"')) {
            int pos = i;
            int cnt = 1;
            ++i;
            //bound check
            if ( (i+1) >= textLen) return;
            while (i < textLen) {
                if (text[i] == QLatin1Char('\"')) {
                    ++cnt;
                    ++i;
                    break;
                }
                ++i; ++cnt;
                //bound check
                if ( (i+1) >= textLen) {
                    ++cnt;
                    break;
                }
            }
            setFormat(pos, cnt, _formats[CodeString]);
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
    if (document()->firstBlock().text() != QLatin1String("---")) {
        return;
    }

    if (text == QLatin1String("---")) {
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

    if (!text.startsWith(startText) && text.contains(startText))
        return;

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
void MarkdownHighlighter::setHeadingStyles(MarkdownHighlighter::HighlighterState rule,
                                           const QRegularExpressionMatch &match,
                                           const int capturedGroup) {
    MarkdownHighlighter::HighlighterState state = static_cast<HighlighterState>(currentBlockState());
    QTextCharFormat f = _formats[state];

    if (rule == HighlighterState::Link) {
        QTextCharFormat link = _formats[Link];
        link.setFontPointSize(f.fontPointSize());
        if (capturedGroup == 1) {
            setFormat(match.capturedStart(capturedGroup),
                  match.capturedLength(capturedGroup),
                  link);
        }
        return;
    }
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

        bool contains = false;
        if (text.contains(rule.shouldContain[0])) {
            contains = true;
        }
        if ( (!contains && !rule.shouldContain[1].isEmpty()) && text.contains(rule.shouldContain[1])){
            contains = true;
        }
        if ( (!contains && !rule.shouldContain[2].isEmpty()) && text.contains(rule.shouldContain[2])){
            contains = true;
        }
        if (!contains)
            continue;

        QRegularExpression expression(rule.pattern);
        QRegularExpressionMatchIterator iterator = expression.globalMatch(text);
        uint8_t capturingGroup = rule.capturingGroup;
        uint8_t maskedGroup = rule.maskedGroup;
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

                if (currentBlockState() >= H1 && currentBlockState() <= H6) {
                    //setHeadingStyles(format, match, maskedGroup);

                } else {

                    setFormat(match.capturedStart(maskedGroup),
                              match.capturedLength(maskedGroup),
                              currentMaskedFormat);
                }
            }

            if (currentBlockState() >= H1 && currentBlockState() <= H6) {
                setHeadingStyles(rule.state, match, capturingGroup);

            } else {
                setFormat(match.capturedStart(capturingGroup),
                          match.capturedLength(capturingGroup),
                          format);
            }
        }
    }
}

void MarkdownHighlighter::highlightInlineRules(const QString &text)
{
    if (text.isEmpty())
        return;

    bool isEmStrongDone = false;

    for (int i = 0; i < text.length(); ++i) {
        if (text.at(i) == QLatin1Char('`') || text.at(i) == QLatin1Char('~')) {
            i = highlightInlineSpans(text, i, text.at(i));
        } else if (text.at(i) == QLatin1Char('<') && i + 3 < text.length() &&
                   text.at(i + 1) == QLatin1Char('!') && text.at(i + 2) == QLatin1Char('-') &&
                   text.at(i + 3) == QLatin1Char('-')
                   ) {
            highlightInlineComment(text, i);
        } else if (!isEmStrongDone &&
                 (text.at(i) == QLatin1Char('*') || text.at(i) == QLatin1Char('_'))) {
            highlightEmAndStrong(text, i);
            isEmStrongDone = true;
        }
    }
}

/** @brief highlight inline code spans -> `code` and highlight strikethroughs
 *
 * ---- TESTS ----
`foo`
-> <code>foo</code>
`` foo ` bar ``
-> <code>foo ` bar</code>
` `` `
-> <code>``</code>
`foo\`bar`
-><code>foo\</code>bar`
``foo`bar``
-><code>foo`bar</code>
` foo `` bar `
<code>foo `` bar</code>
*/
int MarkdownHighlighter::highlightInlineSpans(const QString &text, int currentPos, const QChar c)
{
    if (currentPos + 1 >= text.length())
        return currentPos;

    for (int i = currentPos; i < text.length(); ++i) {
        if (text.at(i) != c)
            continue;

        //found a backtick
        int len = 0;
        int pos = i;

        if (i != 0 && text.at(i - 1) == QChar('\\'))
            continue;

        //keep moving forward in backtick sequence;
        while(pos < text.length() && text.at(pos) == c) {
            ++len;
            ++pos;
        }

        const QStringRef seq = text.midRef(i, len);
        int start = i;
        i += len;
        int next = text.indexOf(seq, i);
        if (next == -1) {
            return currentPos + len;
        }
        if (next + len < text.length() && text.at(next + len) == c)
            continue;

        QTextCharFormat fmt = QSyntaxHighlighter::format(start + 1);
        QTextCharFormat inlineFmt = _formats[NoState];
        if (c != QLatin1Char('~'))
            inlineFmt = _formats[InlineCodeBlock];
        inlineFmt.setFontUnderline(fmt.fontUnderline());
        inlineFmt.setUnderlineStyle(fmt.underlineStyle());
        if (fmt.fontPointSize() > 0)
            inlineFmt.setFontPointSize(fmt.fontPointSize());
        inlineFmt.setFontItalic(fmt.fontItalic());
        if (c == QLatin1Char('~'))
            inlineFmt.setFontStrikeOut(true);
        setFormat(start + 1, next - start, inlineFmt);
        //highlight backticks as masked
        setFormat(start, len, _formats[MaskedSyntax]);
        setFormat(next, len, _formats[MaskedSyntax]);

        i = next + len;
        currentPos = i;
    }

    return currentPos;
}

struct Delimiter {
    int pos;
    int len;
    int jump;
    int end;
    bool open;
    bool close;
    QChar marker;
};

bool isMDAsciiPunct(QChar ch) {
    switch (ch.toLatin1()) {
    case 0x21/* ! */:
    case 0x22/* " */:
    case 0x23/* # */:
    case 0x24/* $ */:
    case 0x25/* % */:
    case 0x26/* & */:
    case 0x27/* ' */:
    case 0x28/* ( */:
    case 0x29/* ) */:
    case 0x2A/* * */:
    case 0x2B/* + */:
    case 0x2C/* , */:
    case 0x2D/* - */:
    case 0x2E/* . */:
    case 0x2F/* / */:
    case 0x3A/* : */:
    case 0x3B/* ; */:
    case 0x3C/* < */:
    case 0x3D/* = */:
    case 0x3E/* > */:
    case 0x3F/* ? */:
    case 0x40/* @ */:
    case 0x5B/* [ */:
    case 0x5C/* \ */:
    case 0x5D/* ] */:
    case 0x5E/* ^ */:
    case 0x5F/* _ */:
    case 0x60/* ` */:
    case 0x7B/* { */:
    case 0x7C/* | */:
    case 0x7D/* } */:
    case 0x7E/* ~ */:
        return true;
    default:
        return false;
    }
}

void scanDelims(const QString &text, const int start, const bool canSplitWord,
                 bool &canOpen, bool &canClose, int &length ) {
    int pos = start;
    const int textLen = text.length();
    const QChar marker = text.at(start);
    bool leftFlanking = true;
    bool rightFlanking = true;

    const QChar lastChar = start > 0 ? text[start - 1] : QChar('\0');

    while (pos < textLen && text[pos] == marker) {
            ++pos;
    }
    length = pos - start;


    const QChar nextChar = pos + 1 < textLen ? text.at(pos) : QChar('\0');

    const bool isLastPunct =  isMDAsciiPunct(lastChar) || lastChar.isPunct();
    const bool isNextPunct = isMDAsciiPunct(nextChar) || nextChar.isPunct();

    //treat line end and start as whitespace
    const bool isLastWhiteSpace = lastChar.isNull() ? true : lastChar.isSpace();
    const bool isNextWhiteSpace = nextChar.isNull() ? true : nextChar.isSpace();

    if (isNextWhiteSpace) {
        leftFlanking = false;
    } else if (isNextPunct) {
        if (!(isLastWhiteSpace || isLastPunct))
            leftFlanking = false;
    }

    if (isLastWhiteSpace) {
        rightFlanking = false;
    } else if (isLastPunct) {
        if (!(isNextWhiteSpace || isNextPunct))
            rightFlanking = false;
    }

//    qDebug () << isNextWhiteSpace << marker;
//    qDebug () << text << leftFlanking << rightFlanking << lastChar << nextChar;

    if (!canSplitWord) {
        canOpen = leftFlanking && (!rightFlanking || isLastPunct);
        canClose = rightFlanking && (!leftFlanking || isNextPunct);
    } else {
        canOpen = leftFlanking;
        canClose = rightFlanking;
    }
}

/* Forward declarations */
int collectEmDelims(const QString &text, int curPos, QList<Delimiter> &delims);
void balancePairs(QList<Delimiter> &delims);

/**
 * @brief highlights Em/Strong in text editor
 */
void MarkdownHighlighter::highlightEmAndStrong(const QString &text, const int pos){
    //1. collect all em/strong delimiters
    QList<Delimiter> delims;
    for (int i = pos; i < text.length(); ++i) {
        if (text.at(i) != QLatin1Char('_') && text.at(i) != QLatin1Char('*'))
            continue;
        i = collectEmDelims(text, i, delims);
        --i;
    }

    //2. Balance pairs
    balancePairs(delims);

    //start,length -> helper for applying masking later
    QVector<QPair<int, int>> masked;
    masked.reserve(delims.size());

    //3. final processing & highlighting
    for (int i = delims.length() - 1; i >= 0; --i) {
        const auto &startDelim = delims.at(i);
        if (startDelim.marker != QLatin1Char('_') && startDelim.marker != QLatin1Char('*'))
            continue;
        if (startDelim.end == -1)
            continue;

        const auto &endDelim = delims.at(startDelim.end);

        const bool isStrong = i > 0 &&
                delims.at(i - 1).end == startDelim.end + 1 &&
                delims.at(i - 1).pos == startDelim.pos - 1 &&
                delims.at(startDelim.end + 1).pos == endDelim.pos + 1 &&
                delims.at(i - 1).marker == startDelim.marker;
        if (isStrong) {
//            qDebug () << "St: " << startDelim.pos << endDelim.pos;
//            qDebug () << "St Txt: "<< text.mid(startDelim.pos, endDelim.pos - startDelim.pos);
            int k = startDelim.pos;
            while(text.at(k) == startDelim.marker)
                ++k; //look for first letter after the delim chain
            //per character highlighting
            int boldLen = endDelim.pos - startDelim.pos;
            while (k != (startDelim.pos + boldLen)) {
                QTextCharFormat fmt = QSyntaxHighlighter::format(k);
                fmt.setFontWeight(QFont::Bold);
                setFormat(k, 1, fmt);
                k++;
            }
            masked.append({startDelim.pos - 1, 2});
            masked.append({endDelim.pos, 2});
            i--;
        } else {
//            qDebug () << "Em: " << startDelim.pos << endDelim.pos;
//            qDebug () << "Em Txt: " << text.mid(startDelim.pos, endDelim.pos - startDelim.pos);
            int k = startDelim.pos;
            while(text.at(k) == startDelim.marker)
                ++k;
            int itLen = endDelim.pos - startDelim.pos;
            while (k != (startDelim.pos + itLen)) {
                QTextCharFormat fmt = QSyntaxHighlighter::format(k);
                fmt.setFontItalic(true);
                setFormat(k, 1, fmt);
                k++;
            }
            masked.append({startDelim.pos, 1});
            masked.append({endDelim.pos, 1});
        }
    }

    //4. Apply masked syntax
    for (int i = 0; i < masked.length(); ++i) {
        QTextCharFormat maskedFmt = _formats[MaskedSyntax];
        MarkdownHighlighter::HighlighterState state = static_cast<HighlighterState>(currentBlockState());
        if (_formats[state].fontPointSize() > 0)
            maskedFmt.setFontPointSize(_formats[state].fontPointSize());
        setFormat(masked.at(i).first, masked.at(i).second, maskedFmt);
    }
    masked.squeeze();
}

int MarkdownHighlighter::highlightInlineComment(const QString &text, int pos)
{
    int start = pos;
    pos += 4;

    if (pos >= text.length())
        return pos;

    int commentEnd = text.indexOf(QLatin1String("-->"), pos);
    if (commentEnd == -1)
        return pos;

    commentEnd += 3;
    setFormat(start, commentEnd - start, _formats[Comment]);
    return commentEnd;
}

int collectEmDelims(const QString &text, int curPos, QList<Delimiter> &delims) {
    const QChar marker = text.at(curPos);

    if (marker != QLatin1Char('_') && marker != QLatin1Char('*'))
        return curPos;

    bool canOpen = false;
    bool canClose = false;
    int length = 0;
    scanDelims(text, curPos, marker == QLatin1Char('*'), canOpen, canClose, length);
    for (int i = 0; i < length; ++i) {
        Delimiter d = {
            .pos = curPos + i,
            .len = length,
            .jump = i,
            .end = -1,
            .open = canOpen,
            .close = canClose,
            .marker = marker
        };
        delims.append(d);
    }
    return curPos + length;
}

void balancePairs(QList<Delimiter> &delims) {
    for (int i = 0; i < delims.length(); ++i) {
        const auto &lastDelim = delims.at(i);

        if (!lastDelim.close)
            continue;

        int j = i - lastDelim.jump - 1;

        while (j >= 0) {
            const auto &curDelim = delims.at(j);
            if (curDelim.open && curDelim.marker == lastDelim.marker &&
                curDelim.end < 0) {
                bool oddMatch = (curDelim.close || lastDelim.open) &&
                        curDelim.len != -1 &&
                        lastDelim.len != -1 &&
                        (curDelim.len + lastDelim.len) % 3 == 0;
                if (!oddMatch) {
                    delims[i].jump = i - j;
                    delims[i].open = false;
                    delims[j].end = i;
                    delims[j].jump = 0;
                    break;
                }
            }
            j -= curDelim.jump + 1;
        }
    }
}

void MarkdownHighlighter::setHighlightingOptions(const HighlightingOptions options) {
    _highlightingOptions = options;
}
