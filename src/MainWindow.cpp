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

#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Constants.h"
#include "Util.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QDebug>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QStringList>

using namespace FaceReco;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("FaceReco " + VERSION_STRING);

    const QString &sourceFilename = qApp->arguments().size() >= 2 ? qApp->arguments().at(1) : QString();

    // Setup worker object and thread for frame processing.
    processer.moveToThread(&processerThread);
    processer.initialize();
    processer.setDatabase(&db);
    processer.start(sourceFilename);    
    connect(&processer, SIGNAL(frameProcessed()), this, SLOT(updateWindows()));
    connect(&processer, SIGNAL(personChanged(quint32,bool)), this, SLOT(updatePersonStatus(quint32,bool)));
    connect(&processer, SIGNAL(personChanged(quint32,bool)), this, SLOT(updatePersonDatabaseStatus(quint32)));
    connect(&processer, SIGNAL(personChanged(quint32,bool)), this, SLOT(updateDatabaseStatus()));
    connect(&processer, SIGNAL(personUpdated(quint32)), this, SLOT(updatePersonDatabaseStatus(quint32)));
    connect(&processer, SIGNAL(personUpdated(quint32)), this, SLOT(updateDatabaseStatus()));
    connect(&processer, SIGNAL(newTrackDetected()), this, SLOT(clearPersonStatus()));
    connect(&processer, SIGNAL(personNotFound()), this, SLOT(clearPersonStatus()));
    connect(&processer, SIGNAL(searchStatisticsChanged(quint32,quint32,quint32)), this, SLOT(updateSearchStatistics(quint32,quint32,quint32)));
    connect(&processer, SIGNAL(processingStarted()), this, SLOT(disableDatabaseGroup()));
    connect(&processer, SIGNAL(processingStarted()), this, SLOT(setPauseButton()));
    connect(&processer, SIGNAL(processingStopped()), this, SLOT(enableDatabaseGroup()));
    connect(&processer, SIGNAL(processingStopped()), this, SLOT(setPlayButton()));
    processerThread.start();

    // Setup windows.

    QVBoxLayout *layout1 = new QVBoxLayout();
    layout1->setContentsMargins(0, 0, 0, 0);
    layout1->addWidget(&labelCaptureSource);

    wCaptureSource.setParent(this);
    wCaptureSource.setWindowFlags(Qt::Window);
    wCaptureSource.resize(FrameProcesser::frameSize(sourceFilename));
    wCaptureSource.setWindowTitle("FaceReco - CAPTURE SOURCE");
    wCaptureSource.setLayout(layout1);
    wCaptureSource.show();
    wCaptureSource.move(0, 0);

    QVBoxLayout *layout2 = new QVBoxLayout();
    layout2->setContentsMargins(0, 0, 0, 0);
    layout2->addWidget(&labelTrackMonitor);

    const QRect screenGeometry = QApplication::desktop()->availableGeometry();

    wTrackMonitor.setParent(this);
    wTrackMonitor.setWindowFlags(Qt::Window);
    wTrackMonitor.resize(TRACK_WINDOW_WIDTH, TRACK_WINDOW_HEIGHT);
    wTrackMonitor.setWindowTitle("FaceReco - TRACK MONITOR");
    wTrackMonitor.setLayout(layout2);
    wTrackMonitor.show();
    wTrackMonitor.move(screenGeometry.width() - wTrackMonitor.frameGeometry().width(), screenGeometry.height() - wTrackMonitor.frameGeometry().height());

    move(0, wCaptureSource.frameGeometry().height());

    ui->personNew->setVisible(false);
    ui->databasePersonCount->setText(QString::number(db.personCount()));
    ui->databaseTrackCount->setText(QString::number(db.trackCount()));
    ui->databaseHistogramCount->setText(QString::number(db.histogramCount()));
    updateSize(db.size(), ui->databaseSize);

    if (sourceFilename.isEmpty())
    {
        ui->sourceCameraButton->setChecked(true);
    }
    else
    {
        ui->sourceFileButton->setChecked(true);
        ui->sourceFile->setText(sourceFilename);
    }

    ui->comboBox->setItemData(0, "Learns new faces and recognizes already learned faces on the fly while processing videos.", Qt::ToolTipRole);
    ui->comboBox->setItemData(1, "Recognizes already learned faces on the fly while processing videos.", Qt::ToolTipRole);
    ui->comboBox->setItemData(2, "Detected face tracks are not added to the database. Recognition result is shown after all frames of a track are processed (track is lost).", Qt::ToolTipRole);
}

