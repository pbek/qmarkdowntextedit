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
 * mainwindow.cpp
 *
 * Example to show the QMarkdownTextEdit widget
 */

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qmarkdowntextedit.h"
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QToolBar *toolBar = new QToolBar;
    addToolBar(toolBar);
    QAction *openAction = new QAction(QIcon::fromTheme("document-open"), tr("Open..."));
    openAction->setShortcut(QKeySequence::Open);
    connect(openAction, &QAction::triggered, this, &MainWindow::open);

    QAction *saveAction = new QAction(QIcon::fromTheme("document-save"), tr("Save"));
    saveAction->setShortcut(QKeySequence::Save);
    QAction *saveAsAction = new QAction(QIcon::fromTheme("document-save-as"), tr("Save as..."));
    saveAsAction->setShortcut(QKeySequence::SaveAs);
    QAction *quitAction = new QAction(QIcon::fromTheme("view-close"), tr("Quit"));
    quitAction->setShortcut(QKeySequence::Quit);
    connect(quitAction, &QAction::triggered, this, &MainWindow::onQuit);

    m_loadedContent = ui->textEdit->toPlainText();

    toolBar->addActions({openAction, saveAction, saveAsAction, quitAction});
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open" << filename;
        return;
    }

    m_filename = filename;
    m_loadedContent = QString::fromLocal8Bit(file.readAll());
    ui->textEdit->setPlainText(m_loadedContent);
}

void MainWindow::saveToFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Failed to open" << filename;
        return;
    }

    m_filename = filename;

    m_loadedContent = ui->textEdit->toPlainText();
    file.write(m_loadedContent.toLocal8Bit());
}

void MainWindow::open()
{
    QString filename = QFileDialog::getOpenFileName();
    if (filename.isEmpty()) {
        return;
    }
    loadFile(filename);
}

void MainWindow::save()
{
    if (!m_filename.isEmpty()) {
        saveAs();
        return;
    }

    saveToFile(m_filename);
}

void MainWindow::saveAs()
{
    QString filename = QFileDialog::getSaveFileName();
    if (filename.isEmpty()) {
        return;
    }

    saveToFile(filename);
}

void MainWindow::onQuit()
{
    if (ui->textEdit->toPlainText() != m_loadedContent) {
        if (QMessageBox::question(this, tr("Not saved"), tr("Document not saved, sure you want to quit?")) != QMessageBox::Yes) {
            return;
        }
    }
    close();
}
