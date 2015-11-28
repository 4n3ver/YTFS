=====================
Your Tune File System
=====================

:Info: Specialized file system aimed to manage music library
:Authors: William Morlan, Yoel Ivan

Dependecies
===========

+ **FUSE**

  To get started, you have to have fuse library installed in your system, along with the source code
  
+ **taglib**

  Taglib library also need to be installed (along with the source code) in your system so the file system can manage the usic based on ID3 Tag.
  
+ **curl**

  *cURL* command line utillities are needed for back storage operation
  
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
