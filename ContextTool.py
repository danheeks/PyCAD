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
        
    def IsEnabled(self):
        return True
        
class ObjectTitleTool(ContextTool):
    def __init__(self, object):
        self.object = object
        ContextTool.__init__(self)

    def BitmapPath(self):
        return self.object.GetIconFilePath()

    def BitmapSize(self):
        return 16
    
    def GetTitle(self):
        return self.object.GetTitle()
        
    def IsEnabled(self):
        return False
        
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

    def BitmapPath(self):
        return self.object.GetIconFilePath()

    def BitmapSize(self):
        return 16
    
class SelectTool(ContextTool):
    def __init__(self, object, control_pressed):
        ContextTool.__init__(self)
        self.object = object
        self.control_pressed = control_pressed
    
    def GetTitle(self):
        if self.control_pressed: return "Add To Selection"
        return "Select"

    def BitmapName(self):
        return "select"

    def Run(self, event):
        if not self.control_pressed:
            cad.ClearSelection(False)
        cad.Select(self.object)
         
class EditTool(ContextTool):
    def __init__(self, object):
        ContextTool.__init__(self)
        self.object = object
    
    def GetTitle(self):
        return "Edit"

    def BitmapName(self):
        return "edit"

    def Run(self, event):
        wx.GetApp().EditUndoably(self.object)
         
class DeleteTool(ContextTool):
    def __init__(self, object):
        ContextTool.__init__(self)
        self.object = object
    
    def GetTitle(self):
        return "Delete"

    def BitmapName(self):
        return "delete"

    def Run(self, event):
        cad.DeleteUndoably(self.object)       
        
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