MainWindow::~MainWindow()
{
    processer.quitWorkerThreads();
    processerThread.quit();
    processerThread.wait();
    delete ui;
}


void MainWindow::updateWindows()
{
    labelCaptureSource.setPixmap(QPixmap::fromImage(processer.lastCaptureFrame()));
    labelTrackMonitor.setPixmap(QPixmap::fromImage(processer.lastTrackMonitorFrame()));
}

void MainWindow::updatePersonStatus(const quint32 personId, const bool newPersonAdded)
{
    ui->personImage->setPixmap(QPixmap::fromImage(db.getFaceImage(personId)));
    ui->personId->setText(QString::number(personId));
    ui->personNew->setVisible(newPersonAdded);

    if (newPersonAdded)
    {
        ui->comboBoxPersonId1->addItem(QString::number(personId));
        ui->comboBoxPersonId2->addItem(QString::number(personId));
    }
}

void MainWindow::updatePersonDatabaseStatus(const quint32 personId)
{
    ui->personTrackCount->setText(QString::number(db.trackCount(personId)));
    ui->personHistogramCount->setText(QString::number(db.histogramCount(personId)));
    updateSize(db.size(personId), ui->personSize);
}

void MainWindow::clearPersonStatus()
{
    ui->personImage->clear();
    ui->personId->clear();
    ui->personNew->setVisible(false);
    ui->personTrackCount->clear();
    ui->personHistogramCount->clear();
    ui->personSize->clear();
}

void MainWindow::updateDatabaseStatus()
{
    const quint32 personCount = db.personCount();
    const quint32 trackCount = db.trackCount();
    const quint32 histogramCount = db.histogramCount();
    ui->databasePersonCount->setText(QString::number(personCount));
    ui->databaseTrackCount->setText(QString::number(trackCount));
    ui->databaseHistogramCount->setText(QString::number(histogramCount));
    ui->databaseAvgHistogramsPerPerson->setText(QString::number(static_cast<float>(histogramCount) / personCount, 'f', 0));
    ui->databaseAvgHistogramsPerTrack->setText(QString::number(static_cast<float>(histogramCount) / trackCount, 'f', 0));
    updateSize(db.size(), ui->databaseSize);
}

void MainWindow::updateSearchStatistics(const quint32 searchTime, const quint32 histogramsSearched, const quint32 histogramsCompared)
{
    ui->searchTime->setText(QString::number(searchTime) + " ms");
    ui->searchHistogramsSearched->setText(QString::number(histogramsSearched));
    ui->searchHistogramsCompared->setText(QString::number(histogramsCompared));
}

void MainWindow::disableDatabaseGroup()
{
    ui->groupBoxDatabase->setEnabled(false);
}

void MainWindow::enableDatabaseGroup()
{
    ui->groupBoxDatabase->setEnabled(true);
}

void MainWindow::setPlayButton()
{
    ui->processingButton->setText("Play");
}

void MainWindow::setPauseButton()
{
    ui->processingButton->setText("Pause");
}

