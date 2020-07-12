import wx

app_name = None

class HeeksConfig(wx.Config):
    def __init__(self, disabled = False):
        global app_name
        if app_name == None:
            app_name = wx.GetApp().GetAppConfigName()
        wx.Config.__init__(self, app_name)
        self.disabled = disabled
        
    def Write(self, key, value):
        if self.disabled:
            return False
        return super().Write(key, value)