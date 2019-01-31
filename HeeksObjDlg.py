from HDialog import HDialog
from HDialog import HControl
from HDialog import control_border
import PictureFrame
import wx

class HeeksObjDlg(HDialog):
    def __init__(self, object, title = '', top_level = True, picture = True):
        HDialog.__init__(self, title)
        self.object = object
        self.leftControls = []
        self.rightControls = []
        self.picture = None
        
        if picture:
            self.picture = PictureFrame.PictureWindow(self, wx.Size(300, 200))
            pictureSizer = wx.BoxSizer(wx.VERTICAL)
            pictureSizer.Add(self.picture, 1, wx.GROW)
            self.rightControls.append(HControl(wx.ALL, sizer = pictureSizer))
        else:
            self.picture = None
            
        if top_level:
            self.AddControlsAndCreate()
            self.picture.SetFocus()
            
    def GetData(self, object):
        if self.ignore_event_functions:
            return
        self.ignore_event_functions = True
        self.GetDataRaw(object)
        self.ignore_event_functions = False        
        
    def SetFromData(self, object):
        save_ignore_event_functions = self.ignore_event_functions
        self.ignore_event_functions = True
        self.SetFromDataRaw(object)
        self.ignore_event_functions = False       
        
    def GetDataRaw(self, object):
        pass
        
    def SetFromDataRaw(self, object):
        pass
    
    def SetPictureByName(self, name):
        self.SetPictureByNameAndFolder(name, 'heeksobj')
        
    def SetPictureByNameAndFolder(self, name, folder):
        if self.picture:
            self.picture.SetPicture(wx.GetApp().cad_dir + '/bitmaps/' + folder + '/' + name + '.png')
            
    def SetPictureByWindow(self, w):
        self.SetPicture('general')
        
    def SetPicture(self):
        w = self.FindFocus()
        if self.picture:
            self.SetPictureByWindow(w)
            
    def OnChildFocus(self, event):
        if self.ignore_event_functions:
            return
        if event.GetWindow():
            self.SetPicture()
    
    def OnComboOrCheck(self, event):
        if self.ignore_event_functions:
            return
        self.SetPicture()
        
    def AddControlsAndCreate(self, object):
        self.ignore_event_functions = True
        sizerMain = wx.BoxSizer(wx.HORIZONTAL)
        
        # add left sizer
        sizerLeft = wx.BoxSizer(wx.VERTICAL)
        sizerMain.Add(sizerLeft, 0, wx.ALL, control_border)
        
        for control in self.leftControls:
            control.AddToSizer(sizerLeft)
            
        # add right sizer
        sizerRight = wx.BoxSizer(wx.VERTICAL)
        sizerMain.Add(sizerRight, 0, wx.ALL, control_border)
        
        # add OK and Cancel to right side
        self.rightControls.append(self.MakeOkAndCancel(wx.HORIZONTAL))
        
        # add right controls
        for control in self.rightControls:
            control.AddToSizer(sizerRight)
            
        self.SetFromData(object)
        
        self.SetSizer( sizerMain )
        sizerMain.SetSizeHints( self )
        sizerMain.Fit( self )
        
        self.ignore_event_functions = False
        
        self.Bind(wx.EVT_CHILD_FOCUS, self.OnChildFocus)
        
        self.SetPicture()
