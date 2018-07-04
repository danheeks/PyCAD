import wx
import sys
import cad
from Frame import Frame

class App(wx.App):
    def __init__(self):
        save_out = sys.stdout
        save_err = sys.stderr
        wx.App.__init__(self)
        sys.stdout = save_out
        sys.stderr = save_err
        
    def OnInit(self):
        result = cad.OnInit()

        # make a wxWidgets application
        self.frame = Frame()
        self.frame.Show()
        
        return result
