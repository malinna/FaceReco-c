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

#include "HistogramWriter.h"
#include <QSharedPointer>

using namespace cv;

HistogramWriter::HistogramWriter(QObject *parent) :
    QObject(parent),
    shouldContinueWriting(false),
    db(0)
{
    connect(this, SIGNAL(triggerStart(quint32,quint32,bool)), this, SLOT(handleStart(quint32,quint32,bool)), Qt::QueuedConnection);
    connect(this, SIGNAL(triggerStop()), this, SLOT(handleStop()), Qt::QueuedConnection);
    connect(this, SIGNAL(triggerPartialWrite(quint32,quint32,bool)), this, SLOT(writeNext(quint32,quint32,bool)), Qt::QueuedConnection);
}

void HistogramWriter::pushHistogram(const Mat &histogram)
{
    QMutexLocker locker(&dataMutex);

    histograms.push_front(histogram);
}

void HistogramWriter::pushFaceImage(const Mat &faceImage)
{
    QMutexLocker locker(&dataMutex);

    faceImages.push_front(faceImage);
}

const Mat HistogramWriter::popHistogram()
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

const Mat HistogramWriter::popFaceImage()
{
    QMutexLocker locker(&dataMutex);

    Mat faceImage;
    if (faceImages.isEmpty())
    {
        faceImage = Mat();
    }
    else
    {
        faceImage = faceImages.takeLast();
    }

    return faceImage;
}

void HistogramWriter::startContinuousWriting(const quint32 personId, const quint32 trackId)
{
    emit triggerStart(personId, trackId, false);
}

void HistogramWriter::startQueueOnlyWriting(const quint32 personId, const quint32 trackId)
{
    emit triggerStart(personId, trackId, true);
}

void HistogramWriter::stop()
{
    emit triggerStop();
}

void HistogramWriter::handleStart(const quint32 personId, const quint32 trackId, const bool stopWhenQueueIsEmpty)
{
    shouldContinueWriting = true;

    emit triggerPartialWrite(personId, trackId, stopWhenQueueIsEmpty);
}

void HistogramWriter::handleStop()
{
    shouldContinueWriting = false;
    histograms.clear();
}

void HistogramWriter::writeNext(const quint32 personId, const quint32 trackId, const bool stopWhenQueueIsEmpty)
{
    if (!shouldContinueWriting)
    {
        return;
    }

    Q_ASSERT(db);
    Q_ASSERT(personId <= db->personCount());

    Mat histogram = popHistogram();

    if (histogram.empty())
    {
        if (stopWhenQueueIsEmpty)
        {
            shouldContinueWriting = false;
            emit writingDone();
            return;
        }
    }
    else
    {
        // Check if this histogram is a histogram of a new person.
        if (personId == db->personCount())
        {
            QSharedPointer<Track> track(new Track);
            track->addHistogram(histogram);

            QSharedPointer<Person> person(new Person("<unknown>"));
            person->addTrack(track);
            person->setFaceImage(popFaceImage());

            const quint32 assignedPersonId = db->addPerson(person);
            emit personAdded(assignedPersonId);
        }

        // Check if this histogram is a histogram of a new track of known person.
        else if (trackId == db->trackCount(personId))
        {
            QSharedPointer<Track> track(new Track);
            track->addHistogram(histogram);
            db->addTrack(personId, track);

            emit trackAdded(personId);
        }

        // This histogram is a histogram of known person and known track.
        else
        {
            db->addHistogram(personId, trackId, histogram);

            emit histogramAdded(personId);
        }
    }

    emit triggerPartialWrite(personId, trackId, stopWhenQueueIsEmpty);
}
