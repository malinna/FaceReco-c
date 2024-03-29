# FaceReco

![source](https://img.shields.io/badge/source-C++/Qt-green.svg)
[![license](https://img.shields.io/badge/license-BSD-green.svg)](LICENSE)
![platform](https://img.shields.io/badge/platform-windows_64--bit-blue.svg)

FaceReco is a face recognition system, which can learn and recognize faces from video. It is developed by Marko Linna at the [University of Oulu](https://www.oulu.fi/en) in the **Research Work on Information Processing** course. The C++/Qt sources are available under the [BSD 3-Clause](LICENSE) license. This software was developed for research purposes and there is no other support available than this readme file.

![ui](https://lh5.googleusercontent.com/F63f2nglHhQ6egtCBjuJcIABlxnb8BJXV1KV0F9Yr7P-KUCdmME0YxIWxUrDbSA6zRk=w2400)
![ui](https://lh5.googleusercontent.com/IW2SgPxl4fYvP406K69bgMN0YOtB5cmo0R_IUZanFYIDi7hrJmH5DMkt2ADofZoJ1YQ=w2400)
![ui](https://lh5.googleusercontent.com/f8LhcTYwez37hMPaomo7HjoWg6bwAh5suWJMC9ao4cgaOnxrvJg4oJ6JQBc-6UFpyu4=w2400)

Check [this](https://www.youtube.com/watch?v=QxhqC-4yHiA) video to see how the system performs on the [Honda/UCSD video database](http://vision.ucsd.edu/~leekc/HondaUCSDVideoDatabase/HondaUCSD.html).

FaceReco utilizes 3rd party software [Chehra](https://sites.google.com/site/chehrahome/) for facial landmark detection and tracking. Faces are represented using Local Binary Patterns ([LBP](http://www.scholarpedia.org/article/Local_Binary_Patterns)). For more detailed information about the system, read the following paper:

* Linna M, [Kannala J](https://users.aalto.fi/~kannalj1/) and [Rahtu E](https://esa.rahtu.fi/). Online Face Recognition System Based on Local Binary Patterns and Facial Landmark Tracking. *Advanced Concepts for Intelligent Vision Systems (ACIVS 2015)*. [[PDF](https://users.aalto.fi/~kannalj1/publications/acivs2015.pdf)]

---
## System requirements

FaceReco runs on Windows 64-bit hosts (tested with Windows 7).

## Prerequisites

In order to compile FaceReco, the following dependencies must be installed.
* Visual Studio 2012
* Qt 5.2.1 for Windows (VS2012 x64)
  - You can find it in Qt Archive: [http://download.qt.io/archive/qt/](http://download.qt.io/archive/qt/)
* OpenCV 2.4.9
* Chehra
  - Homepage: [https://sites.google.com/site/chehrahome/](https://sites.google.com/site/chehrahome/)

## Compilation

The following steps are required in order to compile FaceReco with OpenCV and Chehra.

1. Get the **Chehra C++ Tracking Code (VS2010 and VS2012)** from [here](https://sites.google.com/site/chehrahome/).
2. Unzip the software to desired location.
3. Update `CHEHRA_ROOT` variable in FaceReco.pro to point to Chehra root folder.
4. Update `OPENCV_ROOT` variable in FaceReco.pro to point to OpenCV 2.4.9 root folder.
5. Open Chehra_Linker.h from Include folder and comment out all the header includes but `"opencv2/opencv.hpp"`.
6. Compile FaceReco with Qt Creator.

## License

Copyright (c) 2015 Marko Linna.

Licensed under the [BSD 3-Clause](LICENSE) license.

## Citation

Please site the following paper in your publications if FaceReco helps your research.

    @inproceedings{linna2015online,
      title={Online Face Recognition System Based on Local Binary Patterns and Facial Landmark Tracking},
      author={Linna, Marko and Kannala, Juho and Rahtu, Esa},
      booktitle={International Conference on Advanced Concepts for Intelligent Vision Systems},
      pages={403--414},
      year={2015},
      organization={Springer}
    }
