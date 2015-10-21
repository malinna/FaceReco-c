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

#ifndef DATABASE_H
#define DATABASE_H

#include "Person.h"
#include <QList>
#include <QSharedPointer>
#include <QMutex>
#include <QPair>
#include <QImage>
#include <limits>

class Database
{
public:
    Database();

    bool isEmpty() const;

    const quint32 personCount() const;
    const quint32 trackCount() const;
    const quint32 trackCount(quint32 personId) const;
    const quint32 histogramCount() const;
    const quint32 histogramCount(quint32 personId) const;
    const quint32 histogramCount(quint32 personId, quint32 trackId) const;
    const quint64 size() const;
    const quint64 size(quint32 personId) const;

    const cv::Mat getHistogram(quint32 personId, quint32 trackId, quint32 histogramId) const;
    QString getName(quint32 personId) const;
    const QImage getFaceImage(quint32 personId) const;
    const Person* getPerson(quint32 personId) const;

    quint32 addPerson(QSharedPointer<Person> &person);
    quint32 addTrack(quint32 personId, QSharedPointer<Track> &track);
    quint32 addHistogram(quint32 personId, quint32 trackId, const cv::Mat &histogram);

    bool save(const QString &filename);
    bool load(const QString &filename);
    void clear();

    int mergePerson(const quint32 personId1, const quint32 personId2);

public:
    struct Indices
    {
        Indices() : personId(0), trackId(0), histogramId(0) {}
        Indices(quint32 i1, quint32 i2, quint32 i3) : personId(i1), trackId(i2), histogramId(i3) {}

        bool operator==(const Indices& rhs) const
        {
            return this->personId == rhs.personId &&
                   this->trackId == rhs.trackId &&
                   this->histogramId == rhs.histogramId;
        }

        bool operator!=(const Indices& rhs) const
        {
            return this->personId != rhs.personId ||
                   this->trackId != rhs.trackId ||
                   this->histogramId != rhs.histogramId;
        }

        quint32 personId;
        quint32 trackId;
        quint32 histogramId;
    };

    /**
     * @brief Iterator to be used in searching.
     *
     * Note: Database must have at least one person and every person must have at
     * least one track with at least one histogram in order to this iterator to
     * work.
     */
    class Iterator
    {
    public:
        Iterator(const Database &db) : db(db)
        {
            reset();
        }

        void reset()
        {
            personId = 0;
            memory.clear();

            const quint32 personCount = db.personCount();
            for (quint32 i = 0; i < personCount; i++)
            {
                QList<quint32> trackList;
                for (quint32 j = 0; j < db.trackCount(i); j++)
                {
                    trackList.append(0);
                }

                memory.append(qMakePair(0, trackList));
            }
        }

        Indices indices() const
        {
            quint32 trackId = memory.at(personId).first;
            quint32 histogramId = memory.at(personId).second.at(trackId);

            return Indices(personId, trackId, histogramId);
        }

        bool isAtBeginning() const
        {
            const Indices currentIndices = indices();
            return currentIndices.personId == 0 &&
                   currentIndices.trackId == 0 &&
                   currentIndices.histogramId == 0 ? true : false;
        }

        Iterator& operator++() // Prefix increment.
        {
            quint32 &trackId = memory[personId].first;
            const quint32 currentPersonId = personId;
            const quint32 currentTrackId = trackId;
            const QList<quint32> &trackList = memory.at(personId).second;
            quint32 &nextHistogramId = memory[personId].second[currentTrackId];

            nextHistogramId++;

            if (db.trackCount(personId) > 1)
            {
                while (true)
                {
                    trackId++;
                    if (trackId == db.trackCount(personId))
                    {
                        trackId = 0;
                    }
                    if (trackList.at(trackId) == db.histogramCount(personId, trackId))
                    {
                        if (trackId == currentTrackId)
                        {
                            trackId = std::numeric_limits<quint32>::max();
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else
            {
                if (trackList.at(trackId) == db.histogramCount(personId, trackId))
                {
                    trackId = std::numeric_limits<quint32>::max();
                }
            }

            if (db.personCount() > 1)
            {
                while (true)
                {
                    personId++;
                    if (personId == db.personCount())
                    {
                        personId = 0;
                    }
                    if (memory.at(personId).first == std::numeric_limits<quint32>::max())
                    {
                        if (personId == currentPersonId)
                        {
                            reset();
                            break;
                        }
                        else
                        {
                            continue;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
            }
            else
            {
                 if (memory.at(personId).first == std::numeric_limits<quint32>::max())
                 {
                     reset();
                 }
            }

            return *this;
        }

    private:
        const Database &db;
        quint32 personId;
        QList<QPair<quint32, QList<quint32> > > memory;

    };

private:
    QList<QSharedPointer<Person> > persons;

    mutable QMutex mutex;

    quint32 totalTrackCount;
    quint32 totalHistogramCount;
    quint64 sizeInBytes;

};

#endif // DATABASE_H
