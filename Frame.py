import wx
import wx.aui
from GraphicsCanvas import GraphicsCanvas
from TreeCanvas import TreeCanvas
from PropertiesCanvas import PropertiesCanvas
from ObjPropsCanvas import ObjPropsCanvas
import cad
import sys
#sys.path.append("c:\wheremypluginis")

#import my_cool_plugin

class Frame(wx.Frame):
    def __init__(self):
        wx.Frame.__init__(self, None, -1, 'HeeksCAM ( Computer Aided Manufacturing )', size = wx.Size(400,400))
        self.SetIcon(wx.Icon("heekscad.png", wx.BITMAP_TYPE_PNG))
        
        self.MakeMenus()
        
        self.aui_manager = wx.aui.AuiManager()
        self.aui_manager.SetManagedWindow(self)

        self.graphics_canvas = GraphicsCanvas(self)

        self.aui_manager.AddPane(self.graphics_canvas, wx.aui.AuiPaneInfo().Name('graphics').CentrePane().BestSize(wx.Size(800,800)))
        self.tree_canvas = TreeCanvas(self)
        self.aui_manager.AddPane(self.tree_canvas, wx.aui.AuiPaneInfo().Name('Objects').Caption('Objects').Left().BestSize(wx.Size(300,400)).Position(0))
        self.properties_canvas = ObjPropsCanvas(self)
        self.aui_manager.AddPane(self.properties_canvas, wx.aui.AuiPaneInfo().Name('Properties').Caption('Properties').Left().BestSize(wx.Size(300,400)).Position(1))

        self.Center()
        self.aui_manager.Update()
        
    def MakeMenus(self):
        self.menuBar = wx.MenuBar()

        file_menu = wx.Menu()
        self.Bind(wx.EVT_MENU, self.OnNew, file_menu.Append(wx.ID_ANY, 'New', 'Start a new job'))
        self.Bind(wx.EVT_MENU, self.OnSolid, file_menu.Append(wx.ID_ANY, 'Solid', 'Import a test solid'))
        self.Bind(wx.EVT_MENU, self.OnDan, file_menu.Append(wx.ID_ANY, 'Dan', 'Import a test solid'))
        self.menuBar.Append(file_menu, '&File')

        edit_menu = wx.Menu()
        self.Bind(wx.EVT_MENU, self.OnUndo, edit_menu.Append(wx.ID_ANY, 'Undo', 'Undo last item'))
        self.Bind(wx.EVT_MENU, self.OnRedo, edit_menu.Append(wx.ID_ANY, 'Redo', 'Redo Next item'))
        self.menuBar.Append(edit_menu, '&Edit')

#        my_cool_plugin.AddMenus(self.menuBar)

        self.SetMenuBar(self.menuBar)

        
    def OnNew(self, e):
        wx.MessageBox('OnNew called')
        
    def OnSolid(self, e):
        cad.Import('cutsphere.stl')
        
    def OnDan(self, e):
        wx.MessageBox('Hello')
                                         
    def OnUndo(self, e):
        cad.RollBack()
        
    def OnRedo(self, e):
        cad.RollForward()


