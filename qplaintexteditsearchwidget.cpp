/*
 * MIT License
 *
 * Copyright (c) 2014-2026 Patrizio Bekerle -- <patrizio@bekerle.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "qplaintexteditsearchwidget.h"

#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QRegularExpression>
#include <QScrollBar>
#include <QTextBoundaryFinder>

#include "ui_qplaintexteditsearchwidget.h"

namespace {
bool isEmojiCodePoint(unsigned int codePoint) {
    return codePoint == 0x20E3 || codePoint == 0x00A9 || codePoint == 0x00AE ||
           codePoint == 0x203C || codePoint == 0x2049 || codePoint == 0x2122 ||
           codePoint == 0x2139 ||
           (codePoint >= 0x2194 && codePoint <= 0x21AA) ||
           (codePoint >= 0x231A && codePoint <= 0x2328) ||
           codePoint == 0x23CF ||
           (codePoint >= 0x23E9 && codePoint <= 0x23FA) ||
           codePoint == 0x24C2 ||
           (codePoint >= 0x25AA && codePoint <= 0x25AB) ||
           codePoint == 0x25B6 || codePoint == 0x25C0 ||
           (codePoint >= 0x25FB && codePoint <= 0x25FE) ||
           (codePoint >= 0x2600 && codePoint <= 0x27BF) ||
           (codePoint >= 0x2934 && codePoint <= 0x2935) ||
           (codePoint >= 0x2B05 && codePoint <= 0x2B55) ||
           codePoint == 0x3030 || codePoint == 0x303D || codePoint == 0x3297 ||
           codePoint == 0x3299 ||
           (codePoint >= 0x1F000 && codePoint <= 0x1FAFF);
}

int graphemeCount(const QString &text, int maxCount) {
    if (text.isEmpty()) {
        return 0;
    }

    QTextBoundaryFinder finder(QTextBoundaryFinder::Grapheme, text);
    finder.toStart();

    int count = 0;
    while (finder.toNextBoundary() != -1) {
        ++count;

        if (count >= maxCount) {
            break;
        }
    }

    return count;
}

bool shouldStartSearch(const QString &text) {
    const int minimumSearchLength = 2;
    const int count = graphemeCount(text, minimumSearchLength);
    if (count >= minimumSearchLength) {
        return true;
    }

    if (count != 1) {
        return false;
    }

    const auto codePoints = text.toUcs4();
    for (unsigned int codePoint : codePoints) {
        if (isEmojiCodePoint(codePoint)) {
            return true;
        }
    }

    return false;
}
}    // namespace

QPlainTextEditSearchWidget::QPlainTextEditSearchWidget(QPlainTextEdit *parent)
    : QWidget(parent),
      ui(new Ui::QPlainTextEditSearchWidget),
      selectionColor(0, 180, 0, 100) {
    ui->setupUi(this);
    _textEdit = parent;
    _darkMode = false;
    hide();
    ui->searchCountLabel->setStyleSheet(QStringLiteral("* {color: grey}"));
    // hiding will leave a open space in the horizontal layout
    ui->searchCountLabel->setEnabled(false);
    _currentSearchResult = 0;
    _searchResultCount = 0;
    clearSelectionSearchScope();

    connect(ui->closeButton, &QPushButton::clicked, this,
            &QPlainTextEditSearchWidget::deactivate);
    connect(ui->searchLineEdit, &QLineEdit::textChanged, this,
            &QPlainTextEditSearchWidget::searchLineEditTextChanged);
    connect(ui->searchDownButton, &QPushButton::clicked, this,
            &QPlainTextEditSearchWidget::doSearchDown);
    connect(ui->searchUpButton, &QPushButton::clicked, this,
            &QPlainTextEditSearchWidget::doSearchUp);
    connect(ui->replaceToggleButton, &QPushButton::toggled, this,
            &QPlainTextEditSearchWidget::setReplaceMode);
    connect(ui->replaceButton, &QPushButton::clicked, this,
            &QPlainTextEditSearchWidget::doReplace);
    connect(ui->replaceAllButton, &QPushButton::clicked, this,
            &QPlainTextEditSearchWidget::doReplaceAll);

    connect(&_debounceTimer, &QTimer::timeout, this,
            &QPlainTextEditSearchWidget::performSearch);

    installEventFilter(this);
    ui->searchLineEdit->installEventFilter(this);
    ui->replaceLineEdit->installEventFilter(this);
    _textEdit->installEventFilter(this);
    _textEdit->viewport()->installEventFilter(this);

#ifdef Q_OS_MAC
    // set the spacing to 8 for OS X
    layout()->setSpacing(8);
    ui->buttonFrame->layout()->setSpacing(9);

    // set the margin to 0 for the top buttons for OS X
    QString buttonStyle = QStringLiteral("QPushButton {margin: 0}");
    ui->closeButton->setStyleSheet(buttonStyle);
    ui->searchDownButton->setStyleSheet(buttonStyle);
    ui->searchUpButton->setStyleSheet(buttonStyle);
    ui->replaceToggleButton->setStyleSheet(buttonStyle);
    ui->matchCaseSensitiveButton->setStyleSheet(buttonStyle);
    ui->selectionOnlyButton->setStyleSheet(buttonStyle);
#endif
}

QPlainTextEditSearchWidget::~QPlainTextEditSearchWidget() { delete ui; }

void QPlainTextEditSearchWidget::activate() { activate(true); }

void QPlainTextEditSearchWidget::activateReplace() {
    // replacing is prohibited if the text edit is readonly
    if (_textEdit->isReadOnly()) {
        return;
    }

    ui->searchLineEdit->setText(_textEdit->textCursor().selectedText());
    ui->searchLineEdit->selectAll();
    activate();
    setReplaceMode(true);
}

void QPlainTextEditSearchWidget::deactivate() {
    stopDebounce();

    hide();

    // Clear the search extra selections when closing the search bar
    clearSearchExtraSelections();

    _textEdit->setFocus();
}

void QPlainTextEditSearchWidget::setReplaceMode(bool enabled) {
    ui->replaceToggleButton->setChecked(enabled);
    ui->replaceLabel->setVisible(enabled);
    ui->replaceLineEdit->setVisible(enabled);
    ui->modeLabel->setVisible(enabled);
    ui->buttonFrame->setVisible(enabled);
    ui->matchCaseSensitiveButton->setVisible(enabled);
    ui->selectionOnlyButton->setVisible(enabled);
}

bool QPlainTextEditSearchWidget::eventFilter(QObject *obj, QEvent *event) {
    if (obj == _textEdit || obj == _textEdit->viewport()) {
        if ((event->type() == QEvent::MouseButtonRelease ||
             event->type() == QEvent::KeyRelease) &&
            ui->selectionOnlyButton->isChecked()) {
            const QTextCursor cursor = _textEdit->textCursor();

            if (cursor.hasSelection() &&
                (cursor.selectionStart() != _selectionSearchStart ||
                 cursor.selectionEnd() != _selectionSearchEnd)) {
                updateSelectionOnlySearchScope(cursor);
            }
        }

        return QWidget::eventFilter(obj, event);
    }

    if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Escape) {
            deactivate();
            return true;
        } else if ((!_debounceTimer.isActive() &&
                    keyEvent->modifiers().testFlag(Qt::ShiftModifier) &&
                    (keyEvent->key() == Qt::Key_Return)) ||
                   (keyEvent->key() == Qt::Key_Up)) {
            doSearchUp();
            return true;
        } else if (!_debounceTimer.isActive() &&
                   ((keyEvent->key() == Qt::Key_Return) ||
                    (keyEvent->key() == Qt::Key_Down))) {
            doSearchDown();
            return true;
        } else if (!_debounceTimer.isActive() &&
                   keyEvent->key() == Qt::Key_F3) {
            doSearch(!keyEvent->modifiers().testFlag(Qt::ShiftModifier));
            return true;
        }

        //        if ((obj == ui->replaceLineEdit) && (keyEvent->key() ==
        //        Qt::Key_Tab)
        //                && ui->replaceToggleButton->isChecked()) {
        //            ui->replaceLineEdit->setFocus();
        //        }

        return false;
    }

    return QWidget::eventFilter(obj, event);
}

void QPlainTextEditSearchWidget::searchLineEditTextChanged(
    const QString &arg1) {
    _searchTerm = arg1;

    if (_debounceTimer.interval() != 0 && shouldStartSearch(_searchTerm)) {
        _debounceTimer.start();
        ui->searchDownButton->setEnabled(false);
        ui->searchUpButton->setEnabled(false);
    } else {
        performSearch();
    }
}

void QPlainTextEditSearchWidget::performSearch() {
    if (!shouldStartSearch(ui->searchLineEdit->text())) {
        clearSearchExtraSelections();
        _searchResultCount = 0;
        _currentSearchResult = 0;
        updateSearchCountLabelText();
        doSearchDown();
        return;
    }

    doSearchCount();
    updateSearchExtraSelections();
    doSearchDown();
}

void QPlainTextEditSearchWidget::clearSearchExtraSelections() {
    _searchExtraSelections.clear();
    setSearchExtraSelections();
}

void QPlainTextEditSearchWidget::updateSearchExtraSelections() {
    _searchExtraSelections.clear();

    if (!shouldStartSearch(ui->searchLineEdit->text())) {
        setSearchExtraSelections();
        return;
    }

    const auto textCursor = _textEdit->textCursor();
    moveToSearchScopeStart();
    const QColor color = selectionColor;
    QTextCharFormat extraFmt;
    extraFmt.setBackground(color);
    int findCounter = 0;
    const int searchMode = ui->modeComboBox->currentIndex();

    while (doSearch(true, false, false)) {
        findCounter++;

        // prevent infinite loops from regular expression searches like "$", "^"
        // or "\b"
        if (searchMode == RegularExpressionMode && findCounter >= 10000) {
            break;
        }

        QTextEdit::ExtraSelection extra = QTextEdit::ExtraSelection();
        extra.format = extraFmt;

        extra.cursor = _textEdit->textCursor();
        _searchExtraSelections.append(extra);
    }

    _textEdit->setTextCursor(textCursor);
    this->setSearchExtraSelections();
}

void QPlainTextEditSearchWidget::setSearchExtraSelections() const {
    this->_textEdit->setExtraSelections(this->_searchExtraSelections);
}

void QPlainTextEditSearchWidget::stopDebounce() {
    _debounceTimer.stop();
    ui->searchDownButton->setEnabled(true);
    ui->searchUpButton->setEnabled(true);
}

void QPlainTextEditSearchWidget::doSearchUp() { doSearch(false); }

void QPlainTextEditSearchWidget::doSearchDown() { doSearch(true); }

bool QPlainTextEditSearchWidget::doReplace(bool forAll) {
    if (_textEdit->isReadOnly()) {
        return false;
    }

    QTextCursor cursor = _textEdit->textCursor();

    if (!forAll && isSelectionOnlySearchEnabled() &&
        !isCurrentSelectionSearchMatch()) {
        moveToSearchScopeStart();

        if (!doSearch(true, false)) {
            _textEdit->setTextCursor(cursor);
            return false;
        }

        cursor = _textEdit->textCursor();
    }

    if (!forAll && cursor.selectedText().isEmpty()) {
        return false;
    }

    const int searchMode = ui->modeComboBox->currentIndex();
    const int replacedStart = cursor.selectionStart();
    const int replacedEnd = cursor.selectionEnd();
    const QString replacementText = ui->replaceLineEdit->text();
    if (searchMode == RegularExpressionMode) {
        QString text = cursor.selectedText();
        text.replace(QRegularExpression(ui->searchLineEdit->text()),
                     replacementText);
        cursor.insertText(text);
    } else {
        cursor.insertText(replacementText);
    }

    if (isSelectionOnlySearchEnabled()) {
        _selectionSearchEnd +=
            cursor.position() - replacedStart - (replacedEnd - replacedStart);
    }

    if (!forAll) {
        const int position = cursor.position();

        if (!doSearch(true)) {
            // restore the last cursor position if text wasn't found any more
            cursor.setPosition(position);
            _textEdit->setTextCursor(cursor);
        }
    }

    return true;
}

void QPlainTextEditSearchWidget::doReplaceAll() {
    if (_textEdit->isReadOnly()) {
        return;
    }

    // start at the top of the search scope
    moveToSearchScopeStart();

    // replace until everything to the bottom is replaced
    while (doSearch(true, false) && doReplace(true)) {
    }
}

/**
 * @brief Searches for text in the text edit
 * @returns true if found
 */
