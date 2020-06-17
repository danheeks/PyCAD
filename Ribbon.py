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
        try:
            toolbar.SetButtonIcon(item_id, bitmap)
        except AttributeError:
            toolbar.DeleteButton(item_id)
            toolbar.InsertButton(self.index, item_id, self.name, bitmap, self.help_str)
        
class ColorButton:
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
        color = self.GetColor()
        bitmap = wx.Bitmap(24, 24)
        dc = wx.MemoryDC(bitmap)
        dc.SetBrush(wx.Brush(wx.Colour(color.red, color.green, color.blue)))
        dc.DrawRectangle(0,0,24,24)
        del dc
        return bitmap
        
    def OnButton(self, event):
        self.SelectNewColour()
        toolbar = event.GetBar()
        button = event.GetButton()
        item_id = toolbar.GetItemId(button)
        try:
            toolbar.SetButtonIcon(item_id, self.ColorBitmap())
        except AttributeError:
            toolbar.DeleteButton(item_id)
            item = toolbar.InsertButton(self.index, item_id, self.name, self.ColorBitmap(), self.help_str)

    def SelectNewColour(self):
        c = self.GetColor()
        data = wx.ColourData()
        data.SetColour(wx.Colour(c.red, c.green, c.blue))
        dlg = wx.ColourDialog(self.ribbon, data)
        if dlg.ShowModal() == wx.ID_OK:
            # Colour did change.
            self.SetColor(self.wxToCadColor(dlg.GetColourData().GetColour()))
        
    def wxToCadColor(self, wxcolor):
        c = cad.Color()
        c.red = wxcolor.red
        c.green = wxcolor.green
        c.blue = wxcolor.blue
        return c
        
class CurrentColorButton(ColorButton):
    def __init__(self, name, help_str):
        ColorButton.__init__(self, name, help_str)
        
    def GetColor(self):
        return cad.GetCurrentColor()
        
    def SetColor(self, c):
        cad.SetCurrentColor(c)
        HeeksConfig().WriteInt("CurrentColor", c.ref())
        self.ribbon.GetParent().graphics_canvas.Refresh()

    
class BackgroundColorButton(ColorButton):
    def __init__(self, name, help_str):
        ColorButton.__init__(self, name, help_str)
        
    def GetColor(self):
        return cad.GetBackgroundColor(self.index)
        
    def SetColor(self, c):
        cad.SetBackgroundColor(self.index, c)
        HeeksConfig().WriteInt("BackgroundColor" + str(self.index), c.ref())
        self.ribbon.GetParent().graphics_canvas.Refresh()


