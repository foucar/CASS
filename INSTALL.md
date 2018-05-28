Install instructions for CASS  {#cassinstall}
=============================
[TOC]

Prerequisites {#pre}
=============

The following software packages need to be installed and available for building
and running CASS:

## QT(version 4.6.x and above) {#qt}
Download source from

    http://download.qt-project.org/archive/qt

to compile and install

    ./configure -prefix somewhere -opensource
    make
    make install

in case you're using a custom version of gcc, then you need to provide the lb
path to that installation during the configure step like follows

    ./configure -prefix somewhere -opensource -confirm-license -R /path/to/gcc/lib


## gSOAP (version 2.8.50 and above) {#gsoap}
Download gSOAP from

    http://sourceforge.net/projects/gsoap2/files/

to compile and install do the following

    ./configure --prefix=/somewhere
    make
    make install

where "somewhere" is the place that you want to install the libraries to be
installed to (ie. /usr/local)


Optional Prerequisites {#optpre}
======================

The following software packages are needed when enabeling optional components in
CASS:

## gcc (version 4.6.4 and above) {#gcc}
Download a recent version of gcc here:

    https://gcc.gnu.org/

after the tarball is extracted in order to compile and install one has to
create a separate build dir.

    mkdir gcc$VERSION-build

download the prerequisites with the help of the script in contrib from within
the src dir

    contrib/download-prerequisites

to compile and install run the follwing from the build dir

    $srcdir/configure --prefix=somewhere
    make
    make install



## QWT (version 6.1.0 and above / 6.1.2 (in case version qt5.4.x is used)) {#qwt}
Download the version of qwt from here:

    http://sourceforge.net/projects/qwt/files/qwt/

to compile and install the libs one needs to modify `qwtconfig.pri` and set
`QWT_INSTALL_PREFIX` to where the libraries should be installed to (eg. /usr/local).
Then one needs to run

    qmake
    make
    make install


## ZLIB (version 1.2.7 or higher) {#zlib}
Download the latest version from

    http://www.zlib.net

to compile and install

    ./configure --prefix=somewhere
    make test
    make install

where "somewhere" is the place that you want to install the libraries to be
installed to (ie. /usr/local)


## HDF5 (version 1.8.5 and above) {#hdf5}
Download hdf5 library from here

    http://www.hdfgroup.org/ftp/HDF5/current/src/

to compile and install do the following:

    LDFLAGS="-Wl,-rpath,/path/to/zlib/lib" \
    ./configure --prefix=/somewhere --enable-threadsafe --with-pthread=/usr
    make
    make check
    make install


where "somewhere" is the place that you want to install the libraries to be
installed to (ie. /usr/local)

Use this command as the LDFLAGS part if you want to use a custom gcc

    LDFLAGS="-Wl,-rpath,/path/to/zlib/lib,-rpath,/path/to/gcc/lib" \

the rest of the lines above are the same.


## VIGRA (version 1.7.0 and above) {#vigra}
Download vigra template library from here:

    http://hci.iwr.uni-heidelberg.de/vigra/

just install the headers by copying them

    cp -r include/ /somewhere/.

where the above `/somewhere/.` is the location where to install the headers


## ROOT (version 5.28.00c and above) {#root}
Download root sources using git from here

    http://root.cern.ch/drupal/content/downloading-root

to compile and install (without the need of setting LD_LIBRARY_PATH later on)

    ./configure --prefix=/somewhere --enable-rpath --etcdir=/somewhere
    make [-j]
    make install

where "somewhere" is the place that you want to install the libraries to be
installed to (ie. /usr/local)


## DOXYGEN (version 1.8.4 and above) {#doxygen}
Download doxygen sources from here:

    http://www.stack.nl/~dimitri/doxygen/download.html

to compile and install do the following:

    ./configure --prefix somewhere
    make
    make install

where "somewhere" is the place that you want to install the libraries to be
installed to (ie. /usr/local)

## GRAPHVIZ used for doxgen {#graphviz}
Download graphviz from

    http://www.graphviz.org

to compile and install do the following:

    ./configure --prefix=/somewhere
    make
    make install

where "somewhere" is the place that you want to install the libraries to be
installed to (ie. /usr/local)


## FFTW (version 3.3.3 or above)  {#fftw}
Download the sources from

    http://www.fftw.org/download

to compile and install do

    ./configure --enable-shared --prefix=/somewhere
    make
    make install

where "somewhere" is the place that you want to install the libraries to be
installed to (ie. /usr/local)


## zeromq (for XFEL Interface) {#zeromq}
Download the sources from

    http://zeromq.org/area:download

to compile and install do

    ./configure --prefix=/somewhere
    make
    make install

Where somwhere is the place that you want to install the libraries to be
installed to (ie. /usr/local).
The additionally required c++ bindings need to be downloaded them from the git
repository at

    https://github.com/zeromq/cppzmq

and copy the header files to the include dir of the zeromq installation

    cp *.hpp /somewhere/.

where "somewhere" is the location of the zeromq include directory


## msgpack (for XFEL Interface) {#msgpack}
Download the c/c++ version of msgpack from the git repository

    https://github.com/msgpack/msgpack-c

just install the headers by copying them

    cp -r include/ /somewhere/.

where the above `/somewhere/.` is the location where to install the headers


## SACLA offline and online libraries {#sacla}
You can find the SACLA libraries at the HPC at SACLA. You need to have a sacla
account in order to get access to the SACLA HPC.


## Achims resort routine  {#achimresorter}
Please contact [Achim Czasch](mailto:czasch@roentdek.com) to get the latest
version of the resort routine.
Be adviced that the CASS interface might have to be adapted to the latest
version.


Needed Environment Variables  {#env}
============================

* QTDIR: needs to point to the Qt installation directory
* PATH: points there where you installed the qt binaries (ie. /usr/local/bin)


Building CASS   {#cass}
=============

To configure CASS for your needs please copy the default config as follows:

    cp cass_defaultconfig.pri cass_myconfig.pri

and make all your changes to the created `cass_myconfig.pri`. By changing the
`PREFIX` variable you can control where cass will be installed. Optionally
this parameter can also be set when calling the qmake utility (see below).

The following sequence of commands will build and install CASS:

    qmake [PREFIX=/path/to/your/preferred/binary/directory]
    make [-j]
    make install
