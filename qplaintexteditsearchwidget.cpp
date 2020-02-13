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

#include "qplaintexteditsearchwidget.h"

#include <QDebug>
#include <QEvent>
#include <QKeyEvent>

#include "ui_qplaintexteditsearchwidget.h"

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

    installEventFilter(this);
    ui->searchLineEdit->installEventFilter(this);
    ui->replaceLineEdit->installEventFilter(this);

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
    hide();
    _textEdit->setFocus();
}

void QPlainTextEditSearchWidget::setReplaceMode(bool enabled) {
    ui->replaceToggleButton->setChecked(enabled);
    ui->replaceLabel->setVisible(enabled);
    ui->replaceLineEdit->setVisible(enabled);
    ui->modeLabel->setVisible(enabled);
    ui->buttonFrame->setVisible(enabled);
    ui->matchCaseSensitiveButton->setVisible(enabled);
}

bool QPlainTextEditSearchWidget::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Escape) {
            deactivate();
            return true;
        } else if ((keyEvent->modifiers().testFlag(Qt::ShiftModifier) &&
                    (keyEvent->key() == Qt::Key_Return)) ||
                   (keyEvent->key() == Qt::Key_Up)) {
            doSearchUp();
            return true;
        } else if ((keyEvent->key() == Qt::Key_Return) ||
                   (keyEvent->key() == Qt::Key_Down)) {
            doSearchDown();
            return true;
        } else if (keyEvent->key() == Qt::Key_F3) {
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
    Q_UNUSED(arg1)
    doSearchCount();
    updateSearchExtraSelections();
    doSearchDown();
}

void QPlainTextEditSearchWidget::updateSearchExtraSelections() {
    _searchExtraSelections.clear();
    const auto textCursor = _textEdit->textCursor();
    _textEdit->moveCursor(QTextCursor::Start);
    const QColor color = selectionColor;
    QTextCharFormat extraFmt;
    extraFmt.setBackground(color);

    while (doSearch(true, false, false)) {
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

void QPlainTextEditSearchWidget::doSearchUp() { doSearch(false); }

void QPlainTextEditSearchWidget::doSearchDown() { doSearch(true); }

bool QPlainTextEditSearchWidget::doReplace(bool forAll) {
    if (_textEdit->isReadOnly()) {
        return false;
    }

    QTextCursor cursor = _textEdit->textCursor();

    if (!forAll && cursor.selectedText().isEmpty()) {
        return false;
    }

    const int searchMode = ui->modeComboBox->currentIndex();
    if (searchMode == RegularExpressionMode) {
        QString text = cursor.selectedText();
        text.replace(QRegExp(ui->searchLineEdit->text()),
                     ui->replaceLineEdit->text());
        cursor.insertText(text);
    } else {
        cursor.insertText(ui->replaceLineEdit->text());
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

    // start at the top
    _textEdit->moveCursor(QTextCursor::Start);

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
    const QString text = ui->searchLineEdit->text();

    if (text.isEmpty()) {
        if (updateUI) {
            ui->searchLineEdit->setStyleSheet(QLatin1String(""));
        }

        return false;
    }

    const int searchMode = ui->modeComboBox->currentIndex();
    const bool caseSensitive = ui->matchCaseSensitiveButton->isChecked();

    QFlags<QTextDocument::FindFlag> options =
        searchDown ? QTextDocument::FindFlag(0) : QTextDocument::FindBackward;
    if (searchMode == WholeWordsMode) {
        options |= QTextDocument::FindWholeWords;
    }

    if (caseSensitive) {
        options |= QTextDocument::FindCaseSensitively;
    }

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

    if (found) {
        const int result =
            searchDown ? ++_currentSearchResult : --_currentSearchResult;
        _currentSearchResult = std::min(result, _searchResultCount);

        updateSearchCountLabelText();
    }

    // start at the top (or bottom) if not found
    if (!found && allowRestartAtTop) {
        _textEdit->moveCursor(searchDown ? QTextCursor::Start
                                         : QTextCursor::End);
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
        const QString bgColorCode =
            _darkMode
                ? (found ? QStringLiteral("#135a13")
                         : QStringLiteral("#8d2b36"))
                : found ? QStringLiteral("#D5FAE2") : QStringLiteral("#FAE9EB");
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
    // Note that we are moving the anchor, so the search will start from the top
    // again! Alternative: Restore cursor position afterwards, but then we will
    // not know
    //              at what _currentSearchResult we currently are
    _textEdit->moveCursor(QTextCursor::Start, QTextCursor::MoveAnchor);

    bool found;
    _searchResultCount = 0;
    _currentSearchResult = 0;

    do {
        found = doSearch(true, false, false);
        if (found) {
            _searchResultCount++;
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

void QPlainTextEditSearchWidget::activate(bool focus) {
    setReplaceMode(ui->modeComboBox->currentIndex() !=
                   SearchMode::PlainTextMode);
    show();

    // preset the selected text as search text if there is any and there is no
    // other search text
    const QString selectedText = _textEdit->textCursor().selectedText();
    if (!selectedText.isEmpty() && ui->searchLineEdit->text().isEmpty()) {
        ui->searchLineEdit->setText(selectedText);
    }

    if (focus) {
        ui->searchLineEdit->setFocus();
    }

    ui->searchLineEdit->selectAll();
    doSearchDown();
}

void QPlainTextEditSearchWidget::reset() {
    ui->searchLineEdit->clear();
    setSearchMode(SearchMode::PlainTextMode);
    setReplaceMode(false);
    ui->searchCountLabel->setEnabled(false);
}

void QPlainTextEditSearchWidget::updateSearchCountLabelText() {
    ui->searchCountLabel->setEnabled(true);
    ui->searchCountLabel->setText(QString("%1/%2").arg(
        _currentSearchResult == 0 ? QChar('-')
                                  : QString::number(_currentSearchResult),
        _searchResultCount == 0 ? QChar('-')
                                : QString::number(_searchResultCount)));
}

void QPlainTextEditSearchWidget::setSearchSelectionColor(const QColor &color) {
    selectionColor = color;
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
