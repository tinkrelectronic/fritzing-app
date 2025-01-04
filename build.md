# Compiling for Windows

Here is how to compile Fritzing in a windows environment.

## Software

[Microsoft Visual Studio Community 2022 (Free)](https://visualstudio.microsoft.com/downloads/)

You will need to install the C++ tools

You will  need to install the Qt Tools for VS extension

Be sure the default Qt is set to your installed Qt version in the Qt Tools VS extension

### Qt 6.8.1 with msvc2022_64 tools installed
[Qt Tools Installer](https://www.qt.io/download-qt-installer-oss)

Additional Libraries (Qt)

You can choose a different Qt version, but you will need to swap the dll's.

[Fritzing Base Folder ](https://github.com/tinkrelectronic/fritzing-app/releases/download/base/fritzingbasefolder.zip)

## Fritzing Source Code
Either download and extract zip or git clone to directory.

## Additional Libraries & Source/Binaries
(download & Extract in fritzing-app directory)
+ [Boost 1_85_0](https://github.com/tinkrelectronic/boost/releases/download/boost-1.85.0/boost_1_85_0.zip)
+ [clipper-6.4.2](https://github.com/tinkrelectronic/clipper/archive/refs/tags/v6.4.2.zip)
+ [Quazip-1.4](https://github.com/tinkrelectronic/quazip/archive/refs/tags/v1.4.zip)
+ [Zlib-1.3.1](https://github.com/tinkrelectronic/zlib/archive/refs/tags/v1.3.1.zip)
+ [ngspice-42](https://github.com/tinkrelectronic/ngspice/archive/refs/tags/v42.zip)
+ [svgpp-1.3.1](https://github.com/tinkrelectronic/svgpp/archive/refs/tags/v1.3.1.zip)
+ [libgit2](https://github.com/tinkrelectronic/libgit2/archive/refs/tags/v1.9.zip)

### Set Up Instructions
Set up the Fritzing Source Code Directory (/fritzing-app)
Extract Additional libraries/source to look like this

+  /fritzing-app/clipper-6.4.2/
+  /fritzing-app/boost_1_85_0/
+  /fritzing-app/quazip-1.4/
+  /fritzing-app/zlib-1.3.1/
+  /fritzing-app/ngspice-42/
+  /fritzing-app/svgpp-1.3.1/
+  /fritzing-app/libgit2/ (Rename to libgit2)

  Open MSVC2022 (continue without code) click extensions and use the Qt tool (Open Qt Project File (.pro)) to open pheonix.pro
  and wait. Once it loads fully you may save the SLN (solution)

  Change to release, then build solution. 

  Note: If you want to build from the original fritzing branch just swap the PRI folder and pheonix.pro file in the root directory from this source.

  This will create the fritzing.exe file.

  These are the dll's (Qt 6.8.1) and other resources without the executable [zipped](https://github.com/tinkrelectronic/fritzing-app/releases/download/base/fritzingbasefolder.zip). Enjoy your tinkering!

### Install Setup File

You may wish to create a setup for distribution, however most of it depends on what you use.

These are the Registry areas of interest.

![image](https://github.com/user-attachments/assets/b53e59d2-984e-4d09-9081-3423f36167fb)

For post install you can make an executable or batch file to notify the windows shell of the new assosciation.


# Compiling for Linux

Here is how to compile in a Linux environment.

## Install Qt 6.8.1 or build from source

[Qt Tools Installer](https://www.qt.io/download-qt-installer-oss)

Additional Libraries (Qt)

Qt Creator

You can of course use another Qt version if you wish.

[Fritzing Base Folder](https://github.com/tinkrelectronic/fritzing-app/releases/download/base/fritzingbasefolder.zip)

## Fritzing Source Code
Download or git clone

## Additional Libraries & Source/Binaries

(download & Extract in fritzing-app directory)
+ [Boost 1_85_0](https://github.com/tinkrelectronic/boost/releases/download/boost-1.85.0/boost_1_85_0.zip)
+ [clipper-6.4.2](https://github.com/tinkrelectronic/clipper/archive/refs/tags/v6.4.2.zip)
+ [Quazip-1.4](https://github.com/tinkrelectronic/quazip/archive/refs/tags/v1.4.zip)
+ [Zlib-1.3.1](https://github.com/tinkrelectronic/zlib/archive/refs/tags/v1.3.1.zip)
+ [ngspice-42](https://github.com/tinkrelectronic/ngspice/releases/download/v42/ngspice-42.tar.gz)
+ [svgpp-1.3.1](https://github.com/tinkrelectronic/svgpp/archive/refs/tags/v1.3.1.zip)
+ [libgit2](https://github.com/tinkrelectronic/libgit2/archive/refs/tags/v1.9.zip)

### Set Up Instructions
Set up the Fritzing Source Code Directory (/fritzing-app)
Extract Additional libraries/source to look like this

+  /fritzing-app/clipper-6.4.2/
+  /fritzing-app/boost_1_85_0/
+  /fritzing-app/quazip-1.4/
+  /fritzing-app/zlib-1.3.1/
+  /fritzing-app/ngspice-42/
+  /fritzing-app/svgpp-1.3.1/
+  /fritzing-app/libgit2/ (Rename to libgit2)

  
+ zlib, quazip and clipper edit the PRI files and swap LIB comments (#)
  

## Build
+ Open the pheonix.pro project file in QT creator
+ Configure Qt version
+ Select Release
+ Build & Cross fingers

## Notes
It's been a long time since I've compiled in linux, even longer with a GUI.
You may need to build the linux libraries for some of the dependancies. I am not sure how these carry over to other distros.
You may also need to install some qt6 libraries to run Fritzing.

OpenSSL v 3.0.15 or higher is required to be installed for build.(fallback)

libglx and libgles may also be required

General Tip -L is specifies libraries directory -l the library file -l: if it starts with an "l"
