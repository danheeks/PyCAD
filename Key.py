import wx
import cad

def KeyCodeFromWx(w):
    return cad.KeyCode(w.GetKeyCode())

