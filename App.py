import wx
import sys
import cad
from Frame import Frame
import os
from HeeksConfig import HeeksConfig

pycad_dir = os.path.dirname(os.path.realpath(__file__))

def OnMessageBox(error_message):
    wx.MessageBox(error_message)

class App(wx.App):
    def __init__(self):
        self.settings_restored = False
        self.recent_files = []
        self.MAX_RECENT_FILES = 20
        self.filepath = None
        self.hideable_windows = []

        save_out = sys.stdout
        save_err = sys.stderr
        wx.App.__init__(self)
        sys.stdout = save_out
        sys.stderr = save_err
        
    def OnInit(self):
        self.RegisterMessageBoxCallback()
        result = self.InitCad()
        
        self.printData = wx.PrintData()
        self.pageSetupData = wx.PageSetupDialogData(self.printData)
        
        self.frame = self.NewFrame()
        self.frame.Show()
        self.LoadConfig()
        self.frame.SetFrameTitle()
        
        return result
    
    def NewFrame(self):
        return Frame(None)

    def LoadConfig(self):
        config = HeeksConfig(self.settings_restored)
        self.LoadRecentFiles(config)
        
    def LoadRecentFiles(self, config):
        for i in range(0, self.MAX_RECENT_FILES):
            key = 'RecentFilePath' + str(i)
            filepath = config.Read(key)
            if len(filepath) == 0:
                break
            self.recent_files.append(filepath)
            
    def WriteRecentFiles(self, config = None):
        if config == None:
            config = HeeksConfig(self.settings_restored)
        index = 0
        for filepath in self.recent_files:
            if index >= self.MAX_RECENT_FILES:
                break
            config.Write('RecentFilePath' + str(index), self.recent_files[index])
            index += 1
            
    def InsertRecentFileItem(self, filepath):
        if filepath in self.recent_files:
            self.recent_files.remove(filepath)
        self.recent_files.insert(0,filepath)
        if len(self.recent_files) > self.MAX_RECENT_FILES:
            self.recent_files.pop()

    def RegisterMessageBoxCallback(self):
        cad.RegisterMessageBoxCallback(OnMessageBox)
        
    def RegisterHideableWindow(self, w):
        self.hideable_windows.append(w)
        
    def RemoveHideableWindow(self, w):
        self.hideable_windows.remove(w)
        
    def InitCad(self):
        result = cad.OnInit()
        cad.SetResFolder(pycad_dir)
        return result
    
