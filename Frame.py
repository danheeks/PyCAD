import wx
import wx.aui
from GraphicsCanvas import GraphicsCanvas
from TreeCanvas import TreeCanvas
from PropertiesCanvas import PropertiesCanvas
from ObjPropsCanvas import ObjPropsCanvas
import cad
import sys

import os
pycad_dir = os.path.dirname(os.path.realpath(__file__))
        
class Frame(wx.Frame):
    def __init__(self, parent, id=-1, title='CAD ( Computer Aided Design )', pos=wx.DefaultPosition, size=wx.DefaultSize, style=wx.DEFAULT_FRAME_STYLE, name=wx.FrameNameStr):
        wx.Frame.__init__(self, parent, id, title, pos, size, style, name)
        self.SetIcon(wx.Icon(pycad_dir + "/heekscad.png", wx.BITMAP_TYPE_PNG))
        
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
        self.current_menu_stack = []

        file_menu = wx.Menu()
        self.Bind(wx.EVT_MENU, self.OnNew, file_menu.Append(wx.ID_ANY, 'New', 'Start a new job'))
        self.Bind(wx.EVT_MENU, self.OnSolid, file_menu.Append(wx.ID_ANY, 'Solid', 'Import a test solid'))
        self.Bind(wx.EVT_MENU, self.OnDan, file_menu.Append(wx.ID_ANY, 'Dan', 'Import a test solid'))
        self.menuBar.Append(file_menu, '&File')

        edit_menu = wx.Menu()
        self.Bind(wx.EVT_MENU, self.OnUndo, edit_menu.Append(wx.ID_ANY, 'Undo', 'Undo last item'))
        self.Bind(wx.EVT_MENU, self.OnRedo, edit_menu.Append(wx.ID_ANY, 'Redo', 'Redo Next item'))
        self.menuBar.Append(edit_menu, '&Edit')

        self.AddExtraMenus();

        self.SetMenuBar(self.menuBar)
        
    def AddExtraMenus(self):
        pass
    
    def CurrentMenu(self):
        if len(self.current_menu_stack) > 0:
            return self.current_menu_stack[-1]
        return None
    
    def SetMenuItemBitmap(self, item, bitmap_name):
        if bitmap_name:
            image = wx.Image(self.BitmapPath(bitmap_name))
            image.Rescale(24, 24)
            item.SetBitmap(wx.Bitmap(image))
            
    def BitmapPath(self, name):
        return pycad_dir + '/bitmaps/'+ name + '.png'
    
    def AddMenu(self, title, bitmap_name = None):
        menu = wx.Menu()
        current_menu = self.CurrentMenu()
        self.current_menu_stack.append(menu)
        if current_menu:
            item = wx.MenuItem(current_menu, wx.ID_ANY, title)
            item.SetSubMenu(menu)
            if bitmap_name != None:
                self.SetMenuItemBitmap(item, self.BitmapPath(bitmap_name))
            current_menu.Append(item)
        else:
            self.menuBar.Append(menu, title)
            
    def EndMenu(self):
        self.current_menu_stack.pop()
        
    def AddMenuItem(self, title, onButton, onUpdate = None, bitmap_name = None):
        item = wx.MenuItem(self.CurrentMenu(), wx.ID_ANY, title)        
        self.SetMenuItemBitmap(item, bitmap_name)
        self.Bind(wx.EVT_MENU, onButton, self.CurrentMenu().Append(item))        
    
    def OnNew(self, e):
        wx.MessageBox('OnNew called')
        
    def OnSolid(self, e):
        cad.Import(pycad_dir + '/cutsphere.stl')
        
    def OnDan(self, e):
        wx.MessageBox('Hello')
                                         
    def OnUndo(self, e):
        cad.RollBack()
        
    def OnRedo(self, e):
        cad.RollForward()