bool QPlainTextEditSearchWidget::doSearch(bool searchDown,
                                          bool allowRestartAtTop,
                                          bool updateUI) {
    if (_debounceTimer.isActive()) {
        stopDebounce();
    }

    const QString text = ui->searchLineEdit->text();

    if (!shouldStartSearch(text)) {
        if (updateUI) {
            ui->searchLineEdit->setStyleSheet(QLatin1String(""));
        }

        return false;
    }

    const int searchMode = ui->modeComboBox->currentIndex();
    const bool caseSensitive = ui->matchCaseSensitiveButton->isChecked();
    const bool selectionOnly = isSelectionOnlySearchEnabled();

    QFlags<QTextDocument::FindFlag> options =
        searchDown ? QTextDocument::FindFlag(0) : QTextDocument::FindBackward;
    if (searchMode == WholeWordsMode) {
        options |= QTextDocument::FindWholeWords;
    }

    if (caseSensitive) {
        options |= QTextDocument::FindCaseSensitively;
    }

    if (selectionOnly) {
        const QTextCursor cursor = _textEdit->textCursor();
        if (!isCursorInSearchScope(cursor)) {
            searchDown ? moveToSearchScopeStart() : moveToSearchScopeEnd();
        }
    }

    // block signal to reduce too many signals being fired and too many updates
    _textEdit->blockSignals(true);

    bool found =
        searchMode == RegularExpressionMode
            ?
#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
            _textEdit->find(
                QRegularExpression(
                    text, caseSensitive
                              ? QRegularExpression::NoPatternOption
                              : QRegularExpression::CaseInsensitiveOption),
                options)
            :
#else
            _textEdit->find(QRegExp(text, caseSensitive ? Qt::CaseSensitive
                                                        : Qt::CaseInsensitive),
                            options)
            :
#endif
            _textEdit->find(text, options);

    if (selectionOnly && found) {
        const QTextCursor cursor = _textEdit->textCursor();
        found = isCursorInSearchScope(cursor);
    }

    _textEdit->blockSignals(false);

    if (found) {
        const int result =
            searchDown ? ++_currentSearchResult : --_currentSearchResult;
        _currentSearchResult = std::min(result, _searchResultCount);

        updateSearchCountLabelText();
    }

    // start at the top (or bottom) if not found
    if (!found && allowRestartAtTop) {
        searchDown ? moveToSearchScopeStart() : moveToSearchScopeEnd();
        found =
            searchMode == RegularExpressionMode
                ?
#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
                _textEdit->find(
                    QRegularExpression(
                        text, caseSensitive
                                  ? QRegularExpression::NoPatternOption
                                  : QRegularExpression::CaseInsensitiveOption),
                    options)
                :
#else
                _textEdit->find(
                    QRegExp(text, caseSensitive ? Qt::CaseSensitive
                                                : Qt::CaseInsensitive),
                    options)
                :
#endif
                _textEdit->find(text, options);

        if (selectionOnly && found) {
            const QTextCursor cursor = _textEdit->textCursor();
            found = isCursorInSearchScope(cursor);
        }

        if (found && updateUI) {
            _currentSearchResult = searchDown ? 1 : _searchResultCount;
            updateSearchCountLabelText();
        }
    }

    if (updateUI) {
        const QRect rect = _textEdit->cursorRect();
        QMargins margins = _textEdit->layout()->contentsMargins();
        const int searchWidgetHotArea = _textEdit->height() - this->height();
        const int marginBottom =
            (rect.y() > searchWidgetHotArea) ? (this->height() + 10) : 0;

        // move the search box a bit up if we would block the search result
        if (margins.bottom() != marginBottom) {
            margins.setBottom(marginBottom);
            _textEdit->layout()->setContentsMargins(margins);
        }

        // add a background color according if we found the text or not
        const QString bgColorCode = _darkMode
                                        ? (found ? QStringLiteral("#135a13")
                                                 : QStringLiteral("#8d2b36"))
                                    : found ? QStringLiteral("#D5FAE2")
                                            : QStringLiteral("#FAE9EB");
        const QString fgColorCode =
            _darkMode ? QStringLiteral("#cccccc") : QStringLiteral("#404040");

        ui->searchLineEdit->setStyleSheet(
            QStringLiteral("* { background: ") + bgColorCode +
            QStringLiteral("; color: ") + fgColorCode + QStringLiteral("; }"));

        // restore the search extra selections after the find command
        this->setSearchExtraSelections();
    }

    return found;
}

