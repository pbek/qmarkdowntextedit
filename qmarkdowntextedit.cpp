/*
 * Copyright (c) 2014-2020 Patrizio Bekerle -- <patrizio@bekerle.com>
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

#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QLayout>
#include <QPainter>
#include <QPainterPath>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QRegularExpressionMatchIterator>
#include <QScrollBar>
#include <QSettings>
#include <QTextBlock>
#include <QTimer>
#include <utility>

#include "markdownhighlighter.h"

static const QByteArray _openingCharacters = QByteArrayLiteral("([{<*\"'_~");
static const QByteArray _closingCharacters = QByteArrayLiteral(")]}>*\"'_~");

QMarkdownTextEdit::QMarkdownTextEdit(QWidget *parent, bool initHighlighter)
    : QPlainTextEdit(parent) {
    installEventFilter(this);
    viewport()->installEventFilter(this);
    _autoTextOptions = AutoTextOption::BracketClosing;

    // markdown highlighting is enabled by default
    _highlightingEnabled = true;
    if (initHighlighter) {
        _highlighter = new MarkdownHighlighter(document());
    }
    //    setHighlightingEnabled(true);

    QFont font = this->font();

    // set the tab stop to the width of 4 spaces in the editor
    constexpr int tabStop = 4;
    QFontMetrics metrics(font);

#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
    setTabStopWidth(tabStop * metrics.width(' '));
#else
    setTabStopDistance(tabStop * metrics.horizontalAdvance(QLatin1Char(' ')));
#endif

    // add shortcuts for duplicating text
    //    new QShortcut( QKeySequence( "Ctrl+D" ), this, SLOT( duplicateText() )
    //    ); new QShortcut( QKeySequence( "Ctrl+Alt+Down" ), this, SLOT(
    //    duplicateText() ) );

    // add a layout to the widget
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addStretch();
    this->setLayout(layout);

    // add the hidden search widget
    _searchWidget = new QPlainTextEditSearchWidget(this);
    this->layout()->addWidget(_searchWidget);

    connect(this, &QPlainTextEdit::textChanged, this,
            &QMarkdownTextEdit::adjustRightMargin);
    connect(this, &QPlainTextEdit::cursorPositionChanged, this,
            &QMarkdownTextEdit::centerTheCursor);

    updateSettings();

    // workaround for disabled signals up initialization
    QTimer::singleShot(300, this, SLOT(adjustRightMargin()));
}

/**
 * Enables or disables the markdown highlighting
 *
 * @param enabled
 */
void QMarkdownTextEdit::setHighlightingEnabled(bool enabled) {
    if (_highlightingEnabled == enabled || _highlighter == nullptr) {
        return;
    }

    _highlightingEnabled = enabled;
    _highlighter->setDocument(enabled ? document() : Q_NULLPTR);

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
    const int rightMargin =
        document()->size().height() > viewport()->size().height() ? 24 : 0;
    margins.setRight(rightMargin);
    layout()->setContentsMargins(margins);
}

