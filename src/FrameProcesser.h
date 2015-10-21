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

#ifndef FRAMEPROCESSER_H
#define FRAMEPROCESSER_H

#include "CaptureSource.h"
#include "HeadTracker.h"
#include "Database.h"
#include "SearchEngine.h"
#include "HistogramWriter.h"
#include <QObject>
#include <QSize>
#include <QScopedPointer>
#include <QImage>
#include <QMutex>
#include <QThread>
#include <QList>

class FrameProcesser : public QObject
{
    Q_OBJECT
public:
    explicit FrameProcesser(QObject *parent = 0);
    virtual ~FrameProcesser();

    void initialize();
    void setDatabase(Database *db);
    void maintainFPS(bool b)  { maintainVideoFPS = b; }

    static QSize frameSize(const QString &sourceFilename=QString());

    const QImage lastCaptureFrame();
    const QImage lastTrackMonitorFrame();

    /**
     * @brief Start processing frames from the given source.
     *
     * NOTE: Do not call this again until processing is stopped with stop()
     * method. If processingReachedEnd() signal emitted, then processing is
     * stopped automatically and there is no need to call stop().
     *
     * @param sourceFilename    A filename of the source video or image
     *                          sequence. If empty, camera is used as a source.
     */
    void start(const QString &sourceFilename=QString());

    void stop();
    void quitWorkerThreads();
    void togglePause();
    void setMode(const int mode);

private:
    void setLastCaptureFrame(const cv::Mat &data);
    void setLastTrackMonitorFrame(const cv::Mat &data);

signals:
    void triggerStart(const QString &sourceFilename);
    void triggerStop();
    void triggerTogglePause();
    void triggerSetMode(const int mode);
    void triggerFrameProcess();

    void newTrackDetected();
    void frameProcessed();
    void processingStarted();
    void processingStopped();
    void personChanged(const quint32 personId, const bool isNewPerson);
    void personUpdated(const quint32 personId);
    void personNotFound();
    void searchStatisticsChanged(const quint32 searchTime, const quint32 histogramsUsed, const quint32 histogramsCompared);

private slots:
    void handleStart(const QString &sourceFilename);
    void handleStop();
    void handleTogglePause();
    void handleSetMode(const int mode);
    void processFrame();

    void handlePersonFound(const quint32 personId, const quint32 searchTime, const quint32 histogramsSearched, const quint32 histogramsCompared);
    void handlePersonNotFound(const quint32 searchTime, const quint32 histogramsSearched, const quint32 histogramsCompared);

    void personAdded(const quint32 personId);
    void trackAdded(const quint32 personId);
    void histogramAdded(const quint32 personId);

private:
    void outputResult(const bool personFound, const quint32 personId, const quint32 searchTime, const quint32 histogramsSearched, const quint32 histogramsCompared);

private:
    bool shouldContinueWorking;

    QMutex  mutex;

    QScopedPointer<CaptureSource> cap;
    QScopedPointer<HeadTracker> tracker;

    // Shared data (protected by mutex).
    cv::Mat lastCaptureFrameData;
    cv::Mat lastTrackMonitorFrameData;

    cv::Mat trackWindowImg;
    cv::Mat lastKeyFrameLandmarks;
    cv::Mat lastKeyFrameHistogram;
    cv::Mat lastTrackFaceImg;

    int trackIndex; // A zero-based index number of a track.
    int printedTrackIndex; // Used in result output.
    unsigned long trackFrameIndex; // A zero-based index number of a frame of the current track.
    int trackWindowIndex;

    QList<cv::Mat> histogramBuffer;

    bool trackLost;
    bool maintainVideoFPS;
    bool endReached;
    bool isSearching;
    bool isWriting;
    bool searchDone;
    int mode; // 0: recognize and learn, 1: recognize only, 2: test mode

    quint32 detectedPersonId;
    bool detectedPersonIsRecognized;

    // Worker object and thread for search engine.
    SearchEngine searchEngine;
    QThread searchEngineThread;

    // Worker object and thread for histogram writer.
    HistogramWriter histogramWriter;
    QThread histogramWriterThread;

    // The face database.
    Database *db;

};

#endif // FRAMEPROCESSER_H
