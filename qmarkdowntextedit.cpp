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


#include "qmarkdowntextedit.h"
#include <QKeyEvent>
#include <QGuiApplication>
#include <QDebug>
#include <QRegularExpression>
#include <QDir>
#include <QDesktopServices>
#include <QSettings>
#include <QLayout>


QMarkdownTextEdit::QMarkdownTextEdit(QWidget *parent)
 : QTextEdit(parent)
{
    installEventFilter( this );
    viewport()->installEventFilter( this );

    // setup the markdown highlighting
    _highlighter = new HGMarkdownHighlighter( document(), 1000 );

    // TODO: migrate to somewhere else
    // load text edit font
    QSettings settings;
    QFont font = this->font();
    QString fontString = settings.value("MainWindow/noteTextEdit.font").toString();

    if ( fontString != "" )
    {
        // set the note text edit font
        font.fromString( fontString );
        setFont( font );

        // set the default size for the highlighter
        _highlighter->setDefaultStyles( font.pointSize() );
    }

    // set the tab stop to the width of 4 spaces in the editor
    const int tabStop = 4;
    QFontMetrics metrics( font );
    setTabStopWidth( tabStop * metrics.width( ' ' ) );
}

bool QMarkdownTextEdit::eventFilter(QObject* obj, QEvent *event)
{
    if ( event->type() == QEvent::KeyPress )
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        if ( ( keyEvent->key() == Qt::Key_Tab ) || ( keyEvent->key() == Qt::Key_Backtab ) )
        {
            // indent selected text (if there is a text selected)
            return increaseSelectedTextIndention( keyEvent->key() == Qt::Key_Backtab );
        }
        else if ( ( keyEvent->key() == Qt::Key_F ) && QGuiApplication::keyboardModifiers() == Qt::ExtraButton24 )
        {
            qDebug()<<"Ctrl + F";
            return false;
        }
        // set cursor to pointing hand if control key was pressed
        else if ( keyEvent->key() == Qt::Key_Control )
        {
            QWidget *viewPort = this->viewport();
            viewPort->setCursor( Qt::PointingHandCursor );
            return false;
        }

        return false;
    }
    else if ( event->type() == QEvent::KeyRelease )
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);

        // reset cursor if control key was released
        if ( keyEvent->key() == Qt::Key_Control )
        {
            QWidget *viewPort = this->viewport();
            viewPort->setCursor( Qt::IBeamCursor );
        }

        return false;
    }
    else if ( event->type() == QEvent::MouseButtonRelease )
    {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);

        // track Ctrl+LeftMouseClick in the noteTextEdit
        if ( ( obj == this->viewport() ) && ( mouseEvent->button() == Qt::LeftButton ) && ( QGuiApplication::keyboardModifiers() == Qt::ExtraButton24 ) )
        {
            // open the link (if any) at the current position in the noteTextEdit
            openLinkAtCursorPosition();
            return false;
        }
    }

    return QTextEdit::eventFilter( obj, event );
}

/**
 * @brief Increases (or decreases) the indention of the selected text (if there is a text selected) in the noteTextEdit
 * @return
 */
bool QMarkdownTextEdit::increaseSelectedTextIndention( bool reverse )
{
    QTextCursor c = this->textCursor();
    QString selectedText = c.selectedText();

    if ( selectedText != "" )
    {
        // we need this strange newline character we are getting in the selected text for newlines
        QString newLine = QString::fromUtf8( QByteArray::fromHex( "e280a9" ) );
        QString newText;

        if ( reverse )
        {
            // unindent text
            newText = selectedText.replace( newLine + "\t", "\n" );

            // remove leading \t
            newText.replace( QRegularExpression( "^\\t" ), "" );
        }
        else
        {
            // indent text
            newText = selectedText.replace( newLine, "\n\t" ).prepend( "\t" );

            // remove trailing \t
            newText.replace( QRegularExpression( "\\t$" ), "" );
        }

        // insert the new text
        c.insertText( newText );

        // update the selection to the new text
        c.setPosition( c.position() - newText.size(), QTextCursor::KeepAnchor );
        this->setTextCursor( c );

        return true;
    }
    // if nothing was selected but we want to reverse the indention check if there is a \t in front or after the cursor and remove it if so
    else if ( reverse )
    {
        int pos = c.position();
        // check for \t in front of cursor
        c.setPosition( pos - 1, QTextCursor::KeepAnchor );
        if ( c.selectedText() != "\t" )
        {
            // (select to) check for \t after the cursor
            c.setPosition( pos );
            c.setPosition( pos + 1, QTextCursor::KeepAnchor );
        }

        if ( c.selectedText() == "\t" )
        {
            c.removeSelectedText();
        }

        return true;
    }

    return false;
}