bool QMarkdownTextEdit::eventFilter(QObject *obj, QEvent *event) {
    // qDebug() << event->type();
    if (event->type() == QEvent::HoverMove) {
        auto *mouseEvent = static_cast<QMouseEvent *>(event);

        QWidget *viewPort = this->viewport();
        // toggle cursor when control key has been pressed or released
        viewPort->setCursor(
            mouseEvent->modifiers().testFlag(Qt::ControlModifier)
                ? Qt::PointingHandCursor
                : Qt::IBeamCursor);
    } else if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);

        // set cursor to pointing hand if control key was pressed
        if (keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            QWidget *viewPort = this->viewport();
            viewPort->setCursor(Qt::PointingHandCursor);
        }

        // disallow keys if text edit hasn't focus
        if (!this->hasFocus()) {
            return true;
        }

        if ((keyEvent->key() == Qt::Key_Escape) && _searchWidget->isVisible()) {
            _searchWidget->deactivate();
            return true;
        } else if ((keyEvent->key() == Qt::Key_Tab) ||
                   (keyEvent->key() == Qt::Key_Backtab)) {
            // handle entered tab and reverse tab keys
            return handleTabEntered(keyEvent->key() == Qt::Key_Backtab);
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
            return handleBracketClosing(QLatin1Char('*'));
        } else if (keyEvent->key() == Qt::Key_QuoteDbl) {
            return quotationMarkCheck(QLatin1Char('"'));
            // apostrophe bracket closing is temporary disabled because
            // apostrophes are used in different contexts
            //        } else if (keyEvent->key() == Qt::Key_Apostrophe) {
            //            return handleBracketClosing("'");
            // underline bracket closing is temporary disabled because
            // underlines are used in different contexts
            //        } else if (keyEvent->key() == Qt::Key_Underscore) {
            //            return handleBracketClosing("_");
        } else if (keyEvent->key() == Qt::Key_QuoteLeft) {
            return quotationMarkCheck(QLatin1Char('`'));
        } else if (keyEvent->key() == Qt::Key_AsciiTilde) {
            return handleBracketClosing(QLatin1Char('~'));
#ifdef Q_OS_MAC
        } else if (keyEvent->modifiers().testFlag(Qt::AltModifier) &&
                   keyEvent->key() == Qt::Key_ParenLeft) {
            // bracket closing for US keyboard on macOS
            return handleBracketClosing(QLatin1Char('{'), QLatin1Char('}'));
#endif
        } else if (keyEvent->key() == Qt::Key_ParenLeft) {
            return handleBracketClosing(QLatin1Char('('), QLatin1Char(')'));
        } else if (keyEvent->key() == Qt::Key_BraceLeft) {
            return handleBracketClosing(QLatin1Char('{'), QLatin1Char('}'));
        } else if (keyEvent->key() == Qt::Key_BracketLeft) {
            return handleBracketClosing(QLatin1Char('['), QLatin1Char(']'));
        } else if (keyEvent->key() == Qt::Key_Less) {
            return handleBracketClosing(QLatin1Char('<'), QLatin1Char('>'));
#ifdef Q_OS_MAC
        } else if (keyEvent->modifiers().testFlag(Qt::AltModifier) &&
                   keyEvent->key() == Qt::Key_ParenRight) {
            // bracket closing for US keyboard on macOS
            return bracketClosingCheck(QLatin1Char('{'), QLatin1Char('}'));
#endif
        } else if (keyEvent->key() == Qt::Key_ParenRight) {
            return bracketClosingCheck(QLatin1Char('('), QLatin1Char(')'));
        } else if (keyEvent->key() == Qt::Key_BraceRight) {
            return bracketClosingCheck(QLatin1Char('{'), QLatin1Char('}'));
        } else if (keyEvent->key() == Qt::Key_BracketRight) {
            return bracketClosingCheck(QLatin1Char('['), QLatin1Char(']'));
        } else if (keyEvent->key() == Qt::Key_Return &&
                   keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {
            QTextCursor cursor = this->textCursor();
            cursor.insertText("  \n");
            return true;
        } else if (keyEvent->key() == Qt::Key_Return &&
                   keyEvent->modifiers().testFlag(Qt::ControlModifier)) {
            QTextCursor cursor = this->textCursor();
            cursor.movePosition(QTextCursor::EndOfBlock);
            cursor.insertText(QStringLiteral("\n"));
            setTextCursor(cursor);
            return true;
        } else if (keyEvent == QKeySequence::Copy ||
                   keyEvent == QKeySequence::Cut) {
            QTextCursor cursor = this->textCursor();
            if (!cursor.hasSelection()) {
                QString text;
                if (cursor.block().length() <= 1)    // no content
                    text = "\n";
                else {
                    // cursor.select(QTextCursor::BlockUnderCursor); //
                    // negative, it will include the previous paragraph
                    // separator
                    cursor.movePosition(QTextCursor::StartOfBlock);
                    cursor.movePosition(QTextCursor::EndOfBlock,
                                        QTextCursor::KeepAnchor);
                    text = cursor.selectedText();
                    if (!cursor.atEnd()) {
                        text += "\n";
                        // this is the paragraph separator
                        cursor.movePosition(QTextCursor::NextCharacter,
                                            QTextCursor::KeepAnchor, 1);
                    }
                }
                if (keyEvent == QKeySequence::Cut) {
                    if (!cursor.atEnd() && text == "\n")
                        cursor.deletePreviousChar();
                    else
                        cursor.removeSelectedText();
                    cursor.movePosition(QTextCursor::StartOfBlock);
                    setTextCursor(cursor);
                }
                qApp->clipboard()->setText(text);
                return true;
            }
        } else if (keyEvent == QKeySequence::Paste) {
            if (qApp->clipboard()->ownsClipboard() &&
                QRegExp(QStringLiteral("[^\n]*\n$"))
                    .exactMatch(qApp->clipboard()->text())) {
                QTextCursor cursor = this->textCursor();
                if (!cursor.hasSelection()) {
                    cursor.movePosition(QTextCursor::StartOfBlock);
                    setTextCursor(cursor);
                }
            }
        } else if ((keyEvent->key() == Qt::Key_Down) &&
                   keyEvent->modifiers().testFlag(Qt::ControlModifier) &&
                   keyEvent->modifiers().testFlag(Qt::AltModifier)) {
            // duplicate text with `Ctrl + Alt + Down`
            duplicateText();
            return true;
#ifndef Q_OS_MAC
        } else if ((keyEvent->key() == Qt::Key_Down) &&
                   keyEvent->modifiers().testFlag(Qt::ControlModifier) &&
                   !keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {
            // scroll the page down
            auto *scrollBar = verticalScrollBar();
            scrollBar->setSliderPosition(scrollBar->sliderPosition() + 1);
            return true;
        } else if ((keyEvent->key() == Qt::Key_Up) &&
                   keyEvent->modifiers().testFlag(Qt::ControlModifier) &&
                   !keyEvent->modifiers().testFlag(Qt::ShiftModifier)) {
            // scroll the page up
            auto *scrollBar = verticalScrollBar();
            scrollBar->setSliderPosition(scrollBar->sliderPosition() - 1);
            return true;
#endif
        } else if ((keyEvent->key() == Qt::Key_Down) &&
                   keyEvent->modifiers().testFlag(Qt::NoModifier)) {
            // if you are in the last line and press cursor down the cursor will
            // jump to the end of the line
            QTextCursor cursor = textCursor();
            if (cursor.position() >= document()->lastBlock().position()) {
                cursor.movePosition(QTextCursor::EndOfBlock);
                setTextCursor(cursor);
            }
            return false;
        } else if ((keyEvent->key() == Qt::Key_Up) &&
                   keyEvent->modifiers().testFlag(Qt::NoModifier)) {
            // if you are in the first line and press cursor up the cursor will
            // jump to the start of the line
            QTextCursor cursor = textCursor();
            QTextBlock block = document()->firstBlock();
            int endOfFirstLinePos = block.position() + block.length();

            if (cursor.position() <= endOfFirstLinePos) {
                cursor.movePosition(QTextCursor::StartOfBlock);
                setTextCursor(cursor);
            }
            return false;
        } else if (keyEvent->key() == Qt::Key_Return) {
            return handleReturnEntered();
        } else if ((keyEvent->key() == Qt::Key_F3)) {
            _searchWidget->doSearch(
                !keyEvent->modifiers().testFlag(Qt::ShiftModifier));
            return true;
        } else if ((keyEvent->key() == Qt::Key_Z) &&
                   (keyEvent->modifiers().testFlag(Qt::ControlModifier)) &&
                   !(keyEvent->modifiers().testFlag(Qt::ShiftModifier))) {
            undo();
            return true;
        } else if ((keyEvent->key() == Qt::Key_Down) &&
                   (keyEvent->modifiers().testFlag(Qt::ControlModifier)) &&
                   (keyEvent->modifiers().testFlag(Qt::ShiftModifier))) {
            moveTextUpDown(false);
            return true;
        } else if ((keyEvent->key() == Qt::Key_Up) &&
                   (keyEvent->modifiers().testFlag(Qt::ControlModifier)) &&
                   (keyEvent->modifiers().testFlag(Qt::ShiftModifier))) {
            moveTextUpDown(true);
            return true;
        }

        return false;
    } else if (event->type() == QEvent::KeyRelease) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);

        // reset cursor if control key was released
        if (keyEvent->key() == Qt::Key_Control) {
            resetMouseCursor();
        }

        return false;
    } else if (event->type() == QEvent::MouseButtonRelease) {
        _mouseButtonDown = false;
        auto *mouseEvent = static_cast<QMouseEvent *>(event);

        // track `Ctrl + Click` in the text edit
        if ((obj == this->viewport()) &&
            (mouseEvent->button() == Qt::LeftButton) &&
            (QGuiApplication::keyboardModifiers() == Qt::ExtraButton24)) {
            // open the link (if any) at the current position
            // in the noteTextEdit
            openLinkAtCursorPosition();
            return true;
        }
    } else if (event->type() == QEvent::MouseButtonPress) {
        _mouseButtonDown = true;
    } else if (event->type() == QEvent::MouseButtonDblClick) {
        _mouseButtonDown = true;
    } else if (event->type() == QEvent::Wheel) {
        auto *wheel = dynamic_cast<QWheelEvent*>(event);

        // emit zoom signals
        if (wheel->modifiers() == Qt::ControlModifier) {
            if (wheel->angleDelta().y() > 0) {
                emit zoomIn();
            } else {
                emit zoomOut();
            }

            return true;
        }
    }

    return QPlainTextEdit::eventFilter(obj, event);
}

