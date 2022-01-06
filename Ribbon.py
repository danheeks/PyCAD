import wx
import wx.ribbon as RB
import cad
from HeeksConfig import HeeksConfig

control_border = 2

class BitmapChangingButton:
    def __init__(self, drop_down = None):
        self.index = None
        self.toolbar = None
        self.drop_down = drop_down
        self.item_id = None
    
    def ChangeBitmap(self):
        try:
            self.toolbar.SetButtonIcon(self.item_id, self.GetBitmap())
            self.toolbar.SetButtonText(self.item_id, self.GetName())
        except AttributeError:
            self.toolbar.DeleteButton(self.item_id)
            if self.drop_down != None:
                self.toolbar.InsertHybridButton(self.index, self.item_id, self.GetName(), self.GetBitmap(), self.GetHelpStr())
            else:
                self.toolbar.InsertButton(self.index, self.item_id, self.GetName(), self.GetBitmap(), self.GetHelpStr())
        
    def AddToToolbar(self, toolbar):
        self.toolbar = toolbar
        self.index = toolbar.GetButtonCount()
        self.item_id = Ribbon.AddToolBarTool2(toolbar, self.GetName(), self.GetBitmap(), self.GetHelpStr(), self.OnButton, None, self.drop_down)       
            
    def OnButton(self, event):
        self.ChangeBitmap()
        
    def GetRibbon(self):
        ribbon = self.toolbar.GetParent().GetParent().GetParent()
        if ribbon == None:
            ribbon = wx.GetApp().frame.ribbon
        return ribbon

    def GetHelpStr(self):
        return ''
    
class ModeButton(BitmapChangingButton):
    def __init__(self, getter, setter, bitmap_on, bitmap_off, config_str, on_name, off_name, on_help = '', off_help = ''):
        self.getter = getter
        self.setter = setter
        self.bitmap_on = bitmap_on
        self.bitmap_off = bitmap_off
        self.config_str = config_str
        self.on_name = on_name
        self.off_name = off_name
        self.on_help = on_help
        self.off_help = off_help
        self.mode = self.getter()
        BitmapChangingButton.__init__(self)
        
    def OnButton(self, event):
        self.mode = not self.mode
        self.setter(self.mode)
        # remember config
        config = HeeksConfig()
        config.WriteBool(self.config_str, self.mode)
        BitmapChangingButton.OnButton(self, event)
        
    def GetName(self):
        return self.on_name if self.mode == True else self.off_name
    
    def GetBitmap(self):
        return self.GetRibbon().Image(self.bitmap_on if self.mode == True else self.bitmap_off)

    def GetHelpStr(self):
        return self.on_help if self.mode == True else self.off_help
        
class ScreenTextButton(BitmapChangingButton):
    def __init__(self):
        self.mode = int(cad.GetGraphicsTextMode())
        BitmapChangingButton.__init__(self, self.OnDropDown)
        
    def OnModeChanged(self):
        cad.SetGraphicsTextMode(cad.GraphicsTextMode(self.mode))
        # remember config
        config = HeeksConfig()
        config.WriteInt("GraphicsTextMode", self.mode)
        
    def OnButton(self, event):
        self.mode += 1
        if self.mode == 3:
            self.mode = 0
        self.OnModeChanged()
        BitmapChangingButton.OnButton(self, event)
        
    def GetName(self):
        if self.mode == 1: return 'Screen Text Title'
        if self.mode == 2: return 'Screen Full Help'
        return 'No Screen Text'
    
    def GetBitmap(self):
        bm = 'empty'
        if self.mode == 1: bm = 'title'
        elif self.mode == 2: bm = 'fulltext'
        return self.GetRibbon().Image(bm)
        
    def OnDropDown(self, event):
        menu = wx.Menu()
        ribbon = self.GetRibbon()
        recent_id = ribbon.ID_RECENT_FIRST
        ribbon.Bind(wx.EVT_MENU, self.OnScreenTextMode, menu.Append(recent_id, 'No Screen Text'))
        ribbon.Bind(wx.EVT_MENU, self.OnScreenTextMode, menu.Append(recent_id + 1, 'Screen Text Title'))
        ribbon.Bind(wx.EVT_MENU, self.OnScreenTextMode, menu.Append(recent_id + 2, 'Screen Full Help'))
        event.PopupMenu(menu);
        
    def OnScreenTextMode(self, event):
        self.mode = event.GetId() - self.GetRibbon().ID_RECENT_FIRST
        self.OnModeChanged()
        self.ChangeBitmap()

