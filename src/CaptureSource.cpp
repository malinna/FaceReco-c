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

#include "CaptureSource.h"
#include <QDebug>

using namespace cv;

CaptureSource::CaptureSource(const int width, const int height, const int cameraIndex) :
    cap(cameraIndex),
    isCameraSource(true),
    captureSourceFPS(0),
    windowWidth(0),
    windowHeight(0),
    filename()
{
    if (cap.isOpened())
    {
        cap.set(CV_CAP_PROP_FRAME_WIDTH, width);
        cap.set(CV_CAP_PROP_FRAME_HEIGHT, height);

        windowWidth = width;
        windowHeight = height;
    }
}

CaptureSource::CaptureSource(const QString &filename) :
    cap(filename.toStdString()),
    isCameraSource(false),
    windowWidth(0),
    windowHeight(0),
    filename(filename)
{
    if (cap.isOpened())
    {
        Mat frame;
        cap >> frame;

        if (!frame.empty())
        {
            windowWidth = static_cast<int>(cap.get(CV_CAP_PROP_FRAME_WIDTH));
            windowHeight = static_cast<int>(cap.get(CV_CAP_PROP_FRAME_HEIGHT));
            captureSourceFPS = static_cast<int>(cap.get(CV_CAP_PROP_FPS) + 0.5);

            cap.set(CV_CAP_PROP_POS_FRAMES, 0); //Set index to 0 (start frame).
        }
    }
}

bool CaptureSource::isOpened()
{
    return cap.isOpened();
}

Mat CaptureSource::queryFrame()
{
    Mat frame;
    cap >> frame;
    return frame;
}
