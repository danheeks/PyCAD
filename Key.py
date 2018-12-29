import wx
import cad

def KeyEventFromWx(w):
    e = cad.KeyEvent()
    e.m_key_code = cad.KeyCode(w.GetKeyCode())
    return e
