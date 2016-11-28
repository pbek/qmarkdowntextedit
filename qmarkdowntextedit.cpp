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


#include "qmarkdowntextedit.h"
#include <QKeyEvent>
#include <QGuiApplication>
#include <QDebug>
#include <QRegularExpression>
#include <QDir>
#include <QDesktopServices>
#include <QLayout>
#include <QTimer>
#include <QSettings>
#include <QTextBlock>


QMarkdownTextEdit::QMarkdownTextEdit(QWidget *parent)
        : QTextEdit(parent) {
    installEventFilter(this);
    viewport()->installEventFilter(this);
    _autoTextOptions = AutoTextOption::None;
    _openingCharacters =
            QStringList() << "(" << "[" << "{" << "<" << "*" << "\"";
    _closingCharacters =
            QStringList() << ")" << "]" << "}" << ">" << "*" << "\"";

    // markdown highlighting es enabled by default
    _highlightingEnabled = true;
    _highlighter = new MarkdownHighlighter(document());
//    setHighlightingEnabled(true);

    QFont font = this->font();

    // set the tab stop to the width of 4 spaces in the editor
    const int tabStop = 4;
    QFontMetrics metrics(font);
    setTabStopWidth(tabStop * metrics.width(' '));

    // add shortcuts for duplicating text
//    new QShortcut( QKeySequence( "Ctrl+D" ), this, SLOT( duplicateText() ) );
//    new QShortcut( QKeySequence( "Ctrl+Alt+Down" ), this, SLOT( duplicateText() ) );

    // add a layout to the widget
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setMargin(0);
    layout->addStretch();
    this->setLayout(layout);

    // add the hidden search widget
    _searchWidget = new QTextEditSearchWidget(this);
    this->layout()->addWidget(_searchWidget);

    QObject::connect(this, SIGNAL(textChanged()),
                     this, SLOT(adjustRightMargin()));

    // workaround for disabled signals up initialization
    QTimer::singleShot(300, this, SLOT(adjustRightMargin()));
}

/**
 * Enables or disables the markdown highlighting
 *
 * @param enabled
 */
void QMarkdownTextEdit::setHighlightingEnabled(bool enabled) {
    if (_highlightingEnabled == enabled) {
        return;
    }

    _highlighter->setDocument(enabled ? document() : Q_NULLPTR);
    _highlightingEnabled = enabled;

    if (enabled) {
        _highlighter->rehighlight();
    }
}

/**
 * Leave a little space on the right side if the document is too long, so
 * that the search buttons don't get visually blocked by the scroll bar
 */
void QMarkdownTextEdit::adjustRightMargin() {
    QMargins margins = layout()->contentsMargins();
    int rightMargin = document()->size().height() >
                      viewport()->size().height() ? 24 : 0;
    margins.setRight(rightMargin);
    layout()->setContentsMargins(margins);
}

