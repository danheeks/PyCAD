import wx
import wx.aui
from GraphicsCanvas import myGLCanvas

class Frame(wx.Frame):
    def __init__(self):
        wx.Frame.__init__(self, None, -1, 'HeeksCAM ( Computer Aided Manufacturing )')
        self.SetIcon(wx.Icon("HeeksCAD.ico", wx.BITMAP_TYPE_ICO))
        
        self.MakeMenus()
        
        self.aui_manager = wx.aui.AuiManager()
        self.aui_manager.SetManagedWindow(self)

        self.g = myGLCanvas(self)

        self.aui_manager.AddPane(self.g, wx.aui.AuiPaneInfo().Name('graphics').CentrePane().BestSize(wx.Size(800,600)))
        self.g2 = myGLCanvas(self)
        self.aui_manager.AddPane(self.g2, wx.aui.AuiPaneInfo().Name('Objects').Caption('Objects').Left().BestSize(wx.Size(300,200)).Position(0))

        self.Center()
        self.aui_manager.Update()
        
    def MakeMenus(self):
        self.menuBar = wx.MenuBar()
        file_menu = wx.Menu()
        self.Bind(wx.EVT_MENU, self.OnNew, file_menu.Append(wx.ID_ANY, 'New', 'Start a new job'))
        self.menuBar.Append(file_menu, '&File')
        self.SetMenuBar(self.menuBar)

        
    def OnNew(self, e):
        wx.MessageBox('OnNew called')
                                         
    


