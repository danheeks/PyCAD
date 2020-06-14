import wx
import wx.ribbon
import cad

class RibbonButtonData:
    def __init__(self, title, bitmap, caption, on_button, on_update_button = None, on_dropdown = None, key_flags = 0, key_code = 0):
        self.title = title
        self.bitmap = bitmap
        self.caption = caption
        self.on_button = on_button
        self.on_update_button = on_update_button
        self.on_dropdown = on_dropdown

class Ribbon(wx.ribbon.RibbonBar):
    def __init__(self, parent):
        self.next_id = parent.ID_NEXT_ID
        wx.ribbon.RibbonBar.__init__(self, parent, style = wx.ribbon.RIBBON_BAR_FLOW_HORIZONTAL | wx.ribbon.RIBBON_BAR_SHOW_PAGE_LABELS | wx.ribbon.RIBBON_BAR_SHOW_PANEL_EXT_BUTTONS | wx.ribbon.RIBBON_BAR_SHOW_HELP_BUTTON)
        self.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        
        main_page = wx.ribbon.RibbonPage(self, wx.ID_ANY, 'File')
        main_page.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)

        panel = wx.ribbon.RibbonPanel(main_page, wx.ID_ANY, 'File', self.Image('new'))
        toolbar = wx.ribbon.RibbonButtonBar(panel)
        self.AddToolBarTool(toolbar,RibbonButtonData('New', self.Image('new'), 'New File', parent.OnNew))
        self.AddToolBarTool(toolbar, RibbonButtonData("Open", self.Image("open"), "Open file", parent.OnOpen, None, self.OnOpenDropdown))
        self.AddToolBarTool(toolbar, RibbonButtonData("Import", self.Image("import"), "Import file", parent.OnImport));
        self.AddToolBarTool(toolbar, RibbonButtonData("Export", self.Image("export"), "Export file", parent.OnExport));
        self.AddToolBarTool(toolbar, RibbonButtonData("Save", self.Image("save"), "Save file", parent.OnSave, parent.OnUpdateSave));
        self.AddToolBarTool(toolbar, RibbonButtonData("Save As", self.Image("saveas"), "Save file with given name", parent.OnSaveAs));
        self.AddToolBarTool(toolbar, RibbonButtonData("Restore Defaults", self.Image("restore"), "Restore all defaults", parent.OnResetDefaults));
        self.AddToolBarTool(toolbar, RibbonButtonData("About", self.Image("about"), "Software Information", parent.OnAbout));

        panel = wx.ribbon.RibbonPanel(main_page, wx.ID_ANY, 'Print', self.Image('print'))
        toolbar = wx.ribbon.RibbonButtonBar(panel)
        self.AddToolBarTool(toolbar, RibbonButtonData("Print", self.Image("print"), "Print the view to a printer", parent.OnPrint))
        self.AddToolBarTool(toolbar, RibbonButtonData("Page Setup", self.Image("psetup"), "Setup the printer layout", parent.OnPageSetup))
        self.AddToolBarTool(toolbar, RibbonButtonData("Print Preview", self.Image("ppreview"), "Show a preview of the print view", parent.OnPrintPreview))

        panel = wx.ribbon.RibbonPanel(main_page, wx.ID_ANY, 'Edit', self.Image('cut'))
        toolbar = wx.ribbon.RibbonButtonBar(panel)
        self.AddToolBarTool(toolbar, RibbonButtonData("Undo", self.Image('undo'), 'Undo the previous command', parent.OnUndo, parent.OnUpdateUndo, None))
        self.AddToolBarTool(toolbar, RibbonButtonData("Redo", self.Image('redo'), 'Redo the next command', parent.OnRedo, parent.OnUpdateRedo, None))
        self.AddToolBarTool(toolbar, RibbonButtonData("Cut", self.Image('cut'), 'Cut selected items to the clipboard', parent.OnCut, parent.OnUpdateCut))
        self.AddToolBarTool(toolbar, RibbonButtonData("Copy", self.Image('copy'), 'Copy selected items to the clipboard', parent.OnCopy, parent.OnUpdateCopy))
        self.AddToolBarTool(toolbar, RibbonButtonData("Paste", self.Image('paste'), 'Paste items from the clipboard', parent.OnPaste, parent.OnUpdatePaste))
        self.AddToolBarTool(toolbar, RibbonButtonData("Delete", self.Image('delete'), 'Delete selected items', parent.OnDelete, parent.OnUpdateDelete))
        self.AddToolBarTool(toolbar, RibbonButtonData("Select", self.Image('select'), 'Select Mode', parent.OnSelectMode))

        main_page.Realize()

        
        geom_page = wx.ribbon.RibbonPage(self, wx.ID_ANY, 'Geom')
        geom_page.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        
        panel = wx.ribbon.RibbonPanel(geom_page, wx.ID_ANY, 'Sketches', self.Image('lines'))
        toolbar = wx.ribbon.RibbonButtonBar(panel)
        self.AddToolBarTool(toolbar,RibbonButtonData('Lines', self.Image('lines'), 'Draw a sketch with lines and arcs', parent.OnLines))
        self.AddToolBarTool(toolbar,RibbonButtonData('Rectangles', self.Image('rect'), 'Draw rectangles', parent.OnLines))
        self.AddToolBarTool(toolbar,RibbonButtonData('Obrounds', self.Image('obround'), 'Draw obrounds', parent.OnLines))
        self.AddToolBarTool(toolbar,RibbonButtonData('Polygons', self.Image('pentagon'), 'Draw polygons', parent.OnLines))
        self.AddToolBarTool(toolbar,RibbonButtonData('Gear', self.Image('gear'), 'Add a gear', parent.OnGear))
        
        panel = wx.ribbon.RibbonPanel(geom_page, wx.ID_ANY, 'Circles', self.Image('circ3p'))
        toolbar = wx.ribbon.RibbonButtonBar(panel)
        self.AddToolBarTool(toolbar,RibbonButtonData('3 Points', self.Image('circ3p'), 'Draw circles through 3 points', parent.OnCircles3p))
        self.AddToolBarTool(toolbar,RibbonButtonData('2 Points', self.Image('circ2p'), 'Draw circles, centre and point', parent.OnCircles2p))
        self.AddToolBarTool(toolbar,RibbonButtonData('Radius', self.Image('circpr'), 'Draw circles, centre and radius', parent.OnCircles1p))
        
        panel = wx.ribbon.RibbonPanel(geom_page, wx.ID_ANY, 'OtherDrawing', self.Image('point'))
        toolbar = wx.ribbon.RibbonButtonBar(panel)
        self.AddToolBarTool(toolbar,RibbonButtonData('Infinite Line', self.Image('iline'), 'Drawing Infinite Lines', parent.OnILine))
        self.AddToolBarTool(toolbar,RibbonButtonData('Points', self.Image('point'), 'Drawing Points', parent.OnPoints))
        self.AddToolBarTool(toolbar,RibbonButtonData('Spline', self.Image('splpts'), 'Spline Through Points', parent.OnGear))
        
        panel = wx.ribbon.RibbonPanel(geom_page, wx.ID_ANY, 'Text', self.Image('text'))
        toolbar = wx.ribbon.RibbonButtonBar(panel)
        self.AddToolBarTool(toolbar,RibbonButtonData('Text', self.Image('text'), 'Add a text object', parent.OnText))
        self.AddToolBarTool(toolbar,RibbonButtonData('Dimensioning', self.Image('dimension'), 'Add a dimension', parent.OnDimensioning))
        
        panel = wx.ribbon.RibbonPanel(geom_page, wx.ID_ANY, 'Transformations', self.Image('movet'))
        toolbar = wx.ribbon.RibbonButtonBar(panel)
        self.AddToolBarTool(toolbar,RibbonButtonData('Move Translate', self.Image('movet'), 'Translate selected items', parent.OnMoveTranslate))
        self.AddToolBarTool(toolbar,RibbonButtonData('Copy Translate', self.Image('copyt'), 'Copy and translate selected items', parent.OnCopyTranslate))
        self.AddToolBarTool(toolbar,RibbonButtonData('Move Rotate', self.Image('mover'), 'Rotate selected items', parent.OnMoveRotate))
        self.AddToolBarTool(toolbar,RibbonButtonData('Copy Rotate', self.Image('copyr'), 'Copy and rotate selected items', parent.OnCopyRotate))
        self.AddToolBarTool(toolbar,RibbonButtonData('Move Mirror', self.Image('copym'), 'Mirror selected items', parent.OnMirror))
        self.AddToolBarTool(toolbar,RibbonButtonData('Move Scale', self.Image('moves'), 'Scale selected items', parent.OnMoveScale))
        self.AddToolBarTool(toolbar,RibbonButtonData('Move Origin', self.Image('coords3'), 'Move selected items from one origin to another', parent.OnOriginTransform))
        
        panel = wx.ribbon.RibbonPanel(geom_page, wx.ID_ANY, 'Snapping', self.Image('endof'))
        self.snapping_toolbar = wx.ribbon.RibbonButtonBar(panel)
        self.endof_button = self.AddToolBarTool(self.snapping_toolbar,RibbonButtonData('Endof', self.Image('endof' if cad.GetDigitizeEnd() else 'endofgray'), 'Snap to end point', self.OnEndof))
        self.inters_button = self.AddToolBarTool(self.snapping_toolbar,RibbonButtonData('Inters', self.Image('inters' if cad.GetDigitizeInters() else 'intersgray'), 'Snap to intersection', self.OnInters))
        self.centre_button = self.AddToolBarTool(self.snapping_toolbar,RibbonButtonData('Centre', self.Image('centre' if cad.GetDigitizeCentre() else 'centregray'), 'Snap to centre', self.OnCentre))
        self.midpoint_button = self.AddToolBarTool(self.snapping_toolbar,RibbonButtonData('Midpoint', self.Image('midpoint' if cad.GetDigitizeMidpoint() else 'midpointgray'), 'Snap to midpoint', self.OnMidpoint))
        self.grid_button = self.AddToolBarTool(self.snapping_toolbar,RibbonButtonData('Grid', self.Image('snap' if cad.GetDigitizeSnapToGrid() else 'snapgray'), 'Snap to grid', self.OnSnap, None, self.OnSnapDropdown if cad.GetDigitizeSnapToGrid() else None))

        geom_page.Realize()
        

        view_page = wx.ribbon.RibbonPage(self, wx.ID_ANY, 'View')
        view_page.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        
        panel = wx.ribbon.RibbonPanel(view_page, wx.ID_ANY, 'Magnify', self.Image('mag'))
        toolbar = wx.ribbon.RibbonButtonBar(panel)
        self.AddToolBarTool(toolbar,RibbonButtonData('Mag Extents', self.Image('magextents'), 'Zoom in to fit the extents of the drawing into the graphics window', parent.OnMagExtents))
        self.AddToolBarTool(toolbar,RibbonButtonData('Mag No Rotation', self.Image('magnorot'), 'Zoom in to fit the extents of the drawing into the graphics window, but without rotating the view', parent.OnMagNoRot))
        self.AddToolBarTool(toolbar,RibbonButtonData('Zoom Window', self.Image('mag'), 'Zoom in to a dragged window', parent.OnMag))
        
        panel = wx.ribbon.RibbonPanel(view_page, wx.ID_ANY, 'General', self.Image('fullscreen'))
        toolbar = wx.ribbon.RibbonButtonBar(panel)
        self.AddToolBarTool(toolbar,RibbonButtonData('View Back', self.Image('magprev'), 'Go back to previous view', parent.OnMagPrevious))
        self.AddToolBarTool(toolbar,RibbonButtonData('FullScreen', self.Image('fullscreen'), 'Switch to full screen view ( press escape to return )', parent.OnFullScreen))
        self.AddToolBarTool(toolbar,RibbonButtonData('Redraw', self.Image('redraw'), 'Redraw', parent.OnRedraw))
        
        panel = wx.ribbon.RibbonPanel(view_page, wx.ID_ANY, 'Specific Views', self.Image('magxy'))
        toolbar = wx.ribbon.RibbonButtonBar(panel)
        self.AddToolBarTool(toolbar,RibbonButtonData('View XY Front', self.Image('magxy'), 'View XY Front', parent.OnMagXY))
        self.AddToolBarTool(toolbar,RibbonButtonData('View XY Back', self.Image('magxym'), 'View XY Back', parent.OnMagXYM))
        self.AddToolBarTool(toolbar,RibbonButtonData('View XZ Top', self.Image('magxz'), 'View XZ Top', parent.OnMagXZ))
        self.AddToolBarTool(toolbar,RibbonButtonData('View XZ Bottom', self.Image('magxzm'), 'View XZ Bottom', parent.OnMagXZM))
        self.AddToolBarTool(toolbar,RibbonButtonData('View YZ Right', self.Image('magyz'), 'View YZ Right', parent.OnMagYZ))
        self.AddToolBarTool(toolbar,RibbonButtonData('View YZ Left', self.Image('magyzm'), 'View YZ Left', parent.OnMagYZM))
        self.AddToolBarTool(toolbar,RibbonButtonData('View XY Isometric', self.Image('magxyz'), 'View XY Isometric', parent.OnMagXYZ))
        
        panel = wx.ribbon.RibbonPanel(view_page, wx.ID_ANY, 'View Dragging', self.Image('viewrot'))
        toolbar = wx.ribbon.RibbonButtonBar(panel)
        self.AddToolBarTool(toolbar,RibbonButtonData('View Rotate', self.Image('viewrot'), 'Enter view rotating mode', parent.OnViewRotate))
        self.AddToolBarTool(toolbar,RibbonButtonData('View Zoom', self.Image('zoom'), 'Drag to zoom in and out', parent.OnViewZoom))
        self.AddToolBarTool(toolbar,RibbonButtonData('View Pan', self.Image('pan'), 'Drag to move view', parent.OnViewPan))

        panel = wx.ribbon.RibbonPanel(view_page, wx.ID_ANY, 'Windows', self.Image('viewrot'))
        self.windows_toolbar = wx.ribbon.RibbonButtonBar(panel)
        objects_button_is_ticked = parent.aui_manager.GetPane(parent.tree_canvas).IsShown()
        properties_button_is_ticked = parent.aui_manager.GetPane(parent.properties_canvas).IsShown()
        self.objects_button = self.AddToolBarTool(self.windows_toolbar, RibbonButtonData('Objects', self.Image('objects' if objects_button_is_ticked else 'objectsgray'), 'Objects', self.OnObjects))
        self.properties_button = self.AddToolBarTool(self.windows_toolbar, RibbonButtonData('Properties', self.Image('properties' if objects_button_is_ticked else 'propertiesgray'), 'Properties', self.OnProperties))

        view_page.Realize()


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
        id_to_return = self.next_id
        self.next_id += 1
        return id_to_return
                        
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
        
    def OnKeyDown(self, event):
        wx.GetApp().OnKeyDown(event)
    
    def OnEndof(self, e):
        cad.SetDigitizeEnd( not cad.GetDigitizeEnd() )
        self.snapping_toolbar.DeleteButton(self.endof_button)
        self.snapping_toolbar.InsertButton(0, self.endof_button, 'Endof', self.Image('endof' if cad.GetDigitizeEnd() else 'endofgray'), 'Snap to end point')
    
    def OnInters(self, e):
        cad.SetDigitizeInters( not cad.GetDigitizeInters() )
        self.snapping_toolbar.DeleteButton(self.inters_button)
        self.snapping_toolbar.InsertButton(1, self.inters_button, 'Inters', self.Image('inters' if cad.GetDigitizeInters() else 'intersgray'), 'Snap to intersection')
    
    def OnCentre(self, e):
        cad.SetDigitizeCentre( not cad.GetDigitizeCentre() )
        self.snapping_toolbar.DeleteButton(self.centre_button)
        self.snapping_toolbar.InsertButton(2, self.centre_button, 'Centre', self.Image('centre' if cad.GetDigitizeCentre() else 'centregray'), 'Snap to centre')
    
    def OnMidpoint(self, e):
        cad.SetDigitizeMidpoint( not cad.GetDigitizeMidpoint() )
        self.snapping_toolbar.DeleteButton(self.midpoint_button)
        self.snapping_toolbar.InsertButton(3, self.midpoint_button, 'Midpoint', self.Image('midpoint' if cad.GetDigitizeMidpoint() else 'midpointgray'), 'Snap to mid point')
    
    def OnSnap(self, e):
        cad.SetDigitizeSnapToGrid( not cad.GetDigitizeSnapToGrid() )
        self.snapping_toolbar.DeleteButton(self.grid_button)
        if cad.GetDigitizeSnapToGrid():
            self.snapping_toolbar.InsertHybridButton(4, self.grid_button, 'Grid', self.Image('snap'), 'Snap to Grid')
        else:
            self.snapping_toolbar.InsertButton(4, self.grid_button, 'Grid', self.Image('snapgray'), 'Snap to Grid')

    def OnSnapDropdown(self, e):
        if cad.GetDigitizeSnapToGrid():
            cad.SetDigitizeGridSize(wx.GetApp().InputLength('Edit Snap Grid', 'Snap Grid', cad.GetDigitizeGridSize()))

    def OnObjects(self, e):
        f = self.GetParent()
        pane_info = f.aui_manager.GetPane(f.tree_canvas)
        is_shown = pane_info.IsShown()
        
        if pane_info.IsOk():
            pane_info.Show(not is_shown)
            is_shown = not is_shown
            f.aui_manager.Update()
            
        f.ribbon.SetObjectsButtonImage()
        
    def SetObjectsButtonImage(self, turned_off = False):
        pane_info = self.GetParent().aui_manager.GetPane(self.GetParent().tree_canvas)
        is_shown = pane_info.IsShown()
        if turned_off:
            is_shown = False
            self.object_button_is_ticked = is_shown
            self.windows_toolbar.DeleteButton(self.objects_button)
            self.windows_toolbar.InsertButton(0, self.objects_button, 'Objects', self.Image('objects' if is_shown else 'objectsgray'), 'Objects')
            self.windows_toolbar.Realize()
            
    def OnProperties(self, e):
        f = self.GetParent()
        pane_info = f.aui_manager.GetPane(f.properties_canvas)
        is_shown = pane_info.IsShown()
        
        if pane_info.IsOk():
            pane_info.Show(not is_shown)
            is_shown = not is_shown
            f.aui_manager.Update()
            
        f.ribbon.SetPropertiesButtonImage()
        
    def SetPropertiesButtonImage(self, turned_off = False):
        pane_info = self.GetParent().aui_manager.GetPane(self.GetParent().properties_canvas)
        is_shown = pane_info.IsShown()
        if turned_off:
            is_shown = False
            self.properties_button_is_ticked = is_shown
            self.windows_toolbar.DeleteButton(self.properties_button)
            self.windows_toolbar.InsertButton(0, self.properties_button, 'Properties', self.Image('properties' if is_shown else 'propertiesgray'), 'Properties')
            self.windows_toolbar.Realize()            