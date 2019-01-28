import wxHDialog
import PictureFrame

class HeeksObjDlg(HDialog):
    def __init__(self, object, title = '', top_level = True, picture = True):
        HDialog.__init__(self, title)
        
        if picture:
            self.picture = PictureFrame.PictureWindow(self, wx.Size(300, 200))
            pictureSizer = wx.BoxSizer(wx.VERTICAL)
            pictureSizer.Add(self.picture, 1, wx.GROW)
            self.rightControls.apend(
        
    def SetPicture(self):
        w = self.FindFocus()
        if self.picture:
            self.SetPictureBtWindow(s)
            