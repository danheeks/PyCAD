import wx
import cad

class ToolBarTool:
    def __init__(self):
        pass
    
    def BitmapPath(self):
        return wx.GetApp().cad_dir + '/bitmaps/'+ self.BitmapName() + '.png'
    
    def BitmapName(self):
        return "unknown"
    
    def Run(self, event):
        wx.MessageBox("default ToolBarTool.Run() called" + str(event))
        
class CadToolBarTool(ToolBarTool):
    def __init__(self, title, bitmap_name, method):
        ToolBarTool.__init__(self)
        self.title = title
        self.bitmap_name = bitmap_name
        self.method = method
        
    def GetTitle(self):
        return self.title
    
    def BitmapName(self):
        return self.bitmap_name
    
    def Run(self, event):
        self.method()
