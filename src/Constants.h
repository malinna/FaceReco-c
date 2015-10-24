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

#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <QString>
#include "opencv2/opencv.hpp"

#define MODE_LEARN_AND_RECOGNIZE    0
#define MODE_RECOGNIZE_ONLY         1
#define MODE_TEST                   2

#define DISPLAY_FRAME_INFO
//#define DISPLAY_LANDMARK_LABELS

const QString VERSION_STRING("v0.1");
const QString LOG_FILE("log.txt");

// Webcam resolution.
const int DEFAULT_CAM_WIDTH = 320;
const int DEFAULT_CAM_HEIGHT = 240;

const int TRACK_WINDOW_WIDTH = 800;
const int TRACK_WINDOW_HEIGHT = 600;
const int TRACK_WINDOW_GRID_X = 6;
const int TRACK_WINDOW_GRID_Y = 4;
const cv::Size TRACK_WINDOW_FRAME_SIZE(TRACK_WINDOW_WIDTH / TRACK_WINDOW_GRID_X, TRACK_WINDOW_HEIGHT / TRACK_WINDOW_GRID_Y);

// If false, video files are processed at maximum speed.
const bool MAINTAIN_VIDEO_FPS = true;

// Size of the normalized face image.
const cv::Size ALIGNED_FACE_IMAGE_SIZE(130, 151);

// LBP parameters.
const int LBP_GRID_X = 7;
const int LBP_GRID_Y = 7;
const int EXTENDED_LBP_RADIUS = 2;
const int EXTENDED_LBP_SAMPLING_POINTS = 8;

// This parameter specifies how often key frames are taken. It specifies the
// length of the delta vector (between landmarks of the last and the current key
// frame) that should be reached in order to take a new key frame. The bigger
// the value, the less key frames are taken. And vice versa.
const float LANDMARK_DELTA_MIN_THRESHOLD = 0.03f;

// The following constants define size of the quadrangle around the face.
// They are multiplied with the distance between eyes to get dimension of the
// quadrangle. The bigger the factors are, the larger the quadrangle is.
const float LEFT_DISTANCE_FACTOR    = 0.96f;
const float RIGHT_DISTANCE_FACTOR   = 0.96f;
const float UP_DISTANCE_FACTOR      = 0.70f;
const float DOWN_DISTANCE_FACTOR    = 1.64f;

const quint32 MIN_SEARCH_TIME_MS = 1000;
const quint32 MAX_SEARCH_TIME_MS = 1000;

// If true, every detected face track is processed (even if it have only 1
// frame).
const bool SHOW_RESULT_WITH_SHORT_TRACKS = true;

// This parameter is used by the search engine and it defines the distance when
// a histogram is considered to match with some other histogram.
// The bigger the value, the bigger chance to end up selecting wrong person.
// The smaller the value, the bigger chance to end up selecting no person at all.
const float HISTOGRAM_DISTANCE_THRESHOLD = 0.37f;

#endif // CONSTANTS_H
