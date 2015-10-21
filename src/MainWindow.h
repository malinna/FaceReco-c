/*
 * Copyright (c) 2015, Marko Linna
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "FrameProcesser.h"
#include "Database.h"
#include <QMainWindow>
#include <QLabel>
#include <QThread>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

private:
    void updateSize(const quint64 sizeInBytes, QLabel *sizeLabel);

private slots:
    void updateWindows();
    void updatePersonStatus(const quint32 personId, const bool newPersonAdded);
    void updatePersonDatabaseStatus(const quint32 personId);
    void clearPersonStatus();
    void updateDatabaseStatus();
    void updateSearchStatistics(const quint32 searchTime, const quint32 histogramsSearched, const quint32 histogramsCompared);
    void disableDatabaseGroup();
    void enableDatabaseGroup();
    void setPlayButton();
    void setPauseButton();

    void on_processingButton_clicked();
    void on_selectFileButton_clicked();
    void on_sourceCameraButton_clicked();
    void on_sourceFileButton_clicked();
    void on_saveButton_clicked();
    void on_loadButton_clicked();
    void on_emptyButton_clicked();
    void on_mergeButton_clicked();

    void on_comboBox_activated(int index);

private:
    Ui::MainWindow *ui;

    QWidget wCaptureSource;
    QWidget wTrackMonitor;

    QLabel labelCaptureSource;
    QLabel labelTrackMonitor;

    // Worker object and thread for frame processer.
    FrameProcesser processer;
    QThread processerThread;

    // The face database.
    Database db;

};

#endif // MAINWINDOW_H