void QMarkdownTextEdit::centerTheCursor() {
    if (_mouseButtonDown || !_centerCursor) {
        return;
    }

    // centers the cursor every time, but not on the top and bottom
    // bottom is done by setCenterOnScroll() in updateSettings()
    centerCursor();

    /*
        QRect cursor = cursorRect();
        QRect vp = viewport()->rect();

        qDebug() << __func__ << " - 'cursor.top': " << cursor.top();
        qDebug() << __func__ << " - 'cursor.bottom': " << cursor.bottom();
        qDebug() << __func__ << " - 'vp': " << vp.bottom();

        int bottom = 0;
        int top = 0;

        qDebug() << __func__ << " - 'viewportMargins().top()': "
                 << viewportMargins().top();

        qDebug() << __func__ << " - 'viewportMargins().bottom()': "
                 << viewportMargins().bottom();

        int vpBottom = viewportMargins().top() + viewportMargins().bottom() +
    vp.bottom(); int vpCenter = vpBottom / 2; int cBottom = cursor.bottom() +
    viewportMargins().top();

        qDebug() << __func__ << " - 'vpBottom': " << vpBottom;
        qDebug() << __func__ << " - 'vpCenter': " << vpCenter;
        qDebug() << __func__ << " - 'cBottom': " << cBottom;


        if (cBottom >= vpCenter) {
            bottom = cBottom + viewportMargins().top() / 2 +
    viewportMargins().bottom() / 2 - (vp.bottom() / 2);
    //        bottom = cBottom - (vp.bottom() / 2);
    //        bottom *= 1.5;
        }

    //    setStyleSheet(QString("QPlainTextEdit {padding-bottom:
    %1px;}").arg(QString::number(bottom)));

    //    if (cursor.top() < (vp.bottom() / 2)) {
    //        top = (vp.bottom() / 2) - cursor.top() + viewportMargins().top() /
    2 + viewportMargins().bottom() / 2;
    ////        top *= -1;
    ////        bottom *= 1.5;
    //    }
        qDebug() << __func__ << " - 'top': " << top;
        qDebug() << __func__ << " - 'bottom': " << bottom;
        setViewportMargins(0,top,0, bottom);


    //    QScrollBar* scrollbar = verticalScrollBar();
    //
    //    qDebug() << __func__ << " - 'scrollbar->value();': " <<
    scrollbar->value();;
    //    qDebug() << __func__ << " - 'scrollbar->maximum();': "
    //             << scrollbar->maximum();;


    //    scrollbar->setValue(scrollbar->value() - offset.y());
    //
    //    setViewportMargins

    //    setViewportMargins(0, 0, 0, bottom);
    */
}

/*
 * Handle the undo event ourselves
 * Retains the selected text as selected after undo if
 * bracket closing was used otherwise performs normal undo
 */
void QMarkdownTextEdit::undo() {
    QTextCursor cursor = textCursor();
    // if no text selected, call undo
    if (!cursor.hasSelection()) {
        QPlainTextEdit::undo();
        return;
    }

    // if text is selected and bracket closing was used
    // we retain our selection
    if (_handleBracketClosingUsed) {
        // get the selection
        int selectionEnd = cursor.selectionEnd();
        int selectionStart = cursor.selectionStart();
        // call undo
        QPlainTextEdit::undo();
        // select again
        cursor.setPosition(selectionStart - 1);
        cursor.setPosition(selectionEnd - 1, QTextCursor::KeepAnchor);
        this->setTextCursor(cursor);
        _handleBracketClosingUsed = false;
    } else {
        // if text was selected but bracket closing wasn't used
        // do normal undo
        QPlainTextEdit::undo();
        return;
    }
}

void QMarkdownTextEdit::moveTextUpDown(bool up) {
    QTextCursor cursor = textCursor();
    QTextCursor move = cursor;

    move.setVisualNavigation(false);

    move.beginEditBlock();    // open an edit block to keep undo operations sane
    bool hasSelection = cursor.hasSelection();

    if (hasSelection) {
        // if there's a selection inside the block, we select the whole block
        move.setPosition(cursor.selectionStart());
        move.movePosition(QTextCursor::StartOfBlock);
        move.setPosition(cursor.selectionEnd(), QTextCursor::KeepAnchor);
        move.movePosition(
            move.atBlockStart() ? QTextCursor::Left : QTextCursor::EndOfBlock,
            QTextCursor::KeepAnchor);
    } else {
        move.movePosition(QTextCursor::StartOfBlock);
        move.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    }

    // get the text of the current block
    QString text = move.selectedText();

    move.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    move.removeSelectedText();

    if (up) {    // up key
        move.movePosition(QTextCursor::PreviousBlock);
        move.insertBlock();
        move.movePosition(QTextCursor::Left);
    } else {    // down key
        move.movePosition(QTextCursor::EndOfBlock);
        if (move.atBlockStart()) {    // empty block
            move.movePosition(QTextCursor::NextBlock);
            move.insertBlock();
            move.movePosition(QTextCursor::Left);
        } else {
            move.insertBlock();
        }
    }

    int start = move.position();
    move.clearSelection();
    move.insertText(text);
    int end = move.position();

    // reselect
    if (hasSelection) {
        move.setPosition(end);
        move.setPosition(start, QTextCursor::KeepAnchor);
    } else {
        move.setPosition(start);
    }

    move.endEditBlock();

    setTextCursor(move);
}

/**
 * Resets the cursor to Qt::IBeamCursor
 */
void QMarkdownTextEdit::resetMouseCursor() const {
    QWidget *viewPort = viewport();
    viewPort->setCursor(Qt::IBeamCursor);
}

/**
 * Resets the cursor to Qt::IBeamCursor if the widget looses the focus
 */
void QMarkdownTextEdit::focusOutEvent(QFocusEvent *event) {
    resetMouseCursor();
    QPlainTextEdit::focusOutEvent(event);
}

/**
 * Enters a closing character after an opening character if needed
 *
 * @param openingCharacter
 * @param closingCharacter
 * @return
 */
