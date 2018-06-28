# PyCAD
Experiments in making a CAD software on the Raspberry Pi, using Python, with wxPython boost-python, OpenGL and my geometry derived from HeeksCNC project.
This will involve building a CAD python module and a GEOM python module
I am hoping that wxPython and CAD can share objects, so that I can do
import wx\
import cad\
frame = wx.Frame(None, -1, 'test')\
graphics_canvas = cad.GraphicsCanvas(frame)

## How to build ##
Start with a clean Raspbian installation

### Get essentials ###
```
sudo apt-get install git build-essential debhelper cmake libboost-python-dev python-dev libwxgtk2.8-dev python-wxgtk2.8 freeglut3-dev
```

### Fetch sources ###
```
git clone https://github.com/danheeks/PyCAD.git
```

### build CAD python module ###
cd PyCAD/CAD\
mkdir build\
cd build\
cmake -DCMAKE_INSTALL_PREFIX:PATH=/usr ..\
make

### try the test.py ###
python ../test.py

### problems June 2018 ###
I'm getting\ 
Traceback (most recent call last):\
  File "<stdin>", line 1, in <module>\
ImportError: ./cad.so: undefined symbol: _ZTI10wxGLCanvas
