# PyCAD
Experiments in making a CAD software on the Raspberry Pi, using Python, with wxPython boost-python, OpenGL and my geometry derived from HeeksCNC project.\
This involves building a cad.so python module and a geom.so python module\
All the user interface is done with wxPython scripts.\
The graphics canvas is created by wxPython.\
The OpenGL commands are done in the c++ by my CAD module.\
Some object classes are defined in Python ( like https://github.com/danheeks/PyCAD/blob/master/Gear.py )\
derived from a cad object base class defined in the cad module.

## How to build ##
Start with a clean Raspbian installation, I just went for the full one. https://www.raspberrypi.org/downloads/raspbian/

### build wxPython ###
https://wiki.wxpython.org/BuildWxPythonOnRaspberryPi \
or if you have already built it and copied its whl to your Windows laptop ( this is for me )\
on your laptop open a command line
```
pscp c:\tmp\wxPython-4.1.0-cp37-cp37m-linux_armv7l.whl pi@192.168.3.110
```
( type in password )\
on pi
```
pip3 install wxPython-4.1.0-cp37-cp37m-linux_armv7l.whl
```

### Get essentials ###
```
sudo apt-get install git build-essential debhelper cmake python3-dev freeglut3-dev
```

### Build boost python for python3 ###
download the latest boost\
(look here https://www.boost.org/users/download/ and copy the path for the download\
which ends with bz2, then on RPI type wget followed by the path)\
unzip with tar xf followed by filename\
cd into boost folder
```
./bootstrap.sh --with-python-version=3.7
sudo nano project-config.jam
```
change\
libraries = ;\
to
```
libraries = --with-python ;
```
then
```
./b2
sudo ./b2 install
cd ../
```

### Fetch sources ###
```
git clone https://github.com/danheeks/PyCAD.git
```

### build CAD python module ###
```
cd PyCAD/CAD
mkdir build
cd build
cmake ..
make
cd ../../
cp CAD/build/cad.so ./
```

### build Geom python module ###
```
cd Geom
mkdir build
cd build
cmake ..
make
cd ../../
cp Geom/build/geom.so ./

```

### build Step python module ###
( this stage still in development )
First build OCE; hours of building; download using wget latest file ending tar.gz from:\
https://github.com/tpaviot/oce/releases \
unzip using tar xf\
cd into the folder starting with oce
```
mkdir build
cd build
cmake ..
make
sudo make install
cd ../../
```

build step
```
cd Step
mkdir build
cd build
cmake ..
make
cd ../../
cp Step/build/step.so ./

```

### try the test.py ###
```
python3 test.py
```

### Development Plan ###
I am now building boost python for Python3 ok on a RPI 4.\
The plan is to get back to the same functionality as HeeksCNC 1.4, but working from python 3, using PyCAD.\
The idea is that an installed version of PyCAD will be in a known place, so extensions can just borrow from it.\
The idea is for it to be hackable, so all the user interface is done with python.