bool QMarkdownTextEdit::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::HoverMove) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        QWidget *viewPort = this->viewport();
        // toggle cursor when control key has been pressed or released
        viewPort->setCursor(mouseEvent->modifiers().testFlag(
                Qt::ControlModifier) ?
                            Qt::PointingHandCursor :
                            Qt::IBeamCursor);
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        // set cursor to pointing hand if control key was pressed
        if (keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            QWidget *viewPort = this->viewport();
            viewPort->setCursor(Qt::PointingHandCursor);
        }

        // disallow keys if text edit hasn't focus
        if (!this->hasFocus()) {
            return true;
        }

        // reset cursor if control key was released
        if (!keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            QWidget *viewPort = this->viewport();
            viewPort->setCursor(Qt::IBeamCursor);
        }

        if ((keyEvent->key() == Qt::Key_Escape) && _searchWidget->isVisible()) {
            _searchWidget->deactivate();
            return true;
        } else if ((keyEvent->key() == Qt::Key_Tab) ||
                 (keyEvent->key() == Qt::Key_Backtab)) {
            // handle entered tab and reverse tab keys
            return handleTabEntered(
                    keyEvent->key() == Qt::Key_Backtab);
        } else if ((keyEvent->key() == Qt::Key_F) &&
                 keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            _searchWidget->activate();
            return true;
        } else if ((keyEvent->key() == Qt::Key_R) &&
                 keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            _searchWidget->activateReplace();
            return true;
//        } else if (keyEvent->key() == Qt::Key_Delete) {
        } else if (keyEvent->key() == Qt::Key_Backspace) {
            return handleBracketRemoval();
        } else if (keyEvent->key() == Qt::Key_Asterisk) {
            return handleBracketClosing("*");
        } else if (keyEvent->key() == Qt::Key_QuoteDbl) {
            return handleBracketClosing("\"");
        } else if (keyEvent->key() == Qt::Key_ParenLeft) {
            return handleBracketClosing("(", ")");
        } else if (keyEvent->key() == Qt::Key_BraceLeft) {
            return handleBracketClosing("{", "}");
        } else if (keyEvent->key() == Qt::Key_BracketLeft) {
            return handleBracketClosing("[", "]");
        } else if (keyEvent->key() == Qt::Key_Less) {
            return handleBracketClosing("<", ">");
        } else if ((keyEvent->key() == Qt::Key_Down) &&
                 keyEvent->modifiers().testFlag(Qt::ControlModifier) &&
                 keyEvent->modifiers().testFlag(Qt::AltModifier)) {
            // duplicate text with `Ctrl + Alt + Down`
            duplicateText();
            return true;
        } else if ((keyEvent->key() == Qt::Key_Down) &&
                keyEvent->modifiers().testFlag(Qt::NoModifier)) {
            // if you are in the last line and press cursor down the cursor will
            // jump to the end of the line
            QTextCursor c = textCursor();
            if (c.position() >= document()->lastBlock().position()) {
                c.movePosition(QTextCursor::EndOfLine);
                setTextCursor(c);
            }
            return false;
        } else if ((keyEvent->key() == Qt::Key_Up) &&
                   keyEvent->modifiers().testFlag(Qt::NoModifier)) {
            // if you are in the first line and press cursor up the cursor will
            // jump to the start of the line
            QTextCursor c = textCursor();
            QTextBlock block = document()->firstBlock();
            int endOfFirstLinePos = block.position() + block.length();

            if (c.position() <= endOfFirstLinePos) {
                c.movePosition(QTextCursor::StartOfLine);
                setTextCursor(c);
            }
            return false;
        } else if (keyEvent->key() == Qt::Key_Return) {
            return handleReturnEntered();
        } else if ((keyEvent->key() == Qt::Key_F3)) {
            _searchWidget->doSearch(
                    !keyEvent->modifiers().testFlag(Qt::ShiftModifier));
            return true;
        }

        return false;
    } else if (event->type() == QEvent::KeyRelease) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        // reset cursor if control key was released
        if (keyEvent->key() == Qt::Key_Control) {
            QWidget *viewPort = this->viewport();
            viewPort->setCursor(Qt::IBeamCursor);
        }

        return false;
    } else if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        // track `Ctrl + Click` in the text edit
        if ((obj == this->viewport()) &&
            (mouseEvent->button() == Qt::LeftButton) &&
            (QGuiApplication::keyboardModifiers() == Qt::ExtraButton24)) {
            // open the link (if any) at the current position
            // in the noteTextEdit
            openLinkAtCursorPosition();
            return true;
        }
    }

    return QTextEdit::eventFilter(obj, event);
}

/**
 * Enters a closing character after an opening character if needed
 *
 * @param openingCharacter
 * @param closingCharacter
 * @return
 */
bool QMarkdownTextEdit::handleBracketClosing(QString openingCharacter,
                                             QString closingCharacter) {
    // check if bracket closing is enabled
    if (!(_autoTextOptions & AutoTextOption::BracketClosing)) {
        return false;
    }

    QTextCursor c = textCursor();
    int positionInBlock = c.position() - c.block().position();

    // only allow the closing if the cursor was at the end of a block
    if (positionInBlock != c.block().text().count()) {
        return false;
    }

    if (closingCharacter.isEmpty()) {
        closingCharacter = openingCharacter;
    }

    // we don't want to close "*" when used in a list
    if (openingCharacter == "*") {
        // return if the cursor is at the beginning of the block
        if (positionInBlock == 0) {
            return false;
        }

        // get the current text from the block
        QString text = c.block().text();
        // remove everything after the cursor
        text.remove(positionInBlock, text.count());
        // remove whitespaces
        text.remove(QRegularExpression("\\s"));

        // return if there were just whitespaces in front of the cursor
        if (text.isEmpty()) {
            return false;
        }
    }

    // check if there already was entered an opening character before when
    // opening and closing characters are the same
    if ((openingCharacter == closingCharacter) &&
        (c.block().text().count(openingCharacter) % 2 == 1)) {
        return false;
    }

    c.insertText(openingCharacter);
    c.insertText(closingCharacter);
    c.setPosition(c.position() - 1);
    setTextCursor(c);
    return true;
}

