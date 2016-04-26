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
#include "ui_qtexteditsearchwidget.h"
#include <QEvent>
#include <QKeyEvent>
#include <QDebug>

QTextEditSearchWidget::QTextEditSearchWidget(QTextEdit *parent) :
    QWidget(parent),
    ui(new Ui::QTextEditSearchWidget)
{
    ui->setupUi(this);
    _textEdit = parent;
    hide();

    QObject::connect(ui->closeButton, SIGNAL(clicked()),
                     this, SLOT(deactivate()));
    QObject::connect(ui->searchLineEdit, SIGNAL(textChanged(const QString &)),
                     this, SLOT(searchLineEditTextChanged(const QString &)));
    QObject::connect(ui->searchDownButton, SIGNAL(clicked()),
                     this, SLOT(doSearchDown()));
    QObject::connect(ui->searchUpButton, SIGNAL(clicked()),
                     this, SLOT(doSearchUp()));

    installEventFilter(this);
    ui->searchLineEdit->installEventFilter(this);
}

QTextEditSearchWidget::~QTextEditSearchWidget()
{
    delete ui;
}

void QTextEditSearchWidget::activate() {
    show();
    ui->searchLineEdit->setFocus();
    ui->searchLineEdit->selectAll();
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
        } else if (keyEvent->key() == Qt::Key_F3) {
            doSearch(!keyEvent->modifiers().testFlag(Qt::ShiftModifier));
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
    QString text = ui->searchLineEdit->text();

    if (text == "") {
        ui->searchLineEdit->setStyleSheet("* { background: none; }");
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
    ui->searchLineEdit->setStyleSheet("* { background: " + colorCode + "; }");
}

