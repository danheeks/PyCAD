import wx

class PictureFrame(wx.ScrolledWindow):
    def __init__(self, parent, bitmap):
        wx.ScrolledWindow.__init__(self, parent)
        self.bitmap = bitmap
        self.SetScrollRate(10,10)
        w = self.bitmap.GetWidth()
        h = self.bitmap.GetHeight()
        self.SetVirtualSize(w,h)
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        
    def OnPaint(self, event):
        dc = wx.PaintDC(self)
        dc.DrawBitmap(self.bitmap, 0,0, False)
        
class PictureWindow(wx.Window):
    def __init__(self, parent, size):
        wx.Window.__init__(self, parent, wx.ID_ANY, wx.DefaultPosition, size)
        self.bitmap = None
        self.bitmaps = {}
        self.Bind(wx.EVT_PAINT, self.OnPaint)
        
    def OnPaint(self, event):
        dc = wx.PaintDC(self)
        if self.bitmap:
            dc.DrawBitmap(self.bitmap, 0,0, False)
            
    def SetBitmap(self, bitmap):
        self.bitmap = bitmap
        self.Refresh()
    
    def SetPicture(self, filepath):
        if not filepath in self.bitmaps:
            self.bitmaps[filepath] = wx.Bitmap(wx.Image(filepath, wx.BITMAP_TYPE_PNG))
        self.SetBitmap(self.bitmaps[filepath])
