/*
 * Copyright (c) 2014-2022 Patrizio Bekerle -- <patrizio@bekerle.com>
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

#include "qplaintexteditsearchwidget.h"

#include <QEvent>
#include <QKeyEvent>

#include "ui_qplaintexteditsearchwidget.h"

QPlainTextEditSearchWidget::QPlainTextEditSearchWidget(QPlainTextEdit *parent)
    : QWidget(parent),
      _textEdit(parent),
      _darkMode(false),
      _ui(new Ui::QPlainTextEditSearchWidget),
      _latestSearchLowestCursorIndex(0),
      _latestSearchSelectionIndex(0),
      _modeChangedFlag(false),
      _selectionColor(0, 180, 0, 100) {

    _ui->setupUi(this);
    hide();

    _ui->searchCountLabel->setStyleSheet(QStringLiteral("* {color: grey}"));
    // hiding will leave an open space in the horizontal layout
    _ui->searchCountLabel->setEnabled(false);

    connect(_ui->closeButton, &QPushButton::clicked, this,
            &QPlainTextEditSearchWidget::deactivate);
    connect(_ui->searchLineEdit, &QLineEdit::textChanged, this,
            &QPlainTextEditSearchWidget::searchLineEditTextChanged);
    connect(_ui->searchDownButton, &QPushButton::clicked, this,
            &QPlainTextEditSearchWidget::doSearchDown);
    connect(_ui->searchUpButton, &QPushButton::clicked, this,
            &QPlainTextEditSearchWidget::doSearchUp);
    connect(_ui->replaceToggleButton, &QPushButton::toggled, this,
            &QPlainTextEditSearchWidget::setReplaceMode);
    connect(_ui->replaceButton, &QPushButton::clicked, this,
            &QPlainTextEditSearchWidget::doReplace);
    connect(_ui->replaceAllButton, &QPushButton::clicked, this,
            &QPlainTextEditSearchWidget::doReplaceAll);
    connect(&_debounceTimer, &QTimer::timeout,
            this, &QPlainTextEditSearchWidget::triggerSearch);
    connect(_textEdit, &QPlainTextEdit::textChanged, this,
            &QPlainTextEditSearchWidget::invalidateSearch);

    installEventFilter(this);
    _ui->searchLineEdit->installEventFilter(this);
    _ui->replaceLineEdit->installEventFilter(this);

#ifdef Q_OS_MAC
    // set the spacing to 8 for OS X
    layout()->setSpacing(8);
    ui->buttonFrame->layout()->setSpacing(9);

    // set the margin to 0 for the top buttons for OS X
    QString buttonStyle = "QPushButton {margin: 0}";
    ui->closeButton->setStyleSheet(buttonStyle);
    ui->searchDownButton->setStyleSheet(buttonStyle);
    ui->searchUpButton->setStyleSheet(buttonStyle);
    ui->replaceToggleButton->setStyleSheet(buttonStyle);
    ui->matchCaseSensitiveButton->setStyleSheet(buttonStyle);
#endif
}

QPlainTextEditSearchWidget::~QPlainTextEditSearchWidget() {
    delete _ui;
}

void QPlainTextEditSearchWidget::setSearchText(const QString &searchText) {
    _ui->searchLineEdit->setText(searchText);
}

void QPlainTextEditSearchWidget::setSearchMode(SearchMode searchMode) {
    _ui->modeComboBox->setCurrentIndex(searchMode);
}

void QPlainTextEditSearchWidget::setDebounceDelay(uint debounceDelay) {
    _debounceTimer.setInterval(static_cast<int>(debounceDelay));
}

void QPlainTextEditSearchWidget::setDarkMode(bool enabled) {
    _darkMode = enabled;
}

void QPlainTextEditSearchWidget::activate() {
    activate(true);
}

void QPlainTextEditSearchWidget::deactivate() {
    stopDebounce();

    hide();

    clearSearchExtraSelections();

    _textEdit->setFocus();
}

void QPlainTextEditSearchWidget::activateReplace() {
    // replacing is prohibited if the text edit is readonly
    if (_textEdit->isReadOnly()) {
        return;
    }

    _ui->searchLineEdit->setText(_textEdit->textCursor().selectedText());
    _ui->searchLineEdit->selectAll();

    activate();
    setReplaceMode(true);
}

void QPlainTextEditSearchWidget::triggerSearch()
{
    performSearch(SearchDirection::Down);
}

void QPlainTextEditSearchWidget::setReplaceMode(bool enabled) {
    _ui->replaceToggleButton->setChecked(enabled);
    _ui->replaceLabel->setVisible(enabled);
    _ui->replaceLineEdit->setVisible(enabled);
    _ui->modeLabel->setVisible(enabled);
    _ui->buttonFrame->setVisible(enabled);
    _ui->matchCaseSensitiveButton->setVisible(enabled);
}

void QPlainTextEditSearchWidget::invalidateSearch()
{
    _searchTermUsedInLatestSearch.clear();
    _latestSearchResults.clear();
    _latestSearchSelectionIndex = 0;
    _latestSearchLowestCursorIndex = 0;
}

bool QPlainTextEditSearchWidget::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Escape) {
            deactivate();
            return true;
        } else if (!_debounceTimer.isActive() &&
                   (keyEvent->modifiers().testFlag(Qt::ShiftModifier) &&
                    (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Up))) {
            doSearchUp();
            return true;
        } else if (!_debounceTimer.isActive() &&
                    (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Down)) {
            doSearchDown();
            return true;
        } else if (!_debounceTimer.isActive() && keyEvent->key() == Qt::Key_F3) {
//            doSearch(!keyEvent->modifiers().testFlag(Qt::ShiftModifier));
//            return true;
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

void QPlainTextEditSearchWidget::searchLineEditTextChanged(const QString &searchTerm) {
    if (_debounceTimer.interval() != 0 && !searchTerm.isEmpty()) {
        _debounceTimer.start();
        _ui->searchDownButton->setEnabled(false);
        _ui->searchUpButton->setEnabled(false);
    } else {
        triggerSearch();
    }
}

void QPlainTextEditSearchWidget::performSearch(SearchDirection direction)
{
    stopDebounce();

    const auto searchTerm = _ui->searchLineEdit->text();
    if (searchTerm.isEmpty()) {
        return;
    }

    _directionUsedInLatestSearch = direction;

    const auto searchMode = static_cast<SearchMode>(_ui->modeComboBox->currentIndex());

    if (searchMode == RegularExpressionMode) {
        // Prevent stuck application when the user enters just start or end markers
        static const QRegularExpression regExp(R"(^[\^\$]+$)");
        if (regExp.match(searchTerm).hasMatch()) {
            clearSearchExtraSelections();
            return;
        }
    }

    if (searchTerm == _searchTermUsedInLatestSearch && !_modeChangedFlag) {
        if (_latestSearchResults.size() == 0) {
            return;
        }

        const auto currentCursor = _textEdit->textCursor();
        int selectionIndexInSearchResults;
        int numeElementsSearched{0};

        for (selectionIndexInSearchResults = _latestSearchLowestCursorIndex;
             _latestSearchResults[selectionIndexInSearchResults] < currentCursor &&
             numeElementsSearched != _latestSearchResults.size();
             ++selectionIndexInSearchResults) {
            ++numeElementsSearched;
            if (selectionIndexInSearchResults == _latestSearchResults.size() - 1) {
                selectionIndexInSearchResults = -1;
            }
        }

        if (numeElementsSearched >= _latestSearchResults.size()) {
            selectionIndexInSearchResults = _latestSearchLowestCursorIndex;
        }

        _latestSearchSelectionIndex = selectionIndexInSearchResults;
        stepSelectionIndexInCurrentSearch(0);
    } else {
        if (_modeChangedFlag) {
            _modeChangedFlag = false;
        }

        _latestSearchLowestCursorIndex = 0;

        _latestSearchResults = doSearch(SearchOptions {
                                               .searchDirection = direction,
                                               .searchFromTheBeginning = false,
                                               .wrapSearch = true,
                                               .findMultiOccurances = true
                                           });
        _searchTermUsedInLatestSearch = searchTerm;

        _latestSearchSelectionIndex = 0;
        stepSelectionIndexInCurrentSearch(0);
    }
}

QList<QTextCursor> QPlainTextEditSearchWidget::doSearch(const SearchOptions &searchOptions)
{
    const QString searchTerm = _ui->searchLineEdit->text();
    if (searchTerm.isEmpty()) {
        return {};
    }

    const SearchMode searchMode = static_cast<SearchMode>(_ui->modeComboBox->currentIndex());
    const bool caseSensitive = _ui->matchCaseSensitiveButton->isChecked();

    QFlags<QTextDocument::FindFlag> findFlags{};

    if (searchMode == WholeWordsMode) {
        findFlags |= QTextDocument::FindWholeWords;
    }
    if (caseSensitive) {
        findFlags |= QTextDocument::FindCaseSensitively;
    }
    if (searchOptions.searchDirection == SearchDirection::Up) {
        findFlags |= QTextDocument::FindBackward;
    }

    _textEdit->blockSignals(true);

    const auto document = _textEdit->document();

    QList<QTextCursor> searchResults;
    QTextCursor searchResult;
    QTextCursor firstSearchResult;
    QTextCursor lowestPosResult;
    bool continueSearch{false};
    bool wrappedSearch{false};

    QTextCursor nextSearchPosition =_textEdit->textCursor();
    if (searchOptions.searchFromTheBeginning) {
        nextSearchPosition.movePosition(searchOptions.searchDirection == SearchDirection::Down ? QTextCursor::Start : QTextCursor::End);
    } else {
        nextSearchPosition.movePosition(QTextCursor::StartOfBlock);
    }

    do {
        continueSearch = false;

        searchResult = (searchMode == SearchMode::RegularExpressionMode ?
                            document->find(QRegularExpression(searchTerm, caseSensitive
                                                              ? QRegularExpression::NoPatternOption
                                                              : QRegularExpression::CaseInsensitiveOption),
                                           nextSearchPosition,
                                           findFlags)
                          :
                            document->find(searchTerm, nextSearchPosition, findFlags));

        if (!searchResult.isNull()) {
            if (firstSearchResult.isNull()) {
                firstSearchResult = searchResult;
            } else if (firstSearchResult == searchResult) {
                // wrapped and found again
                break;
            }

            if (lowestPosResult.isNull() || searchResult < lowestPosResult) {
                lowestPosResult = searchResult;
                _latestSearchLowestCursorIndex = searchResults.size();
            }

            searchResults.push_back(searchResult);
            nextSearchPosition = searchResult;
            nextSearchPosition.movePosition(searchOptions.searchDirection == SearchDirection::Down ? QTextCursor::NextCharacter :
                                                                                                     QTextCursor::PreviousCharacter);
        }

        if (searchOptions.findMultiOccurances) {
            if (!searchResult.isNull()) {
                continueSearch = true;
            } else if (!wrappedSearch && searchOptions.wrapSearch) {
                nextSearchPosition.movePosition(searchOptions.searchDirection == SearchDirection::Down ? QTextCursor::Start :
                                                                                                         QTextCursor::End);

                _textEdit->moveCursor(searchOptions.searchDirection == SearchDirection :: Down ? QTextCursor::Start :
                                                                                                 QTextCursor::End);
                wrappedSearch = true;
                continueSearch = true;
            }
        } else {
            if (searchResult.isNull() && (!wrappedSearch && searchOptions.wrapSearch)) {
                _textEdit->moveCursor(searchOptions.searchDirection == SearchDirection :: Down ? QTextCursor::Start :
                                                                                                 QTextCursor::End);
                wrappedSearch = true;
                continueSearch = true;
            }
        }
    } while (continueSearch);

    _textEdit->blockSignals(false);

    return searchResults;
}

void QPlainTextEditSearchWidget::doSearchUp()
{
    if (_searchTermUsedInLatestSearch.isEmpty()) {
        performSearch(SearchDirection::Up);
    } else {
        stepSelectionIndexInCurrentSearch(-1);
    }
}

void QPlainTextEditSearchWidget::doSearchDown()
{
    if (_searchTermUsedInLatestSearch.isEmpty()) {
        performSearch(SearchDirection::Down);
    } else {
        stepSelectionIndexInCurrentSearch(1);
    }
}

void QPlainTextEditSearchWidget::stepSelectionIndexInCurrentSearch(int stepOffset)
{
    // add a background color according if we found the text or not
    const QString bgColorCode =
            _darkMode
            ? (_latestSearchResults.size() != 0 ? QStringLiteral("#135a13")
                     : QStringLiteral("#8d2b36"))
            : _latestSearchResults.size() != 0 ? QStringLiteral("#D5FAE2") : QStringLiteral("#FAE9EB");
    const QString fgColorCode =
            _darkMode ? QStringLiteral("#cccccc") : QStringLiteral("#404040");

    _ui->searchLineEdit->setStyleSheet(
                QStringLiteral("* { background: ") + bgColorCode +
                QStringLiteral("; color: ") + fgColorCode + QStringLiteral("; }"));

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

    if (_latestSearchResults.size() == 0) {
        clearSearchCountLabelText();
        return;
    }

    if (stepOffset != 0) {
        bool negativeOffset = (stepOffset < 0);
        stepOffset = abs(stepOffset) % _latestSearchResults.size();
        if (negativeOffset) { stepOffset *= -1; };
        if (_directionUsedInLatestSearch == SearchDirection::Up) { stepOffset *= -1; }

        _latestSearchSelectionIndex += stepOffset;
        if (_latestSearchSelectionIndex >= _latestSearchResults.size()) {
            _latestSearchSelectionIndex -= _latestSearchResults.size();
        } else if (_latestSearchSelectionIndex < 0) {
            _latestSearchSelectionIndex += _latestSearchResults.size();
        }
    }

    updateSearchCountLabelText();

    _textEdit->setTextCursor(_latestSearchResults[_latestSearchSelectionIndex]);

    updateSearchExtraSelections();
}

void QPlainTextEditSearchWidget::updateSearchCountLabelText() {
    _ui->searchCountLabel->setEnabled(true);

    int currentPositionInSearch{0};
    if (_directionUsedInLatestSearch == SearchDirection::Down) {
        currentPositionInSearch = (_latestSearchSelectionIndex + _latestSearchResults.size() - _latestSearchLowestCursorIndex) % _latestSearchResults.size() + 1;
    } else {
        currentPositionInSearch = (_latestSearchResults.size() - _latestSearchSelectionIndex + _latestSearchLowestCursorIndex) % _latestSearchResults.size() + 1;
    }

    _ui->searchCountLabel->setText(QString("%1/%2").arg(
                                      QString::number(currentPositionInSearch),
                                      QString::number(_latestSearchResults.size())));
}

void QPlainTextEditSearchWidget::clearSearchCountLabelText() {
    _ui->searchCountLabel->setText("0/0");
}

void QPlainTextEditSearchWidget::updateSearchExtraSelections() {
    if (_latestSearchResults.size() == 0) {
        return;
    }

    QList<QTextEdit::ExtraSelection> searchExtraSelections;
    const QColor color = _selectionColor;
    QTextCharFormat extraFormat;
    extraFormat.setBackground(color);

    for (const auto cursor: _latestSearchResults) {
        QTextEdit::ExtraSelection extra = QTextEdit::ExtraSelection();
        extra.format = extraFormat;
        extra.cursor = cursor;
        searchExtraSelections.push_back(extra);
    }

    _textEdit->setExtraSelections(searchExtraSelections);
}

void QPlainTextEditSearchWidget::clearSearchExtraSelections() {
    this->_textEdit->setExtraSelections({});
}

void QPlainTextEditSearchWidget::stopDebounce()
{
    _debounceTimer.stop();
    _ui->searchDownButton->setEnabled(true);
    _ui->searchUpButton->setEnabled(true);
}

bool QPlainTextEditSearchWidget::doReplace() {
    if (_textEdit->isReadOnly() || _latestSearchResults.isEmpty()) {
        return false;
    }

    _textEdit->blockSignals(true);

    const auto replacementText = _ui->replaceLineEdit->text();

    _latestSearchResults[_latestSearchSelectionIndex].insertText(replacementText);
    _latestSearchResults.removeAt(_latestSearchSelectionIndex);

    if (_directionUsedInLatestSearch == SearchDirection::Down) {
        if (_latestSearchSelectionIndex < _latestSearchLowestCursorIndex) {
            _latestSearchLowestCursorIndex -= 1;
        } else if (_latestSearchSelectionIndex == _latestSearchLowestCursorIndex && _latestSearchLowestCursorIndex >= _latestSearchResults.size()) {
            _latestSearchLowestCursorIndex = 0;
        }

        if (_latestSearchSelectionIndex == _latestSearchResults.size()) {
            _latestSearchSelectionIndex = 0;
        }
    } else {
        if (_latestSearchSelectionIndex <= _latestSearchLowestCursorIndex) {
            --_latestSearchLowestCursorIndex;
        }
        --_latestSearchSelectionIndex;
        if (_latestSearchSelectionIndex < 0) {
            _latestSearchSelectionIndex = _latestSearchResults.size() - 1;
        }
        if (_latestSearchLowestCursorIndex < 0) {
            _latestSearchLowestCursorIndex = _latestSearchResults.size() - 1;
        }
    }

    stepSelectionIndexInCurrentSearch(0);

    _textEdit->blockSignals(false);

    return true;
}

void QPlainTextEditSearchWidget::doReplaceAll() {
    while (doReplace());
}

void QPlainTextEditSearchWidget::activate(bool focus) {
    setReplaceMode(_ui->modeComboBox->currentIndex() !=
                   SearchMode::PlainTextMode);
    show();

    // preset the selected text as search text
    const QString selectedText = _textEdit->textCursor().selectedText();
    if (!selectedText.isEmpty()) {
        _ui->searchLineEdit->setText(selectedText);
    }

    if (focus) {
        _ui->searchLineEdit->setFocus();
    }
    _ui->searchLineEdit->selectAll();

    triggerSearch();
}

void QPlainTextEditSearchWidget::reset() {
    _ui->searchLineEdit->clear();
    setSearchMode(SearchMode::PlainTextMode);
    setReplaceMode(false);
    _ui->searchCountLabel->setEnabled(false);
}

void QPlainTextEditSearchWidget::on_modeComboBox_currentIndexChanged(
    int index) {
    Q_UNUSED(index);
    _modeChangedFlag = true;
    triggerSearch();
}

void QPlainTextEditSearchWidget::on_matchCaseSensitiveButton_toggled(
    bool checked) {
    Q_UNUSED(checked)
    _modeChangedFlag = true;
    triggerSearch();
}
