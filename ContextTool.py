import wx
import cad

class ContextTool:
    def __init__(self):
        pass
    
    def IsAToolList(self):
        return False
    
    def GetTitle(self):
        return "Unknown"
    
    def BitmapPath(self):
        return wx.GetApp().cad_dir + '/bitmaps/'+ self.BitmapName() + '.png'
    
    def BitmapName(self):
        return "unknown"
    
    def BitmapSize(self):
        return 24
    
    def Run(self, event):
        wx.MessageBox("default ContextTool.Run() called" + str(event))
        
    def GetTools(self):
        return []
        
class ContextToolList(ContextTool):
    def __init__(self, title):
        ContextTool.__init__(self)
        self.title = title
        self.tools = []
        
    def IsAToolList(self):
        return True
    
    def GetTitle(self):
        return self.title
        
    def GetTools(self):
        return self.tools
    
class ObjectToolList(ContextToolList):
    def __init__(self, object):
        self.object = object
        ContextToolList.__init__(self, object.GetTitle())
        self.AddSpecificTools()

    def BitmapPath(self):
        return self.object.GetIconFilePath()

    def BitmapSize(self):
        return 16
    
    def AddSpecificTools(self):
        type = self.object.GetType()
        if type == cad.OBJECT_TYPE_SKETCH:
            if self.object.GetNumChildren() > 1:
                self.tools.append(CADContextTool("Split Sketch", "splitsketch", self.SplitSketch))
            
    def SplitSketch(self):
        wx.MessageBox("Splitting " + str(self.object))
            
class CADContextTool(ContextTool):
    def __init__(self, title, bitmap_name, method):
        ContextTool.__init__(self)
        self.title = title
        self.bitmap_name = bitmap_name
        self.method = method
        
    def GetTitle(self):
        return self.title
    
    def BitmapName(self):
        return self.bitmap_name
    
    def Run(self, event):
        self.method()