class Ribbon(RB.RibbonBar):
    next_id = 0
    
    def __init__(self, parent):
        RB.RibbonBar.__init__(self, parent, style = RB.RIBBON_BAR_FLOW_HORIZONTAL | RB.RIBBON_BAR_SHOW_PAGE_LABELS | RB.RIBBON_BAR_SHOW_PAGE_ICONS | RB.RIBBON_BAR_SHOW_PANEL_EXT_BUTTONS | RB.RIBBON_BAR_SHOW_HELP_BUTTON)
        self.ID_RECENT_FIRST = wx.ID_HIGHEST + 1
        Ribbon.next_id = self.ID_RECENT_FIRST + wx.GetApp().MAX_RECENT_FILES
        self.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        
        main_page = RB.RibbonPage(self, wx.ID_ANY, 'File', self.Image('file'))
        main_page.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        
        app = wx.GetApp()

        panel = RB.RibbonPanel(main_page, wx.ID_ANY, 'File', self.Image('new'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar, 'New', 'new', 'New File', app.OnNew)
        Ribbon.AddToolBarTool(toolbar, "Open", 'open', "Open file", app.OnOpen, None, self.OnOpenDropdown)
        Ribbon.AddToolBarTool(toolbar, "Import", 'import', "Import file", app.OnImport)
        Ribbon.AddToolBarTool(toolbar, "Export", 'export', "Export file", app.OnExport)
        Ribbon.AddToolBarTool(toolbar, "Save", 'save', "Save file", app.OnSave, app.OnUpdateSave)
        Ribbon.AddToolBarTool(toolbar, "Save As", 'saveas', "Save file with given name", app.OnSaveAs)
        Ribbon.AddToolBarTool(toolbar, "Restore Defaults", 'restore', "Restore all defaults", app.OnResetDefaults)
        Ribbon.AddToolBarTool(toolbar, "About", 'about', "Software Information", app.OnAbout)

        panel = RB.RibbonPanel(main_page, wx.ID_ANY, 'Print', self.Image('print'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar, "Print", 'print', "Print the view to a printer", app.OnPrint)
        Ribbon.AddToolBarTool(toolbar, "Page Setup", 'psetup', "Setup the printer layout", app.OnPageSetup)
        Ribbon.AddToolBarTool(toolbar, "Print Preview", 'ppreview', "Show a preview of the print view", app.OnPrintPreview)

        panel = RB.RibbonPanel(main_page, wx.ID_ANY, 'Edit', self.Image('cut'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar, "Undo", 'undo', 'Undo the previous command', app.OnUndo, app.OnUpdateUndo, None)
        Ribbon.AddToolBarTool(toolbar, "Redo", 'redo', 'Redo the next command', app.OnRedo, app.OnUpdateRedo, None)
        Ribbon.AddToolBarTool(toolbar, "Cut", 'cut', 'Cut selected items to the clipboard', app.OnCut, app.OnUpdateCut)
        Ribbon.AddToolBarTool(toolbar, "Copy", 'copy', 'Copy selected items to the clipboard', app.OnCopy, app.OnUpdateCopy)
        Ribbon.AddToolBarTool(toolbar, "Paste", 'paste', 'Paste items from the clipboard', app.OnPaste, app.OnUpdatePaste)
        Ribbon.AddToolBarTool(toolbar, "Delete", 'delete', 'Delete selected items', app.OnDelete, app.OnUpdateDelete)
        Ribbon.AddToolBarTool(toolbar, "Select", 'select', 'Select Mode', app.OnSelectMode)
        Ribbon.AddToolBarTool(toolbar, "Filter", 'filter', 'Edit theSelection Filter', app.OnFilter)

        main_page.Realize()

        
        self.geom_page = RB.RibbonPage(self, wx.ID_ANY, 'Geom', self.Image('geom'))
        self.geom_page.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        
        panel = RB.RibbonPanel(self.geom_page, wx.ID_ANY, 'Sketches', self.Image('lines'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar,'Lines', 'lines', 'Draw a sketch with lines and arcs', app.OnLines)
        Ribbon.AddToolBarTool(toolbar,'Rectangles', 'rect', 'Draw rectangles', app.OnLines)
        Ribbon.AddToolBarTool(toolbar,'Obrounds', 'obround', 'Draw obrounds', app.OnLines)
        Ribbon.AddToolBarTool(toolbar,'Polygons', 'pentagon', 'Draw polygons', app.OnLines)
        Ribbon.AddToolBarTool(toolbar,'Gear', 'gear', 'Add a gear', app.OnGear)
        CurrentColorButton('Drawing Color', 'Edit current drawing color').AddToToolbar(toolbar)
        
        panel = RB.RibbonPanel(self.geom_page, wx.ID_ANY, 'Circles', self.Image('circ3p'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar,'3 Points', 'circ3p', 'Draw circles through 3 points', app.OnCircles3p)
        Ribbon.AddToolBarTool(toolbar,'2 Points', 'circ2p', 'Draw circles, centre and point', app.OnCircles2p)
        Ribbon.AddToolBarTool(toolbar,'Radius', 'circpr', 'Draw circles, centre and radius', app.OnCircles1p)
        
        panel = RB.RibbonPanel(self.geom_page, wx.ID_ANY, 'OtherDrawing', self.Image('point'))
        self.other_drawing_toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(self.other_drawing_toolbar,'Infinite Line', 'iline', 'Drawing Infinite Lines', app.OnILine)
        Ribbon.AddToolBarTool(self.other_drawing_toolbar,'Points', 'point', 'Drawing Points', app.OnPoints)
        Ribbon.AddToolBarTool(self.other_drawing_toolbar,'Spline', 'splpts', 'Spline Through Points', app.OnGear)
        
        panel = RB.RibbonPanel(self.geom_page, wx.ID_ANY, 'Text', self.Image('text'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar,'Text', 'text', 'Add a text object', app.OnText)
        Ribbon.AddToolBarTool(toolbar,'Dimensioning', 'dimension', 'Add a dimension', app.OnDimensioning)
        
        panel = RB.RibbonPanel(self.geom_page, wx.ID_ANY, 'Transformations', self.Image('movet'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar,'Move Translate', 'movet', 'Translate selected items', app.OnMoveTranslate)
        Ribbon.AddToolBarTool(toolbar,'Copy Translate', 'copyt', 'Copy and translate selected items', app.OnCopyTranslate)
        Ribbon.AddToolBarTool(toolbar,'Move Rotate', 'mover', 'Rotate selected items', app.OnMoveRotate)
        Ribbon.AddToolBarTool(toolbar,'Copy Rotate', 'copyr', 'Copy and rotate selected items', app.OnCopyRotate)
        Ribbon.AddToolBarTool(toolbar,'Move Mirror', 'copym', 'Mirror selected items', app.OnMirror)
        Ribbon.AddToolBarTool(toolbar,'Move Scale', 'moves', 'Scale selected items', app.OnMoveScale)
        Ribbon.AddToolBarTool(toolbar,'Move Origin', 'coords3', 'Move selected items from one origin to another', app.OnOriginTransform)
        
        panel = RB.RibbonPanel(self.geom_page, wx.ID_ANY, 'Snapping', self.Image('endof'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddOnOffButton(toolbar, 'Endof', 'endof', 'endofgray', cad.GetDigitizeEnd, cad.SetDigitizeEnd, 'Snap to end point')
        Ribbon.AddOnOffButton(toolbar, 'Inters', 'inters', 'intersgray', cad.GetDigitizeInters, cad.SetDigitizeInters, 'Snap to intersection')
        Ribbon.AddOnOffButton(toolbar, 'Centre', 'centre', 'centregray', cad.GetDigitizeCentre, cad.SetDigitizeCentre, 'Snap to centre')
        Ribbon.AddOnOffButton(toolbar, 'Midpoint', 'midpoint', 'midpointgray', cad.GetDigitizeMidpoint, cad.SetDigitizeMidpoint, 'Snap to midpoint')
        Ribbon.AddOnOffButton(toolbar, 'Grid', 'snap', 'snapgray', cad.GetDigitizeSnapToGrid, cad.SetDigitizeSnapToGrid, 'Snap to grid', self.OnSnapDropdown)

        self.geom_page.Realize()
        

        view_page = RB.RibbonPage(self, wx.ID_ANY, 'View', self.Image('view'))
        view_page.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        
        panel = RB.RibbonPanel(view_page, wx.ID_ANY, 'Magnify', self.Image('mag'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar,'Mag Extents', 'magextents', 'Zoom in to fit the extents of the drawing into the graphics window', app.OnMagExtents)
        Ribbon.AddToolBarTool(toolbar,'Mag No Rotation', 'magnorot', 'Zoom in to fit the extents of the drawing into the graphics window, but without rotating the view', app.OnMagNoRot)
        Ribbon.AddToolBarTool(toolbar,'Zoom Window', 'mag', 'Zoom in to a dragged window', app.OnMag)
        
        panel = RB.RibbonPanel(view_page, wx.ID_ANY, 'General', self.Image('fullscreen'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar,'View Back', 'magprev', 'Go back to previous view', app.OnMagPrevious)
        Ribbon.AddToolBarTool(toolbar,'FullScreen', 'fullscreen', 'Switch to full screen view ( press escape to return )', app.OnFullScreen)
        Ribbon.AddToolBarTool(toolbar,'Redraw', 'redraw', 'Redraw', app.OnRedraw)
        
        panel = RB.RibbonPanel(view_page, wx.ID_ANY, 'Specific Views', self.Image('magxy'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar,'View XY Front', 'magxy', 'View XY Front', app.OnMagXY)
        Ribbon.AddToolBarTool(toolbar,'View XY Back', 'magxym', 'View XY Back', app.OnMagXYM)
        Ribbon.AddToolBarTool(toolbar,'View XZ Top', 'magxz', 'View XZ Top', app.OnMagXZ)
        Ribbon.AddToolBarTool(toolbar,'View XZ Bottom', 'magxzm', 'View XZ Bottom', app.OnMagXZM)
        Ribbon.AddToolBarTool(toolbar,'View YZ Right', 'magyz', 'View YZ Right', app.OnMagYZ)
        Ribbon.AddToolBarTool(toolbar,'View YZ Left', 'magyzm', 'View YZ Left', app.OnMagYZM)
        Ribbon.AddToolBarTool(toolbar,'View XY Isometric', 'magxyz', 'View XY Isometric', app.OnMagXYZ)
        
        panel = RB.RibbonPanel(view_page, wx.ID_ANY, 'View Dragging', self.Image('viewrot'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar,'View Rotate', 'viewrot', 'Enter view rotating mode', app.OnViewRotate)
        Ribbon.AddToolBarTool(toolbar,'View Zoom', 'zoom', 'Drag to zoom in and out', app.OnViewZoom)
        Ribbon.AddToolBarTool(toolbar,'View Pan', 'pan', 'Drag to move view', app.OnViewPan)

        panel = RB.RibbonPanel(view_page, wx.ID_ANY, 'Windows', self.Image('viewrot'))
        toolbar = RB.RibbonButtonBar(panel)
        self.window_button_index = 0
        self.AddWindowButton(toolbar, 'Objects')
        self.AddWindowButton(toolbar, 'Properties')
        self.AddWindowButton(toolbar, 'Input')

        view_page.Realize()

        # Add extra pages before the Options page
        wx.GetApp().AddExtraRibbonPages(self)
        
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
        BackgroundColorButton('Background Color Top', 'Edit top background color').AddToToolbar(toolbar)
        BackgroundColorButton('Background Color Bottom', 'Edit bottom background color').AddToToolbar(toolbar)


        options_page.Realize()
                
        self.Realize()
        
    def AddOnOffButton(toolbar, name, bitmap_name, bitmap_name_off, get_fn, set_fn, help_str, drop_down = None):
        OnOffButton(name, bitmap_name, bitmap_name_off, get_fn, set_fn, help_str, drop_down).AddToToolbar(toolbar)
        
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
        image = wx.Image(wx.GetApp().BitmapPath(name))
        scale = float(24) / image.GetHeight()
        new_width = float(image.GetWidth()) * scale
        image.Rescale(int(new_width), 24)
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
        index = event.GetId() - self.ID_RECENT_FIRST
        file_path = wx.GetApp().recent_files[index]
        wx.GetApp().OnOpenFilepath(file_path)
    
    def OnOpenDropdown(self, event):
        menu = wx.Menu()
        
        recent_id = self.ID_RECENT_FIRST
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
        try:
            toolbar.SetButtonIcon(item_id, bitmap)
        except AttributeError:
            toolbar.DeleteButton(item_id)
            item = toolbar.InsertButton(index, item_id, name, bitmap, help_str)
            toolbar.SetItemClientData(item, (self.GetParent().aui_manager.GetPane(name).window, index))
        
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
