
/*************************************************************
 * Open Ventilator
 * Copyright (C) 2020 - Marcelo Varanda
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **************************************************************
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTimer>
#include <QDebug>
//#include "../ArduinoVent/vent.h"
#include "vent.h"
#include "hal.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->bt_func, SIGNAL (pressed()),this, SLOT (onBtFuncRelease()));
    connect(ui->bt_func, SIGNAL (released()),this, SLOT (onBtFuncRelease()));

    halInit(ui->plainTextEdit);
    init_libs();
    ventSetup();

    timerId = startTimer(1);

}

MainWindow::~MainWindow()
{
    killTimer(timerId);
    delete ui;
}

void MainWindow::onBtFuncPressed()
{
    qDebug() << "onBtFuncPressed";
}

void MainWindow::onBtFuncRelease()
{
    qDebug() << "onBtFuncRelease";
}


void MainWindow::timerEvent(QTimerEvent *event)
{
    ventLoop();
}

