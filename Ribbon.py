import wx
import wx.ribbon

class RibbonButtonData:
    def __init__(self, title, bitmap, caption, on_button, on_update_button = None, on_dropdown = None, key_flags = 0, key_code = 0):
        self.title = title
        self.bitmap = bitmap
        self.caption = caption
        self.on_button = on_button
        self.on_update_button = on_update_button
        self.on_dropdown = on_dropdown
        self.key_flags = key_flags
        self.key_code = key_code

class Ribbon(wx.ribbon.RibbonBar):
    def __init__(self, parent):
        self.next_id = parent.ID_NEXT_ID
        wx.ribbon.RibbonBar.__init__(self, parent, style = wx.ribbon.RIBBON_BAR_FLOW_HORIZONTAL | wx.ribbon.RIBBON_BAR_SHOW_PAGE_LABELS | wx.ribbon.RIBBON_BAR_SHOW_PANEL_EXT_BUTTONS | wx.ribbon.RIBBON_BAR_SHOW_HELP_BUTTON)
        
        main_page = wx.ribbon.RibbonPage(self, wx.ID_ANY, 'File', self.Image('new'))

        panel = wx.ribbon.RibbonPanel(main_page, wx.ID_ANY, 'File', self.Image('new'))
        toolbar = wx.ribbon.RibbonButtonBar(panel)
        self.AddToolBarTool(toolbar,RibbonButtonData('New', self.Image('new'), 'New File', parent.OnNew))
        self.AddToolBarTool(toolbar, RibbonButtonData("Open", self.Image("open"), "Open file", parent.OnOpen, None, self.OnOpenDropdown))
        self.AddToolBarTool(toolbar, RibbonButtonData("Import", self.Image("import"), "Import file", parent.OnImport));
        self.AddToolBarTool(toolbar, RibbonButtonData("Export", self.Image("export"), "Export file", parent.OnExport));
        self.AddToolBarTool(toolbar, RibbonButtonData("Save", self.Image("save"), "Save file", parent.OnSave, parent.OnUpdateSave));
        self.AddToolBarTool(toolbar, RibbonButtonData("Save As", self.Image("saveas"), "Save file with given name", parent.OnSaveAs));
        self.AddToolBarTool(toolbar, RibbonButtonData("Restore Defaults", self.Image("restore"), "Restore all defaults", parent.OnResetDefaults));
#        self.AddToolBarTool(toolbar, RibbonButtonData("About", self.Image("about"), "Software Information", parent.OnAbout));

        panel = wx.ribbon.RibbonPanel(main_page, wx.ID_ANY, 'Print', self.Image('print'))
        toolbar = wx.ribbon.RibbonButtonBar(panel)
        self.AddToolBarTool(toolbar, RibbonButtonData("Print", self.Image("print"), "Print the view to a printer", parent.OnPrint))
        self.AddToolBarTool(toolbar, RibbonButtonData("Page Setup", self.Image("psetup"), "Setup the printer layout", parent.OnPageSetup))
        self.AddToolBarTool(toolbar, RibbonButtonData("Print Preview", self.Image("ppreview"), "Show a preview of the print view", parent.OnPrintPreview))

        main_page.Realize()

        
        geom_page = wx.ribbon.RibbonPage(self, wx.ID_ANY, 'Geom', self.Image('lines'))
        
        panel = wx.ribbon.RibbonPanel(geom_page, wx.ID_ANY, 'Sketches', self.Image('lines'))
        toolbar = wx.ribbon.RibbonButtonBar(panel)
        self.AddToolBarTool(toolbar,RibbonButtonData('Lines', self.Image('lines'), 'Draw a sketch with lines and arcs', parent.OnLines))
        
        geom_page.Realize()
        
        self.Realize()
        
    def Image(self, name):
        image = wx.Image(self.GetParent().BitmapPath(name))
        image.Rescale(24, 24)
        return wx.Bitmap(image)
                        
    def MakeNextIDForTool(self, data):
        return wx.ID_ANY
                        
    def AddToolBarTool(self, toolbar, data):
        if data.on_dropdown != None:
            toolbar.AddHybridButton(self.next_id, data.title, data.bitmap, data.caption)
            toolbar.Bind(wx.ribbon.EVT_RIBBONBUTTONBAR_DROPDOWN_CLICKED, data.on_dropdown, id=self.next_id)
        else:
            toolbar.AddButton(self.next_id, data.title, data.bitmap, data.caption)
        toolbar.Bind(wx.ribbon.EVT_RIBBONBUTTONBAR_CLICKED, data.on_button, id=self.next_id)
        self.next_id += 1
        #self.AddAcceleratro
                        
    def SetHeightAndImages(self):
        height = self.GetBestHeight(2000)
        self.GetParent().aui_manager.GetPane(self).MinSize(wx.Size(-1, height))
        
    def OnRecentFile(self, event):
        index = event.GetId() - self.GetParent().ID_RECENT_FIRST
        file_path = wx.GetApp().recent_files[index]
        self.GetParent().OnOpenFilepath(file_path)
    
    def OnOpenDropdown(self, event):
        menu = wx.Menu()
        
        recent_id = self.GetParent().ID_RECENT_FIRST
        for filepath in wx.GetApp().recent_files:
            self.Bind(wx.EVT_MENU, self.OnRecentFile, menu.Append(recent_id, filepath))
            recent_id += 1
        event.PopupMenu(menu);
            