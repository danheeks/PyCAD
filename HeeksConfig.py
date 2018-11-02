import wx

class HeeksConfig(wx.Config):
    def __init__(self, disabled = False):
        wx.Config.__init__(self, wx.GetApp().GetAppName())
        self.disabled = disabled
        
    def Write(self, key, value):
        if self.disabled:
            return False
        return super().Write(key, value)