/*
 * Copyright (c) 2014-2023 Patrizio Bekerle -- <patrizio@bekerle.com>
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

namespace Ui {
class QPlainTextEditSearchWidget;
}

class QPlainTextEditSearchWidget : public QWidget {
    Q_OBJECT

   public:
    enum SearchMode { PlainTextMode, WholeWordsMode, RegularExpressionMode };

    explicit QPlainTextEditSearchWidget(QPlainTextEdit *parent = nullptr);
    bool doSearch(bool searchDown = true, bool allowRestartAtTop = true,
                  bool updateUI = true);
    void setDarkMode(bool enabled);
    ~QPlainTextEditSearchWidget();

    void setSearchText(const QString &searchText);
    void setSearchMode(SearchMode searchMode);
    void setDebounceDelay(uint debounceDelay);
    void activate(bool focus);
    void clearSearchExtraSelections();
    void updateSearchExtraSelections();

   private:
    Ui::QPlainTextEditSearchWidget *ui;
    int _searchResultCount;
    int _currentSearchResult;
    QList<QTextEdit::ExtraSelection> _searchExtraSelections;
    QColor selectionColor;
    QTimer _debounceTimer;
    QString _searchTerm;
    void setSearchExtraSelections() const;
    void stopDebounce();

   protected:
    QPlainTextEdit *_textEdit;
    bool _darkMode;
    bool eventFilter(QObject *obj, QEvent *event) override;

   public Q_SLOTS:
    void activate();
    void deactivate();
    void doSearchDown();
    void doSearchUp();
    void setReplaceMode(bool enabled);
    void activateReplace();
    bool doReplace(bool forAll = false);
    void doReplaceAll();
    void reset();
    void doSearchCount();

   protected Q_SLOTS:
    void searchLineEditTextChanged(const QString &arg1);
    void performSearch();
    void updateSearchCountLabelText();
    void setSearchSelectionColor(const QColor &color);
   private Q_SLOTS:
    void on_modeComboBox_currentIndexChanged(int index);
    void on_matchCaseSensitiveButton_toggled(bool checked);
};
