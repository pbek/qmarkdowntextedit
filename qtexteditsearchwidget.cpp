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
    QObject::connect(ui->replaceToggleButton, SIGNAL(toggled(bool)),
                     this, SLOT(setReplaceMode(bool)));
    QObject::connect(ui->replaceButton, SIGNAL(clicked()),
                     this, SLOT(doReplace()));
    QObject::connect(ui->replaceAllButton, SIGNAL(clicked()),
                     this, SLOT(doReplaceAll()));

    installEventFilter(this);
    ui->searchLineEdit->installEventFilter(this);
    ui->replaceLineEdit->installEventFilter(this);

#ifdef Q_OS_MAC
    // set the spacing to 9 for OS X
    layout()->setSpacing(9);
    ui->buttonFrame->layout()->setSpacing(9);

    // set the margin to 0 for the top buttons for OS X
    QString buttonStyle = "QPushButton {margin: 0}";
    ui->closeButton->setStyleSheet(buttonStyle);
    ui->searchDownButton->setStyleSheet(buttonStyle);
    ui->searchUpButton->setStyleSheet(buttonStyle);
    ui->replaceToggleButton->setStyleSheet(buttonStyle);
#endif
}

QTextEditSearchWidget::~QTextEditSearchWidget() {
    delete ui;
}

void QTextEditSearchWidget::activate() {
    setReplaceMode(false);
    show();
    ui->searchLineEdit->setFocus();
    ui->searchLineEdit->selectAll();
    doSearchDown();
}

void QTextEditSearchWidget::activateReplace() {
    activate();
    ui->searchLineEdit->setText(_textEdit->textCursor().selectedText());
    setReplaceMode(true);
}

void QTextEditSearchWidget::deactivate() {
    hide();
    _textEdit->setFocus();
}

void QTextEditSearchWidget::setReplaceMode(bool enabled) {
    ui->replaceToggleButton->setChecked(enabled);
    ui->replaceLabel->setVisible(enabled);
    ui->replaceLineEdit->setVisible(enabled);
    ui->buttonFrame->setVisible(enabled);
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

//        if ((obj == ui->replaceLineEdit) && (keyEvent->key() == Qt::Key_Tab)
//                && ui->replaceToggleButton->isChecked()) {
//            ui->replaceLineEdit->setFocus();
//        }

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

void QTextEditSearchWidget::doReplace() {
    if (_textEdit->isReadOnly()) {
        return;
    }

    QTextCursor c = _textEdit->textCursor();

    if (!c.selectedText().isEmpty()) {
        c.insertText(ui->replaceLineEdit->text());
        doSearch(true);
    }
}

void QTextEditSearchWidget::doReplaceAll() {
    if (_textEdit->isReadOnly()) {
        return;
    }

    while (doSearch(true)) {
        _textEdit->textCursor().insertText(ui->replaceLineEdit->text());
    }
}

/**
 * @brief Searches for text in the text edit
 */
bool QTextEditSearchWidget::doSearch(bool searchDown) {
    QString text = ui->searchLineEdit->text();

    if (text == "") {
        ui->searchLineEdit->setStyleSheet("* { background: none; }");
        return false;
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

    return found;
}

