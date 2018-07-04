# PyCAD
Experiments in making a CAD software on the Raspberry Pi, using Python, with wxPython boost-python, OpenGL and my geometry derived from HeeksCNC project.\
This will involve building a CAD python module and a GEOM python module\
The plan is that all the wx will be done with wxPython scripts.\
The gl canvas will be created by wxPython.\
The gl commands will be done in the c++ by my CAD module.\
Cad object classes will be defined in Python, derived from a cad object base class defined in the cad module.\
Cad objects will implement GetProperties, GetProperty, SetProperty, GetLines, GetTriangles, GetName, GetIcon, etc.

## How to build ##
Start with a clean Raspbian installation

### Get essentials ###
```
sudo apt-get install git build-essential debhelper cmake libboost-python-dev python-dev freeglut3-dev
```

### Fetch sources ###
```
git clone https://github.com/danheeks/PyCAD.git
```

### build CAD python module ###
cd PyCAD/CAD\
mkdir build\
cd build\
cmake ..\
make

### try the test.py ###
cp cad.so ../../
cd ../../
python test.py

### problems June 2018 ###
I couldn't get cad module to share classes with wxPython, but cad will be more reusable when it doesn't have any wx in it.\
So, I am writing GUI to be in wxPython and the cad to implement the rendering.\
First test will be cad rendering the background and grid.

