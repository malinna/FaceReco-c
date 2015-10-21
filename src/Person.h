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

#ifndef PERSON_H
#define PERSON_H

#include "opencv2/opencv.hpp"
#include "Track.h"
#include <QList>
#include <QSharedPointer>
#include <QString>

class Person
{
public:
    Person(const QString &name = QString());

    const quint32 trackCount() const        { return tracks.size(); }
    const quint32 histogramCount() const    { return totalHistogramCount; }
    const QString& getName() const          { return personName; }

    quint64 size() const { return sizeInBytes; }

    const Track& getTrack(quint32 trackId) const;

    quint32 addTrack(const QSharedPointer<Track> &track);
    quint32 addHistogram(quint32 trackId, const cv::Mat &histogram);

    void setFaceImage(const cv::Mat &img);
    const cv::Mat getFaceImage() const      { return faceImage; }

    friend QDataStream& operator<< (QDataStream &out, const Person &person);
    friend QDataStream& operator>> (QDataStream &in, Person &person);

    Person& operator+ (const Person &personToAdd);
    bool operator!= (const Person &otherPerson) const;

private:
    QList<QSharedPointer<Track> > tracks;

    quint32 totalHistogramCount;    
    quint64 sizeInBytes;

    cv::Mat faceImage;
    QString personName;

};

#endif // PERSON_H
