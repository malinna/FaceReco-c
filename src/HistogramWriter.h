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

#ifndef HISTOGRAMWRITER_H
#define HISTOGRAMWRITER_H

#include "Database.h"
#include <QObject>
#include <QList>
#include <QMutex>

class HistogramWriter : public QObject
{
    Q_OBJECT
public:
    explicit HistogramWriter(QObject *parent = 0);

    void setDatabase(Database *db)   { this->db = db; }

    void pushHistogram(const cv::Mat &histogram);
    void pushFaceImage(const cv::Mat &faceImage);

    /**
     * @brief Start continuous writing of histograms.
     *
     * Histograms in queue are written to database until stopped. The queue is
     * polled and new histograms are eventually popped and written.
     * Writing can be stopped by calling stopWriting() slot. When stopped,
     * the queue is emptied and those histograms that were in it won't be
     * written.
     *
     * @param personId  A personId of a person to whom histograms are written.
     *                  If the histogram is a histogram of a new person, then
     *                  person count of the database should be passed.
     * @param trackId   A trackId of a track to which histograms are written.
     *                  If the histogram is a histogram of a new track of known
     *                  person, then track count of the person should be passed.
     */
    void startContinuousWriting(const quint32 personId, const quint32 trackId);

    /**
     * @brief Start queue only writing of histograms.
     *
     * All histograms in queue are written to database and then writing is
     * stopped and writingDone() signal emitted.
     *
     * @param personId  A personId of a person to whom histograms are written.
     *                  If the histogram is a histogram of a new person, then
     *                  person count of the database should be passed.
     * @param trackId   A trackId of a track to which histograms are written.
     *                  If the histogram is a histogram of a new track of known
     *                  person, then track count of the person should be passed.
     */
    void startQueueOnlyWriting(const quint32 personId, const quint32 trackId);

    void stop();

private:
    const cv::Mat popHistogram();
    const cv::Mat popFaceImage();

signals:
    void triggerStart(const quint32 personId, const quint32 trackId, const bool stopWhenQueueIsEmpty);
    void triggerStop();
    void triggerPartialWrite(const quint32 personId, const quint32 trackId, const bool stopWhenQueueIsEmpty);
    void writingDone();
    void personAdded(const quint32 personId);
    void trackAdded(const quint32 personId);
    void histogramAdded(const quint32 personId);

private slots:
    void handleStart(const quint32 personId, const quint32 trackId, const bool stopWhenQueueIsEmpty);
    void handleStop();
    void writeNext(const quint32 personId, const quint32 trackId, const bool stopWhenQueueIsEmpty);

private:
    bool shouldContinueWriting;

    QMutex  dataMutex;

    QList<cv::Mat>  histograms;
    QList<cv::Mat>  faceImages;

    Database *db;

};

#endif // HISTOGRAMWRITER_H