class OnOffButton(BitmapChangingButton):
    def __init__(self, name, bitmap_name, bitmap_name_off, get_fn, set_fn, help_str, drop_down = None):
        BitmapChangingButton.__init__(self, drop_down)
        self.name = name
        self.bitmap_name = bitmap_name
        self.bitmap_name_off = bitmap_name_off
        self.get_fn = get_fn
        self.set_fn = set_fn
        self.help_str = help_str
        self.on = self.get_fn()
            
    def OnButton(self, event):
        self.on = not self.on
        self.set_fn(self.on)

        # remember config
        config = HeeksConfig()
        config.WriteBool(self.name, self.on)
        
        BitmapChangingButton.OnButton(self, event)
        
    def GetName(self):
        return self.name
    
    def GetBitmap(self):
        bm = self.bitmap_name if self.on else self.bitmap_name_off
        return self.GetRibbon().Image(bm)
        
    def GetHelpStr(self):
        return self.help_str
            
class WindowOnOffButton(BitmapChangingButton):
    def __init__(self, name, bitmap_path):
        BitmapChangingButton.__init__(self)
        self.name = name
        self.index = None
        self.bitmap_path = bitmap_path
        
    def GetAuiManager(self):
        ribbon = self.GetRibbon()
        if ribbon == None:
            frame = wx.GetApp().frame
        else:
            frame = ribbon.GetParent()            
        return frame.aui_manager
    
    def GetAuiPane(self):
        return self.GetAuiManager().GetPane(self.name)
        
    def IsShown(self):
        return self.GetAuiPane().IsShown()

    def BitmapName(self):
        bitmap_name = self.name.lower()
        if not self.IsShown(): bitmap_name += 'gray'
        return bitmap_name
    
    def GetName(self):
        return self.name
    
    def GetBitmap(self):
        return self.GetRibbon().Image2(self.bitmap_path + '/' + self.BitmapName() + '.png')
    
    def GetHelpStr(self):
        help_str = 'Hide' if self.IsShown() else 'Show'
        help_str += ' ' + self.name + ' Window'
        return help_str
    
    def OnButton(self, event):
        pane_info = self.GetAuiPane()
        
        if pane_info.IsOk():
            shown = not self.IsShown()
            pane_info.Show(shown)
            self.GetAuiManager().Update()
            
        BitmapChangingButton.OnButton(self, event)
 
        
class ColorButton(BitmapChangingButton):
    def __init__(self, name, help_str):
        BitmapChangingButton.__init__(self)
        self.name = name
        self.help_str = help_str
        
    def GetName(self):
        return self.name
    
    def GetHelpStr(self):
        return self.help_str
        
    def GetBitmap(self):
        return self.ColorBitmap()
        
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
        BitmapChangingButton.OnButton(self, event)

    def SelectNewColour(self):
        c = self.GetColor()
        data = wx.ColourData()
        data.SetColour(wx.Colour(c.red, c.green, c.blue))
        dlg = wx.ColourDialog(self.GetRibbon(), data)
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
        self.GetRibbon().GetParent().graphics_canvas.Refresh()

    
class BackgroundColorButton(ColorButton):
    def __init__(self, name, help_str):
        ColorButton.__init__(self, name, help_str)
        
    def GetColor(self):
        return cad.GetBackgroundColor(self.index)
        
    def SetColor(self, c):
        cad.SetBackgroundColor(self.index, c)
        HeeksConfig().WriteInt("BackgroundColor" + str(self.index), c.ref())
        self.GetRibbon().GetParent().graphics_canvas.Refresh()
        
class GrayedButton:
    def __init__(self, name, bitmap_path, help_str, on_button):
        self.name = name
        self.bitmap_path = bitmap_path
        self.help_str = help_str
        self.on_button = on_button
        
    def AddToToolbar(self, toolbar):
        ribbon = toolbar.GetParent().GetParent().GetParent()
        return Ribbon.AddToolBarTool2(toolbar, self.name, ribbon.Image(self.bitmap_path), self.help_str, self.on_button)

