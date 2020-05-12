from PropertiesCanvas import PropertiesCanvas
import cad

# This is a property grid for listing the app's options

class Options(PropertiesCanvas):
    def __init__(self, parent):
        PropertiesCanvas.__init__(self, parent)
        
        # permanently add all the options at the beginning
        options = wx.GetApp().GetOptions()
        for option in options:
            self.AddProperty(option)

        