bool QMarkdownTextEdit::handleBracketClosing(const QChar openingCharacter,
                                             QChar closingCharacter) {
    // check if bracket closing or read-only are enabled
    if (!(_autoTextOptions & AutoTextOption::BracketClosing) || isReadOnly()) {
        return false;
    }

    QTextCursor cursor = textCursor();

    if (closingCharacter.isNull()) {
        closingCharacter = openingCharacter;
    }

    const QString selectedText = cursor.selectedText();

    // When user currently has text selected, we prepend the openingCharacter
    // and append the closingCharacter. E.g. 'text' -> '(text)'. We keep the
    // current selectedText selected.
    if (!selectedText.isEmpty()) {
        // Insert. The selectedText is overwritten.
        const QString newText =
            openingCharacter + selectedText + closingCharacter;
        cursor.insertText(newText);

        // Re-select the selectedText.
        const int selectionEnd = cursor.position() - 1;
        const int selectionStart = selectionEnd - selectedText.length();

        cursor.setPosition(selectionStart);
        cursor.setPosition(selectionEnd, QTextCursor::KeepAnchor);
        this->setTextCursor(cursor);
        _handleBracketClosingUsed = true;
        return true;
    }

    // get the current text from the block (inserted character not included)
    // Remove whitespace at start of string (e.g. in multilevel-lists).
    const QString text = cursor.block().text().remove(QRegExp("^\\s+"));

    const int pib = cursor.positionInBlock();
    bool isPreviousAsterisk = pib > 0 && text.at(pib - 1) == '*';
    bool isNextAsterisk = pib < text.length() && text.at(pib) == '*';
    bool isMaybeBold = isPreviousAsterisk && isNextAsterisk;
    if (pib < text.length() && !isMaybeBold && !text.at(pib).isSpace()) {
        return false;
    }

    // Default positions to move the cursor back.
    int cursorSubtract = 1;
    // Special handling for `*` opening character, as this could be:
    // - start of a list (or sublist);
    // - start of a bold text;
    if (openingCharacter == QLatin1Char('*')) {
        // don't auto complete in code block
        bool isInCode =
            MarkdownHighlighter::isCodeBlock(cursor.block().userState());
        // we only do auto completion if there is a space before the cursor pos
        bool hasSpaceOrAsteriskBefore = !text.isEmpty() && pib > 0 &&
                                        (text.at(pib - 1).isSpace() ||
                                         text.at(pib - 1) == QLatin1Char('*'));
        // This could be the start of a list, don't autocomplete.
        bool isEmpty = text.isEmpty();

        if (isInCode || !hasSpaceOrAsteriskBefore || isEmpty) {
            return false;
        }

        // bold
        if (isPreviousAsterisk && isNextAsterisk) {
            cursorSubtract = 1;
        }

        // User wants: '**'.
        // Not the start of a list, probably bold text. We autocomplete with
        // extra closingCharacter and cursorSubtract to 'catchup'.
        if (text == QLatin1String("*")) {
            cursor.insertText(QStringLiteral("*"));
            cursorSubtract = 2;
        }
    }

    // Auto completion for ``` pair
    if (openingCharacter == QLatin1Char('`')) {
        if (QRegExp(QStringLiteral("[^`]*``")).exactMatch(text)) {
            cursor.insertText(QStringLiteral("``"));
            cursorSubtract = 3;
        }
    }

    // don't auto complete in code block
    if (openingCharacter == QLatin1Char('<') &&
        MarkdownHighlighter::isCodeBlock(cursor.block().userState())) {
        return false;
    }

    cursor.beginEditBlock();
    cursor.insertText(openingCharacter);
    cursor.insertText(closingCharacter);
    cursor.setPosition(cursor.position() - cursorSubtract);
    cursor.endEditBlock();

    setTextCursor(cursor);
    return true;
}

/**
 * Checks if the closing character should be output or not
 *
 * @param openingCharacter
 * @param closingCharacter
 * @return
 */
bool QMarkdownTextEdit::bracketClosingCheck(const QChar openingCharacter,
                                            QChar closingCharacter) {
    // check if bracket closing or read-only are enabled
    if (!(_autoTextOptions & AutoTextOption::BracketClosing) || isReadOnly()) {
        return false;
    }

    if (closingCharacter.isNull()) {
        closingCharacter = openingCharacter;
    }

    QTextCursor cursor = textCursor();
    const int positionInBlock = cursor.positionInBlock();

    // get the current text from the block
    const QString text = cursor.block().text();
    const int textLength = text.length();

    // if we are at the end of the line we just want to enter the character
    if (positionInBlock >= textLength) {
        return false;
    }

    const QChar currentChar = text.at(positionInBlock);

    //    if (closingCharacter == openingCharacter) {

    //    }

    qDebug() << __func__ << " - 'currentChar': " << currentChar;

    // if the current character is not the closing character we just want to
    // enter the character
    if (currentChar != closingCharacter) {
        return false;
    }

    const QStringRef leftText = text.leftRef(positionInBlock);
    const int openingCharacterCount = leftText.count(openingCharacter);
    const int closingCharacterCount = leftText.count(closingCharacter);

    // if there were enough opening characters just enter the character
    if (openingCharacterCount < (closingCharacterCount + 1)) {
        return false;
    }

    // move the cursor to the right and don't enter the character
    cursor.movePosition(QTextCursor::Right);
    setTextCursor(cursor);
    return true;
}

/**
 * Checks if the closing character should be output or not or if a closing
 * character after an opening character if needed
 *
 * @param quotationCharacter
 * @return
 */
bool QMarkdownTextEdit::quotationMarkCheck(const QChar quotationCharacter) {
    // check if bracket closing or read-only are enabled
    if (!(_autoTextOptions & AutoTextOption::BracketClosing) || isReadOnly()) {
        return false;
    }

    QTextCursor cursor = textCursor();
    const int positionInBlock = cursor.positionInBlock();

    // get the current text from the block
    const QString &text = cursor.block().text();
    const int textLength = text.length();

    // if last char is not space, we are at word end, no autocompletion

    const bool isBacktick = quotationCharacter == '`';
    if (!isBacktick && positionInBlock != 0 &&
        !text.at(positionInBlock - 1).isSpace()) {
        return false;
    }

    // if we are at the end of the line we just want to enter the character
    if (positionInBlock >= textLength) {
        return handleBracketClosing(quotationCharacter);
    }

    const QChar currentChar = text.at(positionInBlock);

    // if the current character is not the quotation character we just want to
    // enter the character
    if (currentChar != quotationCharacter) {
        return handleBracketClosing(quotationCharacter);
    }

    // move the cursor to the right and don't enter the character
    cursor.movePosition(QTextCursor::Right);
    setTextCursor(cursor);
    return true;
}

/***********************************
 * helper methods for char removal
 * Rules for (') and ("):
 * if [sp]" -> opener (sp = space)
 * if "[sp] -> closer
 ***********************************/
bool isQuotOpener(int position, const QString &text) {
    if (position == 0) return true;
    const int prevCharPos = position - 1;
    return text.at(prevCharPos).isSpace();
}
bool isQuotCloser(int position, const QString &text) {
    const int nextCharPos = position + 1;
    if (nextCharPos >= text.length()) return true;
    return text.at(nextCharPos).isSpace();
}