/**
 * Handles removing of matching brackets and other markdown characters
 * Only works with backspace to remove text
 *
 * @return
 */
bool QMarkdownTextEdit::handleBracketRemoval() {
    // check if bracket removal is enabled
    if (!(_autoTextOptions & AutoTextOption::BracketRemoval)) {
        return false;
    }

    QTextCursor c = textCursor();

    // return if some text was selected
    if (!c.selectedText().isEmpty()) {
        return false;
    }

    int position = c.position();
    int positionInBlock = position - c.block().position();

    // return if backspace was pressed at the beginning of a block
    if (positionInBlock == 0) {
        return false;
    }

    // get the current text from the block
    QString text = c.block().text();
    QString charInFront = text.at(positionInBlock - 1);
    int openingCharacterIndex = _openingCharacters.indexOf(charInFront);

    // return if the character in front of the cursor is no opening character
    if (openingCharacterIndex == -1) {
        return false;
    }

    QString closingCharacter = _closingCharacters.at(openingCharacterIndex);

    // remove everything in front of the cursor
    text.remove(0, positionInBlock);
    int closingCharacterIndex = text.indexOf(closingCharacter);

    // return if no closing character was found in the text after the cursor
    if (closingCharacterIndex == -1) {
        return false;
    }

    // removing the closing character
    c.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor,
                   closingCharacterIndex);
    c.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    c.removeSelectedText();

    // moving the cursor back to the old position so the previous character
    // can be removed
    c.setPosition(position);
    setTextCursor(c);
    return false;
}

/**
 * Increases (or decreases) the indention of the selected text
 * (if there is a text selected) in the noteTextEdit
 * @return
 */
bool QMarkdownTextEdit::increaseSelectedTextIndention(bool reverse) {
    QTextCursor c = this->textCursor();
    QString selectedText = c.selectedText();

    if (selectedText != "") {
        // we need this strange newline character we are getting in the
        // selected text for newlines
        QString newLine = QString::fromUtf8(QByteArray::fromHex("e280a9"));
        QString newText;

        if (reverse) {
            // un-indent text

            // remove strange newline characters
            newText = selectedText.replace(
                    QRegularExpression(newLine + "[\\t ]"), "\n");

            // remove leading \t or space
            newText.remove(QRegularExpression("^[\\t ]"));
        } else {
            // indent text
            newText = selectedText.replace(newLine, "\n\t").prepend("\t");

            // remove trailing \t
            newText.replace(QRegularExpression("\\t$"), "");
        }

        // insert the new text
        c.insertText(newText);

        // update the selection to the new text
        c.setPosition(c.position() - newText.size(), QTextCursor::KeepAnchor);
        this->setTextCursor(c);

        return true;
    } else if (reverse) {
        // if nothing was selected but we want to reverse the indention check
        // if there is a \t in front or after the cursor and remove it if so
        int pos = c.position();
        // get character in front of cursor
        c.setPosition(pos - 1, QTextCursor::KeepAnchor);

        // check for \t or space in front of cursor
        QRegularExpression re("[\\t ]");
        QRegularExpressionMatch match = re.match(c.selectedText());

        if (!match.hasMatch()) {
            // (select to) check for \t or space after the cursor
            c.setPosition(pos);
            c.setPosition(pos + 1, QTextCursor::KeepAnchor);
        }

        match = re.match(c.selectedText());

        if (match.hasMatch()) {
            c.removeSelectedText();
        }

        return true;
    }

    return false;
}


/**
 * @brief Opens the link (if any) at the current cursor position
 */
