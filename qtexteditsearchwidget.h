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

#ifndef QTEXTEDITSEARCHWIDGET_H
#define QTEXTEDITSEARCHWIDGET_H

#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QWidget>

class QTextEditSearchWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QTextEditSearchWidget(QTextEdit *parent = 0);
    void activate();
    void deactivate();

protected:
    QTextEdit *_textEdit;
    QLabel *_label;
    QLineEdit *_searchLineEdit;
    bool eventFilter(QObject *obj, QEvent *event);
    void doSearch(bool searchDown = true);

signals:

public slots:
};

#endif // QTEXTEDITSEARCHWIDGET_H