/**
 * @brief Counts the search results
 */
void QPlainTextEditSearchWidget::doSearchCount() {
    if (!shouldStartSearch(ui->searchLineEdit->text())) {
        _searchResultCount = 0;
        _currentSearchResult = 0;
        updateSearchCountLabelText();
        return;
    }

    // Note that we are moving the anchor, so the search will start from the top
    // again! Alternative: Restore cursor position afterward, but then we will
    // not know
    //              at what _currentSearchResult we currently are
    moveToSearchScopeStart();

    bool found;
    _searchResultCount = 0;
    _currentSearchResult = 0;
    const int searchMode = ui->modeComboBox->currentIndex();

    do {
        found = doSearch(true, false, false);
        if (found) {
            _searchResultCount++;
        }

        // prevent infinite loops from regular expression searches like "$", "^"
        // or "\b"
        if (searchMode == RegularExpressionMode &&
            _searchResultCount >= 10000) {
            break;
        }
    } while (found);

    updateSearchCountLabelText();
}

void QPlainTextEditSearchWidget::setDarkMode(bool enabled) {
    _darkMode = enabled;
}

void QPlainTextEditSearchWidget::setSearchText(const QString &searchText) {
    ui->searchLineEdit->setText(searchText);
}

void QPlainTextEditSearchWidget::setSearchMode(SearchMode searchMode) {
    ui->modeComboBox->setCurrentIndex(searchMode);
}

