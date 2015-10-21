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

#include "SearchEngine.h"
#include "LBPImage.h"
#include "Util.h"
#include "Constants.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QDebug>
#include <QtGlobal>
#include <QMutexLocker>
#include <limits>

using namespace cv;
using namespace FaceReco;

SearchEngine::SearchEngine(QObject *parent) :
    QObject(parent),
    shouldContinueSearching(false),
    threshold(HISTOGRAM_DISTANCE_THRESHOLD),
    db(0)
{
    connect(this, SIGNAL(triggerStart(bool,quint32,quint32)), this, SLOT(handleStart(bool,quint32,quint32)), Qt::QueuedConnection);
    connect(this, SIGNAL(triggerStop(bool)), this, SLOT(handleStop(bool)), Qt::QueuedConnection);
    connect(this, SIGNAL(triggerPartialSearch(bool,quint32,quint32)), this, SLOT(handlePartialSearch(bool,quint32,quint32)), Qt::QueuedConnection);
}

void SearchEngine::pushHistogram(const Mat &histogram)
{
    QMutexLocker locker(&dataMutex);

    histograms.push_front(histogram);
}

const Mat SearchEngine::popHistogram()
{
    QMutexLocker locker(&dataMutex);

    Mat histogram;
    if (histograms.isEmpty())
    {
        histogram = Mat();
    }
    else
    {
        histogram = histograms.takeLast();
    }

    return histogram;
}

void SearchEngine::start_HC(const quint32 histogramCount)
{
    emit triggerStart(false, histogramCount, 0);
}

void SearchEngine::start_TC(const quint32 minSearchTimeMs, const quint32 maxSearchTimeMs)
{
    emit triggerStart(true, minSearchTimeMs, maxSearchTimeMs);
}

void SearchEngine::stop(const bool analyzeResultsSoFar)
{
    emit triggerStop(analyzeResultsSoFar);
}

void SearchEngine::handleStart(const bool isTimeConstrained, const quint32 parameter0, const quint32 parameter1)
{
    Q_ASSERT(parameter0 > 0);
    Q_ASSERT(db);

    if (db->isEmpty())
    {
        shouldContinueSearching = false;

        emit personNotFound(0, 0, 0);
    }
    else
    {
        shouldContinueSearching = true;
        resultFound = false;
        histogramsCompared = 0;
        histogramToCompare = Mat();
        dbIterator.reset(new Database::Iterator(*db));
        results.clear();
        timer.restart();

        emit triggerPartialSearch(isTimeConstrained, parameter0, parameter1);
    }
}

void SearchEngine::handleStop(const bool analyzeResultsSoFar)
{
    histograms.clear();

    if (!shouldContinueSearching)
    {
        // Search has been already stopped.
        return;
    }

    if (analyzeResultsSoFar)
    {
        analyzeResults();
    }

    shouldContinueSearching = false;
}

void SearchEngine::handlePartialSearch(const bool isTimeConstrained, const quint32 parameter0, const quint32 parameter1)
{
    if (!shouldContinueSearching)
    {
        return;
    }

    Q_ASSERT(db);

    if (isTimeConstrained)
    {
        if ((timer.elapsed() > parameter1) ||
            (timer.elapsed() > parameter0 && resultFound))
        {
            handleStop(true);
            return;
        }
    }

    if (histogramToCompare.empty())
    {
        histogramToCompare = popHistogram();
        if (histogramToCompare.empty())
        {
            // Histogram queue is empty. Go back to event loop and try again.
            emit triggerPartialSearch(isTimeConstrained, parameter0, parameter1);
            return;
        }

        dbIterator->reset();
    }

    const Database::Indices indices = dbIterator->indices();
    const Mat &databaseHistogram = db->getHistogram(indices.personId,
                                                    indices.trackId,
                                                    indices.histogramId);

    const float distance = LBPImage::distance(databaseHistogram, histogramToCompare);
    histogramsCompared++;

    ++(*dbIterator.data());

    bool resultAppended = false;

    if (distance < threshold)
    {
        results.append(qMakePair(distance, indices.personId));
        resultAppended = true;
        resultFound = true;
    }
    else if (dbIterator->isAtBeginning())
    {
        // Whole database iterated through. No person found for current
        // histogram.
        results.append(qMakePair(std::numeric_limits<float>::max(),
                                 std::numeric_limits<quint32>::max()));
        resultAppended = true;
    }

    if (resultAppended)
    {
        if (!isTimeConstrained && results.size() == parameter0)
        {
            handleStop(true);
            return;
        }

        histogramToCompare = Mat();
    }

    emit triggerPartialSearch(isTimeConstrained, parameter0, parameter1);
}

void SearchEngine::analyzeResults()
{
    const quint32 searchTime = timer.elapsed();
    const quint32 histogramsSearched = results.size();

    float minDistance = std::numeric_limits<float>::max();
    quint32 personId = std::numeric_limits<quint32>::max();

    for (int i = 0; i < results.size(); i++)
    {
        if (results.at(i).first < minDistance)
        {
            minDistance = results.at(i).first;
            personId = results.at(i).second;
        }

        //qDebug() << "Result:" << i << " dist:" << minDistance << " personId:" << personId;
    }

    if (personId == std::numeric_limits<quint32>::max())
    {
        emit personNotFound(searchTime, histogramsSearched, histogramsCompared);
    }
    else
    {
        emit personFound(personId, searchTime, histogramsSearched, histogramsCompared);
    }

    histograms.clear();
}
