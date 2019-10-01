from PropertiesCanvas import PropertiesCanvas
from PropertiesCanvas import PropertiesObserver
import cad
import ToolImage

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
        self.inRemoveAndAddAll = False
        self.objects = []     
        #self.make_initial_properties_in_refresh = False
        self.observer = InputModeObserver(self)
        cad.RegisterObserver(self.observer)
        
    def AddToolBar(self):
        self.toolBar = wx.ToolBar(self, style=wx.TB_NODIVIDER | wx.TB_FLAT)
        self.previous_tools = []
        self.toolBar.SetToolBitmapSize(wx.Size(ToolImage.GetBitmapsSize(), ToolImage.GetBitmapsSize()))
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
            
        # to do // compare to previous_list
        
        # to do if(tools_changed){// remake tool bar
            
        self.inRemoveAndAddAll = False
    
    def OnSize(self, event):
        size = self.GetClientSize()
        if self.toolBar.GetToolsCount() > 0:
            toolbar_size = self.toolBar.GetClientSize()
            toolbar_height = ToolImage.GetBitmapSize() + self.EXTRA_TOOLBAR_HEIGHT
            self.toolBar.SetSize(0,0,size.x, size.y - toolbar_height)
            self.toolBar.Show(False)
            
        event.Skip()
    
        
       
        