bool QMarkdownTextEdit::openLinkAtCursorPosition() {
    QTextCursor c = this->textCursor();
    int clickedPosition = c.position();

    // select the text in the clicked block and find out on
    // which position we clicked
    c.movePosition(QTextCursor::StartOfBlock);
    int positionFromStart = clickedPosition - c.position();
    c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

    QString selectedText = c.selectedText();

    // find out which url in the selected text was clicked
    QString urlString = getMarkdownUrlAtPosition(selectedText,
                                                 positionFromStart);
    QUrl url = QUrl(urlString);
    bool isRelativeFileUrl = urlString.startsWith("file://..");

    if (url.isValid() || isRelativeFileUrl) {
        qDebug() << __func__ << " - 'emit urlClicked( urlString )': "
            << urlString;

        emit urlClicked(urlString);

        // ignore some schemata
        if (!(_ignoredClickUrlSchemata.contains(url.scheme()) ||
                isRelativeFileUrl)) {
            // open the url
            openUrl(urlString);
        }

        return true;
    }

    return false;
}

/**
 * Handles clicked urls
 *
 * examples:
 * - <http://www.qownnotes.org> opens the webpage
 * - <file:///path/to/my/file/QOwnNotes.pdf> opens the file
 *   "/path/to/my/file/QOwnNotes.pdf" if the operating system supports that
 *  handler
 */
void QMarkdownTextEdit::openUrl(QString urlString) {
    qDebug() << "QMarkdownTextEdit " << __func__ << " - 'urlString': "
        << urlString;

    QDesktopServices::openUrl(QUrl(urlString));
}

/**
 * @brief Returns the highlighter instance
 * @return
 */
MarkdownHighlighter *QMarkdownTextEdit::highlighter() {
    return _highlighter;
}

/**
 * @brief Returns the searchWidget instance
 * @return
 */
QTextEditSearchWidget *QMarkdownTextEdit::searchWidget() {
    return _searchWidget;
}

/**
 * @brief Sets url schemata that will be ignored when clicked on
 * @param urlSchemes
 */
void QMarkdownTextEdit::setIgnoredClickUrlSchemata(
        QStringList ignoredUrlSchemata) {
    _ignoredClickUrlSchemata = ignoredUrlSchemata;
}

/**
 * @brief Returns a map of parsed markdown urls with their link texts as key
 *
 * @param text
 * @return parsed urls
 */
QMap<QString, QString> QMarkdownTextEdit::parseMarkdownUrlsFromText(
        QString text) {
    QMap<QString, QString> urlMap;

    // match urls like this: [this url](http://mylink)
    QRegularExpression re("(\\[.*?\\]\\((.+?://.+?)\\))");
    QRegularExpressionMatchIterator i = re.globalMatch(text);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString linkText = match.captured(1);
        QString url = match.captured(2);
        urlMap[linkText] = url;
    }

    // match urls like this: <http://mylink>
    re = QRegularExpression("(<(.+?://.+?)>)");
    i = re.globalMatch(text);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString linkText = match.captured(1);
        QString url = match.captured(2);
        urlMap[linkText] = url;
    }

    // match urls like this: http://mylink
    re = QRegularExpression("(\\b\\w+?:\\/\\/[^\\s\\)>]+)");
    i = re.globalMatch(text);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString linkText = match.captured(1);
        QString url = match.captured(1);
        urlMap[linkText] = url;
    }

    return urlMap;
}

/**
 * @brief Returns the markdown url at position
 * @param text
 * @param position
 * @return url string
 */
QString QMarkdownTextEdit::getMarkdownUrlAtPosition(
        QString text, int position) {
    QString url;

    // get a map of parsed markdown urls with their link texts as key
    QMap<QString, QString> urlMap = parseMarkdownUrlsFromText(text);

    QMapIterator<QString, QString> i(urlMap);
    while (i.hasNext()) {
        i.next();
        QString linkText = i.key();
        QString urlString = i.value();

        int foundPositionStart = text.indexOf(linkText);

        if (foundPositionStart >= 0) {
            // calculate end position of found linkText
            int foundPositionEnd = foundPositionStart + linkText.size();

            // check if position is in found string range
            if ((position >= foundPositionStart) &&
                (position <= foundPositionEnd)) {
                url = urlString;
            }
        }
    }

    return url;
}

/**
 * @brief Duplicates the text in the text edit
 */
