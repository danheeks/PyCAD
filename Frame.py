import wx
import wx.aui
from GraphicsCanvas import GraphicsCanvas
from TreeCanvas import TreeCanvas
from InputModeCanvas import InputModeCanvas
from PropertiesCanvas import PropertiesCanvas
from ObjPropsCanvas import ObjPropsCanvas
from Ribbon import Ribbon
from HeeksConfig import HeeksConfig


class Frame(wx.Frame):
    def __init__(self, parent, id=-1, pos=wx.DefaultPosition, size=wx.DefaultSize, style=wx.DEFAULT_FRAME_STYLE, name=wx.FrameNameStr):
        wx.Frame.__init__(self, parent, id, '', pos, size, style, name)

        config = HeeksConfig()
        self.aui_manager = None
        self.windows_visible = {}
        
        self.SetIcon(wx.Icon(wx.GetApp().cad_dir + "/heekscad.png", wx.BITMAP_TYPE_PNG))
        
        self.aui_manager = wx.aui.AuiManager()
        self.aui_manager.SetManagedWindow(self)

        self.graphics_canvas = self.MakeGraphicsCanvas()

        self.aui_manager.AddPane(self.graphics_canvas, wx.aui.AuiPaneInfo().Name('graphics').CentrePane().BestSize(wx.Size(800,800)))
        self.tree_canvas = TreeCanvas(self)
        self.aui_manager.AddPane(self.tree_canvas, wx.aui.AuiPaneInfo().Name('Objects').Caption('Objects').Left().BestSize(wx.Size(300,400)).Position(0))
        
        self.input_mode_canvas = InputModeCanvas(self)
        self.aui_manager.AddPane(self.input_mode_canvas, wx.aui.AuiPaneInfo().Name('Input').Caption('Input').Left().BestSize(wx.Size(300,200)).Position(0))
        
        self.properties_canvas = ObjPropsCanvas(self)
        self.aui_manager.AddPane(self.properties_canvas, wx.aui.AuiPaneInfo().Name('Properties').Caption('Properties').Left().BestSize(wx.Size(300,200)).Position(2))
        
        wx.GetApp().RegisterHideableWindow(self.tree_canvas)
        wx.GetApp().RegisterHideableWindow(self.input_mode_canvas)
        wx.GetApp().RegisterHideableWindow(self.properties_canvas)
        #wx.GetApp().RegisterHideableWindow(self.ribbon)
        
        wx.GetApp().AddExtraWindows(self)
        
        self.ribbon = Ribbon(self)
        self.aui_manager.AddPane(self.ribbon, wx.aui.AuiPaneInfo().ToolbarPane().Name('Ribbon').Top().Movable(False).Gripper(False))

        perspective = config.Read('AuiPerspective', 'default')
        if perspective != 'default':
            self.aui_manager.LoadPerspective(perspective)
        self.ribbon.SetHeightAndImages()
        maximised = config.ReadBool('AuiMaximised', False)
        if maximised:
            self.Maximize()
        self.aui_manager.Update()
        
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_MOVE, self.OnMove)
        self.Bind(wx.EVT_CLOSE, self.OnClose)
        self.gears = []
        
    def MakeGraphicsCanvas(self):
        return GraphicsCanvas(self)
        
    def __del__(self):
        if self.aui_manager:
            str = self.aui_manager.SavePerspective()
            config = HeeksConfig()
            config.Write('AuiPerspective', str)
        
    def OnSize(self, e):
        size = e.GetSize()
        config = HeeksConfig()
        config.WriteInt('MainFrameWidth', size.GetWidth())
        config.WriteInt('MainFrameHeight', size.GetHeight())
        
    def OnMove(self, e):
        pos = self.GetPosition()
        config = HeeksConfig()
        config.WriteInt('MainFramePosX', pos.x)
        config.WriteInt('MainFramePosY', pos.y)
        
    def OnClose(self, e):
        if e.CanVeto() and wx.GetApp().CheckForModifiedDoc() == wx.CANCEL:
            e.Veto()
            return
        e.Skip()
        self.aui_manager.UnInit()
    
    def SetFrameTitle(self):
        s = wx.GetApp().GetAppTitle() + ' - '
        #s = self.GetTitle() + ' - '
        if wx.GetApp().filepath:
            s += wx.GetApp().filepath
        else:
            s += 'Untitled'
        self.SetTitle(s)
    
