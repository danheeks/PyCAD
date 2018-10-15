import wx
import sys
import cad
from Frame import Frame
import os

pycad_dir = os.path.dirname(os.path.realpath(__file__))

def OnMessageBox(error_message):
    wx.MessageBox(error_message)

class App(wx.App):
    def __init__(self):
        import os
        full_path_here = os.path.abspath( __file__ )
        full_path_here = full_path_here.replace("\\", "/")
        slash = full_path_here.rfind("/")
        self.res_folder = full_path_here
        if slash != -1:
            self.res_folder = full_path_here[0:slash]

        save_out = sys.stdout
        save_err = sys.stderr
        wx.App.__init__(self)
        sys.stdout = save_out
        sys.stderr = save_err
        
    def OnInit(self):
        self.RegisterMessageBoxCallback()
        result = self.InitCad()
        
        # make a wxWidgets application
        self.frame = Frame(None)
        self.frame.Show()
        
        return result

    def RegisterMessageBoxCallback(self):
        cad.RegisterMessageBoxCallback(OnMessageBox)
        
    def InitCad(self):
        result = cad.OnInit()
        cad.SetResFolder(pycad_dir)
        return result
        
    def GetResFolder(self):
        if self.res_folder:
            return self.res_folder