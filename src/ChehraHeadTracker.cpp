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

#include "ChehraHeadTracker.h"
#include "Util.h"
#include <QString>
#include <QDebug>

using namespace FaceReco;
using namespace cv;

const QString CHEHRA_MODEL("Chehra_t1.0.model");
const QString HAAR_CASCADE("haarcascade_frontalface_alt_tree.xml");

// Indices of important facial landmarks.
const int MIRRORED_LEFT_EYE = 19;
const int MIRRORED_RIGHT_EYE = 25;

const int EYE_LANDMARK_COUNT = 6;

ChehraHeadTracker::ChehraHeadTracker()
{
    // Set tracking parameters for Chehra algorithm.
    fcheck_interval = 2; //Failure checker to be executed after every 2 frames (default: 5).
    fcheck_score_treshold = -0.25;
    fcheck_fail_treshold = 2; // Reinitialize after failure on 2 consecutive frames.
}

ChehraHeadTracker::~ChehraHeadTracker()
{
}

bool ChehraHeadTracker::init()
{
    qDebug() << "Loading face detector model...";

    if (!faceCascade.load(HAAR_CASCADE.toStdString()))
    {
        qDebug() << "Failed to load face detector model:" << HAAR_CASCADE;

        return false;
    }

    qDebug() << "Loading face tracker model... (this takes a while)";

    try
    {
        chehra.reset(Chehra_Tracker::CreateChehraLinker(CHEHRA_MODEL.toStdString().c_str()));
    }
    catch (...)
    {
        qDebug() << "Exception thrown by Chehra_Tracker::CreateChehraLinker()";
    }

    if (chehra.isNull())
    {
        qDebug() << "Failed to load face tracker model:" << CHEHRA_MODEL;

        return false;
    }

    return true;
}

bool ChehraHeadTracker::reset()
{
    try
    {
        chehra->Reinitialize();
    }
    catch (...)
    {
        qDebug() << "Exception thrown by Chehra_Linker::Reinitialize()";

        return false;
    }

    return true;
}

bool ChehraHeadTracker::track(Mat frame)
{
    Q_ASSERT(chehra.data());

    // Chehra wants frame to be in grayscale format.
    Mat frameForChehra;
    if (frame.channels() == 3)
    {
        cvtColor(frame, frameForChehra, CV_BGR2GRAY);
    }
    else
    {
        frameForChehra = frame;
    }

    int retVal = -1;
    try
    {
        retVal = chehra->TrackFrame(frameForChehra,
                                    fcheck_interval,
                                    fcheck_score_treshold,
                                    fcheck_fail_treshold,
                                    faceCascade);

    }
    catch (...)
    {
        qDebug() << "Exception thrown by Chehra_Linker::TrackFrame()";
    }

    if (retVal != 0)
    {
        reset();

        return false;
    }

    // Chehra returns coordinates of the face landmarks in 98x1 1-channel
    // matrix. Convert the matrix to the 49x1 2-channel form.
    facialLandmarks = convert1chTo2ch(chehra->_bestFaceShape);

    leftEye = centerPoint(facialLandmarks(Rect(0, MIRRORED_LEFT_EYE, 1, EYE_LANDMARK_COUNT)));
    rightEye = centerPoint(facialLandmarks(Rect(0, MIRRORED_RIGHT_EYE, 1, EYE_LANDMARK_COUNT)));

    pitch = chehra->_PitchDeg;
    yaw = chehra->_YawDeg;
    roll = chehra->_RollDeg;

    doPostprocessing(frame);

    alignedLeftEye = centerPoint(alignedFacialLandmarks(Rect(0, MIRRORED_LEFT_EYE, 1, EYE_LANDMARK_COUNT)));
    alignedRightEye = centerPoint(alignedFacialLandmarks(Rect(0, MIRRORED_RIGHT_EYE, 1, EYE_LANDMARK_COUNT)));

    return true;
}

Mat ChehraHeadTracker::convert1chTo2ch(Mat &input)
{
    const int R = input.rows / 2;
    Mat mv[] = { input(Rect(0, 0, 1, R)), input(Rect(0, R, 1, R)) };
    Mat output(R, 1, CV_32FC2);
    merge(mv, 2, output);
    return output;
}