class Ribbon(RB.RibbonBar):
    next_id = 0
    
    def __init__(self, parent):
        RB.RibbonBar.__init__(self, parent, style = RB.RIBBON_BAR_FLOW_HORIZONTAL | RB.RIBBON_BAR_SHOW_PAGE_LABELS | RB.RIBBON_BAR_SHOW_PAGE_ICONS | RB.RIBBON_BAR_SHOW_PANEL_EXT_BUTTONS | RB.RIBBON_BAR_SHOW_HELP_BUTTON)
        self.ID_RECENT_FIRST = wx.ID_HIGHEST + 1
        Ribbon.next_id = self.ID_RECENT_FIRST + wx.GetApp().MAX_RECENT_FILES
        self.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        
        self.main_page = RB.RibbonPage(self, wx.ID_ANY, 'File', self.Image('file'))
        self.main_page.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        
        app = wx.GetApp()

        panel = RB.RibbonPanel(self.main_page, wx.ID_ANY, 'File', self.Image('new'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar, 'New', 'new', 'New File', app.OnNew)
        Ribbon.AddToolBarTool(toolbar, "Open", 'open', "Open file", app.OnOpen, None, self.OnOpenDropdown)
        Ribbon.AddToolBarTool(toolbar, "Import", 'import', "Import file", app.OnImport)
        Ribbon.AddToolBarTool(toolbar, "Export", 'export', "Export file", app.OnExport)
        Ribbon.AddToolBarTool(toolbar, "Save", 'save', "Save file", app.OnSave, app.OnUpdateSave)
        Ribbon.AddToolBarTool(toolbar, "Save As", 'saveas', "Save file with given name", app.OnSaveAs)
        Ribbon.AddToolBarTool(toolbar, "Restore Defaults", 'restore', "Restore all defaults", app.OnResetDefaults)
        Ribbon.AddToolBarTool(toolbar, "About", 'about', "Software Information", app.OnAbout)

        panel = RB.RibbonPanel(self.main_page, wx.ID_ANY, 'Print', self.Image('print'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar, "Print", 'print', "Print the view to a printer", app.OnPrint)
        Ribbon.AddToolBarTool(toolbar, "Page Setup", 'psetup', "Setup the printer layout", app.OnPageSetup)
        Ribbon.AddToolBarTool(toolbar, "Print Preview", 'ppreview', "Show a preview of the print view", app.OnPrintPreview)

        panel = RB.RibbonPanel(self.main_page, wx.ID_ANY, 'Edit', self.Image('cut'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar, "Undo", 'undo', 'Undo the previous command', app.OnUndo, app.OnUpdateUndo, None)
        Ribbon.AddToolBarTool(toolbar, "Redo", 'redo', 'Redo the next command', app.OnRedo, app.OnUpdateRedo, None)
        Ribbon.AddToolBarTool(toolbar, "Cut", 'cut', 'Cut selected items to the clipboard', app.OnCut, app.OnUpdateCut)
        Ribbon.AddToolBarTool(toolbar, "Copy", 'copy', 'Copy selected items to the clipboard', app.OnCopy, app.OnUpdateCopy)
        Ribbon.AddToolBarTool(toolbar, "Paste", 'paste', 'Paste items from the clipboard', app.OnPaste, app.OnUpdatePaste)
        Ribbon.AddToolBarTool(toolbar, "Delete", 'delete', 'Delete selected items', app.OnDelete, app.OnUpdateDelete)
        Ribbon.AddToolBarTool(toolbar, "Select", 'select', 'Select Mode', app.OnSelectMode)
        Ribbon.AddToolBarTool(toolbar, "Filter", 'filter', 'Edit theSelection Filter', app.OnFilter)

        self.main_page.Realize()

        
        self.geom_page = RB.RibbonPage(self, wx.ID_ANY, 'Geom', self.Image('geom'))
        self.geom_page.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        
        panel = RB.RibbonPanel(self.geom_page, wx.ID_ANY, 'Sketches', self.Image('lines'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar,'Lines', 'lines', 'Draw a sketch with lines and arcs', app.OnLines)
        Ribbon.AddToolBarTool(toolbar,'Rectangles', 'rect', 'Draw rectangles', app.OnRectangles)
        Ribbon.AddToolBarTool(toolbar,'Obrounds', 'obround', 'Draw obrounds', app.OnObrounds)
        Ribbon.AddToolBarTool(toolbar,'Polygons', 'pentagon', 'Draw polygons', app.OnPolygons)
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
        
        panel = RB.RibbonPanel(self.geom_page, wx.ID_ANY, 'Text', self.Image('text'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar,'Text', 'text', 'Add a text object', app.OnText)
        Ribbon.AddToolBarTool(toolbar,'Dimensioning', 'dimension', 'Add a dimension', app.OnDimensioning)
        
        panel = RB.RibbonPanel(self.geom_page, wx.ID_ANY, 'Snapping', self.Image('endof'))
        toolbar = RB.RibbonButtonBar(panel)
        OnOffButton('Endof', 'endof', 'endofgray', cad.GetDigitizeEnd, cad.SetDigitizeEnd, 'Snap to end point').AddToToolbar(toolbar)
        OnOffButton('Inters', 'inters', 'intersgray', cad.GetDigitizeInters, cad.SetDigitizeInters, 'Snap to intersection').AddToToolbar(toolbar)
        OnOffButton('Centre', 'centre', 'centregray', cad.GetDigitizeCentre, cad.SetDigitizeCentre, 'Snap to centre').AddToToolbar(toolbar)
        OnOffButton('Midpoint', 'midpoint', 'midpointgray', cad.GetDigitizeMidpoint, cad.SetDigitizeMidpoint, 'Snap to midpoint').AddToToolbar(toolbar)
        OnOffButton('Grid', 'snap', 'snapgray', cad.GetDigitizeSnapToGrid, cad.SetDigitizeSnapToGrid, 'Snap to grid', self.OnSnapDropdown).AddToToolbar(toolbar)

        self.geom_page.Realize()


        move_page = RB.RibbonPage(self, wx.ID_ANY, 'Move', self.Image('coordsys'))
        move_page.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        
        panel = RB.RibbonPanel(move_page, wx.ID_ANY, 'Set Origin', self.Image('coordsys'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar,'3 Points', 'coordsys', 'Pick 3 points', app.OnSetOrigin3Points)
        Ribbon.AddToolBarTool(toolbar,'1 point', 'coordsys', 'Pick 1 point', app.OnSetOrigin1Point)
        
        panel = RB.RibbonPanel(move_page, wx.ID_ANY, 'Transformations', self.Image('movet'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar,'Move Translate', 'movet', 'Translate selected items', app.OnMoveTranslate)
        Ribbon.AddToolBarTool(toolbar,'Copy Translate', 'copyt', 'Copy and translate selected items', app.OnCopyTranslate)
        Ribbon.AddToolBarTool(toolbar,'Move Rotate', 'mover', 'Rotate selected items', app.OnMoveRotate)
        Ribbon.AddToolBarTool(toolbar,'Copy Rotate', 'copyr', 'Copy and rotate selected items', app.OnCopyRotate)
        Ribbon.AddToolBarTool(toolbar,'Mirror', 'copym', 'Mirror selected items', app.OnMirror)
        Ribbon.AddToolBarTool(toolbar,'Scale', 'moves', 'Scale selected items', app.OnMoveScale)
        Ribbon.AddToolBarTool(toolbar,'Move Origin', 'coords3', 'Move selected items from one origin to another', app.OnOriginTransform)

        move_page.Realize()
        

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
        for w in wx.GetApp().hideable_windows:
            WindowOnOffButton(self.GetParent().aui_manager.GetPane(w).name, wx.GetApp().hideable_windows[w]).AddToToolbar(toolbar)

        view_page.Realize()

        # Add extra pages before the Options page
        wx.GetApp().AddExtraRibbonPages(self)
        
        self.options_page = RB.RibbonPage(self, wx.ID_ANY, 'Options', self.Image('settings'))
        self.options_page.Bind(wx.EVT_KEY_DOWN, self.OnKeyDown)
        
        wx.GetApp().AddOptionsRibbonPanels(self)
                
        self.Realize()
        
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
    
    def Image2(self, full_path):
        image = wx.Image(full_path)
        scale = float(24) / image.GetHeight()
        new_width = float(image.GetWidth()) * scale
        image.Rescale(int(new_width), 24)
        return wx.Bitmap(image)
        
    def Image(self, name):
        return self.Image2(wx.GetApp().BitmapPath(name))
                        
    def MakeNextIDForTool(self, data):
        return wx.ID_ANY
                        
    def AddToolBarTool2(toolbar, title, bitmap, caption, on_button, on_update_button = None, on_dropdown = None):
        if on_dropdown != None:
            toolbar.AddHybridButton(Ribbon.next_id, title, bitmap, help_string = caption)
            toolbar.Bind(RB.EVT_RIBBONBUTTONBAR_DROPDOWN_CLICKED, on_dropdown, id=Ribbon.next_id)
        else:
            toolbar.AddButton(Ribbon.next_id, title, bitmap, help_string = caption)
        toolbar.Bind(RB.EVT_RIBBONBUTTONBAR_CLICKED, on_button, id=Ribbon.next_id)
        
        if on_update_button != None:
            toolbar.Bind(wx.EVT_UPDATE_UI, on_update_button, id=Ribbon.next_id)
 
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
        value = wx.GetApp().InputLength('Edit Snap Grid', 'Snap Grid', cad.GetDigitizeGridSize())
        if value != None:
            cad.SetDigitizeGridSize(value)

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
        
    def OnInternalIdle(self):
        if wx.UpdateUIEvent.CanUpdate(self):
            self.UpdateWindowUI(wx.UPDATE_UI_FROMIDLE)
        
        

if __name__ == '__main__':
    from SolidApp import SolidApp
    app = SolidApp()
    app.MainLoop()
