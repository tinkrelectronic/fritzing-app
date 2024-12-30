# Compiling for Windows

Here is how to compile Fritzing in a windows environment.

## Software

Microsoft Visual Studio Community 2022 (Free)

You will need to install the C++ tools

You will  need to install the Qt Tools for VS extension

Be sure the default Qt is set to your installed Qt version in the Qt Tools VS extension

## Qt 6.8.1 with msvc2022_64 tools installed
Additional Libraries (Qt)

### Fritzing Source
Either download and extract zip or git clone to directory.

## Additional Libraries & Source/Binaries
+ [Boost 1_85_0](https://github.com/tinkrelectronic/boost/releases/download/boost-1.85.0/boost_1_85_0.zip)
+ [clipper-6.4.2](https://github.com/tinkrelectronic/clipper/archive/refs/tags/v6.4.2.zip)
+ [Quazip 1.4](https://github.com/tinkrelectronic/quazip/archive/refs/tags/v1.4.zip)
+ [Zlib 1.3.1](https://github.com/tinkrelectronic/zlib/archive/refs/tags/v1.3.1.zip)
+ [ngspice-42](https://github.com/tinkrelectronic/ngspice/archive/refs/tags/v42.zip)
+ [svgpp-1.3.1](https://github.com/tinkrelectronic/svgpp/archive/refs/tags/v1.3.1.zip)

### New release.bat in the works to help set up.

+ Set up the Fritzing Source Code Directory
+ Extract Additional libraries/source to look like this
  /fritzing-app/clipper-6.4.2/
  /fritzing-app/boost_1_85_0/
  /fritzing-app/quazip-1.4/
  /fritzing-app/zlib-1.3.1/
  /fritzing-app/ngspice-42/
  /fritzing-app/svgpp-1.3.1/

  in MSVC2022 click extensions and use the Qt tool to open pheonix.pro
  and wait. Once it loads fully you may save the SVN (solution)
  
  
