import wx
import sys
import cad
from Frame import Frame
import os
from HeeksConfig import HeeksConfig
import ContextTool
import ToolBarTool
import SelectMode
from HDialog import HDialog
from HDialog import control_border
from NiceTextCtrl import LengthCtrl
from Printout import Printout
from PointDrawing import point_drawing
import geom
import Gear
from About import AboutBox
from FilterDlg import FilterDlg
import HImage
from Ribbon import RB

pycad_dir = os.path.dirname(os.path.realpath(__file__))
HEEKS_WILDCARD_STRING = 'Heeks files |*.heeks;*.HEEKS'
wx_image_extensions = ['bmp','png','jpeg','jpg','gif','pcx','pnm','tif','tga','iff','xpm','ico','cur','ani'] # couldn't find GetHandlers in wxpython


pycad_dir = os.path.dirname(os.path.realpath(__file__))

def OnMessageBox(error_message):
    wx.MessageBox(error_message)
    
def OnInputMode():
    wx.GetApp().OnInputMode()
    
def OnPaint():
    if wx.GetApp().coordsys_for_P3P != None:
        wx.GetApp().RenderCoordSys()
    
tools = []
save_filter_for_StartPickObjects = 0
save_just_one_for_EndPickObjects = False
save_mode_for_EndPickObjects = None

def CreateImage(): return HImage.HImage()
def CreateGear(): return Gear.Gear()

def ImportImageFile():
    image = HImage.HImage(cad.GetFilePathForImportExport())
    cad.AddUndoably(image)

