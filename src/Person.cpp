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

#include "Person.h"
#include <QtGlobal>

using namespace cv;

Person::Person(const QString &name) :
    totalHistogramCount(0),
    sizeInBytes(0),
    personName(name)
{
}

const Track& Person::getTrack(quint32 trackId) const
{
    Q_ASSERT(trackId < static_cast<quint32>(tracks.size()));

    return *tracks.at(trackId).data();
}

quint32 Person::addTrack(const QSharedPointer<Track> &track)
{
    const quint32 trackId = tracks.size();
    tracks.append(track);

    totalHistogramCount += track->histogramCount();
    sizeInBytes += track->size();

    return trackId;
}

quint32 Person::addHistogram(quint32 trackId, const Mat &histogram)
{
    Q_ASSERT(trackId < static_cast<quint32>(tracks.size()));

    Track& track = *tracks.at(trackId).data();
    totalHistogramCount++;
    sizeInBytes += histogram.step[0] * histogram.rows;

    return track.addHistogram(histogram);
}

void Person::setFaceImage(const Mat &img)
{
    faceImage = img;

    if (!faceImage.empty())
    {
        // Convert BGR-format (OpenCV) to RGB-format (Qt).
        cvtColor(faceImage, faceImage, CV_BGR2RGB);
    }
}

QDataStream& operator<< (QDataStream &out, const Person &person)
{
    out << person.trackCount() << person.size() << person.histogramCount() <<
           person.getName();

    const char* dataPtr = reinterpret_cast<char*>(person.faceImage.data);
    const int dataSize = static_cast<int>(person.faceImage.step[0] * person.faceImage.rows);
    out << person.faceImage.rows << person.faceImage.cols << person.faceImage.type() <<
           person.faceImage.step[0] << QByteArray::fromRawData(dataPtr, dataSize);

    for (quint32 i = 0; i < person.trackCount(); i++)
    {
        out << person.getTrack(i);
    }

    return out;
}

QDataStream& operator>> (QDataStream &in, Person &person)
{
    person.tracks.clear();

    quint32 trackCount;
    in >> trackCount >> person.sizeInBytes >> person.totalHistogramCount >>
          person.personName;

    int rows;
    int cols;
    int type;
    size_t step;
    QByteArray data;
    in >> rows >> cols >> type >> step >> data;

    Mat(rows, cols, type, data.data(), step).copyTo(person.faceImage);

    for (quint32 i = 0; i < trackCount; i++)
    {
        QSharedPointer<Track> track(new Track);

        in >> *track.data();

        person.tracks.append(track);
    }

    return in;
}

Person& Person::operator+ (const Person &personToAdd)
{
    for (int i = 0; i < personToAdd.tracks.size(); i++)
    {
        this->addTrack(personToAdd.tracks.at(i));
    }

    return *this;
}

bool Person::operator!= (const Person &otherPerson) const
{
    if (this->sizeInBytes != otherPerson.sizeInBytes ||
        this->totalHistogramCount != otherPerson.totalHistogramCount ||
        this->personName != otherPerson.personName ||
        this->tracks.size() != otherPerson.tracks.size())
    {
        return true;
    }

    const Mat &mat1 = this->getFaceImage();
    const Mat &mat2 = otherPerson.getFaceImage();

    if (QByteArray::fromRawData(reinterpret_cast<char*>(mat1.data), static_cast<int>(mat1.step[0] * mat1.rows)) !=
        QByteArray::fromRawData(reinterpret_cast<char*>(mat2.data), static_cast<int>(mat2.step[0] * mat2.rows)))
    {
        return true;
    }

    for (int i = 0; i < this->tracks.size(); i++)
    {
        if (this->getTrack(i) != otherPerson.getTrack(i))
        {
            return true;
        }
    }

    return false;
}
