import sys
from SolidApp import SolidApp
#from App import App

app = SolidApp()

if len(sys.argv) > 1:
    import cad
    cad.Import(sys.argv[1])

app.MainLoop()

