
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
#include "event.h"

unsigned int gAnalogPressure = 300;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->bt_func, SIGNAL (pressed()),this, SLOT (onBtFuncPressed()));
    connect(ui->bt_func, SIGNAL (released()),this, SLOT (onBtFuncRelease()));
    connect(ui->bt_right, SIGNAL (pressed()),this, SLOT (onBtRightPressed()));
    connect(ui->bt_right, SIGNAL (released()),this, SLOT (onBtRightRelease()));
    connect(ui->bt_left, SIGNAL (pressed()),this, SLOT (onBtLeftPressed()));
    connect(ui->bt_left, SIGNAL (released()),this, SLOT (onBtLeftRelease()));

    ui->PressureSlider->setMaximum(613);
    ui->PressureSlider->setValue(gAnalogPressure);
    ui->lb_pressure->setNum((int) gAnalogPressure);
    connect(ui->PressureSlider, SIGNAL (valueChanged(int)),this, SLOT (onPressureSliderChange(int)));

    ui->lb_input_valve_on->hide();
    ui->lb_output_valve_on->hide();

#ifdef Q_OS_WIN32
    QFont font = QFont ("Courier New");
    font.setPointSize (20);
    ui->plainTextEdit->setFont(font);

    QFont f = QFont ("Courier New");
    f.setPointSize (12);
    ui->lb_press->setFont(f);
    ui->lb_exhalation->setFont(f);
    ui->lb_exhalation->setText("Exhalation");
    ui->lb_inspiration->setFont(f);
    ui->lb_inspiration->setText("Inspiration");
    ui->bt_left->setFont(f);
    ui->bt_right->setFont(f);
    ui->bt_func->setFont(f);
#endif

    halInit(    ui->plainTextEdit,
                ui->lb_input_valve_on,
                ui->lb_input_valve_off,
                ui->lb_output_valve_on,
                ui->lb_output_valve_off
                );
    ventSetup();

    timerId = startTimer(1);

}

MainWindow::~MainWindow()
{
    killTimer(timerId);
    delete ui;
}

void MainWindow::onPressureSliderChange(int v)
{
    LOGV("Pressure = %d\n", v);
    gAnalogPressure = v;
    ui->lb_pressure->setNum(v);
}

void MainWindow::onBtFuncPressed()
{
    CEvent::post(EVT_KEY_PRESS, KEY_SET);
}

void MainWindow::onBtFuncRelease()
{
    CEvent::post(EVT_KEY_RELEASE, KEY_SET);
}

void MainWindow::onBtRightPressed()
{
    CEvent::post(EVT_KEY_PRESS, KEY_INCREMENT);
}

void MainWindow::onBtRightRelease()
{
    CEvent::post(EVT_KEY_RELEASE, KEY_INCREMENT);
}

void MainWindow::onBtLeftPressed()
{
    CEvent::post(EVT_KEY_PRESS, KEY_DECREMENT);
}

void MainWindow::onBtLeftRelease()
{
    CEvent::post(EVT_KEY_RELEASE, KEY_DECREMENT);
}

void MainWindow::onBtEnterPressed()
{
    CEvent::post(EVT_KEY_PRESS, KEY_RESERVE);
}

void MainWindow::onBtEnterRelease()
{
    CEvent::post(EVT_KEY_RELEASE, KEY_RESERVE);
}

void MainWindow::timerEvent(QTimerEvent *event)
{
    ventLoop();
}

