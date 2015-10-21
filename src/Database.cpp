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

#include "Database.h"
#include <QMutexLocker>
#include <QtGlobal>
#include <QFile>
#include <QDataStream>
#include <QDebug>

using namespace cv;

Database::Database() :
    totalTrackCount(0),
    totalHistogramCount(0),
    sizeInBytes(0)
{
}

bool Database::isEmpty() const
{
    QMutexLocker locker(&mutex);

    return persons.isEmpty();
}

const quint32 Database::personCount() const
{
    QMutexLocker locker(&mutex);

    return persons.size();
}

const quint32 Database::trackCount() const
{
    QMutexLocker locker(&mutex);

    return totalTrackCount;
}

const quint32 Database::trackCount(quint32 personId) const
{
    QMutexLocker locker(&mutex);

    if (personId < static_cast<quint32>(persons.size()))
    {
        const Person& person = *persons.at(personId).data();
        return person.trackCount();
    }

    return 0;
}

const quint32 Database::histogramCount() const
{
    QMutexLocker locker(&mutex);

    return totalHistogramCount;
}

const quint32 Database::histogramCount(quint32 personId) const
{
    QMutexLocker locker(&mutex);

    if (personId < static_cast<quint32>(persons.size()))
    {
        const Person& person = *persons.at(personId).data();
        return person.histogramCount();
    }

    return 0;
}

const quint32 Database::histogramCount(quint32 personId, quint32 trackId) const
{
    QMutexLocker locker(&mutex);

    if (personId < static_cast<quint32>(persons.size()))
    {
        const Person& person = *persons.at(personId).data();
        if (trackId < person.trackCount())
        {
            const Track& track = person.getTrack(trackId);
            return track.histogramCount();
        }
    }

    return 0;
}

const quint64 Database::size() const
{
    QMutexLocker locker(&mutex);

    return sizeInBytes;
}

const quint64 Database::size(quint32 personId) const
{
    QMutexLocker locker(&mutex);

    if (personId < static_cast<quint32>(persons.size()))
    {
        const Person& person = *persons.at(personId).data();
        return person.size();
    }

    return 0;
}

const Mat Database::getHistogram(quint32 personId, quint32 trackId, quint32 histogramId) const
{
    QMutexLocker locker(&mutex);

    Mat histogram;
    if (personId < static_cast<quint32>(persons.size()))
    {
        const Person& person = *persons.at(personId).data();
        if (trackId < person.trackCount())
        {
            const Track& track = person.getTrack(trackId);
            histogram = track.getHistogram(histogramId);
        }
    }

    return histogram;
}

QString Database::getName(quint32 personId) const
{
    QMutexLocker locker(&mutex);

    QString name;
    if (personId < static_cast<quint32>(persons.size()))
    {
        const Person& person = *persons.at(personId).data();
        name = person.getName();
    }

    return name;
}

const QImage Database::getFaceImage(quint32 personId) const
{
    QMutexLocker locker(&mutex);

    Mat faceImage;
    if (personId < static_cast<quint32>(persons.size()))
    {
        const Person& person = *persons.at(personId).data();
        faceImage = person.getFaceImage();
    }

    return QImage(faceImage.data,
                  faceImage.cols,
                  faceImage.rows,
                  static_cast<int>(faceImage.step),
                  QImage::Format_RGB888);
}

const Person* Database::getPerson(quint32 personId) const
{
    QMutexLocker locker(&mutex);

    if (personId < static_cast<quint32>(persons.size()))
    {
        return persons.at(personId).data();
    }

    return 0;
}

quint32 Database::addPerson(QSharedPointer<Person> &person)
{
    QMutexLocker locker(&mutex);

    Q_ASSERT(person->histogramCount() > 0);

    totalTrackCount += person->trackCount();
    totalHistogramCount += person->histogramCount();
    sizeInBytes += person->size();

    const quint32 personId = persons.size();
    persons.append(person);

    // The database now owns and manages the given person object. The caller
    // won't be able to directly access it after resetting the shared pointer.
    person.reset();

    return personId;
}

quint32 Database::addTrack(quint32 personId, QSharedPointer<Track> &track)
{
    QMutexLocker locker(&mutex);

    Q_ASSERT(personId < static_cast<quint32>(persons.size()));
    Person& person = *persons.at(personId).data();

    totalTrackCount++;
    totalHistogramCount += track->histogramCount();
    sizeInBytes += track->size();

    const quint32 trackId = person.addTrack(track);

    // The database now owns and manages the given track object. The caller
    // won't be able to directly access it after resetting the shared pointer.
    track.reset();

    return trackId;
}

quint32 Database::addHistogram(quint32 personId, quint32 trackId, const Mat &histogram)
{
    QMutexLocker locker(&mutex);

    Q_ASSERT(personId < static_cast<quint32>(persons.size()));
    Person& person = *persons[personId].data();

    totalHistogramCount++;
    sizeInBytes += histogram.step[0] * histogram.rows;

    return person.addHistogram(trackId, histogram);
}

bool Database::save(const QString &filename)
{
    QMutexLocker locker(&mutex);

    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        qDebug() << "Failed to open a file for writing:" << filename;

        return false;
    }

    QDataStream fileStream(&file);
    fileStream.setVersion(QDataStream::Qt_5_2);

    const quint32 personCount = persons.size();
    fileStream << totalTrackCount << totalHistogramCount << sizeInBytes << personCount;

    for (quint32 i = 0; i < personCount; i++)
    {
        fileStream << *persons.at(i).data();
    }

    if (fileStream.status() != QDataStream::Ok)
    {
        qDebug() << "Failed to save database to file:" << filename;

        return false;
    }

    qDebug() << "Database saved to file:" << filename;

    return true;
}

bool Database::load(const QString &filename)
{
    QMutexLocker locker(&mutex);

    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to open a file for reading:" << filename;

        return false;
    }

    QDataStream fileStream(&file);
    fileStream.setVersion(QDataStream::Qt_5_2);

    persons.clear();

    quint32 personCount;
    fileStream >> totalTrackCount >> totalHistogramCount >> sizeInBytes >> personCount;

    for (quint32 i = 0; i < personCount; i++)
    {
        QSharedPointer<Person> person(new Person);

        fileStream >> *person.data();

        persons.append(person);
    }

    if (fileStream.status() != QDataStream::Ok)
    {
        qDebug() << "Failed to load database from a file:" << filename;

        return false;
    }

    qDebug() << "Database loaded from file:" << filename;

    return true;
}

void Database::clear()
{
    QMutexLocker locker(&mutex);

    persons.clear();
    totalTrackCount = 0;
    totalHistogramCount = 0;
    sizeInBytes = 0;

    qDebug() << "Database cleared.";
}

int Database::mergePerson(const quint32 personId1, const quint32 personId2)
{
    QMutexLocker locker(&mutex);

    if (personId1 < static_cast<quint32>(persons.size()) &&
        personId2 < static_cast<quint32>(persons.size()) &&
        personId1 != personId2)
    {
        Person& updatedPerson = *persons[personId1].data();
        const Person& personToAdd = *persons.at(personId2).data();

        updatedPerson = updatedPerson + personToAdd;

        persons.removeAt(personId2);

        return personId2 > personId1 ? personId1 : personId1 - 1;
    }

    return -1;
}
