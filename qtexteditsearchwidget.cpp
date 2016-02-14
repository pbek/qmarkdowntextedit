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

#include "qtexteditsearchwidget.h"
#include <QEvent>
#include <QHBoxLayout>
#include <QKeyEvent>
#include <QDebug>
#include <QPushButton>

QTextEditSearchWidget::QTextEditSearchWidget(QTextEdit *parent) : QWidget(
        parent) {
    _textEdit = parent;
    this->hide();
    this->setAutoFillBackground(true);
    QHBoxLayout *layout = new QHBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setMargin(0);

    // add the close button
    _closeButton = new QPushButton();
    _closeButton->setIcon(QIcon::fromTheme(
                    "window-close",
                    QIcon(":/media/window-close.svg")));
    _closeButton->setToolTip("close search");
    _closeButton->setFlat(true);
    QObject::connect(_closeButton, SIGNAL(clicked()), this, SLOT(deactivate()));
    layout->addWidget(_closeButton);

    // add the find label
    _label = new QLabel("Find:");
    layout->addWidget(_label);

    // add the search line edit
    _searchLineEdit = new QLineEdit;
    _searchLineEdit->setPlaceholderText("find in text");
    _searchLineEdit->setClearButtonEnabled(true);
    layout->addWidget(_searchLineEdit);
    QObject::connect(
            _searchLineEdit,
            SIGNAL(textChanged(const QString &)),
            this,
            SLOT(searchLineEditTextChanged(const QString &)));

    // add the search forward button
    _searchDownButton = new QPushButton();
    _searchDownButton->setIcon(QIcon(":/media/go-bottom.svg"));
    _searchDownButton->setIcon(QIcon::fromTheme(
            "go-bottom",
            QIcon(":/media/go-bottom.svg")));
    _searchDownButton->setToolTip("search forward");
    _searchDownButton->setFlat(true);
    QObject::connect(_searchDownButton, SIGNAL(clicked()), this,
                     SLOT(doSearchDown()));
    layout->addWidget(_searchDownButton);

    // add the search backward button
    _searchUpButton = new QPushButton();
    _searchUpButton->setIcon(QIcon(":/media/go-top.svg"));
    _searchUpButton->setIcon(QIcon::fromTheme(
            "go-top",
            QIcon(":/media/go-top.svg")));
    _searchUpButton->setToolTip("search backward");
    _searchUpButton->setFlat(true);
    QObject::connect(_searchUpButton, SIGNAL(clicked()), this,
                     SLOT(doSearchUp()));
    layout->addWidget(_searchUpButton);

    this->setLayout(layout);

    installEventFilter(this);
    _searchLineEdit->installEventFilter(this);
}

void QTextEditSearchWidget::activate() {
    show();
    _searchLineEdit->setFocus();
    _searchLineEdit->selectAll();
    doSearchDown();
}

void QTextEditSearchWidget::deactivate() {
    hide();
    _textEdit->setFocus();
}

bool QTextEditSearchWidget::eventFilter(QObject *obj, QEvent *event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

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
        }

        return false;
    }

    return QWidget::eventFilter(obj, event);
}

void QTextEditSearchWidget::searchLineEditTextChanged(const QString &arg1) {
    Q_UNUSED(arg1);
    doSearchDown();
}

void QTextEditSearchWidget::doSearchUp() {
    doSearch(false);
}

void QTextEditSearchWidget::doSearchDown() {
    doSearch(true);
}

/**
 * @brief Searches for text in the text edit
 */
void QTextEditSearchWidget::doSearch(bool searchDown) {
    QString text = _searchLineEdit->text();

    if (text == "") {
        _searchLineEdit->setStyleSheet("* { background: none; }");
        return;
    }

    QTextDocument::FindFlag options = searchDown ? QTextDocument::FindFlag(0)
                                                 : QTextDocument::FindBackward;
    bool found = _textEdit->find(text, options);

    // start at the top if not found
    if (!found) {
        _textEdit->moveCursor(
                searchDown ? QTextCursor::Start : QTextCursor::End);
        found = _textEdit->find(text, options);
    }

    QRect rect = _textEdit->cursorRect();
    QMargins margins = _textEdit->layout()->contentsMargins();
    int searchWidgetHotArea = _textEdit->height() - this->height();
    int marginBottom = (rect.y() > searchWidgetHotArea) ? (this->height() + 10)
                                                        : 0;

    // move the search box a bit up if we would block the search result
    if (margins.bottom() != marginBottom) {
        margins.setBottom(marginBottom);
        _textEdit->layout()->setContentsMargins(margins);
    }

    // add a background color according if we found the text or not
    QString colorCode = found ? "#D5FAE2" : "#FAE9EB";
    _searchLineEdit->setStyleSheet("* { background: " + colorCode + "; }");
}

