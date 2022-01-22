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

#pragma once

#include <QPlainTextEdit>
#include <QTimer>
#include <QWidget>

#include "searchoptions.h"

namespace Ui {
class QPlainTextEditSearchWidget;
}

class QPlainTextEditSearchWidget : public QWidget {
    Q_OBJECT

public:
    enum SearchMode { PlainTextMode, WholeWordsMode, RegularExpressionMode };

    explicit QPlainTextEditSearchWidget(QPlainTextEdit *parent = nullptr);
    ~QPlainTextEditSearchWidget();

    void setSearchText(const QString &searchText);
    void setSearchMode(SearchMode searchMode);
    void setDebounceDelay(uint debounceDelay);
    void setDarkMode(bool enabled);
    void activate(bool focus);

protected:
    QPlainTextEdit *_textEdit;
    bool _darkMode;
    bool eventFilter(QObject *obj, QEvent *event);

private:
    Ui::QPlainTextEditSearchWidget *_ui;

    QString _searchTermUsedInLatestSearch;
    SearchDirection _directionUsedInLatestSearch;
    QList<QTextCursor> _latestSearchResults;
    int _latestSearchLowestCursorIndex;
    int _latestSearchSelectionIndex;
    bool _modeChangedFlag;
    QColor _selectionColor;
    QTimer _debounceTimer;

    void performSearch(SearchDirection direction);
    QList<QTextCursor> doSearch(const SearchOptions &searchOptions);
    void stepSelectionIndexInCurrentSearch(int stepOffset);
    void clearSearchExtraSelections();
    void updateSearchExtraSelections();
    void stopDebounce();

public slots:
    void setReplaceMode(bool enabled);
    void activate();
    void deactivate();
    void activateReplace();
    void triggerSearch();
    void doSearchDown();
    void doSearchUp();
    bool doReplace();
    void doReplaceAll();
    void reset();

private slots:
    void invalidateSearch();
    void searchLineEditTextChanged(const QString &searchTerm);
    void updateSearchCountLabelText();
    void clearSearchCountLabelText();

    void on_modeComboBox_currentIndexChanged(int index);
    void on_matchCaseSensitiveButton_toggled(bool checked);
};
