/*
 * Copyright (c) 2014-2019 Patrizio Bekerle -- http://www.bekerle.com
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

#include "mainwindow.h"
#include <QApplication>
#include <QFileInfo>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString filename;
    if (argc > 1) {
        filename = argv[1];
    }
    if (!filename.isEmpty() && !QFileInfo(filename).isReadable()) {
        qWarning() << filename << "is not a readable file";
        return 1;
    }

    MainWindow w;
    w.show();

    if (!filename.isEmpty()) {
        w.loadFile(filename);
    }


    return a.exec();
}
