=====================
Your Tune File System
=====================

:Info: Specialized file system aimed to manage music library
:Authors: William Morlan, Yoel Ivan

Dependencies
============

+ **FUSE**

  To get started, you have to have fuse library installed in your system, along with the source code
  
+ **taglib**

  Taglib library also need to be installed (along with the source code) in your system so the file system can manage the usic based on ID3 Tag.
  
+ **curl**

  *cURL* command line utillities are needed for back storage operation
  
Dependencies for Optional Feature
=================================

+ **FFmpeg** and **libfftw3**

  *chromaprint* lib dependencies to compute *FFT*, to install:
  
  - ``$ sudo add-apt-repository ppa:mc3man/trusty-media``
  
  - ``$ sudo apt-get update``
  
  - ``$ sudo apt-get dist-upgrade``
  
  - ``$ sudo apt-get install ffmpeg libfftw3-dev libfftw3-doc``

+ **chromaprint**
  
  library to extract fingerprint from audio file, required for *auto-tag* feature,
  to install:
  
  - ``$ git clone https://bitbucket.org/acoustid/chromaprint.git``
  
  - ``$ cd chromaprint``
  
  - ``$ cmake -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=OFF``
  
+ **pip3**, **pyacoustid**, and **mutagen**

    required python module to access *acoustid* database to get metadata of fingerprinted audiofile, to install:
    
    - ``$ sudo apt-get install python3-pip``
    
    - ``$ sudo pip3 install  pyacoustid mutagen``
  
Compiling
=========

type in the command below for compiling the file system

``$ gcc -Wall ytfs.c `pkg-config fuse --cflags --libs` -o ytfs -ltag_c``

Installation
============

+ Setting up your username by typing command below, makesure you run the command as with your username

  ``sed -i -e "s/ubuntu/$USER/g" ytfs.c``
  
+ Deploy the file system by typing command below, ``tmp/ytfs`` is the recommended ``[MOUNT_LOCATION]``

  ``./ytfs [MOUNT_LOCATION]``