void QMarkdownTextEdit::duplicateText() {
    QTextCursor c = this->textCursor();
    QString selectedText = c.selectedText();

    // duplicate line if no text was selected
    if (selectedText == "") {
        int position = c.position();

        // select the whole line
        c.movePosition(QTextCursor::StartOfLine);
        c.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);

        int positionDiff = c.position() - position;
        selectedText = "\n" + c.selectedText();

        // insert text with new line at end of the selected line
        c.setPosition(c.selectionEnd());
        c.insertText(selectedText);

        // set the position to same position it was in the duplicated line
        c.setPosition(c.position() - positionDiff);
    } else {
        // duplicate selected text
        c.setPosition(c.selectionEnd());
        int selectionStart = c.position();

        // insert selected text
        c.insertText(selectedText);
        int selectionEnd = c.position();

        // select the inserted text
        c.setPosition(selectionStart);
        c.setPosition(selectionEnd, QTextCursor::KeepAnchor);
    }

    this->setTextCursor(c);
}

void QMarkdownTextEdit::setText(const QString & text) {
    QTextEdit::setText(text);
    adjustRightMargin();
}

void QMarkdownTextEdit::setHtml(const QString &text) {
    QTextEdit::setHtml(text);
    adjustRightMargin();
}

void QMarkdownTextEdit::setPlainText(const QString & text) {
    QTextEdit::setPlainText(text);
    adjustRightMargin();
}

/**
 * Uses an other widget as parent for the search widget
 */
void QMarkdownTextEdit::initSearchFrame(QWidget *searchFrame) {
    _searchFrame = searchFrame;

    // remove the search widget from our layout
    layout()->removeWidget(_searchWidget);

    QLayout *layout = _searchFrame->layout();

    // create a grid layout for the frame and add the search widget to it
    if (layout == NULL) {
        layout = new QVBoxLayout();
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);
    }

    layout->addWidget(_searchWidget);
    _searchFrame->setLayout(layout);
}

/**
 * Hides the text edit and the search widget
 */
void QMarkdownTextEdit::hide() {
    _searchWidget->hide();
    QWidget::hide();
}

/**
 * Handles an entered return key
 */
bool QMarkdownTextEdit::handleReturnEntered() {
    QTextCursor c = this->textCursor();
    int position = c.position();

    c.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
    QString currentLineText = c.selectedText();

    // if return is pressed and there is just a list symbol then we want to
    // remove the list symbol
    QRegularExpression re("^\\s*[+\\-\\*]\\s*$");
    QRegularExpressionMatchIterator i = re.globalMatch(currentLineText);
    if (i.hasNext()) {
        c.removeSelectedText();
        return true;
    }

    // if the current line starts with a list character (possibly after
    // whitespaces) add the whitespaces at the next line too
    re = QRegularExpression("^(\\s*)([+\\-\\*])(\\s?)");
    i = re.globalMatch(currentLineText);
    if (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        QString whitespaces = match.captured(1);
        QString listCharacter = match.captured(2);
        QString whitespaceCharacter = match.captured(3);

        c.setPosition(position);
        c.insertText("\n" + whitespaces + listCharacter + whitespaceCharacter);

        // scroll to the cursor if we are at the bottom of the document
        ensureCursorVisible();
        return true;
    }

    return false;
}

/**
 * Handles entered tab or reverse tab keys
 */
bool QMarkdownTextEdit::handleTabEntered(bool reverse) {
    QTextCursor c = this->textCursor();

    // only check for lists if we haven't a text selected
    if (c.selectedText().isEmpty()) {
        c.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        QString currentLineText = c.selectedText();

        // check if we want to indent or un-indent a list
        QRegularExpression re("^(\\s*)([+\\-\\*])(\\s?)$");
        QRegularExpressionMatchIterator i = re.globalMatch(currentLineText);

        if (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            QString whitespaces = match.captured(1);
            QString listCharacter = match.captured(2);
            QString whitespaceCharacter = match.captured(3);

            // add or remove one tabulator key
            if (reverse) {
                whitespaces.chop(1);
            } else {
                whitespaces += "\t";
            }

            c.insertText(whitespaces + listCharacter + whitespaceCharacter);
            return true;
        }
    }

    // check if we want to intent the whole text
    return increaseSelectedTextIndention(reverse);
}

/**
 * Sets the auto text options
 */
void QMarkdownTextEdit::setAutoTextOptions(AutoTextOptions options) {
    _autoTextOptions = options;
}