class App(wx.App):
    def __init__(self):
        self.version_number = '2 0' # just main number and sub number. I'm not using sub-sub numbers
        self.settings_restored = False
        self.recent_files = []
        self.MAX_RECENT_FILES = 20
        self.filepath = None
        self.hideable_windows = {} # map of window:bitmap_path
        self.cad_dir = pycad_dir
        self.bitmap_path = self.cad_dir + '/bitmaps'
        self.inMainLoop = False
        self.select_mode = SelectMode.SelectMode()
        self.coordsys_for_P3P = None
        self.paint_registered = False
        self.import_file_types = [] # a list of tuples, example entry:   ( ['step', 'stp'], 'Step Files' )
        self.export_file_types = [] # a list of tuples
        
        save_out = sys.stdout
        save_err = sys.stderr
        wx.App.__init__(self)
        sys.stdout = save_out
        sys.stderr = save_err
        
    def GetAppTitle(self):
        # The title to appear on the title bar
        return 'Python-Based CAD Software'
        
    def GetAppConfigName(self):
        # The name of the registry item
        return 'HeeksCAD'
            
    def OnInit(self):
        self.RegisterMessageBoxCallback()
        self.InitCad()
        self.RegisterObjectTypes()
        
        self.printData = wx.PrintData()
        self.pageSetupData = wx.PageSetupDialogData(self.printData)
            
        wx.InitAllImageHandlers()
        
        config = HeeksConfig()
        
        width = config.ReadInt('MainFrameWidth', -1);
        height = config.ReadInt('MainFrameHeight', -1);
        x = config.ReadInt('MainFramePosX', -1);
        y = config.ReadInt('MainFramePosY', -1);
        
        if width < 0:
            width = -1
            height = -1
            x = -1
            y = -1
        else:
            stored_rect = wx.Rect(x, y, width, height)
            in_display = False
            for idx in range(wx.Display.GetCount()):
                d = wx.Display(idx)
                rect = d.GetGeometry()
                if rect.Contains(wx.Point(x, y)):
                    in_display = True
            if in_display == False:
                width = -1
                height = -1
                x = -1
                y = -1
                
        self.LoadConfig()
        
        self.frame = self.NewFrame(wx.Point(x, y), wx.Size(width, height))
        self.frame.Show()
        self.OnNewOrOpen(False)
        cad.ClearHistory()
        cad.SetLikeNewFile()
        self.frame.SetFrameTitle()
        
        return True
    
    def RegisterObjectTypes(self):
        HImage.type = cad.RegisterObjectType("Image", CreateImage)
        Gear.type = cad.RegisterObjectType("Gear", CreateGear)
        self.RegisterImportFileTypes(wx_image_extensions, 'Picture Files', ImportImageFile)
        import Svg
        self.RegisterExportFileTypes(['svg'], 'Svg Files', Svg.Export)
    
    def OnNewOrOpen(self, open):
        pass
    
    def NewFrame(self, pos=wx.DefaultPosition, size=wx.DefaultSize):
        return Frame(None, pos = pos, size = size)

    def LoadConfig(self):
        config = HeeksConfig(self.settings_restored)
        self.LoadRecentFiles(config)

        # snapping
        cad.SetDigitizeEnd(config.ReadBool("Endof", True))
        cad.SetDigitizeInters(config.ReadBool("Inters", False))
        cad.SetDigitizeCentre(config.ReadBool("Centre", True))
        cad.SetDigitizeMidpoint(config.ReadBool("Midpoint", False))
        cad.SetDigitizeSnapToGrid(config.ReadBool("Grid", True))
        
        cad.SetRotateUpright(config.ReadBool("RotateUpright", False))
        cad.SetGraphicsTextMode(cad.GraphicsTextMode(config.ReadInt("TextMode", int(cad.GraphicsTextMode.FullHelp))))
        
        cad.SetBackgroundColor(0, cad.Color(config.ReadInt("BackgroundColor0", cad.Color(230, 255, 255).ref())))
        cad.SetBackgroundColor(1, cad.Color(config.ReadInt("BackgroundColor1", cad.Color(255, 255, 255).ref())))
        cad.SetCurrentColor(cad.Color(config.ReadInt("CurrentColor", cad.Color(0, 0, 0).ref())))
        cad.SetAntialiasing(config.ReadBool('Antialiasing', False))
        cad.SetShowDatum(config.ReadBool('ShowDatum', True))

    def GetDefaultDir(self):
        default_directory = os.getcwd()
        
        if len(self.recent_files) > 0:
            default_directory = self.recent_files[0]
            default_directory = os.path.dirname(os.path.realpath(default_directory))
            
        return default_directory
        
    def LoadRecentFiles(self, config):
        for i in range(0, self.MAX_RECENT_FILES):
            key = 'RecentFilePath' + str(i)
            filepath = config.Read(key)
            if len(filepath) == 0:
                break
            self.recent_files.append(filepath)
            
    def WriteRecentFiles(self, config = None):
        if config == None:
            config = HeeksConfig(self.settings_restored)
        index = 0
        for filepath in self.recent_files:
            if index >= self.MAX_RECENT_FILES:
                break
            config.Write('RecentFilePath' + str(index), self.recent_files[index])
            index += 1
            
    def InsertRecentFileItem(self, filepath):
        if filepath in self.recent_files:
            self.recent_files.remove(filepath)
        self.recent_files.insert(0,filepath)
        if len(self.recent_files) > self.MAX_RECENT_FILES:
            self.recent_files.pop()

    def RegisterMessageBoxCallback(self):
        cad.RegisterMessageBoxCallback(OnMessageBox)
        
    def RegisterHideableWindow(self, w, bitmap_path = None):
        if bitmap_path == None:
            bitmap_path = self.bitmap_path
        self.hideable_windows[w] = bitmap_path
        
    def RemoveHideableWindow(self, w):
        del self.hideable_windows[w]
        
    def InitCad(self):
        cad.SetInputMode(self.select_mode);
        cad.SetResFolder(pycad_dir)
        cad.SetInputModeCallback(OnInputMode)
        
    def OnInputMode(self):
        self.frame.input_mode_canvas.RemoveAndAddAll()
        self.frame.graphics_canvas.Refresh()
        
    def SplitSketch(self):
        new_sketches = self.object.Split()
        cad.StartHistory()
        cad.DeleteUndoably(self.object)
        for sketch in new_sketches:
            cad.AddUndoably(sketch, self.object.GetOwner(), None)
        cad.EndHistory()
        
    def FitArcs(self):
        value = self.InputLength('Set tolerance for Fit Arcs', 'tolerance', geom.get_accuracy())
        if value != None:
            geom.set_accuracy(value)
            sketch = self.object
            sketch.__class__ = cad.Sketch
            curve = sketch.GetCurve()
            curve.FitArcs()
            cad.StartHistory()
            cad.DeleteUndoably(self.object)
            cad.AddUndoably(cad.NewSketchFromCurve(curve))
            cad.EndHistory()        
        
    def CopyUndoably(self, object, copy_with_new_data):
        copy_undoable = CopyObjectUndoable(object, copy_with_new_data)
        cad.PyIncref(copy_undoable)
        cad.DoUndoable(copy_undoable)

    def EditUndoably(self, object):
        if object.HasEdit() == False:
            return
        
        copy_object = object.MakeACopy()
        
        if copy_object:
            if copy_object.Edit():
                self.CopyUndoably(object, copy_object)
                #cad.CopyUndoably(object, copy_object)

    def GetObjectTools(self, object, control_pressed, from_tree_canvas = False):
        tools = []
        self.object = object
        type = self.object.GetType()
        if type == cad.OBJECT_TYPE_SKETCH:
            if self.object.GetNumChildren() > 1:
                tools.append(ContextTool.CADContextTool("Split Sketch", "splitsketch", self.SplitSketch))
                tools.append(ContextTool.CADContextTool("Fit Arcs", "fitarcs", self.FitArcs))
                
        if len(tools)>0:
            tools.append(None) # a separator
            
        if not from_tree_canvas: # tree window doesn't need a "Select" menu item, because right clicking will have selected the object anyway
            tools.append(ContextTool.SelectTool(object, control_pressed))
        if object.HasEdit():
            tools.append(ContextTool.EditTool(object))
        if object.CanBeDeleted():
            tools.append(ContextTool.DeleteTool(object))
        return tools
        
    def GetTools(self, x, y, control_pressed):
        self.frame.graphics_canvas.SetCurrent(self.frame.graphics_canvas.context)# should be somewhere else, but for now fixes a problem
        
        # get object tools
        objects = cad.ObjectsUnderWindow(cad.IRect(x, y), False, True, self.select_mode.filter, False)
        global tools
        tools = []
        make_container = len(objects)>1
        for object in objects:
            object_tools = self.GetObjectTools(object, control_pressed)
            if make_container:
                tool_list = ContextTool.ObjectToolList(object)
                tool_list.tools += object_tools
                tools.append(tool_list)
            else:
                tools.append(ContextTool.ObjectTitleTool(object))# object name with icon
                tools += object_tools
            
        return tools
    
    def GetSelectionFilterTools(self, filter):
        # get tools for the selection given filter
        tools = []

        if filter.Size() > 0:
            tools.append(ContextTool.CADContextTool('Delete Marked Items', 'delete', self.DeleteMarkedList))
                
        if self.SketchChildTypeInFilter(filter):
            tools.append(ContextTool.CADContextTool('Make to Sketch', 'makesketch', self.MakeToSketch))

        if filter.IsTypeInFilter(cad.OBJECT_TYPE_SKETCH):
            tools.append(ContextTool.CADContextTool('Combine sketches', 'sketchjoin', cad.CombineSelectedSketches))
        
        first_coord_sys = None
        second_coord_sys = None
        
        for object in cad.GetSelectedObjects():
            if object.GetType() == cad.OBJECT_TYPE_COORD_SYS:
                if first_coord_sys == None: first_coord_sys = object
                elif second_coord_sys == None: second_coord_sys = object

        return tools
    
    def GetSelectionTools(self):
        return self.GetSelectionFilterTools(cad.GetSelectionTypes())
    
    def SketchChildTypeInFilter(self, filter):
        if filter.IsTypeInFilter(cad.OBJECT_TYPE_SKETCH_LINE): return True
        if filter.IsTypeInFilter(cad.OBJECT_TYPE_SKETCH_ARC): return True
        return False
    
    def DeleteMarkedList(self):
        cad.DeleteObjectsUndoably(cad.GetSelectedObjects())
        
    def MakeToSketch(self):
        objects_to_delete = []
        sketch = cad.NewSketch()
        
        for object in cad.GetSelectedObjects():
            t = object.GetType()
            if self.ObjectCanBeSketchChild(object):
                new_object = object.MakeACopy()
                objects_to_delete.append(object)
                sketch.Add(new_object)
                
        cad.AddUndoably(sketch)
        cad.DeleteObjectsUndoably(objects_to_delete)
    
    def AddPointToDrawing(self):
        cad.AddDrawingPoint()
        
    def EndDrawing(self):
        cad.EndDrawing()
    
    def GetInputModeTools(self):
        tools = []
        input_mode_class = cad.GetInputMode().__class__
        if input_mode_class == cad.LineArcDrawing:
            tools.append(ToolBarTool.CadToolBarTool('Add Point', 'add', self.AddPointToDrawing))
            tools.append(ToolBarTool.CadToolBarTool('Stop Drawing', 'enddraw', self.EndDrawing))
        
        return tools
    
    def SetMenuItemBitmap(self, item, tool):
        bitmap_path = tool.BitmapPath()
        if bitmap_path:
            image = wx.Image(bitmap_path)
            image.Rescale(tool.BitmapSize(), tool.BitmapSize())
            item.SetBitmap(wx.Bitmap(image))
    
    def AddToolToListAndMenu(self, tool, menu):
        if tool == None:
            menu.AppendSeparator()
        else:
            if tool.IsAToolList():
                menu2 = wx.Menu()
                for tool2 in tool.GetTools():
                    self.AddToolToListAndMenu(tool2, menu2)
                item = wx.MenuItem(menu, wx.ID_ANY, tool.GetTitle())
                item.SetSubMenu(menu2)
                self.SetMenuItemBitmap(item, tool)
                menu.Append(item)
            else:
                item = wx.MenuItem(menu, wx.ID_ANY, tool.GetTitle())        
                self.SetMenuItemBitmap(item, tool)
                self.Bind(wx.EVT_MENU, tool.Run, menu.Append(item))     
                if not tool.IsEnabled():
                    menu.Enable(item.GetId(), False)
        
    def GetDropDownTools(self, x, y, control_pressed):
        tools = self.GetTools(x, y, control_pressed)

        # add a separator
        if len(tools) > 0 and tools[-1] != None:
            tools.append(None) # separator
        
        # add selection tools
        tools += self.GetSelectionTools()
        
        #cad.GetSelectedItemsTools(marked_object, new_point, True)
        #GenerateIntersectionMenuOptions( f_list );
        #self.AddToolListWithSeparator(tools, cad.GetInputMode().GetTools())
        
        #if(m_current_coordinate_system)f_list.push_back(&coord_system_unset);
        
        #// exit full screen
        #if(m_frame->IsFullScreen() && point.x>=0 && point.y>=0)temp_f_list.push_back(new CFullScreenTool);
        #AddToolListWithSeparator(f_list, temp_f_list);
                                   
        return tools
        
        
    def DoDropDownMenu(self, wnd, x, y, control_pressed):
        tools = self.GetDropDownTools(x, y, control_pressed)
        menu = wx.Menu()
        for tool in tools:
            self.AddToolToListAndMenu(tool, menu)
        wnd.PopupMenu(menu, wx.Point(x, y))
        
    def RestoreDefaults(self):
        config = HeeksConfig()
        config.DeleteAll()
        self.settings_restored = True
        
    def StartPickObjects(self, str, filter, just_one):
        global save_filter_for_StartPickObjects
        global save_just_one_for_EndPickObjects
        global save_mode_for_EndPickObjects
        save_mode_for_EndPickObjects = cad.GetInputMode()
        self.select_mode.prompt = str
        save_just_one_for_EndPickObjects = self.select_mode.just_one
        self.select_mode.just_one = just_one
        cad.SetInputMode(self.select_mode)
        save_filter_for_StartPickObjects = self.select_mode.filter
        self.select_mode.filter = filter
        
    def EndPickObjects(self):
        global save_filter_for_StartPickObjects
        global save_just_one_for_EndPickObjects
        global save_mode_for_EndPickObjects
        self.select_mode.filter = save_filter_for_StartPickObjects
        self.select_mode.prompt = ''
        self.select_mode.just_one = save_just_one_for_EndPickObjects
        cad.SetInputMode(save_mode_for_EndPickObjects)
        
    def PickObjects(self, str, filter = -1, just_one = False):
        if self.inMainLoop:
            wx.MessageBox('recursive call to PickObjects')
            return
        
        self.StartPickObjects(str, filter, just_one)

        self.OnRun()
        
        return self.EndPickObjects()
        
    def OnRun(self):
        self.inMainLoop = True
        wx.App.OnRun(self)
        self.inMainLoop = False        
        
    def PickPosition(self, title):
        save_mode = cad.GetInputMode()
        digitizing = cad.GetDigitizing()
        digitizing.wants_to_exit_main_loop = False
        digitizing.prompt = title
        cad.SetInputMode(digitizing)
        
        self.OnRun()

        return_point = None
        if digitizing.digitized_point.type != cad.DigitizeType.DIGITIZE_NO_ITEM_TYPE:
            import geom
            return_point = geom.Point3D(digitizing.digitized_point.point)
        cad.SetInputMode(save_mode);
        return return_point
        
    def GetViewport(self):
        return self.frame.graphics_canvas.viewport
    
    def IsSolidApp(self):
        return False
    
    def AddExtraWindows(self, frame):
        pass
    
    def GetOptions(self):
        properties = []
        
        properties.append(PyProperty("End Beyond Full Profile", 'end_beyond_full_profile', self))
        
        return properties
        
    def CheckForNumberOrMore(self, min_num, types, msg, caption):
        num_found = 0
        for object in cad.GetSelectedObjects():
            for type in types:
                if object.GetType() == type:
                    num_found += 1
                    break
                
        if num_found < min_num:
            filter = cad.Filter()
            for t in types: filter.AddType(t)
            objects = self.PickObjects(msg, filter, False)
            if cad.GetNumSelected() < min_num:
                wx.MessageBox(msg)
                return False
        return True

    def InputLength(self, prompt, name, value):
        dlg = HDialog('Input')
        sizerMain = wx.BoxSizer(wx.VERTICAL)
        static_label = wx.StaticText(dlg, label = prompt)
        sizerMain.Add( static_label, 0, wx.ALL, wx.ALIGN_LEFT, control_border )
        value_control = LengthCtrl(dlg)
        value_control.SetValue(value)
        dlg.AddLabelAndControl( sizerMain, name, value_control )
        dlg.MakeOkAndCancel( wx.HORIZONTAL ).AddToSizer( sizerMain )
        dlg.SetSizer( sizerMain )
        sizerMain.SetSizeHints(dlg)
        sizerMain.Fit(dlg)
        value_control.SetFocus()
        if dlg.ShowModal() == wx.ID_OK:
            return value_control.GetValue()
        return None
        
    def OnKeyDown(self, e):
        k = e.GetKeyCode()
        if k == wx.WXK_DELETE:
            if cad.GetNumSelected() > 0:
                cad.StartHistory()
                for object in cad.GetSelectedObjects():
                    cad.DeleteUndoably(object)
                cad.EndHistory()
                cad.ClearSelection(True)
        elif k == wx.WXK_RETURN:
            if self.inMainLoop:
                self.ExitMainLoop()
                self.Repaint()
        elif k == ord('Z'):
            if e.ControlDown():
                if e.ShiftDown():
                    self.OnRedo(e)
                else:
                    self.OnUndo(e)
        elif k == ord('X'):
            if e.ControlDown():
                self.OnCut(e)
        elif k == ord('C'):
            if e.ControlDown():
                self.OnCopy(e)
        elif k == ord('V'):
            if e.ControlDown():
                self.OnPaste(e)
        else:
            return False
        return True
    
    def Repaint(self):
        self.frame.graphics_canvas.Update()
        self.frame.graphics_canvas.Refresh()
    
    def AddExtraRibbonPages(self, ribbon):
        pass
    
    def AddOptionsRibbonPanels(self, ribbon):
        panel = RB.RibbonPanel(ribbon.options_page, wx.ID_ANY, 'View', ribbon.Image('mag'))
        self.view_toolbar = RB.RibbonButtonBar(panel)
        from Ribbon import ScreenTextButton
        from Ribbon import ModeButton
        ScreenTextButton().AddToToolbar(self.view_toolbar)
        ModeButton( cad.GetRotateUpright, cad.SetRotateUpright, 'rotate upright', 'rotate free', "RotateUpright", 'Rotate Upright', 'Rotate Free').AddToToolbar(self.view_toolbar)
        ModeButton( cad.GetAntialiasing, cad.SetAntialiasing, 'smoothed', 'pixelated', "Antialiasing", 'Smoothed', 'Pixelated', 'Lines are drawn with antialiasing', 'Lines are drawn withput antialiasing').AddToToolbar(self.view_toolbar)
        ModeButton( cad.GetShowDatum, cad.SetShowDatum, 'showdatum', 'hidedatum', "ShowDatum", 'Hide Datum', 'Show Datum', 'Showing the Datum', 'Not Showing the Datum').AddToToolbar(self.view_toolbar)
        
        panel = RB.RibbonPanel(ribbon.options_page, wx.ID_ANY, 'View Colors', ribbon.Image('mag'))
        toolbar = RB.RibbonButtonBar(panel)
        from Ribbon import BackgroundColorButton
        BackgroundColorButton('Background Color Top', 'Edit top background color').AddToToolbar(toolbar)
        BackgroundColorButton('Background Color Bottom', 'Edit bottom background color').AddToToolbar(toolbar)
        
    def OnAntialiasing(self, event):
        cad.SetAntialiasing(event.IsChecked())
        config = HeeksConfig()
        config.WriteBool('Antialiasing', cad.GetAntialiasing())
        self.frame.graphics_canvas.Refresh()
        
    def OnNew(self, e):
        res = self.CheckForModifiedDoc()
        if res != wx.CANCEL:
            cad.Reset()
            self.OnNewOrOpen(False)
            cad.ClearHistory()
            cad.SetLikeNewFile()
            self.filepath = None
            self.frame.SetFrameTitle()
        
    def SaveProject(self, force_dialog = False):
        if self.GetProjectFileName().IsOk():
            self.SaveFile()
        
    def CheckForModifiedDoc(self, force_dialog = False):
        # returns wxCANCEL if not OK to continue opening file
        if cad.IsModified():
            str = 'Save changes to file ' + (self.filepath if self.filepath else 'Untitled')
            res = wx.MessageBox(str, wx.MessageBoxCaptionStr, wx.CANCEL | wx.YES_NO | wx.CENTER | wx.ICON_WARNING)
            if res == wx.CANCEL or res == wx.NO: return res
            if res == wx.YES:
                return self.OnSave(None)

        return wx.OK
    
    def DoFileOpenViewMag(self):
        self.frame.graphics_canvas.viewport.OnMagExtents(True, 25)
    
    def OnOpenFilepath(self, filepath, check = True):
        if check:
            res = self.CheckForModifiedDoc()
        else:
            res = wx.OK
        if res != wx.CANCEL:
            # self.OnBeforeNewOrOpen(True, res)
            cad.Reset()
            if cad.OpenFile(filepath):
                self.DoFileOpenViewMag()
                self.OnNewOrOpen(True)
                cad.ClearHistory()
                cad.SetLikeNewFile()
                self.filepath = filepath
                self.frame.SetFrameTitle()
                self.InsertRecentFileItem(filepath)
                self.WriteRecentFiles()
                return True
            else:
                wx.MessageBox('Invalid file type chosen expecting .heeks')
        return True

    def OnOpen(self, e):
        dialog = wx.FileDialog(self.frame, 'Open File', self.GetDefaultDir(), '', HEEKS_WILDCARD_STRING)
        dialog.CenterOnParent()
        
        if dialog.ShowModal() == wx.ID_OK:
            self.OnOpenFilepath(dialog.GetPath())
                    
    def OnOpenRecent(self, e):
        file_path = self.recent_files[e.GetId() - self.ID_RECENT_FIRST]
        self.OnOpenFilepath(file_path)
        
    def OnUpdateOpenRecent(self, e):
        size = self.recent_files_menu.GetMenuItemCount()
        menu_items = []
        for i in range(0, size):
            menu_items.append(self.recent_files_menu.FindItemByPosition(i))
        for menu_item in menu_items:
            self.recent_files_menu.Delete(menu_item)
            
        recent_id = self.ID_RECENT_FIRST
        for filepath in self.recent_files:
            self.recent_files_menu.Append(recent_id, filepath)
            recent_id += 1
            
    def OnSaveFilepath(self, filepath):
        if cad.SaveFile(filepath):
            self.filepath = filepath        
            cad.SetLikeNewFile()
            self.frame.SetFrameTitle()
            self.InsertRecentFileItem(filepath)
            self.WriteRecentFiles()
            return wx.ID_OK
        return wx.ID_CANCEL
            
    def OnSave(self, e):
        if self.filepath:
            return self.OnSaveFilepath(self.filepath)

        dialog = wx.FileDialog(self.frame, 'Save File', self.GetDefaultDir(), '', HEEKS_WILDCARD_STRING, wx.FD_SAVE | wx.FD_OVERWRITE_PROMPT)
        dialog.SetFilterIndex(1)
        dialog.CenterOnParent()
        if dialog.ShowModal() == wx.ID_CANCEL:
            return wx.ID_CANCEL
        return self.OnSaveFilepath(dialog.GetPath())
            
    def OnUpdateSave(self, e):
        e.Enable(cad.IsModified())            
            
    def OnSaveAs(self, e):
        if self.filepath:
            default_directory = ''
            default_filepath = self.filepath
        else:
            default_directory = self.GetDefaultDir()
            default_filepath = ''            
        
        dialog = wx.FileDialog(self.frame, 'Save File', default_directory, default_filepath, HEEKS_WILDCARD_STRING, wx.FD_SAVE | wx.FD_OVERWRITE_PROMPT)
        dialog.SetFilterIndex(0)
        dialog.CenterOnParent()
        if dialog.ShowModal() != wx.ID_CANCEL:
            self.OnSaveFilepath(dialog.GetPath())
                    
    def GetImportWildcardString(self):
        wild_card_string1 = 'Known Files |*.heeks;*.HEEKS;*.dxf;*.DXF;*.stl'
        wild_card_string2 = '|Heeks files (*.heeks)|*.heeks;*.HEEKS|STL files (*.stl)|*.stl;*.STL|Scalar Vector Graphics files (*.svg)|*.svg;*.SVG|DXF files (*.dxf)|*.dxf;*.DXF|RS274X/Gerber files (*.gbr,*.rs274x)|*.gbr;*.GBR;*.rs274x;*.RS274X;*.pho;*.PHO'
        
        wild_card_string1, wild_card_string2 = AddToWildcardStrings(wild_card_string1, wild_card_string2, self.import_file_types)
        
        return wild_card_string1 + wild_card_string2

    def GetExportWildcardString(self):
        wild_card_string1 = 'Known Files |*.stl;*.dxf;*.cpp;*.py;*.obj'
        wild_card_string2 = '|STL files (*.stl)|*.stl|DXF files (*.dxf)|*.dxf|CPP files (*.cpp)|*.cpp|OpenCAMLib python files (*.py)|*.py|Wavefront .obj files (*.obj)|*.obj'
        
        wild_card_string1, wild_card_string2 = AddToWildcardStrings(wild_card_string1, wild_card_string2, self.export_file_types)
        
        return wild_card_string1 + wild_card_string2
                                         
    def OnImport(self, e):
        config = HeeksConfig()
        default_directory = config.Read('ImportDirectory', self.GetDefaultDir())
        
        dialog = wx.FileDialog(self.frame, 'Import File', default_directory, '', self.GetImportWildcardString())
        dialog.CenterOnParent()
        
        if dialog.ShowModal() == wx.ID_OK:
            filepath = dialog.GetPath()
            res = cad.Import(filepath)
            if res:
                self.DoFileOpenViewMag()
                if self.filepath == None:
                    dot = filepath.rfind('.')
                    if dot != -1:
                        self.filepath = filepath[:dot+1] + 'heeks'
                self.frame.SetFrameTitle()
                config.Write('ImportDirectory', dialog.GetDirectory())
                cad.Repaint()
            
    def GetPathSuffix(self, path):
        dot = path.rfind('.')
        if dot == -1:
            return ''
        return path[dot+1:].lower()
        
    def RegisterImportFileTypes(self, suffix_list, description, ImportCallback):
        self.import_file_types.append((suffix_list, description))
        for suffix in suffix_list:
            cad.RegisterImportFileType(suffix, ImportCallback)
        
    def RegisterExportFileTypes(self, suffix_list, description, ExportCallback):        
        self.export_file_types.append((suffix_list, description))
        for suffix in suffix_list:
            cad.RegisterExportFileType(suffix, ExportCallback)
        
    def OnExport(self, e):
        config = HeeksConfig()
        default_directory = config.Read('ExportDirectory', self.GetDefaultDir())
        dialog = wx.FileDialog(self.frame, 'Export File', default_directory, '', self.GetExportWildcardString(), wx.FD_SAVE | wx.FD_OVERWRITE_PROMPT)
        dialog.CenterOnParent()
        
        if dialog.ShowModal() == wx.ID_OK:
            path = dialog.GetPath()
            suffix = self.GetPathSuffix(path)
            if suffix == 'svg':
                import Svg
                Svg.Export(path)
            else:
                cad.SaveFile(path)
            config.Write('ExportDirectory', dialog.GetDirectory())
            
    def OnAbout(self, e):
        dlg = AboutBox(self.frame)
        dlg.ShowModal()
                
    def OnPrint(self, e):
        printDialogData = wx.PrintDialogData(self.printData)
        printer = wx.Printer(printDialogData)
        self.printout = Printout()
        if printer.Print(self.frame, self.printout, True):
            theApp.printData = printer.GetPrintDialogData().GetPrintData()
        else:
            if wx.Printer.GetLastError() == wx.PRINTER_ERROR:
                wx.MessageBox('There was a problem printing.\nPerhaps your current printer is not set correctly?', 'Printing', wx.OK)
            else:
                wx.MessageBox('You canceled printing', 'Printing', wx.OK)
                
        self.printout = None
            
    def OnPageSetup(self, e):
        pageSetupData = self.printData
        
        pageSetupDialog = wx.PageSetupDialog(self.frame, self.pageSetupData)
        pageSetupDialog.ShowModal()
        
        self.printData = pageSetupDialog.GetPageSetupData().GetPrintData()
        self.pageSetupData = pageSetupDialog.GetPageSetupData()
            
    def OnPrintPreview(self, e):
        printDialogData = wx.PrintDialogData(self.printData)
        preview = wx.PrintPreview(Printout(), Printout(), printDialogData)
        if not preview.IsOk():
            preview = None
            wx.MessageBox('There was a problem previewing.\nPerhaps your current printer is not set correctly?', 'Previewing', wx.OK)

        frame = wx.PreviewFrame(preview, self.frame, 'Print Preview', wx.Point(100,100), wx.Size(600, 650))
        frame.Centre(wx.BOTH)
        frame.Initialize()
        frame.Show()
    
            
    def OnResetDefaults(self, e):
        self.RestoreDefaults()
        wx.MessageBox('You must restart the application for the settings to be changed')
            
    def OnQuit(self, e):
        pass # to do            
            
    def OnUndo(self, e):
        cad.RollBack()
        
    def OnRedo(self, e):
        cad.RollForward()
        
    def CopySelectedItems(self):
        temp_file = wx.StandardPaths.Get().GetTempDir() + '/temp_Heeks_clipboard_file.xml'
        cad.SaveObjects(temp_file, cad.GetSelectedObjects())
        
        f = open(temp_file)
        s = f.read()
        
        if wx.TheClipboard.Open():
            wx.TheClipboard.SetData(wx.TextDataObject(s))
            wx.TheClipboard.Close()
            
        f.close()
        
    def OnCut(self, e):
        self.CopySelectedItems()
        cad.StartHistory()
        for object in cad.GetSelectedObjects():
            cad.DeleteUndoably(object)
        cad.EndHistory()
        
    def OnUpdateCut(self, e):
        e.Enable(cad.GetNumSelected() > 0)            
                
    def OnUpdateUndo(self, e):
        e.Enable(cad.CanUndo())         
        
    def OnUpdateRedo(self, e):
        e.Enable(cad.CanRedo())         
        
    def OnCopy(self, e):
        self.CopySelectedItems()
        
    def OnUpdateCopy(self, e):
        e.Enable(cad.GetNumSelected() > 0)            
        
    def OnPaste(self, e):
        s = None
        
        if wx.TheClipboard.Open():
            if wx.TheClipboard.IsSupported(wx.DataFormat(wx.DF_TEXT)):
                data = wx.TextDataObject()
                wx.TheClipboard.GetData(data)
                s = data.GetText()                
            wx.TheClipboard.Close()
            
        if s == None:
            return

        temp_file = wx.StandardPaths.Get().GetTempDir() + '/temp_Heeks_clipboard_file.xml'
        f = open(temp_file, 'w')
        f.write(s)
        f.close()
        cad.ClearSelection()
        cad.SetMarkNewlyAddedObjects(True)
        cad.Import(temp_file)
        cad.SetMarkNewlyAddedObjects(False)
        
    def IsPasteReady(self):
        if wx.TheClipboard.IsOpened():
            return False
        
        if wx.TheClipboard.Open():
            s = ''
            if wx.TheClipboard.IsSupported(wx.DataFormat(wx.DF_TEXT)):
                data = wx.TextDataObject()
                wx.TheClipboard.GetData(data)
                s = data.GetText()
            wx.TheClipboard.Close()
            
            if s[:19] == '<?xml version="1.0"':
                return True
        
        return False
    
    def OnUpdatePaste(self, e):
        e.Enable(self.IsPasteReady())    
        
    def OnDelete(self, e):
        cad.StartHistory()
        for object in cad.GetSelectedObjects():
            cad.DeleteUndoably(object)
        cad.EndHistory()
        
    def OnUpdateDelete(self, e):
        e.Enable(cad.GetNumSelected() > 0)            
    
    def OnSelectMode(self, e):
        cad.SetInputMode(self.select_mode)
        
    def OnFilter(self, e):
        dlg = FilterDlg()
        if dlg.ShowModal() == wx.ID_OK:
            dlg.SetFilterFromCheckBoxes()
        
    def OnMagPrevious(self, e):
        self.frame.graphics_canvas.viewport.RestorePreviousViewPoint()
        self.frame.graphics_canvas.Refresh()
        
    def OnMag(self, e):
        cad.SetInputMode(cad.GetMagnification())
        
    def OnMagExtents(self, e):
        self.frame.graphics_canvas.viewport.OnMagExtents(True, 6)
        self.frame.graphics_canvas.Refresh()
        
    def OnMagNoRot(self, e):
        self.frame.graphics_canvas.viewport.OnMagExtents(False, 6)
        self.frame.graphics_canvas.Refresh()
        
    def OnMagAxes(self, unitY, unitZ):
        self.frame.graphics_canvas.viewport.ClearViewpoints()
        self.frame.graphics_canvas.viewport.view_point.SetView(unitY, unitZ, 6)
        self.frame.graphics_canvas.viewport.StoreViewPoint()
        self.frame.graphics_canvas.Refresh()
        
    def OnMagXY(self, e):
        self.OnMagAxes(geom.Point3D(0,1,0), geom.Point3D(0,0,1))
        
    def OnMagXYM(self, e):
        self.OnMagAxes(geom.Point3D(0,1,0), geom.Point3D(0,0,-1))
        
    def OnMagXZ(self, e):
        self.OnMagAxes(geom.Point3D(0,0,-1), geom.Point3D(0,1,0))
        
    def OnMagXZM(self, e):
        self.OnMagAxes(geom.Point3D(0,0,1), geom.Point3D(0,-1,0))
        
    def OnMagYZ(self, e):
        self.OnMagAxes(geom.Point3D(0,1,0), geom.Point3D(1,0,0))
        
    def OnMagYZM(self, e):
        self.OnMagAxes(geom.Point3D(0,1,0), geom.Point3D(1,0,0))
        
    def OnMagXYZ(self, e):
        s = 0.5773502691896257
        self.OnMagAxes(geom.Point3D(-s,s,s), geom.Point3D(s,-s,s))
        
    def OnViewRotate(self, e):
        cad.SetInputMode(cad.GetViewRotating())
        
    def OnViewZoom(self, e):
        cad.SetInputMode(cad.GetViewZooming())
        
    def OnViewPan(self, e):
        cad.SetInputMode(cad.GetViewPanning())
        
    def ShowFullScreen(self, show, style = wx.FULLSCREEN_ALL):
        if show:
            self.windows_visible = {}
            for w in self.hideable_windows:
                self.windows_visible[w] = self.frame.aui_manager.GetPane(w).IsShown() and w.IsShown()
                self.frame.aui_manager.GetPane(w).Show(False)
            self.frame.graphics_canvas.SetFocus()# so escape key works to get out
        else:
            for w in self.hideable_windows:
                if w in self.windows_visible:
                    visible = self.windows_visible[w]
                    self.frame.aui_manager.GetPane(w).Show(visible)
            
        res = self.frame.ShowFullScreen(show, style)
        self.frame.aui_manager.Update()
        return res
        
    def OnFullScreen(self, e):
        self.ShowFullScreen(True)
        
    def OnRedraw(self, e):
        cad.GetApp().KillGLLists()
        self.frame.graphics_canvas.Refresh()
        
    def OnLines(self, e):
        cad.SetInputMode(self.select_mode) # mode to return to on ending drawing
        cad.SetLineArcDrawing()
        
    def OnRectangles(self, e):
        cad.SetInputMode(self.select_mode) # mode to return to on ending drawing
        cad.SetRectanglesDrawing()
        
    def OnObrounds(self, e):
        cad.SetInputMode(self.select_mode) # mode to return to on ending drawing
        cad.SetObroundsDrawing()
        
    def OnPolygons(self, e):
        cad.SetInputMode(self.select_mode) # mode to return to on ending drawing
        cad.SetPolygonsDrawing()
        
    def OnCircles3p(self, e):
        cad.SetInputMode(self.select_mode) # mode to return to on ending drawing
        cad.SetCircles3pDrawing()
        
    def OnCircles2p(self, e):
        cad.SetInputMode(self.select_mode) # mode to return to on ending drawing
        cad.SetCircles2pDrawing()
        
    def OnCircles1p(self, e):
        cad.SetInputMode(self.select_mode) # mode to return to on ending drawing
        cad.SetCircle1pDrawing()
        
    def OnILine(self, e):
        cad.SetInputMode(self.select_mode) # mode to return to on ending drawing
        cad.SetILineDrawing()
        
    def OnPoints(self, e):
        cad.SetInputMode(self.select_mode) # mode to return to on ending drawing
        cad.SetInputMode(point_drawing)
        
    def OnGear(self, e):
        gear = Gear.Gear(1.0, 12)
        cad.AddUndoably(gear, None, None)
        
    def OnText(self, e):
        cad.AddUndoably(cad.NewText("text"))
        
    def OnDimensioning(self, e):
        pass
    
    def RenderCoordSys(self):
        p = cad.GetDigitizing().digitized_point.point
        if self.coord_render_mode == 0:
            self.coordsys_for_P3P.o = p
        elif self.coord_render_mode == 1:
            if p != self.coordsys_for_P3P.o:
                self.coordsys_for_P3P.x = (p - self.coordsys_for_P3P.o).Normalized()
                if self.coordsys_for_P3P.x.Dist(self.z_for_P3P) < 0.0000000001 or self.coordsys_for_P3P.x.Dist(-self.z_for_P3P) < 0.0000000001:
                    self.coordsys_for_P3P.y = self.y_for_P3P ^ self.coordsys_for_P3P.x
                else:
                    self.coordsys_for_P3P.y = self.z_for_P3P ^ self.coordsys_for_P3P.x
        elif self.coord_render_mode == 2:
            if p != self.coordsys_for_P3P.o:
                y = (p - self.coordsys_for_P3P.o).Normalized()
                if y.Dist(self.coordsys_for_P3P.x) > 0.0000000001 and y.Dist(-self.coordsys_for_P3P.x) > 0.0000000001:
                    z = self.coordsys_for_P3P.x ^ y
                    self.coordsys_for_P3P.y = z ^ self.coordsys_for_P3P.x
        self.coordsys_for_P3P.OnGlCommands(False, False, False)
    
    def MakeOriginFromPickPoints(self, coordsys, three_points):
        self.coordsys_for_P3P = coordsys
        self.y_for_P3P = coordsys.y
        self.z_for_P3P = coordsys.x ^ coordsys.y
        coordsys.SetVisible(False)
        if self.paint_registered == False:
            cad.RegisterOnGLCommands(OnPaint)
            self.paint_registered = True
        
        self.coord_render_mode = 0
        
        ret = True

        if self.PickPosition('Pick the location') == None:
            ret = False

        if three_points:
            self.coord_render_mode = 1
    
            if ret and self.PickPosition('Pick a point on the x-axis') == None:
                ret = False
    
            self.coord_render_mode = 2
            
            if ret and self.PickPosition('Pick a point where y > 0') == None:
                ret = False
        
        self.coordsys_for_P3P = None
        coordsys.SetVisible(True)
        
        return ret
    
    def OnSetOriginPoints(self, three_points):
        mat = cad.GetDrawMatrix(False)
        o = geom.Point3D(0,0,0).Transformed(mat)
        x = geom.Point3D(1,0,0).Transformed(mat) - o
        y = geom.Point3D(0,1,0).Transformed(mat) - o
        new_object = cad.NewCoordinateSystem("Coordinate System", o, x, y)
        cad.ClearSelection()
        cad.SetInputMode(self.select_mode)

        # and pick from three points
        result = self.MakeOriginFromPickPoints(new_object, three_points)
        if result:
            cad.AddUndoably(new_object)
            cad.Select(new_object)
        else:
            cad.DeleteInternalObject(new_object)

    def OnSetOrigin3Points(self, e):
        self.OnSetOriginPoints(True)

    def OnSetOrigin1Point(self, e):
        self.OnSetOriginPoints(False)
        
    def OnMoveTranslate(self, e):
        from Transform import Translate
        Translate()
        
    def OnCopyTranslate(self, e):
        from Transform import Translate
        Translate(True)
        
    def OnMoveRotate(self, e):
        from Transform import Rotate
        Rotate()
        
    def OnCopyRotate(self, e):
        from Transform import Rotate
        Rotate(True)

    def OnMirror(self, e):
        from Transform import Mirror
        Mirror()
        
    def OnCopyMirror(self, e):
        from Transform import Mirror
        Mirror(True)
        
    def OnMoveScale(self, e):
        pass
    
    def OnOriginTransform(self, e):
        from Transform import OriTransform
        OriTransform()
    
    def OnViewObjects(self, e):
        pane_info = self.aui_manager.GetPane(self.tree_canvas)
        if pane_info.IsOk():
            pane_info.Show(e.IsChecked())
            self.aui_manager.Update()
    
    def OnUpdateViewObjects(self, e):
        e.Check(self.aui_manager.GetPane(self.tree_canvas).IsShown())

    def OnViewInput(self, e):
        pane_info = self.aui_manager.GetPane(self.input_mode_canvas)
        if pane_info.IsOk():
            pane_info.Show(e.IsChecked())
            self.aui_manager.Update()
    
    def OnUpdateViewInput(self, e):
        e.Check(self.aui_manager.GetPane(self.input_mode_canvas).IsShown())

    def OnViewProperties(self, e):
        pane_info = self.aui_manager.GetPane(self.properties_canvas)
        if pane_info.IsOk():
            pane_info.Show(e.IsChecked())
            self.aui_manager.Update()
        
    def OnUpdateViewProperties(self, e):
        e.Check(self.aui_manager.GetPane(self.properties_canvas).IsShown())

    def BitmapPath(self, name):
        return self.bitmap_path + '/'+ name + '.png'
    
    def OnLeftClick(self, event):
        # select one object
        objects = cad.ObjectsUnderWindow(cad.IRect(self.select_mode.button_down_point.x, self.select_mode.button_down_point.y), False, True, self.select_mode.filter, True)
        if len(objects) > 0:
            object = objects[0]
            
            if event.controlDown:
                if cad.ObjectMarked(object):
                    cad.Unselect(object, True)
                else:
                    cad.Select(object)
            else:
                cad.ClearSelection(True)
                cad.Select(object)
        else:
            cad.ClearSelection(True)

        
class CopyObjectUndoable(cad.BaseUndoable):
    def __init__(self, object, copy_object):
        cad.BaseUndoable.__init__(self)
        self.object = object
        self.new_copy = copy_object
        self.old_copy = object.MakeACopy()
        self.old_copy.Clear()
        self.new_copy.Clear()
        
    def Run(self, redo):
        self.object.CopyFrom(self.new_copy)
        cad.WasModified(self.object)
        
    def RollBack(self):
        self.object.CopyFrom(self.old_copy)
        cad.WasModified(self.object)
        
def AddToWildcardStrings(wild_card_string1, wild_card_string2, file_types):
    for suffix_list, description in file_types:
        wild_card_string2 += '|' + description + ' ('
        imageExtStr = ''
        imageExtStr2 = ''
        for suffix in suffix_list:
            wild_card_string1 += ';*.'
            wild_card_string1 += suffix
            if imageExtStr:
                imageExtStr += ' '
                imageExtStr2 += ';'
            imageExtStr += ('*.' + suffix)
            imageExtStr2 += ('*.' + suffix)
        wild_card_string2 += (imageExtStr + ')|' + imageExtStr2)
        
    return wild_card_string1, wild_card_string2
        
        