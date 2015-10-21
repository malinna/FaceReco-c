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

#include "FrameProcesser.h"
#include "ChehraHeadTracker.h"
#include "LBPImage.h"
#include "Util.h"
#include "Constants.h"
#include <QElapsedTimer>
#include <QDebug>
#include <QMutexLocker>
#include <QtGlobal>
#include <limits>

#ifdef Q_OS_WIN
#define NOMINMAX // Suppress the min and max definitions in Windef.h.
#include <windows.h> // For Sleep.
#else
#include <time.h>
#endif

using namespace FaceReco;
using namespace cv;

FrameProcesser::FrameProcesser(QObject *parent) :
    QObject(parent),
    lastTrackMonitorFrameData(TRACK_WINDOW_HEIGHT, TRACK_WINDOW_WIDTH, CV_8UC3),
    trackWindowImg(TRACK_WINDOW_HEIGHT, TRACK_WINDOW_WIDTH, CV_8UC3),
    maintainVideoFPS(MAINTAIN_VIDEO_FPS)
{
    connect(this, SIGNAL(triggerFrameProcess()), this, SLOT(processFrame()), Qt::QueuedConnection);
    connect(this, SIGNAL(triggerStart(QString)), this, SLOT(handleStart(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(triggerStop()), this, SLOT(handleStop()), Qt::QueuedConnection);
    connect(this, SIGNAL(triggerTogglePause()), this, SLOT(handleTogglePause()), Qt::QueuedConnection);
    connect(this, SIGNAL(triggerSetMode(int)), this, SLOT(handleSetMode(int)), Qt::QueuedConnection);
    connect(&histogramWriter, SIGNAL(personAdded(quint32)), this, SLOT(personAdded(quint32)));
    connect(&histogramWriter, SIGNAL(trackAdded(quint32)), this, SLOT(trackAdded(quint32)));
    connect(&histogramWriter, SIGNAL(histogramAdded(quint32)), this, SLOT(histogramAdded(quint32)));

    setMode(MODE_LEARN_AND_RECOGNIZE);

    // Setup worker object and thread for search engine.
    searchEngine.moveToThread(&searchEngineThread);
    connect(&searchEngine, SIGNAL(personFound(quint32,quint32,quint32,quint32)), this, SLOT(handlePersonFound(quint32,quint32,quint32,quint32)));
    connect(&searchEngine, SIGNAL(personNotFound(quint32,quint32,quint32)), this, SLOT(handlePersonNotFound(quint32,quint32,quint32)));
    searchEngineThread.start();

    // Setup worker object and thread for histogram writer.
    histogramWriter.moveToThread(&histogramWriterThread);
    histogramWriterThread.start();

    tracker.reset(new ChehraHeadTracker);
}

FrameProcesser::~FrameProcesser()
{
    quitWorkerThreads();
}

void FrameProcesser::initialize()
{
    if (!tracker->init())
    {
        exit(1);
    }
}

void FrameProcesser::setDatabase(Database *db)
{
    this->db = db;
    searchEngine.setDatabase(db);
    histogramWriter.setDatabase(db);
}

QSize FrameProcesser::frameSize(const QString &sourceFilename)
{
    QSize size;
    if (sourceFilename.isEmpty())
    {
        size.setWidth(DEFAULT_CAM_WIDTH);
        size.setHeight(DEFAULT_CAM_HEIGHT);
    }
    else
    {
        CaptureSource c(sourceFilename);
        size.setWidth(c.width());
        size.setHeight(c.height());
    }

    return size;
}

const QImage FrameProcesser::lastCaptureFrame()
{
    QMutexLocker locker(&mutex);

    return QImage(lastCaptureFrameData.data,
                  lastCaptureFrameData.cols,
                  lastCaptureFrameData.rows,
                  static_cast<int>(lastCaptureFrameData.step),
                  QImage::Format_RGB888);
}

const QImage FrameProcesser::lastTrackMonitorFrame()
{
    QMutexLocker locker(&mutex);

    return QImage(lastTrackMonitorFrameData.data,
                  lastTrackMonitorFrameData.cols,
                  lastTrackMonitorFrameData.rows,
                  static_cast<int>(lastTrackMonitorFrameData.step),
                  QImage::Format_RGB888);
}

void FrameProcesser::setLastCaptureFrame(const Mat &data)
{
    QMutexLocker locker(&mutex);

    // Create a deep copy of the data.
    data.copyTo(lastCaptureFrameData);

    // Convert BGR-format (OpenCV) to RGB-format (Qt).
    cvtColor(lastCaptureFrameData, lastCaptureFrameData, CV_BGR2RGB);
}

void FrameProcesser::setLastTrackMonitorFrame(const Mat &data)
{
    QMutexLocker locker(&mutex);

    // Create a deep copy of the data.
    data.copyTo(lastTrackMonitorFrameData);

    // Convert BGR-format (OpenCV) to RGB-format (Qt).
    cvtColor(lastTrackMonitorFrameData, lastTrackMonitorFrameData, CV_BGR2RGB);
}

void FrameProcesser::start(const QString &sourceFilename)
{
    emit triggerStart(sourceFilename);
}

void FrameProcesser::stop()
{
    emit triggerStop();
}

void FrameProcesser::quitWorkerThreads()
{
    if (searchEngineThread.isRunning())
    {
        searchEngineThread.quit();
        searchEngineThread.wait();
    }

    if (histogramWriterThread.isRunning())
    {
        histogramWriterThread.quit();
        histogramWriterThread.wait();
    }
}

void FrameProcesser::togglePause()
{
    emit triggerTogglePause();
}

void FrameProcesser::setMode(const int mode)
{
    emit triggerSetMode(mode);
}

void FrameProcesser::handleStart(const QString &sourceFilename)
{
    // Create capture source object.
    cap.reset();
    if (sourceFilename.isEmpty())
    {
        cap.reset(new CaptureSource(DEFAULT_CAM_WIDTH, DEFAULT_CAM_HEIGHT));
    }
    else
    {
        cap.reset(new CaptureSource(sourceFilename));
    }

    if (!cap->isOpened())
    {
        qDebug() << "Failed to open capture source:" << (cap->isCameraSourceEnabled() ? qPrintable(QString("CAMERA")) : sourceFilename);
        qDebug() << "Processing not started.";
        return;
    }

    // Initialize.
    lastCaptureFrameData.create(cap->height(), cap->width(), CV_8UC3);
    trackWindowImg = 0;
    shouldContinueWorking = true;
    endReached = false;
    trackIndex = -1;
    printedTrackIndex = -1;
    trackFrameIndex = 0;
    trackWindowIndex = TRACK_WINDOW_GRID_X;
    isSearching = false;
    isWriting = false;
    searchDone = false;
    trackLost = false;
    detectedPersonIsRecognized = false;

    tracker->reset();

    if (cap->isCameraSourceEnabled())
    {
        qDebug() << "Video input: CAMERA";
    }
    else
    {
        qDebug() << "Video input: FILE";
        qDebug() << "Filename:" << sourceFilename;
        qDebug() << "FPS:" << cap->FPS();
    }

    qDebug() << "Resolution:" << qPrintable(QString("%1x%2").arg(cap->width()).arg(cap->height()));

    emit processingStarted();
    emit triggerFrameProcess();

    qDebug() << "Processing started.";
}

void FrameProcesser::handleStop()
{
    searchEngine.stop();
    histogramWriter.stop();    
    shouldContinueWorking = false;

    qDebug() << "Processing stopped.";

    emit processingStopped();
}

void FrameProcesser::handleTogglePause()
{
    if (endReached)
    {
        handleStart(cap->getFilename());
    }
    else
    {
        shouldContinueWorking = !shouldContinueWorking;

        if (shouldContinueWorking)
        {
            emit processingStarted();
            emit triggerFrameProcess();

            qDebug() << "Processing started.";
        }
        else
        {
            emit processingStopped();

            qDebug() << "Processing stopped.";
        }
    }
}

void FrameProcesser::handleSetMode(const int mode)
{
    this->mode = mode;

    QString modeStr;
    switch (mode)
    {
    case MODE_LEARN_AND_RECOGNIZE:
        modeStr = "Recognize and learn";
        break;
    case MODE_RECOGNIZE_ONLY:
        modeStr = "Recognize only";
        break;
    case MODE_TEST:
        modeStr = "Test mode";
        break;
    }

    qDebug() << "Processing mode set to:" << modeStr;
}

void FrameProcesser::processFrame()
{
    if (!shouldContinueWorking)
    {
        return;
    }

    if ((mode == MODE_TEST || SHOW_RESULT_WITH_SHORT_TRACKS) && trackLost && isSearching && !searchDone)
    {
        // Hold frame processing until search engine returns result of the last
        // track.
        int delayMs = 5;
#ifdef Q_OS_WIN
                Sleep(uint(delayMs));
#else
                struct timespec ts = { delayMs / 1000, (delayMs % 1000) * 1000 * 1000 };
                nanosleep(&ts, NULL);
#endif
        emit triggerFrameProcess();
        return;
    }

    Q_ASSERT(cap.data());
    Q_ASSERT(tracker.data());

    QElapsedTimer timer;
    timer.start();

    Mat img = cap->queryFrame();
    if (img.empty())
    {
        searchEngine.stop(true);
        endReached = true;
        handleStop();
        return;
    }

    if (tracker->track(img))
    {
        // Face detected/tracked successfully.

        Mat alignedLandmarks = tracker->getAlignedFacialLandmarks();
        Mat landmarkImg = imCreateImageFromLandmarks(alignedLandmarks, tracker->getAlignedLeftEye(), tracker->getAlignedRightEye(), TRACK_WINDOW_FRAME_SIZE);

        // Get the aligned face image.
        Mat alignedFaceImg = tracker->getAlignedFaceImage();

        // Apply some smoothing to it, make it grayscale and 8-bit format.
        Mat processedFaceImg;
        medianBlur(alignedFaceImg, processedFaceImg, 3);
        cvtColor(processedFaceImg, processedFaceImg, CV_BGR2GRAY);
        processedFaceImg.convertTo(processedFaceImg, CV_8UC1);

        // Calculate LBP image and histogram for the face image.
        LBPImage lbpImg(processedFaceImg);

        imPlotGrid(lbpImg.image(), 7, 7);

        if (trackFrameIndex == 0)
        {
            trackLost = false;
            trackIndex++;
            isSearching = false;
            isWriting = false;
            searchDone = false;
            detectedPersonIsRecognized = false;
            alignedLandmarks.copyTo(lastKeyFrameLandmarks);
            alignedFaceImg.copyTo(lastTrackFaceImg);

            if (mode != MODE_TEST)
            {
                emit newTrackDetected();
            }
        }

        if (!searchDone)
        {
            searchEngine.pushHistogram(lbpImg.histogram());

            if (!isSearching)
            {
                if (mode == MODE_TEST)
                {
                    // This will test every frame of a track.
                    searchEngine.start_HC(std::numeric_limits<quint32>::max());
                }
                else
                {
                    // This will test frames for a specified time.
                    searchEngine.start_TC(MIN_SEARCH_TIME_MS, MAX_SEARCH_TIME_MS);
                }

                isSearching = true;
            }
        }

        // Calculate delta vectors.
        Mat delta = alignedLandmarks - lastKeyFrameLandmarks;
        Mat deltaImg = imCreateImageFromDeltaVector(alignedLandmarks, tracker->getAlignedLeftEye(), tracker->getAlignedRightEye(), delta, TRACK_WINDOW_FRAME_SIZE);
        double maxDelta = maxVectorLength(delta);

        // If this frame is a key frame.
        if (trackFrameIndex == 0 || (maxDelta > LANDMARK_DELTA_MIN_THRESHOLD))
        {
            if (isWriting)
            {
                histogramWriter.pushHistogram(lbpImg.histogram());
            }
            else
            {
                histogramBuffer.append(lbpImg.histogram());
            }

            if (trackIndex == 0 && trackFrameIndex == 0)
            {
                lastKeyFrameHistogram = lbpImg.histogram().clone();
            }

            float hdist = LBPImage::distance(lbpImg.histogram(), lastKeyFrameHistogram);
            imPlotFrame(trackWindowImg, processedFaceImg, trackWindowIndex, trackIndex, trackFrameIndex, hdist, maxDelta);

            alignedLandmarks.copyTo(lastKeyFrameLandmarks);

            lastKeyFrameHistogram = lbpImg.histogram();

            if (++trackWindowIndex / TRACK_WINDOW_GRID_X >= TRACK_WINDOW_GRID_Y)
            {
                trackWindowIndex = TRACK_WINDOW_GRID_X;
            }

            imPlotStatus(trackWindowImg, "Tracking...", trackWindowIndex);
        }

        // Update track monitor image.
        imPlotGrid(processedFaceImg, 7, 7);
        imPlotFrame(trackWindowImg, tracker->getFaceImage(), 0, trackIndex, trackFrameIndex);
        imPlotFrame(trackWindowImg, landmarkImg, 1);
        imPlotFrame(trackWindowImg, deltaImg, 2, -1, -1, -1.0, -1.0, 0);
        imPlotFrame(trackWindowImg, processedFaceImg, 3);
        imPlotFrame(trackWindowImg, lbpImg.image(), 4);

        // Update capture source image.
        imPlotPose(img, tracker->getPitch(), tracker->getYaw(), tracker->getRoll());
        imPlotLandmarks(img, tracker->getFacialLandmarks());
        switch (mode)
        {
        case MODE_LEARN_AND_RECOGNIZE:
            imPlotROI(img, tracker->getFaceROI(), searchDone ? detectedPersonId : -1, detectedPersonIsRecognized ? "NEW" : "");
            break;
        case MODE_RECOGNIZE_ONLY:
            imPlotROI(img, tracker->getFaceROI(), searchDone && !detectedPersonIsRecognized ? detectedPersonId : -1, detectedPersonIsRecognized ? "UNKNOWN" : "");
            break;
        case MODE_TEST:
            imPlotROI(img, tracker->getFaceROI());
            break;
        }

        // Successfully tracking a face. Increase the frame counter.
        trackFrameIndex++;
    }
    else
    {
        // Track is lost.
        trackLost = true;
        trackFrameIndex = 0;

        searchEngine.stop(SHOW_RESULT_WITH_SHORT_TRACKS || mode == MODE_TEST ? true : false);
        histogramWriter.stop();

        imPlotStatus(trackWindowImg, "Detecting...", trackWindowIndex);
    }

    if (maintainVideoFPS)
    {
        // In case of video, we may need some delay so that processing rate matches
        // with the video FPS.
        if (!cap->isCameraSourceEnabled())
        {
            int neededDelayMs = (int)((1000.0 / cap->FPS()) + 0.5) - timer.elapsed();

            if (neededDelayMs > 0)
            {
#ifdef Q_OS_WIN
                Sleep(uint(neededDelayMs));
#else
                struct timespec ts = { neededDelayMs / 1000, (neededDelayMs % 1000) * 1000 * 1000 };
                nanosleep(&ts, NULL);
#endif
            }
        }
    }

    float FPS = 1000.0 / timer.elapsed();
    imPlotFPS(img, (int)(FPS + 0.5));

    setLastCaptureFrame(img);
    setLastTrackMonitorFrame(trackWindowImg);

    emit frameProcessed();
    emit triggerFrameProcess();
}

void FrameProcesser::handlePersonFound(const quint32 personId, const quint32 searchTime, const quint32 histogramsSearched, const quint32 histogramsCompared)
{
    searchDone = true;
    isSearching = false;
    detectedPersonId = personId;
    printedTrackIndex++;

    outputResult(true, detectedPersonId, searchTime, histogramsSearched, histogramsCompared);

    if (mode == MODE_LEARN_AND_RECOGNIZE)
    {
        // Add new track to found person.
        for (int i = 0; i < histogramBuffer.size(); i++)
        {
            histogramWriter.pushHistogram(histogramBuffer.at(i));
        }

        const quint32 trackId = db->trackCount(personId);
        histogramWriter.startContinuousWriting(personId, trackId);
        isWriting = true;
    }

    histogramBuffer.clear();

    emit personChanged(personId, false);
    emit searchStatisticsChanged(searchTime, histogramsSearched, histogramsCompared);
}

void FrameProcesser::handlePersonNotFound(const quint32 searchTime, const quint32 histogramsSearched, const quint32 histogramsCompared)
{
    searchDone = true;
    isSearching = false;
    detectedPersonId = db->personCount();
    detectedPersonIsRecognized = true;
    printedTrackIndex++;

    outputResult(false, detectedPersonId, searchTime, histogramsSearched, histogramsCompared);

    if (mode == MODE_LEARN_AND_RECOGNIZE)
    {
        // Create new person.

        // Copy the face image to queue of the histogram writer.
        Mat faceImage;
        lastTrackFaceImg.copyTo(faceImage);
        histogramWriter.pushFaceImage(faceImage);

        for (int i = 0; i < histogramBuffer.size(); i++)
        {
            histogramWriter.pushHistogram(histogramBuffer.at(i));
        }

        histogramWriter.startContinuousWriting(detectedPersonId, 0);
        isWriting = true;
    }
    else
    {
        emit personNotFound();
    }

    histogramBuffer.clear();

    emit searchStatisticsChanged(searchTime, histogramsSearched, histogramsCompared);
}

void FrameProcesser::personAdded(const quint32 personId)
{
    emit personChanged(personId, true);
}

void FrameProcesser::trackAdded(const quint32 personId)
{
    emit personChanged(personId, false);
}

void FrameProcesser::histogramAdded(const quint32 personId)
{
    emit personUpdated(personId);
}

void FrameProcesser::outputResult(const bool personFound, const quint32 personId, const quint32 searchTime, const quint32 histogramsSearched, const quint32 histogramsCompared)
{
    QString s = QString("%1: label: %2%3, search time: %4 ms, hm: %5, hc: %6")
            .arg(printedTrackIndex)
            .arg(mode != MODE_LEARN_AND_RECOGNIZE && !personFound ? "NOT FOUND" : QString::number(personId))
            .arg(mode == MODE_LEARN_AND_RECOGNIZE && !personFound ? " (NEW)" : "")
            .arg(searchTime)
            .arg(histogramsSearched)
            .arg(histogramsCompared);

    qDebug() << qPrintable(s);
}
