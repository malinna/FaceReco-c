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

#include "Track.h"
#include <QtGlobal>
#include <QByteArray>

using namespace cv;

Track::Track() :
    sizeInBytes(0)
{
}

quint32 Track::addHistogram(const Mat &histogram)
{
    const quint32 histogramId = histograms.size();

    histograms.append(histogram);
    sizeInBytes += histogram.step[0] * histogram.rows;

    return histogramId;
}

const Mat Track::getHistogram(quint32 histogramId) const
{
    Q_ASSERT(histogramId < static_cast<quint32>(histograms.size()));

    Mat histogram;
    if (histogramId < static_cast<quint32>(histograms.size()))
    {
        histogram = histograms.at(histogramId);
    }

    return histogram;
}

QDataStream &operator<< (QDataStream &out, const Track &track)
{
    out << track.sizeInBytes << track.histogramCount();

    for (quint32 i = 0; i < track.histogramCount(); i++)
    {
        const Mat &histogram = track.getHistogram(i);

        const char* dataPtr = reinterpret_cast<char*>(histogram.data);
        const int dataSize = static_cast<int>(histogram.step[0] * histogram.rows);

        out << histogram.rows << histogram.cols << histogram.type() <<
               histogram.step[0] << QByteArray::fromRawData(dataPtr, dataSize);
    }

    return out;
}

QDataStream &operator>> (QDataStream &in, Track &track)
{
    track.clear();

    quint32 histogramCount;

    in >> track.sizeInBytes >> histogramCount;

    for (quint32 i = 0; i < histogramCount; i++)
    {
        int rows;
        int cols;
        int type;
        size_t step;
        QByteArray data;

        in >> rows >> cols >> type >> step >> data;

        Mat histogram;
        Mat(rows, cols, type, data.data(), step).copyTo(histogram);

        track.histograms.append(histogram);
    }

    return in;
}

bool Track::operator!= (const Track &otherTrack) const
{
    if (this->sizeInBytes != otherTrack.sizeInBytes ||
        this->histograms.size() != otherTrack.histograms.size())
    {
        return true;
    }

    for (int i = 0; i < this->histograms.size(); i++)
    {
        const Mat &histogram1 = this->histograms.at(i);
        const Mat &histogram2 = otherTrack.histograms.at(i);

        QByteArray h1 = QByteArray::fromRawData(reinterpret_cast<char*>(histogram1.data), static_cast<int>(histogram1.step[0] * histogram1.rows));
        QByteArray h2 = QByteArray::fromRawData(reinterpret_cast<char*>(histogram2.data), static_cast<int>(histogram2.step[0] * histogram2.rows));

        if (h1 != h2)
        {
            return true;
        }
    }

    return false;
}
