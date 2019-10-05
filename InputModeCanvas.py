from PropertiesCanvas import PropertiesCanvas
from PropertiesCanvas import PropertiesObserver
import cad
import ToolImage
import wx

EXTRA_TOOLBAR_HEIGHT = 7

class InputModeObserver(PropertiesObserver):
    def __init__(self, window):
        PropertiesObserver.__init__(self, window)
        
    def OnSelectionChanged(self, added, removed):
        self.window.objects = cad.GetSelectedObjects()
        #self.make_initial_properties_in_refresh = True
        self.window.RemoveAndAddAll()
        #self.make_initial_properties_in_refresh = False

    def OnModified(self, modified):
        self.window.RemoveAndAddAll()

# This is a property grid for listing a cad object's properties

class InputModeCanvas(PropertiesCanvas):
    def __init__(self, parent):
        PropertiesCanvas.__init__(self, parent)
        self.toolBar = None
        self.previous_tools = []
        self.inRemoveAndAddAll = False
        self.objects = []     
        #self.make_initial_properties_in_refresh = False
        self.observer = InputModeObserver(self)
        cad.RegisterObserver(self.observer)
        self.AddToolBar()
        self.Bind(wx.EVT_SIZE, self.OnSize)
        
    def AddToolBar(self):
        self.toolBar = wx.ToolBar(self.panel, style=wx.TB_NODIVIDER | wx.TB_FLAT)
        self.previous_tools = []
        self.toolBar.SetToolBitmapSize(wx.Size(ToolImage.GetBitmapSize(), ToolImage.GetBitmapSize()))
        self.toolBar.Realize()
        
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
        title = cad.PropertyStringReadOnly('Input Mode', cad.GetInputMode().GetTitle())
        # to do         if(wxGetApp().input_mode_object->TitleHighlighted())title->m_highlighted = true;
        properties.append(title)
        properties += cad.GetInputMode().GetProperties()

        for property in properties:
            self.AddProperty(property)
            
        # add toolbar buttons
        tools = wx.GetApp().GetInputModeTools()

        # compare to previous_list
#         tools_changed = False
#         if len(tools) != len(self.previous_tools):
#             tools_changed = True
#         else:
#             for pt, t in zip(self.previous_tools, tools):
#                 if t != pt:
#                     tools_changes = True
#                     break
                
#        if tools_changed:
        if True:
            
            self.toolBar.ClearTools()
            for tool in tools:
                image = wx.Image(tool.BitmapPath())
                image.Rescale(24, 24)
                button = self.toolBar.AddTool(wx.ID_ANY, tool.GetTitle(), wx.Bitmap(image))
                self.Bind(wx.EVT_TOOL, tool.Run, button)
                
            self.toolBar.Realize()
            
            self.SizeCode()
                
            self.previous_tools = tools
            
        self.inRemoveAndAddAll = False
        
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
    
        
       
        