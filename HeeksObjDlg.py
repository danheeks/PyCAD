from HDialog import HDialog
from HDialog import HControl
from HDialog import control_border
import PictureFrame
import wx

class HeeksObjDlg(HDialog):
    def __init__(self, object, title = '', add_picture = True):
        HDialog.__init__(self, title)
        self.object = object
        self.add_picture = add_picture
        self.picture = None
        
        sizerMain = wx.BoxSizer(wx.HORIZONTAL)
        
        # add left sizer
        self.sizerLeft = wx.BoxSizer(wx.VERTICAL)
        sizerMain.Add(self.sizerLeft, 0, wx.ALL, control_border)
            
        # add right sizer
        self.sizerRight = wx.BoxSizer(wx.VERTICAL)
        sizerMain.Add(self.sizerRight, 0, wx.ALL, control_border)
        
        self.ignore_event_functions = True
        
        self.AddLeftControls()

        if self.add_picture:
            self.AddPictureControl()
        self.AddRightControls()
            
        self.SetFromData()
        
        self.SetSizer( sizerMain )
        sizerMain.SetSizeHints( self )
        sizerMain.Fit( self )
        
        self.ignore_event_functions = False
        
        self.Bind(wx.EVT_CHILD_FOCUS, self.OnChildFocus)
        
        self.SetPicture()
        self.SetDefaultFocus()

    def AddLeftControls(self):
        pass
    
    def AddRightControls(self):            
        # add OK and Cancel to right side
        self.MakeOkAndCancel(wx.HORIZONTAL).AddToSizer(self.sizerRight)

    def SetDefaultFocus(self):
        if self.picture:
            self.picture.SetFocus()
            
    def GetData(self):
        if self.ignore_event_functions:
            return
        self.ignore_event_functions = True
        self.GetDataRaw()
        self.ignore_event_functions = False
        self.object.WriteDefaultValues()       
        
    def SetFromData(self):
        self.SetFromDataRaw()
        
    def GetDataRaw(self):
        pass
        
    def SetFromDataRaw(self):
        pass
    
    def SetPictureByName(self, name):
        self.SetPictureByNameAndFolder(name, 'heeksobj')
        
    def SetPictureByNameAndFolder(self, name, folder):
        if self.picture:
            self.picture.SetPicture(wx.GetApp().cad_dir + '/bitmaps/' + folder + '/' + name + '.png')
            
    def SetPictureByWindow(self, w):
        self.SetPictureByName('general')
        
    def SetPicture(self):
        w = self.FindFocus()
        if self.picture:
            self.SetPictureByWindow(w)
            
    def OnChildFocus(self, event):
        if self.ignore_event_functions:
            return
        if event.GetWindow():
            self.SetPicture()
        event.Skip()
    
    def OnComboOrCheck(self, event):
        if self.ignore_event_functions:
            return
        self.SetPicture()
        
    def AddPictureControl(self):
        self.picture = PictureFrame.PictureWindow(self, wx.Size(300, 200))
        pictureSizer = wx.BoxSizer(wx.VERTICAL)
        pictureSizer.Add(self.picture, 1, wx.GROW)
        HControl(wx.ALL, sizer = pictureSizer).AddToSizer(self.sizerRight)
        