void MainWindow::updateSize(const quint64 sizeInBytes, QLabel *sizeLabel)
{
    Q_ASSERT(sizeLabel);

    if (sizeInBytes < 1000000)
    {
        const double sizeInKilobytes = static_cast<double>(sizeInBytes) / 1000;
        sizeLabel->setText(QString::number(sizeInKilobytes, 'f', 1) + " kB");
    }
    else if (sizeInBytes < 1000000000)
    {
        const double sizeInMegabytes = static_cast<double>(sizeInBytes) / 1000000;
        sizeLabel->setText(QString::number(sizeInMegabytes, 'f', 1) + " MB");
    }
    else
    {
        const double sizeInGigabytes = static_cast<double>(sizeInBytes) / 1000000000;
        sizeLabel->setText(QString::number(sizeInGigabytes, 'f', 1) + " GB");
    }
}

void MainWindow::on_processingButton_clicked()
{
    processer.togglePause();
}

void MainWindow::on_selectFileButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open file",
                                                    QCoreApplication::applicationDirPath(),
                                                    "Video files (*.avi *.mp4 *.mpg *.mkv);;Image files (*.png *.jpg *.jpeg *.bmp)");

    if (!fileName.isEmpty())
    {
        ui->sourceFile->setText(fileName);

        if (ui->sourceFileButton->isChecked())
        {
            processer.stop();
            wCaptureSource.resize(FrameProcesser::frameSize(ui->sourceFile->text()));
            processer.start(ui->sourceFile->text());
        }
    }
}

void MainWindow::on_sourceCameraButton_clicked()
{
    processer.stop();
    wCaptureSource.resize(FrameProcesser::frameSize());
    processer.start();
}

void MainWindow::on_sourceFileButton_clicked()
{
    if (!ui->sourceFile->text().isEmpty())
    {
        processer.stop();
        wCaptureSource.resize(FrameProcesser::frameSize(ui->sourceFile->text()));
        processer.start(ui->sourceFile->text());
    }
}

void MainWindow::on_saveButton_clicked()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save FaceReco database file",
                                                    QCoreApplication::applicationDirPath(),
                                                    "FaceReco database files (*.fdb)");

    if (!fileName.isEmpty())
    {
        db.save(fileName);
    }
}

void MainWindow::on_loadButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, "Open FaceReco database file",
                                                    QCoreApplication::applicationDirPath(),
                                                    "FaceReco database files (*.fdb)");

    if (!fileName.isEmpty())
    {
        db.load(fileName);

        clearPersonStatus();
        updateDatabaseStatus();
        ui->comboBoxPersonId1->clear();
        ui->comboBoxPersonId2->clear();

        for (quint32 i = 0; i < db.personCount(); i++)
        {
            ui->comboBoxPersonId1->addItem(QString::number(i));
            ui->comboBoxPersonId2->addItem(QString::number(i));
        }

    }
}

void MainWindow::on_emptyButton_clicked()
{
    db.clear();
    clearPersonStatus();
    updateDatabaseStatus();
    ui->comboBoxPersonId1->clear();
    ui->comboBoxPersonId2->clear();
}


void MainWindow::on_mergeButton_clicked()
{
    const int selection1 = ui->comboBoxPersonId1->currentText().toInt();
    const int selection2 = ui->comboBoxPersonId2->currentText().toInt();

    if (selection1 != -1 && selection2 != -1)
    {
        int newPersonId = db.mergePerson(selection1, selection2);

        if (newPersonId != -1)
        {
            updateDatabaseStatus();

            const int currentPersonId = ui->personId->text().toInt();
            const int lastPersonId = ui->comboBoxPersonId1->count() - 1;

            ui->comboBoxPersonId1->removeItem(lastPersonId);
            ui->comboBoxPersonId2->removeItem(lastPersonId);

            if (currentPersonId == selection1)
            {
                updatePersonStatus(newPersonId, false);
                updatePersonDatabaseStatus(newPersonId);
            }
            else if (currentPersonId == selection2)
            {
                clearPersonStatus();
            }
        }
    }
}

void MainWindow::on_comboBox_activated(int index)
{
    processer.setMode(index);
}
