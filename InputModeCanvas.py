from PropertiesCanvas import PropertiesCanvas
from PropertiesCanvas import PropertiesObserver
import cad
import ToolImage
import wx
import Key

EXTRA_TOOLBAR_HEIGHT = 7

class InputModeObserver(PropertiesObserver):
    def __init__(self, window):
        PropertiesObserver.__init__(self, window)
        
    def OnSelectionChanged(self, added, removed):
        self.window.objects = cad.GetSelectedObjects()
        #self.make_initial_properties_in_refresh = True
        self.window.RemoveAndAddAll()
        #self.make_initial_properties_in_refresh = False

# This is a property grid for listing a cad object's properties

class InputModeCanvas(PropertiesCanvas):
    def __init__(self, parent):
        PropertiesCanvas.__init__(self, parent)
        self.toolBar = None
        self.inRemoveAndAddAll = False
        self.objects = []     
        #self.make_initial_properties_in_refresh = False
        self.observer = InputModeObserver(self)
        cad.RegisterObserver(self.observer)
        self.AddToolBar()
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.pg.Bind( wx.EVT_KEY_DOWN, self.OnKeyDown )
        
    def AddToolBar(self):
        self.toolBar = wx.ToolBar(self.panel, style=wx.TB_NODIVIDER | wx.TB_FLAT)
        self.toolBar.SetToolBitmapSize(wx.Size(ToolImage.GetBitmapSize(), ToolImage.GetBitmapSize()))
        self.toolBar.Realize()
        
    def UpdateTitleProperty(self):
        p = wx.GetApp().frame.input_mode_canvas.pg.GetProperty('Input Mode')
        if p != None:
            p.SetValue(wx.GetApp().input_mode_object.GetTitle())
        wx.GetApp().Repaint()
        
    def RemoveAndAddAll(self):
        if self.in_OnPropGridChange:
            return
        
        if self.inRemoveAndAddAll:
            cad.MessageBox('recursion = RemoveAndAddAll')
            return
        self.inRemoveAndAddAll = True
        
        self.ClearProperties()
        
        properties = []
        
        # add the input_mode mode's properties
        title = cad.PropertyStringReadOnly('Input Mode', wx.GetApp().input_mode_object.GetTitle())
        # to do         if(wxGetApp().input_mode_object->TitleHighlighted())title->m_highlighted = true;
        properties.append(title)
        properties += wx.GetApp().input_mode_object.GetProperties()

        for property in properties:
            self.AddProperty(property)
            
        self.RecreateToolbar()
        
        self.SizeCode()
            
        self.inRemoveAndAddAll = False
        
    def RecreateToolbar(self):
        # add toolbar buttons
        tools = wx.GetApp().GetInputModeTools()
        
        self.toolBar.ClearTools()
        for tool in tools:
            image = wx.Image(tool.BitmapPath())
            image.Rescale(24, 24)
            button = self.toolBar.AddTool(wx.ID_ANY, tool.GetTitle(), wx.Bitmap(image))
            self.Bind(wx.EVT_TOOL, tool.Run, button)
            
        self.toolBar.Realize()
        
    def OnKeyDown(self, event):
        if not wx.GetApp().OnKeyDown(event):
            event.Skip()
            
    def SizeCode(self):
        size = self.GetClientSize()
        self.panel.SetSize(0,0,size.x, size.y)
        if self.toolBar.GetToolsCount() > 0:
            toolbar_size = self.toolBar.GetClientSize()
            toolbar_height = ToolImage.GetBitmapSize() + EXTRA_TOOLBAR_HEIGHT
            self.pg.SetSize(0, 0, size.x, size.y - toolbar_height)
            self.toolBar.SetSize(0, size.y - toolbar_height , size.x, toolbar_height )
            self.toolBar.Show()
        else:
            self.pg.SetSize(0,0, size.x, size.y)
            self.toolBar.Show(False)
    
    def OnSize(self, event):
        self.SizeCode()
    
        
       
        