/**
 * Handles removing of matching brackets and other markdown characters
 * Only works with backspace to remove text
 *
 * @return
 */
bool QMarkdownTextEdit::handleBracketRemoval() {
    if (!(_autoTextOptions & AutoTextOption::BracketRemoval) || isReadOnly()) {
        return false;
    }

    QTextCursor cursor = textCursor();

    // return if some text was selected
    if (!cursor.selectedText().isEmpty()) {
        return false;
    }

    int position = cursor.position();
    const int positionInBlock = cursor.positionInBlock();

    // return if backspace was pressed at the beginning of a block
    if (positionInBlock == 0) {
        return false;
    }

    // get the current text from the block
    const QString text = cursor.block().text();

    char charToRemove{};

    // current char
    const char charInFront = text.at(positionInBlock - 1).toLatin1();
    // is it opener?
    int pos = _openingCharacters.indexOf(charInFront);
    // for " and '
    bool isOpener = false;
    bool isCloser = false;
    if (pos == 5 || pos == 6) {
        isOpener = isQuotOpener(positionInBlock - 1, text);
    } else {
        isOpener = pos != -1;
    }
    if (isOpener) {
        charToRemove = _closingCharacters.at(pos);
    } else {
        // is it closer?
        pos = _closingCharacters.indexOf(charInFront);
        if (pos == 5 || pos == 6)
            isCloser = isQuotCloser(positionInBlock - 1, text);
        else
            isCloser = pos == -1 ? false : true;
        if (isCloser)
            charToRemove = _openingCharacters.at(pos);
        else
            return false;
    }

    int charToRemoveIndex = -1;
    if (isOpener) {
        bool closer = true;
        charToRemoveIndex = text.indexOf(charToRemove, positionInBlock);
        if (charToRemoveIndex == -1) return false;
        if (pos == 5 || pos == 6)
            closer = isQuotCloser(charToRemoveIndex, text);
        if (!closer) return false;
        cursor.setPosition(position + (charToRemoveIndex - positionInBlock));
        cursor.deleteChar();
    } else if (isCloser) {
        charToRemoveIndex = text.lastIndexOf(charToRemove, positionInBlock - 2);
        if (charToRemoveIndex == -1) return false;
        bool opener = true;
        if (pos == 5 || pos == 6)
            opener = isQuotOpener(charToRemoveIndex, text);
        if (!opener) return false;
        const int pos = position - (positionInBlock - charToRemoveIndex);
        cursor.setPosition(pos);
        cursor.deleteChar();
        position -= 1;
    } else {
        charToRemoveIndex = text.lastIndexOf(charToRemove, positionInBlock - 2);
        if (charToRemoveIndex == -1) return false;
        const int pos = position - (positionInBlock - charToRemoveIndex);
        cursor.setPosition(pos);
        cursor.deleteChar();
        position -= 1;
    }

    // moving the cursor back to the old position so the previous character
    // can be removed
    cursor.setPosition(position);
    setTextCursor(cursor);
    return false;
}

/**
 * Increases (or decreases) the indention of the selected text
 * (if there is a text selected) in the noteTextEdit
 * @return
 */
bool QMarkdownTextEdit::increaseSelectedTextIndention(
    bool reverse, const QString &indentCharacters) {
    QTextCursor cursor = this->textCursor();
    QString selectedText = cursor.selectedText();

    if (!selectedText.isEmpty()) {
        // we need this strange newline character we are getting in the
        // selected text for newlines
        const QString newLine =
            QString::fromUtf8(QByteArray::fromHex(QByteArrayLiteral("e280a9")));
        QString newText;

        if (reverse) {
            // un-indent text

            //          QSettings settings;
            const int indentSize = indentCharacters == QStringLiteral("\t")
                                       ? 4
                                       : indentCharacters.count();

            // remove leading \t or spaces in following lines
            newText = selectedText.replace(
                QRegularExpression(newLine + QStringLiteral("(\\t| {1,") +
                                   QString::number(indentSize) +
                                   QStringLiteral("})")),
                QStringLiteral("\n"));

            // remove leading \t or spaces in first line
            newText.remove(QRegularExpression(QStringLiteral("^(\\t| {1,") +
                                              QString::number(indentSize) +
                                              QStringLiteral("})")));
        } else {
            // replace trailing new line to prevent an indent of the line after
            // the selection
            newText = selectedText.replace(
                QRegularExpression(QRegularExpression::escape(newLine) +
                                   QStringLiteral("$")),
                QStringLiteral("\n"));

            // indent text
            newText.replace(newLine, QStringLiteral("\n") + indentCharacters)
                .prepend(indentCharacters);

            // remove trailing \t
            newText.remove(QRegularExpression(QStringLiteral("\\t$")));
        }

        // insert the new text
        cursor.insertText(newText);

        // update the selection to the new text
        cursor.setPosition(cursor.position() - newText.size(),
                           QTextCursor::KeepAnchor);
        this->setTextCursor(cursor);

        return true;
    } else if (reverse) {
        const int indentSize = indentCharacters.count();

        // do the check as often as we have characters to un-indent
        for (int i = 1; i <= indentSize; i++) {
            // if nothing was selected but we want to reverse the indention
            // check if there is a \t in front or after the cursor and remove it
            // if so
            const int position = cursor.position();

            if (!cursor.atStart()) {
                // get character in front of cursor
                cursor.setPosition(position - 1, QTextCursor::KeepAnchor);
            }

            // check for \t or space in front of cursor
            QRegularExpression re(QStringLiteral("[\\t ]"));
            QRegularExpressionMatch match = re.match(cursor.selectedText());

            if (!match.hasMatch()) {
                // (select to) check for \t or space after the cursor
                cursor.setPosition(position);

                if (!cursor.atEnd()) {
                    cursor.setPosition(position + 1, QTextCursor::KeepAnchor);
                }
            }

            match = re.match(cursor.selectedText());

            if (match.hasMatch()) {
                cursor.removeSelectedText();
            }

            cursor = this->textCursor();
        }

        return true;
    }

    // else just insert indentCharacters
    cursor.insertText(indentCharacters);

    return true;
}

/**
 * @brief Opens the link (if any) at the current cursor position
 */