/**
 * @brief Opens the link (if any) at the current cursor position
 */
void QMarkdownTextEdit::openLinkAtCursorPosition()
{
    QTextCursor c = this->textCursor();
    int clickedPosition = c.position();

    // select the text in the clicked block and find out on which position we clicked
    c.movePosition( QTextCursor::StartOfBlock );
    int positionFromStart = clickedPosition - c.position();
    c.movePosition( QTextCursor::EndOfBlock, QTextCursor::KeepAnchor );

    QString selectedText = c.selectedText();

    // find out which url in the selected text was clicked
    QUrl url = getMarkdownUrlAtPosition( selectedText, positionFromStart );
    if ( url.isValid() )
    {
        qDebug() << __func__ << " - 'emit urlClicked( url )': " << url;
        emit urlClicked( url );

        // ignore some schemata
        if ( !_ignoredClickUrlSchemata.contains( url.scheme() ) )
        {
            // open the url
            openUrl( url );
        }
    }
}

/**
 * Handles clicked urls
 *
 * examples:
 * - <http://www.qownnotes.org> opens the webpage
 * - <file:///path/to/my/file/QOwnNotes.pdf> opens the file "/path/to/my/file/QOwnNotes.pdf" if the operating system supports that handler
 */
void QMarkdownTextEdit::openUrl( QUrl url )
{
    qDebug() << __func__ << " - 'url': " << url;

    // TODO: migrate to somewhere else
    QString urlString = url.toString();
    QSettings settings;
    QString notesPath = settings.value( "notesPath" ).toString();

    // parse for relative file urls and make them absolute
    urlString.replace( QRegularExpression( "^file:\\/\\/([^\\/].+)$" ), "file://"+ notesPath +"/\\1" );

    QDesktopServices::openUrl( QUrl( urlString ) );
}

/**
 * @brief Returns the highlighter instance
 * @return
 */
HGMarkdownHighlighter *QMarkdownTextEdit::highlighter()
{
    return _highlighter;
}

/**
 * @brief Sets url schemata that will be ignored when clicked on
 * @param urlSchemes
 */
void QMarkdownTextEdit::setIgnoredClickUrlSchemata( QStringList ignoredUrlSchemata )
{
    _ignoredClickUrlSchemata = ignoredUrlSchemata;
}

/**
 * @brief Returns a map of parsed markdown urls with their link texts as key
 *
 * @param text
 * @return parsed urls
 */
QMap<QString, QString> QMarkdownTextEdit::parseMarkdownUrlsFromText( QString text )
{
    QMap<QString, QString> urlMap;

    // match urls like this: [this url](http://mylink)
    QRegularExpression re("(\\[.*?\\]\\((.+?://.+?)\\))");
    QRegularExpressionMatchIterator i = re.globalMatch( text );
    while ( i.hasNext() )
    {
        QRegularExpressionMatch match = i.next();
        QString linkText = match.captured(1);
        QString url = match.captured(2);
        urlMap[linkText] = url;
    }

    // match urls like this: <http://mylink>
    re = QRegularExpression("(<(.+?://.+?)>)");
    i = re.globalMatch( text );
    while ( i.hasNext() )
    {
        QRegularExpressionMatch match = i.next();
        QString linkText = match.captured(1);
        QString url = match.captured(2);
        urlMap[linkText] = url;
    }

    return urlMap;
}

/**
 * @brief Returns the markdown url at position
 * @param text
 * @param position
 * @return url string
 */
QUrl QMarkdownTextEdit::getMarkdownUrlAtPosition( QString text, int position )
{
    QUrl url;

    // get a map of parsed markdown urls with their link texts as key
    QMap<QString, QString> urlMap = parseMarkdownUrlsFromText( text );

    QMapIterator<QString, QString> i( urlMap );
    while ( i.hasNext() )
    {
        i.next();
        QString linkText = i.key();
        QString urlString = i.value();

        int foundPositionStart = text.indexOf( linkText );

        if ( foundPositionStart >= 0 )
        {
            // calculate end position of found linkText
            int foundPositionEnd = foundPositionStart + linkText.size();

            // check if position is in found string range
            if ( ( position >= foundPositionStart ) && ( position <= foundPositionEnd ) )
            {
                url = QUrl( urlString );
            }
        }
    }

    return url;
}

//void QMarkdownTextEdit::createSearchWidget()
//{
//    QWidget widget;

//}