void QPlainTextEditSearchWidget::setDebounceDelay(uint debounceDelay) {
    _debounceTimer.setInterval(static_cast<int>(debounceDelay));
}

void QPlainTextEditSearchWidget::activate(bool focus) {
    setReplaceMode(ui->modeComboBox->currentIndex() !=
                   SearchMode::PlainTextMode);
    show();

    const int verticalScrollBarValue = _textEdit->verticalScrollBar()->value();
    const int horizontalScrollBarValue =
        _textEdit->horizontalScrollBar()->value();

    // Save the original cursor so we can restore the position after the search
    // widget initialization (for #3541)
    const QTextCursor originalCursor = _textEdit->textCursor();

    // Preset the selected text as search text only when the user manually
    // activates the search widget (focus=true). When activated programmatically
    // (focus=false, e.g. from the note search panel), we must not overwrite the
    // search text that was already set by the caller — otherwise a multi-term
    // regex like "(Heading|1)" gets replaced by the currently selected match
    // word (e.g. "Heading"), causing only one term to be searched (for #3541).
    const QString selectedText = originalCursor.selectedText();
    bool hasPresetSelection = false;
    if (focus && !selectedText.isEmpty()) {
        ui->searchLineEdit->setText(selectedText);
        updateSelectionSearchScope(originalCursor);
        hasPresetSelection = true;
    }

    if (focus) {
        ui->searchLineEdit->setFocus();
    }

    ui->searchLineEdit->selectAll();
    updateSearchExtraSelections();

    // If text was preset from a selection, move the cursor one position before
    // the original selection start. QPlainTextEdit::find() searches strictly
    // after the cursor position, so placing it one character before the
    // selection ensures doSearchDown() finds the originally selected word
    // itself rather than the next occurrence in the document (for #3541).
    if (hasPresetSelection) {
        QTextCursor cursor = _textEdit->textCursor();
        const int pos = originalCursor.selectionStart();
        cursor.setPosition(pos > 0 ? pos - 1 : 0);
        _textEdit->setTextCursor(cursor);
    }

    doSearchDown();

    _textEdit->verticalScrollBar()->setValue(verticalScrollBarValue);
    _textEdit->horizontalScrollBar()->setValue(horizontalScrollBarValue);
}