bool QMarkdownTextEdit::openLinkAtCursorPosition() {
    QTextCursor cursor = this->textCursor();
    const int clickedPosition = cursor.position();

    // select the text in the clicked block and find out on
    // which position we clicked
    cursor.movePosition(QTextCursor::StartOfBlock);
    const int positionFromStart = clickedPosition - cursor.position();
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

    const QString selectedText = cursor.selectedText();

    // find out which url in the selected text was clicked
    const QString urlString =
        getMarkdownUrlAtPosition(selectedText, positionFromStart);
    const QUrl url = QUrl(urlString);
    const bool isRelativeFileUrl =
        urlString.startsWith(QLatin1String("file://.."));
    const bool isLegacyAttachmentUrl =
        urlString.startsWith(QLatin1String("file://attachments"));

    qDebug() << __func__ << " - 'emit urlClicked( urlString )': " << urlString;

    emit urlClicked(urlString);

    if ((url.isValid() && isValidUrl(urlString)) || isRelativeFileUrl ||
        isLegacyAttachmentUrl) {
        // ignore some schemata
        if (!(_ignoredClickUrlSchemata.contains(url.scheme()) ||
              isRelativeFileUrl || isLegacyAttachmentUrl)) {
            // open the url
            openUrl(urlString);
        }

        return true;
    }

    return false;
}

/**
 * Checks if urlString is a valid url
 *
 * @param urlString
 * @return
 */
bool QMarkdownTextEdit::isValidUrl(const QString &urlString) {
    const QRegularExpressionMatch match =
        QRegularExpression(R"(^\w+:\/\/.+)").match(urlString);
    return match.hasMatch();
}

/**
 * Handles clicked urls
 *
 * examples:
 * - <https://www.qownnotes.org> opens the webpage
 * - <file:///path/to/my/file/QOwnNotes.pdf> opens the file
 *   "/path/to/my/file/QOwnNotes.pdf" if the operating system supports that
 *  handler
 */
void QMarkdownTextEdit::openUrl(const QString &urlString) {
    qDebug() << "QMarkdownTextEdit " << __func__
             << " - 'urlString': " << urlString;

    QDesktopServices::openUrl(QUrl(urlString));
}

/**
 * @brief Returns the highlighter instance
 * @return
 */
MarkdownHighlighter *QMarkdownTextEdit::highlighter() { return _highlighter; }

/**
 * @brief Returns the searchWidget instance
 * @return
 */
QPlainTextEditSearchWidget *QMarkdownTextEdit::searchWidget() {
    return _searchWidget;
}

/**
 * @brief Sets url schemata that will be ignored when clicked on
 * @param urlSchemes
 */
void QMarkdownTextEdit::setIgnoredClickUrlSchemata(
    QStringList ignoredUrlSchemata) {
    _ignoredClickUrlSchemata = std::move(ignoredUrlSchemata);
}

/**
 * @brief Returns a map of parsed markdown urls with their link texts as key
 *
 * @param text
 * @return parsed urls
 */
QMap<QString, QString> QMarkdownTextEdit::parseMarkdownUrlsFromText(
    const QString &text) {
    QMap<QString, QString> urlMap;
    QRegularExpression regex;
    QRegularExpressionMatchIterator iterator;

    // match urls like this: <http://mylink>
    //    re = QRegularExpression("(<(.+?:\\/\\/.+?)>)");
    regex = QRegularExpression(QStringLiteral("(<(.+?)>)"));
    iterator = regex.globalMatch(text);
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString linkText = match.captured(1);
        QString url = match.captured(2);
        urlMap[linkText] = url;
    }

    // match urls like this: [this url](http://mylink)
    //    QRegularExpression re("(\\[.*?\\]\\((.+?:\\/\\/.+?)\\))");
    regex = QRegularExpression(R"((\[.*?\]\((.+?)\)))");
    iterator = regex.globalMatch(text);
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString linkText = match.captured(1);
        QString url = match.captured(2);
        urlMap[linkText] = url;
    }

    // match urls like this: http://mylink
    regex = QRegularExpression(R"(\b\w+?:\/\/[^\s]+[^\s>\)])");
    iterator = regex.globalMatch(text);
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString url = match.captured(0);
        urlMap[url] = url;
    }

    // match reference urls like this: [this url][1] with this later:
    // [1]: http://domain
    regex = QRegularExpression(R"(\[(.*?)\]\[(.+?)\])");
    iterator = regex.globalMatch(text);
    while (iterator.hasNext()) {
        QRegularExpressionMatch match = iterator.next();
        QString linkText = match.captured(1);
        QString referenceId = match.captured(2);

        // search for the referenced url in the whole text edit
        //        QRegularExpression refRegExp(
        //                "\\[" + QRegularExpression::escape(referenceId) +
        //                        "\\]: (.+?:\\/\\/.+)");
        QRegularExpression refRegExp(QStringLiteral("\\[") +
                                     QRegularExpression::escape(referenceId) +
                                     QStringLiteral("\\]: (.+?)"));
        QRegularExpressionMatch urlMatch = refRegExp.match(toPlainText());

        if (urlMatch.hasMatch()) {
            QString url = urlMatch.captured(1);
            urlMap[linkText] = url;
        }
    }

    return urlMap;
}

/**
 * @brief Returns the markdown url at position
 * @param text
 * @param position
 * @return url string
 */
QString QMarkdownTextEdit::getMarkdownUrlAtPosition(const QString &text,
                                                    int position) {
    QString url;

    // get a map of parsed markdown urls with their link texts as key
    const QMap<QString, QString> urlMap = parseMarkdownUrlsFromText(text);
    QMap<QString, QString>::const_iterator i = urlMap.constBegin();
    for (; i != urlMap.constEnd(); ++i) {
        const QString &linkText = i.key();
        const QString &urlString = i.value();

        const int foundPositionStart = text.indexOf(linkText);

        if (foundPositionStart >= 0) {
            // calculate end position of found linkText
            const int foundPositionEnd = foundPositionStart + linkText.size();

            // check if position is in found string range
            if ((position >= foundPositionStart) &&
                (position <= foundPositionEnd)) {
                url = urlString;
                break;
            }
        }
    }

    return url;
}

/**
 * @brief Duplicates the text in the text edit
 */
void QMarkdownTextEdit::duplicateText() {
    QTextCursor cursor = this->textCursor();
    QString selectedText = cursor.selectedText();

    // duplicate line if no text was selected
    if (selectedText.isEmpty()) {
        const int position = cursor.position();

        // select the whole line
        cursor.movePosition(QTextCursor::StartOfBlock);
        cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);

        const int positionDiff = cursor.position() - position;
        selectedText = "\n" + cursor.selectedText();

        // insert text with new line at end of the selected line
        cursor.setPosition(cursor.selectionEnd());
        cursor.insertText(selectedText);

        // set the position to same position it was in the duplicated line
        cursor.setPosition(cursor.position() - positionDiff);
    } else {
        // duplicate selected text
        cursor.setPosition(cursor.selectionEnd());
        const int selectionStart = cursor.position();

        // insert selected text
        cursor.insertText(selectedText);
        const int selectionEnd = cursor.position();

        // select the inserted text
        cursor.setPosition(selectionStart);
        cursor.setPosition(selectionEnd, QTextCursor::KeepAnchor);
    }

    this->setTextCursor(cursor);
}

