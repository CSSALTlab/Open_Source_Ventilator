#ifndef MAINWINDOW_H
#define MAINWINDOW_H

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

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void update();

private:
    Ui::MainWindow *ui;
    int timerId;

public slots:
    void onBtFuncPressed();
    void onBtFuncRelease();
    void onBtRightPressed();
    void onBtRightRelease();
    void onBtLeftPressed();
    void onBtLeftRelease();
    void onBtEnterPressed();
    void onBtEnterRelease();

    void onPressureSliderChange(int v);

protected:
    void timerEvent(QTimerEvent *event);
};
#endif // MAINWINDOW_H