void QPlainTextEditSearchWidget::reset() {
    ui->searchLineEdit->clear();
    setSearchMode(SearchMode::PlainTextMode);
    setReplaceMode(false);
    ui->selectionOnlyButton->setChecked(false);
    ui->searchCountLabel->setEnabled(false);
}

void QPlainTextEditSearchWidget::updateSearchCountLabelText() {
    ui->searchCountLabel->setEnabled(true);
    ui->searchCountLabel->setText(QStringLiteral("%1/%2").arg(
        _currentSearchResult == 0 ? QChar('-')
                                  : QString::number(_currentSearchResult),
        _searchResultCount == 0 ? QChar('-')
                                : QString::number(_searchResultCount)));
}

void QPlainTextEditSearchWidget::setSearchSelectionColor(const QColor &color) {
    selectionColor = color;
}

bool QPlainTextEditSearchWidget::isSelectionOnlySearchEnabled() const {
    return ui->selectionOnlyButton->isChecked() && hasSelectionSearchScope();
}

bool QPlainTextEditSearchWidget::hasSelectionSearchScope() const {
    return _selectionSearchStart >= 0 &&
           _selectionSearchEnd > _selectionSearchStart;
}

void QPlainTextEditSearchWidget::updateSelectionSearchScope(
    const QTextCursor &cursor) {
    if (cursor.hasSelection()) {
        _selectionSearchStart = cursor.selectionStart();
        _selectionSearchEnd = cursor.selectionEnd();
    }
}

