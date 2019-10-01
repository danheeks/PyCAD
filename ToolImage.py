import wx

button_scale = 0.25
full_size = 96
default_bitmap_size = 24

class ToolImage(wx.Image):
    def __init__(self, path):
        wx.Image.__init__(self, path, wx.BITMAP_TYPE_PNG)
        width = self.GetWidth()
        height = self.GetHeight()
        new_width = int(button_scale * width)
        new_height = int(button_scale * height)
        self.Rescale(new_width, new_height)
        
def GetBitmapSize():
    return int(button_scale * full_size)
    
def SetBitmapSize(size):
    global button_scale
    button_scale = float(size) / full_size