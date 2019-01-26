import wx
import sys
import cad
from Frame import Frame
import os
from HeeksConfig import HeeksConfig
from ContextTool import ContextTool
from ContextTool import ContextToolList
from ContextTool import ObjectToolList

pycad_dir = os.path.dirname(os.path.realpath(__file__))

def OnMessageBox(error_message):
    wx.MessageBox(error_message)
        
def OnContextMenu():
    wx.MessageBox('In OnContextMenu')

class App(wx.App):
    def __init__(self):
        self.settings_restored = False
        self.recent_files = []
        self.MAX_RECENT_FILES = 20
        self.filepath = None
        self.hideable_windows = []
        self.cad_dir = pycad_dir
        
        save_out = sys.stdout
        save_err = sys.stderr
        wx.App.__init__(self)
        sys.stdout = save_out
        sys.stderr = save_err
        
    def OnInit(self):
        self.RegisterMessageBoxCallback()
        self.InitCad()
        
        self.printData = wx.PrintData()
        self.pageSetupData = wx.PageSetupDialogData(self.printData)
        
        self.frame = self.NewFrame()
        self.frame.Show()
        self.LoadConfig()
        self.OnNewOrOpen(False)
        cad.ClearHistory()
        cad.SetLikeNewFile()
        self.frame.SetFrameTitle()
        
        return True
    
    def OnNewOrOpen(self, open):
        pass
    
    def NewFrame(self):
        return Frame(None)

    def LoadConfig(self):
        config = HeeksConfig(self.settings_restored)
        self.LoadRecentFiles(config)
        
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
        cad.SetInputMode(cad.GetSelectMode());
        cad.SetResFolder(pycad_dir)
        cad.SetContextMenuCallback(OnContextMenu)
        
    def GetObjectTools(self, object):
        tools = []
        tools.append(ContextTool())
        return tools
        
    def GetTools(self, x, y, control_pressed):
        objects = cad.GetClickedObjects(x, y)
        tools = []
        make_container = len(objects)>1
        for object in objects:
            if make_container:
                tool_list = ObjectToolList(object)
                tool_list.tools += self.GetObjectTools(object)
                tools.append(tool_list)
            else:
                tools += self.GetObjectTools(object)
            
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