void QPlainTextEditSearchWidget::clearSelectionSearchScope() {
    _selectionSearchStart = -1;
    _selectionSearchEnd = -1;
}

void QPlainTextEditSearchWidget::updateSelectionOnlySearchScope(
    const QTextCursor &cursor) {
    clearSelectionSearchScope();
    updateSelectionSearchScope(cursor);

    doSearchCount();
    updateSearchExtraSelections();

    _textEdit->setTextCursor(cursor);
    setSearchExtraSelections();
}

bool QPlainTextEditSearchWidget::isCurrentSelectionSearchMatch() const {
    const QTextCursor cursor = _textEdit->textCursor();

    if (!cursor.hasSelection()) {
        return false;
    }

    const QString searchText = ui->searchLineEdit->text();
    if (!shouldStartSearch(searchText)) {
        return false;
    }

    const QString selectedText = cursor.selectedText();
    const bool caseSensitive = ui->matchCaseSensitiveButton->isChecked();
    const Qt::CaseSensitivity caseSensitivity =
        caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
    const int searchMode = ui->modeComboBox->currentIndex();

    if (searchMode == RegularExpressionMode) {
        const QRegularExpression re(
            searchText, caseSensitive
                            ? QRegularExpression::NoPatternOption
                            : QRegularExpression::CaseInsensitiveOption);
        const QRegularExpressionMatch match = re.match(selectedText);

        return match.hasMatch() && match.capturedStart() == 0 &&
               match.capturedLength() == selectedText.length();
    }

    return selectedText.compare(searchText, caseSensitivity) == 0;
}

void QPlainTextEditSearchWidget::moveToSearchScopeStart() {
    QTextCursor cursor = _textEdit->textCursor();
    cursor.setPosition(
        isSelectionOnlySearchEnabled() ? _selectionSearchStart : 0,
        QTextCursor::MoveAnchor);
    _textEdit->setTextCursor(cursor);
}

void QPlainTextEditSearchWidget::moveToSearchScopeEnd() {
    QTextCursor cursor = _textEdit->textCursor();
    cursor.setPosition(isSelectionOnlySearchEnabled()
                           ? _selectionSearchEnd
                           : _textEdit->document()->characterCount() - 1,
                       QTextCursor::MoveAnchor);
    _textEdit->setTextCursor(cursor);
}

bool QPlainTextEditSearchWidget::isCursorInSearchScope(
    const QTextCursor &cursor) const {
    if (!isSelectionOnlySearchEnabled()) {
        return true;
    }

    return cursor.hasSelection() &&
           cursor.selectionStart() >= _selectionSearchStart &&
           cursor.selectionEnd() <= _selectionSearchEnd;
}

void QPlainTextEditSearchWidget::on_modeComboBox_currentIndexChanged(
    int index) {
    Q_UNUSED(index)
    doSearchCount();
    doSearchDown();
}

void QPlainTextEditSearchWidget::on_matchCaseSensitiveButton_toggled(
    bool checked) {
    Q_UNUSED(checked)
    doSearchCount();
    doSearchDown();
}

void QPlainTextEditSearchWidget::on_selectionOnlyButton_toggled(bool checked) {
    const QTextCursor cursor = _textEdit->textCursor();

    if (checked) {
        updateSelectionOnlySearchScope(cursor);
        return;
    }

    clearSelectionSearchScope();
    doSearchCount();
    updateSearchExtraSelections();
    doSearchDown();
}