void QMarkdownTextEdit::setText(const QString &text) { setPlainText(text); }

void QMarkdownTextEdit::setPlainText(const QString &text) {
    // clear the dirty blocks vector to increase performance and prevent
    // a possible crash in QSyntaxHighlighter::rehighlightBlock
    _highlighter->clearDirtyBlocks();

    QPlainTextEdit::setPlainText(text);
    adjustRightMargin();
}

/**
 * Uses another widget as parent for the search widget
 */
void QMarkdownTextEdit::initSearchFrame(QWidget *searchFrame, bool darkMode) {
    _searchFrame = searchFrame;

    // remove the search widget from our layout
    layout()->removeWidget(_searchWidget);

    QLayout *layout = _searchFrame->layout();

    // create a grid layout for the frame and add the search widget to it
    if (layout == nullptr) {
        layout = new QVBoxLayout(_searchFrame);
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);
    }

    _searchWidget->setDarkMode(darkMode);
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
    if (isReadOnly()) {
        return true;
    }

    QTextCursor cursor = this->textCursor();
    const int position = cursor.position();

    cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
    const QString currentLineText = cursor.selectedText();

    // if return is pressed and there is just an unordered list symbol then we
    // want to remove the list symbol Valid listCharacters: '+ ', '-' , '* ', '+
    // [ ] ', '+ [x] ', '- [ ] ', '- [x] ', '* [ ] ', '* [x] '.
    QRegularExpression regex(R"(^(\s*)([+|\-|\*] \[(x| |)\]|[+\-\*])(\s+)$)");
    QRegularExpressionMatchIterator iterator =
        regex.globalMatch(currentLineText);
    if (iterator.hasNext()) {
        cursor.removeSelectedText();
        return true;
    }

    // if return is pressed and there is just an ordered list symbol then we
    // want to remove the list symbol
    regex = QRegularExpression(R"(^(\s*)(\d+[\.|\)])(\s+)$)");
    iterator = regex.globalMatch(currentLineText);
    if (iterator.hasNext()) {
        qDebug() << cursor.selectedText();
        cursor.removeSelectedText();
        return true;
    }

    // Check if we are in an unordered list.
    // We are in a list when we have '* ', '- ' or '+ ', possibly with preceding
    // whitespace. If e.g. user has entered '**text**' and pressed enter - we
    // don't want do anymore list-stuff.
    const QChar char0 = currentLineText.trimmed()[0];
    const QChar char1 = currentLineText.trimmed()[1];
    const bool inList =
        ((char0 == QLatin1Char('*') || char0 == QLatin1Char('-') ||
          char0 == QLatin1Char('+')) &&
         char1 == QLatin1Char(' '));

    if (inList) {
        // if the current line starts with a list character (possibly after
        // whitespaces) add the whitespaces at the next line too
        // Valid listCharacters: '+ ', '-' , '* ', '+ [ ] ', '+ [x] ', '- [ ] ',
        // '- [x] ', '* [ ] ', '* [x] '.
        regex =
            QRegularExpression(R"(^(\s*)([+|\-|\*] \[(x| |)\]|[+\-\*])(\s+))");
        iterator = regex.globalMatch(currentLineText);
        if (iterator.hasNext()) {
            const QRegularExpressionMatch match = iterator.next();
            const QString whitespaces = match.captured(1);
            QString listCharacter = match.captured(2);
            const QString whitespaceCharacter = match.captured(4);

            // start new checkbox list item with an unchecked checkbox
            iterator = QRegularExpression(R"(^([+|\-|\*]) \[(x| |)\])")
                           .globalMatch(listCharacter);
            if (iterator.hasNext()) {
                const QRegularExpressionMatch match = iterator.next();
                const QString realListCharacter = match.captured(1);
                listCharacter = realListCharacter + QStringLiteral(" [ ]");
            }

            cursor.setPosition(position);
            cursor.insertText("\n" + whitespaces + listCharacter +
                              whitespaceCharacter);

            // scroll to the cursor if we are at the bottom of the document
            ensureCursorVisible();
            return true;
        }
    }

    // check for ordered lists and increment the list number in the next line
    regex = QRegularExpression(R"(^(\s*)(\d+)([\.|\)])(\s+))");
    iterator = regex.globalMatch(currentLineText);
    if (iterator.hasNext()) {
        const QRegularExpressionMatch match = iterator.next();
        const QString whitespaces = match.captured(1);
        const uint listNumber = match.captured(2).toUInt();
        const QString listMarker = match.captured(3);
        const QString whitespaceCharacter = match.captured(4);

        cursor.setPosition(position);
        cursor.insertText("\n" + whitespaces + QString::number(listNumber + 1) +
                          listMarker + whitespaceCharacter);

        // scroll to the cursor if we are at the bottom of the document
        ensureCursorVisible();
        return true;
    }

    // intent next line with same whitespaces as in current line
    regex = QRegularExpression(R"(^(\s+))");
    iterator = regex.globalMatch(currentLineText);
    if (iterator.hasNext()) {
        const QRegularExpressionMatch match = iterator.next();
        const QString whitespaces = match.captured(1);

        cursor.setPosition(position);
        cursor.insertText("\n" + whitespaces);

        // scroll to the cursor if we are at the bottom of the document
        ensureCursorVisible();
        return true;
    }

    return false;
}

/**
 * Handles entered tab or reverse tab keys
 */
bool QMarkdownTextEdit::handleTabEntered(bool reverse,
                                         const QString &indentCharacters) {
    if (isReadOnly()) {
        return true;
    }

    QTextCursor cursor = this->textCursor();

    // only check for lists if we haven't a text selected
    if (cursor.selectedText().isEmpty()) {
        cursor.movePosition(QTextCursor::StartOfBlock, QTextCursor::KeepAnchor);
        const QString currentLineText = cursor.selectedText();

        // check if we want to indent or un-indent an ordered list
        // Valid listCharacters: '+ ', '-' , '* ', '+ [ ] ', '+ [x] ', '- [ ] ',
        // '- [x] ', '* [ ] ', '* [x] '.
        QRegularExpression re(R"(^(\s*)([+|\-|\*] \[(x| )\]|[+\-\*])(\s+)$)");
        QRegularExpressionMatchIterator i = re.globalMatch(currentLineText);

        if (i.hasNext()) {
            QRegularExpressionMatch match = i.next();
            QString whitespaces = match.captured(1);
            const QString listCharacter = match.captured(2);
            const QString whitespaceCharacter = match.captured(4);

            // add or remove one tabulator key
            if (reverse) {
                // remove one set of indentCharacters or a tabulator
                whitespaces.remove(QRegularExpression(
                    QStringLiteral("^(\\t|") +
                    QRegularExpression::escape(indentCharacters) +
                    QStringLiteral(")")));

            } else {
                whitespaces += indentCharacters;
            }

            cursor.insertText(whitespaces + listCharacter +
                              whitespaceCharacter);
            return true;
        }

        // check if we want to indent or un-indent an ordered list
        re = QRegularExpression(R"(^(\s*)(\d+)([\.|\)])(\s+)$)");
        i = re.globalMatch(currentLineText);

        if (i.hasNext()) {
            const QRegularExpressionMatch match = i.next();
            QString whitespaces = match.captured(1);
            const QString listCharacter = match.captured(2);
            const QString listMarker = match.captured(3);
            const QString whitespaceCharacter = match.captured(4);

            // add or remove one tabulator key
            if (reverse) {
                whitespaces.chop(1);
            } else {
                whitespaces += indentCharacters;
            }

            cursor.insertText(whitespaces + listCharacter + listMarker +
                              whitespaceCharacter);
            return true;
        }
    }

    // check if we want to intent the whole text
    return increaseSelectedTextIndention(reverse, indentCharacters);
}

