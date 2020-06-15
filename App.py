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

pycad_dir = os.path.dirname(os.path.realpath(__file__))

def OnMessageBox(error_message):
    wx.MessageBox(error_message)
    
def OnInputMode():
    wx.GetApp().OnInputMode()
    
tools = []
save_filter_for_StartPickObjects = 0
save_just_one_for_EndPickObjects = False
save_mode_for_EndPickObjects = None

class App(wx.App):
    def __init__(self):
        self.version_number = '2 0' # just main number and sub number. I'm not using sub-sub numbers
        self.settings_restored = False
        self.recent_files = []
        self.MAX_RECENT_FILES = 20
        self.filepath = None
        self.hideable_windows = []
        self.cad_dir = pycad_dir
        self.inMainLoop = False
        self.select_mode = SelectMode.SelectMode()
        
        save_out = sys.stdout
        save_err = sys.stderr
        wx.App.__init__(self)
        sys.stdout = save_out
        sys.stderr = save_err
        
#    def GetAppName(self):
#        # override this in your derived App class, if you don't want to use the executable's name as the app's name
#        return 'Power CAD'
            
    def OnInit(self):
        self.RegisterMessageBoxCallback()
        self.InitCad()
        self.RegisterObjectTypes()
        
        self.printData = wx.PrintData()
        self.pageSetupData = wx.PageSetupDialogData(self.printData)
            
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
        pass
    
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
        
    def RegisterHideableWindow(self, w):
        self.hideable_windows.append(w)
        
    def RemoveHideableWindow(self, w):
        self.hideable_windows.remove(w)
        
    def InitCad(self):
        cad.SetInputMode(wx.GetApp().select_mode);
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

        if type == cad.OBJECT_TYPE_SKETCH:
            if self.object.GetNumChildren() > 1:
                tools.append(ContextTool.CADContextTool("Split Sketch", "splitsketch", self.SplitSketch))
                
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
        objects = cad.ObjectsUnderWindow(cad.IRect(x, y), False, True, self.select_mode.filter)
        global tools
        tools = []
        make_container = len(objects)>1
        for object in objects:
            if make_container:
                tool_list = ContextTool.ObjectToolList(object)
                tool_list.tools += self.GetObjectTools(object, control_pressed)
                tools.append(tool_list)
            else:
                tools += self.GetObjectTools(object, control_pressed)
            
        return tools
    
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
        
    def GetDropDownTools(self, x, y, control_pressed):
        self.tool_index_list = [] # list of tool and index pairs
        tools = self.GetTools(x, y, control_pressed)
        #tools += cad.GetSelectedItemsTools(marked_object, new_point, True)
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
        self.select_mode.prompt_when_doing_a_main_loop = str
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
        digitizing.m_prompt_when_doing_a_main_loop = title
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
            wx.MessageBox(msg, caption)
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
        return value
        
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
            if wx.GetApp().inMainLoop:
                wx.GetApp().ExitMainLoop()
        elif k == ord('Z'):
            if e.ControlDown():
                if e.ShiftDown():
                    self.frame.OnRedo(e)
                else:
                    self.frame.OnUndo(e)
        elif k == ord('X'):
            if e.ControlDown():
                self.frame.OnCut(e)
        elif k == ord('C'):
            if e.ControlDown():
                self.frame.OnCopy(e)
        elif k == ord('V'):
            if e.ControlDown():
                self.frame.OnPaste(e)
        else:
            return False
        return True
        
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
        
        
        