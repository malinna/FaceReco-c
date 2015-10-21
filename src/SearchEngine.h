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

#ifndef SEARCHENGINE_H
#define SEARCHENGINE_H

#include "Database.h"
#include <QObject>
#include <QScopedPointer>
#include <QList>
#include <QMutex>
#include <QPair>
#include <QElapsedTimer>

class SearchEngine : public QObject
{
    Q_OBJECT
public:
    explicit SearchEngine(QObject *parent = 0);

    void setDatabase(Database *db)   { this->db = db; }

    void pushHistogram(const cv::Mat &histogram);

    float distanceThreshold() const { return threshold; }
    void setDistanceThreshold(const float t)    { threshold = t; }

    /**
     * @brief Start histogram-constrained search.
     *
     * NOTE: If search is restarted the ongoing search must be stopped by
     * calling stop() method before this method can be called again.
     *
     * @param histogramCount    Number of histograms used in search.
     */
    void start_HC(const quint32 histogramCount);

    /**
     * @brief Start time-constrained search.
     *
     * NOTE: If search is restarted the ongoing search must be stopped by
     * calling stop() method before this method can be called again.
     *
     * @param minSearchTimeMs  Time in milliseconds how long the search should
     *                         last at minimum. If some result is found in this
     *                         time, the search is stopped and result returned.
     * @param maxSearchTimeMs  Time in milliseconds how long the search should
     *                         last at maximum. If the result is not found in
     *                         this time, "not found" result is returned.
     */
    void start_TC(const quint32 minSearchTimeMs, const quint32 maxSearchTimeMs);

    void stop(const bool analyzeResultsSoFar=false);

private:
    const cv::Mat popHistogram();
    void analyzeResults();

signals:
    void triggerStart(const bool isTimeConstrained, const quint32 parameter0, const quint32 parameter1);
    void triggerStop(const bool analyzeResultsSoFar);
    void triggerPartialSearch(const bool isTimeConstrained, const quint32 parameter0, const quint32 parameter1);
    void personFound(const quint32 personId, const quint32 searchTime, const quint32 histogramsSearched, const quint32 histogramsCompared);
    void personNotFound(const quint32 searchTime, const quint32 histogramsSearched, const quint32 histogramsCompared);

private slots:
    void handleStart(const bool isTimeConstrained, const quint32 parameter0, const quint32 parameter1);
    void handleStop(const bool analyzeResultsSoFar);
    void handlePartialSearch(const bool isTimeConstrained, const quint32 parameter0, const quint32 parameter1);

private:
    bool shouldContinueSearching;
    bool resultFound;

    QMutex  dataMutex;

    QElapsedTimer timer;

    quint32 histogramsCompared;

    QList<cv::Mat>  histograms;
    cv::Mat histogramToCompare;

    float threshold;

    QScopedPointer<Database::Iterator> dbIterator;

    QList<QPair<float, quint32> > results; /**< Contains distance (float) and personId (quint32) */

    Database *db;

};

#endif // SEARCHENGINE_H