/**
 * Sets the auto text options
 */
void QMarkdownTextEdit::setAutoTextOptions(AutoTextOptions options) {
    _autoTextOptions = options;
}

/**
 * @param e
 * @details This does two things
 * 1. Overrides QPlainTextEdit::paintEvent to fix the RTL bug of QPlainTextEdit
 * 2. Paints a rectangle around code block fences [Code taken from
 * ghostwriter(which in turn is based on QPlaintextEdit::paintEvent() with
 * modifications and minor improvements for our use
 */
void QMarkdownTextEdit::paintEvent(QPaintEvent *e) {
    QTextBlock block = firstVisibleBlock();

    QPainter painter(viewport());
    const QRect viewportRect = viewport()->rect();
    // painter.fillRect(viewportRect, Qt::transparent);
    bool firstVisible = true;
    QPointF offset(contentOffset());
    QRectF blockAreaRect;    // Code or block quote rect.
    bool inBlockArea = false;

    bool clipTop = false;
    bool drawBlock = false;
    qreal dy = 0.0;
    bool done = false;

    const QColor &color = MarkdownHighlighter::codeBlockBackgroundColor();
    const int cornerRadius = 5;

    while (block.isValid() && !done) {
        const QRectF r = blockBoundingRect(block).translated(offset);
        const int state = block.userState();

        if (!inBlockArea && MarkdownHighlighter::isCodeBlock(state)) {
            // skip the backticks
            if (!block.text().startsWith(QLatin1String("```")) &&
                !block.text().startsWith(QLatin1String("~~~"))) {
                blockAreaRect = r;
                dy = 0.0;
                inBlockArea = true;
            }

            // If this is the first visible block within the viewport
            // and if the previous block is part of the text block area,
            // then the rectangle to draw for the block area will have
            // its top clipped by the viewport and will need to be
            // drawn specially.
            const int prevBlockState = block.previous().userState();
            if (firstVisible &&
                MarkdownHighlighter::isCodeBlock(prevBlockState)) {
                clipTop = true;
            }
        }
        // Else if the block ends a text block area...
        else if (inBlockArea && MarkdownHighlighter::isCodeBlockEnd(state)) {
            drawBlock = true;
            inBlockArea = false;
            blockAreaRect.setHeight(dy);
        }
        // If the block is at the end of the document and ends a text
        // block area...
        //
        if (inBlockArea && block == this->document()->lastBlock()) {
            drawBlock = true;
            inBlockArea = false;
            dy += r.height();
            blockAreaRect.setHeight(dy);
        }
        offset.ry() += r.height();
        dy += r.height();

        // If this is the last text block visible within the viewport...
        if (offset.y() > viewportRect.height()) {
            if (inBlockArea) {
                blockAreaRect.setHeight(dy);
                drawBlock = true;
            }

            // Finished drawing.
            done = true;
        }
        // If this is the last text block visible within the viewport...
        if (offset.y() > viewportRect.height()) {
            if (inBlockArea) {
                blockAreaRect.setHeight(dy);
                drawBlock = true;
            }
            // Finished drawing.
            done = true;
        }

        if (drawBlock) {
            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(color));

            // If the first visible block is "clipped" such that the previous
            // block is part of the text block area, then only draw a rectangle
            // with the bottom corners rounded, and with the top corners square
            // to reflect that the first visible block is part of a larger block
            // of text.
            //
            if (clipTop) {
                QPainterPath path;
                path.setFillRule(Qt::WindingFill);
                path.addRoundedRect(blockAreaRect, cornerRadius, cornerRadius);
                qreal adjustedHeight = blockAreaRect.height() / 2;
                path.addRect(blockAreaRect.adjusted(0, 0, 0, -adjustedHeight));
                painter.drawPath(path.simplified());
                clipTop = false;
            }
            // Else draw the entire rectangle with all corners rounded.
            else {
                painter.drawRoundedRect(blockAreaRect, cornerRadius,
                                        cornerRadius);
            }

            drawBlock = false;
        }

        // this fixes the RTL bug of QPlainTextEdit
        // https://bugreports.qt.io/browse/QTBUG-7516
        if (block.text().isRightToLeft()) {
            QTextLayout *layout = block.layout();
            // opt = document()->defaultTextOption();
            QTextOption opt = QTextOption(Qt::AlignRight);
            opt.setTextDirection(Qt::RightToLeft);
            layout->setTextOption(opt);
        }

        block = block.next();
        firstVisible = false;
    }

    painter.end();
    QPlainTextEdit::paintEvent(e);
}

/**
 * Overrides QPlainTextEdit::setReadOnly to fix a problem with Chinese and
 * Japanese input methods
 *
 * @param ro
 */
void QMarkdownTextEdit::setReadOnly(bool ro) {
    QPlainTextEdit::setReadOnly(ro);

    // attempted to fix a problem with Chinese and Japanese input methods
    // @see https://github.com/pbek/QOwnNotes/issues/976
    setAttribute(Qt::WA_InputMethodEnabled, !isReadOnly());
}

void QMarkdownTextEdit::doSearch(
    QString &searchText, QPlainTextEditSearchWidget::SearchMode searchMode) {
    _searchWidget->setSearchText(searchText);
    _searchWidget->setSearchMode(searchMode);
    _searchWidget->doSearchCount();
    _searchWidget->activate(false);
}

void QMarkdownTextEdit::hideSearchWidget(bool reset) {
    _searchWidget->deactivate();

    if (reset) {
        _searchWidget->reset();
    }
}

void QMarkdownTextEdit::updateSettings() {
    // if true: centers the screen if cursor reaches bottom (but not top)
    setCenterOnScroll(_centerCursor);
}
