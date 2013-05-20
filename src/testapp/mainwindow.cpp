/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/
**
** This file is part of QtSystemTest.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QStandardItemModel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_clicks(0)
{
    ui->setupUi(this);
    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(close()));

    connect(ui->lineEdit, SIGNAL(textChanged(QString)), ui->textEditDisplay, SLOT(setText(QString)));
    connect(ui->textEdit, SIGNAL(textChanged()), this, SLOT(updateTextEditDisplay()));

    connect(ui->pushButton, SIGNAL(pressed()), this, SLOT(updatePushButtonDisplay()));
    connect(ui->pushButton, SIGNAL(released()), this, SLOT(updatePushButtonDisplay()));
    connect(ui->pushButton, SIGNAL(clicked()), this, SLOT(updateClickCount()));
    connect(ui->radioButton1, SIGNAL(toggled(bool)), this, SLOT(updateRadioButtonDisplay()));
    connect(ui->radioButton2, SIGNAL(toggled(bool)), this, SLOT(updateRadioButtonDisplay()));
    connect(ui->checkBox, SIGNAL(stateChanged(int)), this, SLOT(updateCheckBoxDisplay()));
    connect(ui->comboBox, SIGNAL(activated(int)), this, SLOT(updateComboBoxDisplay()));
    connect(ui->horizontalSlider, SIGNAL(valueChanged(int)), this, SLOT(updateHorizontalSliderDisplay()));

    QWidget *contents = new QWidget;
    contents->setObjectName("contents");
    contents->setFixedSize(QSize(500, 500));
    QPushButton *topLeft = new QPushButton(contents);
    topLeft->setObjectName("topLeft");
    topLeft->setFixedSize(QSize(20,20));
    topLeft->move(10, 10);
    QPushButton *topRight = new QPushButton(contents);
    topRight->setObjectName("topRight");
    topRight->setFixedSize(QSize(20,20));
    topRight->move(470, 10);
    QPushButton *bottomLeft = new QPushButton(contents);
    bottomLeft->setObjectName("bottomLeft");
    bottomLeft->setFixedSize(QSize(20,20));
    bottomLeft->move(10, 470);
    QPushButton *bottomRight = new QPushButton(contents);
    bottomRight->setObjectName("bottomRight");
    bottomRight->setFixedSize(QSize(20,20));
    bottomRight->move(470, 470);

    ui->scrollArea->setWidget(contents);

    QStandardItemModel *model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels(QStringList() << tr("items"));
    QStandardItem *parentItem;
    for (int j = 0; j < 3; ++j) {
        parentItem = model->invisibleRootItem();
        for (int i = 0; i < 4; ++i) {
            QStandardItem *item = new QStandardItem(QString("item %0").arg(i));
            parentItem->appendRow(item);
            parentItem = item;
        }
    }

    ui->treeView->setModel(model);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateClickCount()
{
    m_clicks++;
    updatePushButtonDisplay();
}

void MainWindow::updateTextEditDisplay()
{
    ui->textEditDisplay->setText(ui->textEdit->document()->toHtml());
}

void MainWindow::updatePushButtonDisplay()
{
    ui->pushButtonDisplay->setText(
                tr("clicks: %1 isPressed: %2")
                .arg(m_clicks)
                .arg(ui->pushButton->isDown()));
}

void MainWindow::updateRadioButtonDisplay()
{
    ui->radioButtonDisplay->setText(
                tr("1: %1 2: %2")
                .arg(ui->radioButton1->isChecked())
                .arg(ui->radioButton2->isChecked()));
}

void MainWindow::updateCheckBoxDisplay()
{
    ui->checkBoxDisplay->setText(
                tr("state: %1")
                .arg(ui->checkBox->checkState()));
}

void MainWindow::updateComboBoxDisplay()
{
    ui->checkBoxDisplay->setText(
                tr("option: %1")
                .arg(ui->comboBox->currentText()));
}

void MainWindow::updateHorizontalSliderDisplay()
{
    ui->horizontalSliderDisplay->setText(
                tr("position: %1")
                .arg(ui->horizontalSlider->value()));
}
