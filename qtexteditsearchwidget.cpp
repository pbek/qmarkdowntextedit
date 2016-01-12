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

QTextEditSearchWidget::QTextEditSearchWidget(QTextEdit *parent) : QWidget(parent)
{
    _textEdit = parent;
    this->hide();
    this->setAutoFillBackground( true );
    QHBoxLayout *layout = new QHBoxLayout;

    _label = new QLabel( "Find:" );
    layout->addWidget( _label );

    _searchLineEdit = new QLineEdit;
    layout->addWidget( _searchLineEdit );

    this->setLayout( layout );

    installEventFilter( this );
}

void QTextEditSearchWidget::activate()
{
    this->show();
    _searchLineEdit->setFocus();
}

void QTextEditSearchWidget::deactivate()
{
    this->hide();
    _textEdit->setFocus();
}

bool QTextEditSearchWidget::eventFilter(QObject* obj, QEvent *event)
{
    if ( event->type() == QEvent::KeyPress )
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if ( keyEvent->key() == Qt::Key_Escape )
        {
            deactivate();
            return false;
        }
        else if ( ( keyEvent->modifiers().testFlag( Qt::ShiftModifier ) && ( keyEvent->key() == Qt::Key_Return ) ) || ( keyEvent->key() == Qt::Key_Up ) )
        {
            doSearch( false );
            return true;
        }
        else if ( ( keyEvent->key() == Qt::Key_Return ) || ( keyEvent->key() == Qt::Key_Down ) )
        {
            doSearch();
            return true;
        }

        return false;
    }

    return QWidget::eventFilter( obj, event );
}

/**
 * @brief Searches for text in the text edit
 */
void QTextEditSearchWidget::doSearch( bool searchDown )
{
    QString text = _searchLineEdit->text();

    if ( text == "" )
    {
        _searchLineEdit->setStyleSheet( "* { background: none; }" );
        return;
    }

    QTextDocument::FindFlag options = searchDown ? QTextDocument::FindFlag (0) : QTextDocument::FindBackward;
    bool found = _textEdit->find( text, options );

    // start at the top if not found
    if ( !found )
    {
        _textEdit->moveCursor( searchDown ? QTextCursor::Start : QTextCursor::End );
        found = _textEdit->find( text, options );
    }

    // add a background color according if we found the text or not
    QString colorCode = found ? "#D5FAE2" : "#FAE9EB";
    _searchLineEdit->setStyleSheet( "* { background: " + colorCode + "; }" );
}
