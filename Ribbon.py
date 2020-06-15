import wx
import wx.ribbon as RB
import cad
from HeeksConfig import HeeksConfig

control_border = 2

class OnOffButton:
    def __init__(self, name, bitmap_name, bitmap_name_off, get_fn, set_fn, help_str, drop_down = None):
        self.name = name
        self.bitmap_name = bitmap_name
        self.bitmap_name_off = bitmap_name_off
        self.get_fn = get_fn
        self.set_fn = set_fn
        self.help_str = help_str
        self.drop_down = drop_down
        self.index = None
        
    def AddToToolbar(self, toolbar):
        on = self.get_fn()
        bm = self.bitmap_name if on else self.bitmap_name_off
        self.index = toolbar.GetButtonCount()
        Ribbon.AddToolBarTool(toolbar, self.name, bm, self.help_str, self.OnButton, None, self.drop_down)       
            
    def OnButton(self, event):
        on = self.get_fn()
        on = not on
        self.set_fn(on)

        # remember config
        config = HeeksConfig()
        config.WriteBool(self.name, on)
        
        bm = self.bitmap_name if on else self.bitmap_name_off
        toolbar = event.GetBar()
        button = event.GetButton()
        ribbon = toolbar.GetParent().GetParent().GetParent()
        bitmap = ribbon.Image(bm)
        item_id = toolbar.GetItemId(button)
        toolbar.SetButtonIcon(item_id, bitmap)
        
class BackgroundColorButton:
    def __init__(self, name, help_str):
        self.name = name
        self.help_str = help_str
        self.index = None
        self.ribbon = None
        
    def AddToToolbar(self, toolbar):
        self.index = toolbar.GetButtonCount()
        self.ribbon = toolbar.GetParent().GetParent().GetParent()
        Ribbon.AddToolBarTool2(toolbar, self.name, self.ColorBitmap(), self.help_str, self.OnButton)       
        
    def ColorBitmap(self):
        color = cad.GetBackgroundColor(self.index)
        bitmap = wx.Bitmap(24, 24)
        dc = wx.MemoryDC(bitmap)
        dc.SetBrush(wx.Brush(wx.Colour(color.red, color.green, color.blue)))
        dc.DrawRectangle(0,0,24,24)
        del dc
        return bitmap
        
    def OnButton(self, event):
        self.SelectNewColour(self.index)

        toolbar = event.GetBar()
        button = event.GetButton()
        item_id = toolbar.GetItemId(button)
        toolbar.SetButtonIcon(item_id, self.ColorBitmap())
        
    def SetBackgroundColor(self, index, wxcolor):
        c = cad.Color()
        c.red = wxcolor.red
        c.green = wxcolor.green
        c.blue = wxcolor.blue
        cad.SetBackgroundColor(index, c)
        self.ribbon.GetParent().graphics_canvas.Refresh()

    def SelectNewColour(self, index):
        c = cad.GetBackgroundColor(index)
        data = wx.ColourData()
        data.SetColour(wx.Colour(c.red, c.green, c.blue))
        dlg = wx.ColourDialog(self.ribbon, data)
        if dlg.ShowModal() == wx.ID_OK:
            # Colour did change.
            self.SetBackgroundColor(index, dlg.GetColourData().GetColour())
            HeeksConfig().WriteInt("BackgroundColor" + str(index), cad.GetBackgroundColor(index).ref())
            self.ribbon.GetParent().graphics_canvas.Refresh()


class Ribbon(RB.RibbonBar):
    next_id = 0
    
    def __init__(self, parent):
        RB.RibbonBar.__init__(self, parent, style = RB.RIBBON_BAR_FLOW_HORIZONTAL | RB.RIBBON_BAR_SHOW_PAGE_LABELS | RB.RIBBON_BAR_SHOW_PAGE_ICONS | RB.RIBBON_BAR_SHOW_PANEL_EXT_BUTTONS | RB.RIBBON_BAR_SHOW_HELP_BUTTON)
        Ribbon.next_id = parent.ID_NEXT_ID
        self.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        
        main_page = RB.RibbonPage(self, wx.ID_ANY, 'File', self.Image('file'))
        main_page.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)

        panel = RB.RibbonPanel(main_page, wx.ID_ANY, 'File', self.Image('new'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar, 'New', 'new', 'New File', parent.OnNew)
        Ribbon.AddToolBarTool(toolbar, "Open", 'open', "Open file", parent.OnOpen, None, self.OnOpenDropdown)
        Ribbon.AddToolBarTool(toolbar, "Import", 'import', "Import file", parent.OnImport)
        Ribbon.AddToolBarTool(toolbar, "Export", 'export', "Export file", parent.OnExport)
        Ribbon.AddToolBarTool(toolbar, "Save", 'save', "Save file", parent.OnSave, parent.OnUpdateSave)
        Ribbon.AddToolBarTool(toolbar, "Save As", 'saveas', "Save file with given name", parent.OnSaveAs)
        Ribbon.AddToolBarTool(toolbar, "Restore Defaults", 'restore', "Restore all defaults", parent.OnResetDefaults)
        Ribbon.AddToolBarTool(toolbar, "About", 'about', "Software Information", parent.OnAbout)

        panel = RB.RibbonPanel(main_page, wx.ID_ANY, 'Print', self.Image('print'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar, "Print", 'print', "Print the view to a printer", parent.OnPrint)
        Ribbon.AddToolBarTool(toolbar, "Page Setup", 'psetup', "Setup the printer layout", parent.OnPageSetup)
        Ribbon.AddToolBarTool(toolbar, "Print Preview", 'ppreview', "Show a preview of the print view", parent.OnPrintPreview)

        panel = RB.RibbonPanel(main_page, wx.ID_ANY, 'Edit', self.Image('cut'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar, "Undo", 'undo', 'Undo the previous command', parent.OnUndo, parent.OnUpdateUndo, None)
        Ribbon.AddToolBarTool(toolbar, "Redo", 'redo', 'Redo the next command', parent.OnRedo, parent.OnUpdateRedo, None)
        Ribbon.AddToolBarTool(toolbar, "Cut", 'cut', 'Cut selected items to the clipboard', parent.OnCut, parent.OnUpdateCut)
        Ribbon.AddToolBarTool(toolbar, "Copy", 'copy', 'Copy selected items to the clipboard', parent.OnCopy, parent.OnUpdateCopy)
        Ribbon.AddToolBarTool(toolbar, "Paste", 'paste', 'Paste items from the clipboard', parent.OnPaste, parent.OnUpdatePaste)
        Ribbon.AddToolBarTool(toolbar, "Delete", 'delete', 'Delete selected items', parent.OnDelete, parent.OnUpdateDelete)
        Ribbon.AddToolBarTool(toolbar, "Select", 'select', 'Select Mode', parent.OnSelectMode)

        main_page.Realize()

        
        geom_page = RB.RibbonPage(self, wx.ID_ANY, 'Geom', self.Image('geom'))
        geom_page.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        
        panel = RB.RibbonPanel(geom_page, wx.ID_ANY, 'Sketches', self.Image('lines'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar,'Lines', 'lines', 'Draw a sketch with lines and arcs', parent.OnLines)
        Ribbon.AddToolBarTool(toolbar,'Rectangles', 'rect', 'Draw rectangles', parent.OnLines)
        Ribbon.AddToolBarTool(toolbar,'Obrounds', 'obround', 'Draw obrounds', parent.OnLines)
        Ribbon.AddToolBarTool(toolbar,'Polygons', 'pentagon', 'Draw polygons', parent.OnLines)
        Ribbon.AddToolBarTool(toolbar,'Gear', 'gear', 'Add a gear', parent.OnGear)
        
        panel = RB.RibbonPanel(geom_page, wx.ID_ANY, 'Circles', self.Image('circ3p'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar,'3 Points', 'circ3p', 'Draw circles through 3 points', parent.OnCircles3p)
        Ribbon.AddToolBarTool(toolbar,'2 Points', 'circ2p', 'Draw circles, centre and point', parent.OnCircles2p)
        Ribbon.AddToolBarTool(toolbar,'Radius', 'circpr', 'Draw circles, centre and radius', parent.OnCircles1p)
        
        panel = RB.RibbonPanel(geom_page, wx.ID_ANY, 'OtherDrawing', self.Image('point'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar,'Infinite Line', 'iline', 'Drawing Infinite Lines', parent.OnILine)
        Ribbon.AddToolBarTool(toolbar,'Points', 'point', 'Drawing Points', parent.OnPoints)
        Ribbon.AddToolBarTool(toolbar,'Spline', 'splpts', 'Spline Through Points', parent.OnGear)
        
        panel = RB.RibbonPanel(geom_page, wx.ID_ANY, 'Text', self.Image('text'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar,'Text', 'text', 'Add a text object', parent.OnText)
        Ribbon.AddToolBarTool(toolbar,'Dimensioning', 'dimension', 'Add a dimension', parent.OnDimensioning)
        
        panel = RB.RibbonPanel(geom_page, wx.ID_ANY, 'Transformations', self.Image('movet'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar,'Move Translate', 'movet', 'Translate selected items', parent.OnMoveTranslate)
        Ribbon.AddToolBarTool(toolbar,'Copy Translate', 'copyt', 'Copy and translate selected items', parent.OnCopyTranslate)
        Ribbon.AddToolBarTool(toolbar,'Move Rotate', 'mover', 'Rotate selected items', parent.OnMoveRotate)
        Ribbon.AddToolBarTool(toolbar,'Copy Rotate', 'copyr', 'Copy and rotate selected items', parent.OnCopyRotate)
        Ribbon.AddToolBarTool(toolbar,'Move Mirror', 'copym', 'Mirror selected items', parent.OnMirror)
        Ribbon.AddToolBarTool(toolbar,'Move Scale', 'moves', 'Scale selected items', parent.OnMoveScale)
        Ribbon.AddToolBarTool(toolbar,'Move Origin', 'coords3', 'Move selected items from one origin to another', parent.OnOriginTransform)
        
        panel = RB.RibbonPanel(geom_page, wx.ID_ANY, 'Snapping', self.Image('endof'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddOnOffButton(toolbar, 'Endof', 'endof', 'endofgray', cad.GetDigitizeEnd, cad.SetDigitizeEnd, 'Snap to end point')
        Ribbon.AddOnOffButton(toolbar, 'Inters', 'inters', 'intersgray', cad.GetDigitizeInters, cad.SetDigitizeInters, 'Snap to intersection')
        Ribbon.AddOnOffButton(toolbar, 'Centre', 'centre', 'centregray', cad.GetDigitizeCentre, cad.SetDigitizeCentre, 'Snap to centre')
        Ribbon.AddOnOffButton(toolbar, 'Midpoint', 'midpoint', 'midpointgray', cad.GetDigitizeMidpoint, cad.SetDigitizeMidpoint, 'Snap to midpoint')
        Ribbon.AddOnOffButton(toolbar, 'Grid', 'snap', 'snapgray', cad.GetDigitizeSnapToGrid, cad.SetDigitizeSnapToGrid, 'Snap to grid', self.OnSnapDropdown)

        geom_page.Realize()
        

        view_page = RB.RibbonPage(self, wx.ID_ANY, 'View', self.Image('view'))
        view_page.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        
        panel = RB.RibbonPanel(view_page, wx.ID_ANY, 'Magnify', self.Image('mag'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar,'Mag Extents', 'magextents', 'Zoom in to fit the extents of the drawing into the graphics window', parent.OnMagExtents)
        Ribbon.AddToolBarTool(toolbar,'Mag No Rotation', 'magnorot', 'Zoom in to fit the extents of the drawing into the graphics window, but without rotating the view', parent.OnMagNoRot)
        Ribbon.AddToolBarTool(toolbar,'Zoom Window', 'mag', 'Zoom in to a dragged window', parent.OnMag)
        
        panel = RB.RibbonPanel(view_page, wx.ID_ANY, 'General', self.Image('fullscreen'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar,'View Back', 'magprev', 'Go back to previous view', parent.OnMagPrevious)
        Ribbon.AddToolBarTool(toolbar,'FullScreen', 'fullscreen', 'Switch to full screen view ( press escape to return )', parent.OnFullScreen)
        Ribbon.AddToolBarTool(toolbar,'Redraw', 'redraw', 'Redraw', parent.OnRedraw)
        
        panel = RB.RibbonPanel(view_page, wx.ID_ANY, 'Specific Views', self.Image('magxy'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar,'View XY Front', 'magxy', 'View XY Front', parent.OnMagXY)
        Ribbon.AddToolBarTool(toolbar,'View XY Back', 'magxym', 'View XY Back', parent.OnMagXYM)
        Ribbon.AddToolBarTool(toolbar,'View XZ Top', 'magxz', 'View XZ Top', parent.OnMagXZ)
        Ribbon.AddToolBarTool(toolbar,'View XZ Bottom', 'magxzm', 'View XZ Bottom', parent.OnMagXZM)
        Ribbon.AddToolBarTool(toolbar,'View YZ Right', 'magyz', 'View YZ Right', parent.OnMagYZ)
        Ribbon.AddToolBarTool(toolbar,'View YZ Left', 'magyzm', 'View YZ Left', parent.OnMagYZM)
        Ribbon.AddToolBarTool(toolbar,'View XY Isometric', 'magxyz', 'View XY Isometric', parent.OnMagXYZ)
        
        panel = RB.RibbonPanel(view_page, wx.ID_ANY, 'View Dragging', self.Image('viewrot'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar,'View Rotate', 'viewrot', 'Enter view rotating mode', parent.OnViewRotate)
        Ribbon.AddToolBarTool(toolbar,'View Zoom', 'zoom', 'Drag to zoom in and out', parent.OnViewZoom)
        Ribbon.AddToolBarTool(toolbar,'View Pan', 'pan', 'Drag to move view', parent.OnViewPan)

        panel = RB.RibbonPanel(view_page, wx.ID_ANY, 'Windows', self.Image('viewrot'))
        toolbar = RB.RibbonButtonBar(panel)
        self.window_button_index = 0
        self.AddWindowButton(toolbar, 'Objects')
        self.AddWindowButton(toolbar, 'Properties')
        self.AddWindowButton(toolbar, 'Input')

        view_page.Realize()


        
        options_page = RB.RibbonPage(self, wx.ID_ANY, 'Options', self.Image('settings'))
        options_page.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        
        panel = RB.RibbonPanel(options_page, wx.ID_ANY, 'View', self.Image('mag'))

        check = wx.CheckBox(panel, wx.ID_ANY, 'rotate upright')
        check.SetValue(cad.GetRotateUpright())
        self.Bind(wx.EVT_CHECKBOX, self.OnRotateUpright, check)
        combo = wx.ComboBox(panel, choices = ["None", "Input Mode Title", "Full Help"], style = wx.CB_READONLY )
        combo.Select(cad.GetGraphicsTextMode())
        self.Bind(wx.EVT_COMBOBOX, self.OnScreenText, combo)
        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.AddStretchSpacer(1)
        sizer.Add(check, 0, wx.ALL|wx.EXPAND, 2)
        self.AddLabelAndControl(panel, sizer, 'Screen Text', combo)
        sizer.AddStretchSpacer(1)
        panel.SetSizer(sizer)
        
        panel = RB.RibbonPanel(options_page, wx.ID_ANY, 'View Colors', self.Image('mag'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddBackgroundColorButton(toolbar, 'Background Color Top', 'Edit top background color')
        Ribbon.AddBackgroundColorButton(toolbar, 'Background Color Bottom', 'Edit bottom background color')
        
        options_page.Realize()


                
        self.Realize()
        
    def AddOnOffButton(toolbar, name, bitmap_name, bitmap_name_off, get_fn, set_fn, help_str, drop_down = None):
        OnOffButton(name, bitmap_name, bitmap_name_off, get_fn, set_fn, help_str, drop_down).AddToToolbar(toolbar)
        
    def AddBackgroundColorButton(toolbar, name, help_str):
        BackgroundColorButton(name, help_str).AddToToolbar(toolbar)
        
    def OnOnOffButton(self):
        pass
        
    def AddWindowButton(self, toolbar, name):
        shown = self.GetParent().aui_manager.GetPane(name).IsShown()
        bitmap_name = name.lower()
        if not shown: bitmap_name += 'gray'
        help_str = 'Hide' if shown else 'Show'
        help_str += ' ' + name + ' Window'
        window_button = Ribbon.AddToolBarTool(toolbar, name, bitmap_name, help_str, self.OnWindowButton)
        item = toolbar.GetItemById(window_button)
        toolbar.SetItemClientData(item, (self.GetParent().aui_manager.GetPane(name).window, self.window_button_index))
        self.window_button_index += 1
        
    def AddLabelAndControl(self, panel, sizer, label, control):
        sizer_horizontal = wx.BoxSizer(wx.HORIZONTAL)
        static_label = wx.StaticText(panel, wx.ID_ANY, label)
        sizer_horizontal.Add( static_label, 0, wx.RIGHT + wx.ALIGN_CENTER_VERTICAL, control_border )
        sizer_horizontal.Add( control, 1, wx.LEFT + wx.ALIGN_CENTER_VERTICAL, control_border )
        sizer.Add( sizer_horizontal, 0, wx.EXPAND + wx.ALL, control_border )
        return static_label
        
        
    def Image(self, name):
        image = wx.Image(self.GetParent().BitmapPath(name))
        image.Rescale(24, 24)
        return wx.Bitmap(image)
                        
    def MakeNextIDForTool(self, data):
        return wx.ID_ANY
                        
    def AddToolBarTool2(toolbar, title, bitmap, caption, on_button, on_update_button = None, on_dropdown = None):
        if on_dropdown != None:
            toolbar.AddHybridButton(Ribbon.next_id, title, bitmap, help_string = caption)
            toolbar.Bind(RB.EVT_RIBBONBUTTONBAR_DROPDOWN_CLICKED, on_dropdown, id=Ribbon.next_id)
        else:
            toolbar.AddButton(Ribbon.next_id, title, bitmap, help_string = caption)
        toolbar.Bind(RB.EVT_RIBBONBUTTONBAR_CLICKED, on_button, id=Ribbon.next_id)
        id_to_return = Ribbon.next_id
        Ribbon.next_id += 1
        return id_to_return
                        
    def AddToolBarTool(toolbar, title, bitmap_name, caption, on_button, on_update_button = None, on_dropdown = None):
        ribbon = toolbar.GetParent().GetParent().GetParent()
        bitmap = ribbon.Image(bitmap_name)
        return Ribbon.AddToolBarTool2(toolbar, title, bitmap, caption, on_button, on_update_button, on_dropdown)
                        
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
    
    def OnSnapDropdown(self, e):
        cad.SetDigitizeGridSize(wx.GetApp().InputLength('Edit Snap Grid', 'Snap Grid', cad.GetDigitizeGridSize()))

    def OnWindowButton(self, e):
        f = self.GetParent()
        toolbar = e.GetBar()
        button = e.GetButton()
        window, index = toolbar.GetItemClientData(button)
        pane_info = f.aui_manager.GetPane(window)
        shown = pane_info.IsShown()
        
        if pane_info.IsOk():
            shown = not shown
            pane_info.Show(shown)
            f.aui_manager.Update()

        name = f.aui_manager.GetPane(window).name
        bitmap_name = name.lower()
        if not shown: bitmap_name += 'gray'
        bitmap = self.Image(bitmap_name)
        help_str = 'Hide' if shown else 'Show'
        help_str += ' ' + name + ' Window'
            
        item_id = toolbar.GetItemId(button)
        toolbar.SetButtonIcon(item_id, bitmap)
        
        toolbar.Realize()
            
    def OnRotateUpright(self, event):
        cad.SetRotateUpright(event.IsChecked())
        HeeksConfig().WriteBool("RotateUpright", event.IsChecked())
        
    def OnScreenText(self, event):
        cad.SetGraphicsTextMode(cad.GraphicsTextMode(event.GetSelection()))
        HeeksConfig().WriteInt("TextMode", event.GetSelection())

        cad.Repaint()

if __name__ == '__main__':
    from SolidApp import SolidApp
    app = SolidApp()
    app.MainLoop()
