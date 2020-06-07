import wx
import wx.aui
from GraphicsCanvas import GraphicsCanvas
from TreeCanvas import TreeCanvas
from InputModeCanvas import InputModeCanvas
from PropertiesCanvas import PropertiesCanvas
from ObjPropsCanvas import ObjPropsCanvas
from Options import Options
import cad
import sys
import os
from HeeksConfig import HeeksConfig
from Printout import Printout
from PointDrawing import point_drawing
import geom
import Gear

pycad_dir = os.path.dirname(os.path.realpath(__file__))
HEEKS_WILDCARD_STRING = 'Heeks files |*.heeks;*.HEEKS'

class Frame(wx.Frame):
    def __init__(self, parent, id=-1, pos=wx.DefaultPosition, size=wx.DefaultSize, style=wx.DEFAULT_FRAME_STYLE, name=wx.FrameNameStr):
        wx.Frame.__init__(self, parent, id, '', pos, size, style, name)

        config = HeeksConfig()
        self.aui_manager = None
        self.ID_RECENT_FIRST = wx.ID_HIGHEST + 1
        self.ID_RECENT_MENU = self.ID_RECENT_FIRST + wx.GetApp().MAX_RECENT_FILES
        self.windows_visible = {}
        
        self.SetIcon(wx.Icon(pycad_dir + "/heekscad.png", wx.BITMAP_TYPE_PNG))
        
        self.MakeMenus()
        
        self.aui_manager = wx.aui.AuiManager()
        self.aui_manager.SetManagedWindow(self)

        self.graphics_canvas = self.MakeGraphicsCanvas()

        self.aui_manager.AddPane(self.graphics_canvas, wx.aui.AuiPaneInfo().Name('graphics').CentrePane().BestSize(wx.Size(800,800)))
        self.tree_canvas = TreeCanvas(self)
        self.aui_manager.AddPane(self.tree_canvas, wx.aui.AuiPaneInfo().Name('Objects').Caption('Objects').Left().BestSize(wx.Size(300,400)).Position(0))
        
        self.input_mode_canvas = InputModeCanvas(self)
        self.aui_manager.AddPane(self.input_mode_canvas, wx.aui.AuiPaneInfo().Name('Input').Caption('Input').Left().BestSize(wx.Size(300,200)).Position(0))
        
        self.properties_canvas = ObjPropsCanvas(self)
        self.aui_manager.AddPane(self.properties_canvas, wx.aui.AuiPaneInfo().Name('Properties').Caption('Properties').Left().BestSize(wx.Size(300,200)).Position(2))
        
        #self.options = Options(self)
        #self.aui_manager.AddPane(self.options, wx.aui.AuiPaneInfo().Name('Options').Caption('Options').Left().BestSize(wx.Size(300,200)).Position(1))
        
        wx.GetApp().RegisterHideableWindow(self.tree_canvas)
        wx.GetApp().RegisterHideableWindow(self.input_mode_canvas)
        wx.GetApp().RegisterHideableWindow(self.properties_canvas)
        #wx.GetApp().RegisterHideableWindow(self.options)
        
        self.AddExtraWindows()

        perspective = config.Read('AuiPerspective', 'default')
        if perspective != 'default':
            self.aui_manager.LoadPerspective(perspective)
        self.aui_manager.Update()
        
        self.Bind(wx.EVT_MENU_RANGE, self.OnOpenRecent, id=self.ID_RECENT_FIRST, id2=self.ID_RECENT_FIRST + wx.GetApp().MAX_RECENT_FILES)
        self.Bind(wx.EVT_SIZE, self.OnSize)
        self.Bind(wx.EVT_MOVE, self.OnMove)
        self.Bind(wx.EVT_CLOSE, self.OnClose)
        self.gears = []
        
    def MakeGraphicsCanvas(self):
        return GraphicsCanvas(self)
        
    def __del__(self):
        if self.aui_manager:
            str = self.aui_manager.SavePerspective()
            config = HeeksConfig()
            config.Write('AuiPerspective', str)
        
    def OnSize(self, e):
        size = e.GetSize()
        config = HeeksConfig()
        config.WriteInt('MainFrameWidth', size.GetWidth())
        config.WriteInt('MainFrameHeight', size.GetHeight())
        
    def OnMove(self, e):
        pos = self.GetPosition()
        config = HeeksConfig()
        config.WriteInt('MainFramePosX', pos.x)
        config.WriteInt('MainFramePosY', pos.y)
        
    def OnClose(self, e):
        if e.CanVeto() and self.CheckForModifiedDoc() == wx.CANCEL:
            e.Veto()
            return
        e.Skip()
        self.aui_manager.UnInit()
        
    def MakeMenus(self):
        self.menuBar = wx.MenuBar()
        self.current_menu_stack = []
        self.bitmap_path = pycad_dir + '/bitmaps'

        self.AddMenu('&File')
        self.AddMenuItem('&New', self.OnNew, None, 'new')
        self.AddMenuItem('&Open', self.OnOpen, None, 'open')
        self.recent_files_menu = self.AddMenu('Open Recent', 'recent', self.OnUpdateOpenRecent)
        self.EndMenu()
        self.AddSeparator()
        self.AddMenuItem('&Save\tCtrl+S', self.OnSave, self.OnUpdateSave, 'save')
        self.AddMenuItem('Save &As...', self.OnSaveAs, None, 'saveas')
        self.AddSeparator()
        self.AddMenuItem('&Import', self.OnImport, None, 'import')
        self.AddMenuItem('&Export', self.OnExport, None, 'export')
        self.AddSeparator()
        self.AddMenuItem('&Print...\tCtrl+P', self.OnPrint, None, 'print')
        self.AddMenuItem('Page Setup...', self.OnPageSetup, None, 'psetup')
        self.AddMenuItem('Print Preview', self.OnPrintPreview, None, 'ppreview')
        self.AddSeparator()
        self.AddMenuItem('Restore All Defaults', self.OnResetDefaults, None, 'restore')
        self.AddSeparator()
        self.AddMenuItem('Exit\tCtrl+Q', self.OnQuit, None, 'exit')
        self.EndMenu()
        
        self.AddMenu('&Edit')
        self.AddMenuItem('&Undo', self.OnUndo, self.OnUpdateUndo, 'undo')
        self.AddMenuItem('&Redo', self.OnRedo, self.OnUpdateRedo, 'redo')
        self.AddSeparator()
        self.AddMenuItem('Cut', self.OnCut, self.OnUpdateCut, 'cut')
        self.AddMenuItem('Copy', self.OnCopy, self.OnUpdateCopy, 'copy')
        self.AddMenuItem('Paste', self.OnPaste, self.OnUpdatePaste, 'paste')
        self.AddMenuItem('Delete', self.OnDelete, self.OnUpdateDelete, 'delete')
        self.AddSeparator()
        self.AddMenuItem('Select Mode', self.OnSelectMode, None, 'select')
        self.EndMenu()

        self.AddMenu('View')
        self.AddMenuItem('Previous view', self.OnMagPrevious, None, 'magprev')
        self.AddSeparator()
        self.AddMenuItem('Zoom window', self.OnMag, None, 'mag')
        self.AddMenuItem('Fit view to extents', self.OnMagExtents, None, 'magextents')
        self.AddMenuItem('Fit view to extents, but no rotation', self.OnMagNoRot, None, 'magnorot')
        self.AddMenuItem('View XY Front', self.OnMagXY, None, 'magxy')
        self.AddMenuItem('View XY Back', self.OnMagXYM, None, 'magxym')
        self.AddMenuItem('View XZ Top', self.OnMagXZ, None, 'magxz')
        self.AddMenuItem('View XZ Bottom', self.OnMagXZM, None, 'magxzm')
        self.AddMenuItem('View YZ Right', self.OnMagYZ, None, 'magyz')
        self.AddMenuItem('View YZ Left', self.OnMagYZM, None, 'magyzm')
        self.AddMenuItem('View XZY Isometric', self.OnMagXYZ, None, 'magxyz')
        self.AddSeparator()
        self.AddMenuItem('View rotate', self.OnViewRotate, None, 'viewrot')
        self.AddMenuItem('View zoom', self.OnViewZoom, None, 'zoom')
        self.AddMenuItem('View pan', self.OnViewPan, None, 'pan')
        self.AddMenuItem('Full screen', self.OnFullScreen, None, 'fullscreen')
        self.AddSeparator()
        self.AddMenuItem('Redraw', self.OnRedraw, None, 'redraw')
        self.EndMenu()

        self.AddMenu('&Geometry')
        self.AddMenuItem('Draw a sketch', self.OnLines, None, 'lines')
        self.AddMenuItem('Draw circles through 3 points', self.OnCircles3p, None, 'circ3p')
        self.AddMenuItem('Draw circles, centre and point', self.OnCircles2p, None, 'circ2p')
        self.AddMenuItem('Draw circles, centre and radius', self.OnCircles1p, None, 'circpr')
        self.AddMenuItem('DrawEllipses', self.OnEllipse, None, 'circles')
        self.AddMenuItem('Draw Infinite Lines', self.OnILine, None, 'iline')
        self.AddMenuItem('Draw Points', self.OnPoints, None, 'point')
        self.AddMenuItem('Gear', self.OnGear, None, 'gear')
        self.AddSeparator()
        self.AddMenuItem('Add Text', self.OnText, None, 'text')
        self.AddMenuItem('Add Dimension', self.OnDimensioning, None, 'dimension')
        self.EndMenu()
        
        if wx.GetApp().IsSolidApp():
            wx.GetApp().AddSolidMenu(self)

        self.AddMenu('Set &Origin')
        self.AddMenuItem('Pick 3 points', self.OnCoordinateSystem, None, 'coords3')
        self.AddMenuItem('Pick 1 point', self.OnNewOrigin, None, 'coordsys')
        self.EndMenu()

        self.AddMenu('&Transform')
        self.AddMenuItem('Move Translate', self.OnMoveTranslate, None, 'movet')
        self.AddMenuItem('Copy Translate', self.OnCopyTranslate, None, 'copyt')
        self.AddSeparator()
        self.AddMenuItem('Move Rotate', self.OnMoveRotate, None, 'mover')
        self.AddMenuItem('Copy Rotate', self.OnCopyRotate, None, 'copyr')
        self.AddSeparator()
        self.AddMenuItem('Mirror', self.OnMirror, None, 'copym')
        self.AddMenuItem('Scale', self.OnMoveScale, None, 'moves')
        self.EndMenu()

        self.window_menu = self.AddMenu('&Window')
        self.AddMenuItem('Objects', self.OnViewObjects, self.OnUpdateViewObjects, check_item = True)
        self.AddMenuItem('Input', self.OnViewInput, self.OnUpdateViewInput, check_item = True)
        self.AddMenuItem('Properties', self.OnViewProperties, self.OnUpdateViewProperties, check_item = True)
        self.EndMenu()
        
        self.AddMenu('Text')
        self.AddMenuItem('Convert Heeks Font To C++', self.ConvertLines, None)        
        self.EndMenu()      

        self.AddExtraMenus()

        self.SetMenuBar(self.menuBar)
        
    def ConvertLines(self, e):
        from HeeksFont import ConvertHeeksFont
        ConvertHeeksFont()
        
    def AddExtraMenus(self):
        pass
    
    def AddExtraWindows(self):
        pass
    
    def CurrentMenu(self):
        if len(self.current_menu_stack) > 0:
            return self.current_menu_stack[-1]
        return None
    
    def SetMenuItemBitmap(self, item, bitmap_name):
        if bitmap_name:
            image = wx.Image(self.BitmapPath(bitmap_name))
            image.Rescale(24, 24)
            item.SetBitmap(wx.Bitmap(image))
            
    def BitmapPath(self, name):
        return self.bitmap_path + '/'+ name + '.png'
    
    def AddMenu(self, title, bitmap_name = None, onUpdate = None, id = wx.ID_ANY):
        menu = wx.Menu()
        current_menu = self.CurrentMenu()
        self.current_menu_stack.append(menu)
        if current_menu:
            item = wx.MenuItem(current_menu, id, title)
            item.SetSubMenu(menu)
            if bitmap_name != None:
                self.SetMenuItemBitmap(item, bitmap_name)
            current_menu.Append(item)
            if onUpdate != None:
                self.Bind(wx.EVT_UPDATE_UI, onUpdate, id = item.GetId())
        else:
            self.menuBar.Append(menu, title)
        return menu
            
    def EndMenu(self):
        self.current_menu_stack.pop()
        
    def AddSeparator(self):
        current_menu = self.CurrentMenu()
        if current_menu:
            current_menu.AppendSeparator()
        
    def AddMenuItem(self, title, onButton, onUpdate = None, bitmap_name = None, check_item = False, menu = None):
        men = menu if menu else self.CurrentMenu()
        item = wx.MenuItem(men, wx.ID_ANY, title, kind = wx.ITEM_CHECK if check_item else wx.ITEM_NORMAL )        
        self.SetMenuItemBitmap(item, bitmap_name)
        self.Bind(wx.EVT_MENU, onButton, men.Append(item))     
        if onUpdate:
            self.Bind(wx.EVT_UPDATE_UI, onUpdate, id = item.GetId())  
    
    def OnNew(self, e):
        res = self.CheckForModifiedDoc()
        if res != wx.CANCEL:
            cad.Reset()
            wx.GetApp().OnNewOrOpen(False)
            cad.ClearHistory()
            cad.SetLikeNewFile()
            wx.GetApp().filepath = None
            self.SetFrameTitle()
            
    def SetFrameTitle(self):
        s = wx.GetApp().GetAppName() + ' - '
        #s = self.GetTitle() + ' - '
        if wx.GetApp().filepath:
            s += wx.GetApp().filepath
        else:
            s += 'Untitled'
        self.SetTitle(s)
        
    def SaveProject(self, force_dialog = False):
        if self.GetProjectFileName().IsOk():
            self.SaveFile()
        
    def CheckForModifiedDoc(self, force_dialog = False):
        # returns wxCANCEL if not OK to continue opening file
        if cad.IsModified():
            str = 'Save changes to file ' + (wx.GetApp().filepath if wx.GetApp().filepath else 'Untitled')
            res = wx.MessageBox(str, wx.MessageBoxCaptionStr, wx.CANCEL | wx.YES_NO | wx.CENTER | wx.ICON_WARNING)
            if res == wx.CANCEL or res == wx.NO: return res
            if res == wx.YES:
                return self.OnSave(None)

        return wx.OK
    
    def DoFileOpenViewMag(self):
        self.graphics_canvas.viewport.OnMagExtents(True, 25)
    
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
                wx.GetApp().OnNewOrOpen(True)
                cad.ClearHistory()
                cad.SetLikeNewFile()
                wx.GetApp().filepath = filepath
                self.SetFrameTitle()
                wx.GetApp().InsertRecentFileItem(filepath)
                wx.GetApp().WriteRecentFiles()
                return True
            else:
                wx.MessageBox('Invalid file type chosen expecting .heeks')
        return True

    def OnOpen(self, e):
        dialog = wx.FileDialog(self, 'Open File', wx.GetApp().GetDefaultDir(), '', HEEKS_WILDCARD_STRING)
        dialog.CenterOnParent()
        
        if dialog.ShowModal() == wx.ID_OK:
            self.OnOpenFilepath(dialog.GetPath())
                    
    def OnOpenRecent(self, e):
        file_path = wx.GetApp().recent_files[e.GetId() - self.ID_RECENT_FIRST]
        self.OnOpenFilepath(file_path)
        
    def OnUpdateOpenRecent(self, e):
        size = self.recent_files_menu.GetMenuItemCount()
        menu_items = []
        for i in range(0, size):
            menu_items.append(self.recent_files_menu.FindItemByPosition(i))
        for menu_item in menu_items:
            self.recent_files_menu.Delete(menu_item)
            
        recent_id = self.ID_RECENT_FIRST
        for filepath in wx.GetApp().recent_files:
            self.recent_files_menu.Append(recent_id, filepath)
            recent_id += 1
            
    def OnSaveFilepath(self, filepath):
        if cad.SaveFile(filepath):
            wx.GetApp().filepath = filepath        
            cad.SetLikeNewFile()
            self.SetFrameTitle()
            wx.GetApp().InsertRecentFileItem(filepath)
            wx.GetApp().WriteRecentFiles()
            return wx.ID_OK
        return wx.ID_CANCEL
            
    def OnSave(self, e):
        if wx.GetApp().filepath:
            return self.OnSaveFilepath(wx.GetApp().filepath)

        dialog = wx.FileDialog(self, 'Save File', wx.GetApp().GetDefaultDir(), '', HEEKS_WILDCARD_STRING, wx.FD_SAVE | wx.FD_OVERWRITE_PROMPT)
        dialog.SetFilterIndex(1)
        dialog.CenterOnParent()
        if dialog.ShowModal() == wx.ID_CANCEL:
            return wx.ID_CANCEL
        return self.OnSaveFilepath(dialog.GetPath())
            
    def OnUpdateSave(self, e):
        e.Enable(cad.IsModified())            
            
    def OnSaveAs(self, e):
        if wx.GetApp().filepath:
            default_directory = ''
            default_filepath = wx.GetApp().filepath
        else:
            default_directory = wx.GetApp().GetDefaultDir()
            default_filepath = ''            
        
        dialog = wx.FileDialog(self, 'Save File', default_directory, default_filepath, HEEKS_WILDCARD_STRING, wx.FD_SAVE | wx.FD_OVERWRITE_PROMPT)
        dialog.SetFilterIndex(0)
        dialog.CenterOnParent()
        if dialog.ShowModal() != wx.ID_CANCEL:
            self.OnSaveFilepath(dialog.GetPath())
            
    def GetImportWildcardString(self):
        imageExtStr = '' # to do, get image extensions from wx
        imageExtStr2 = '' # to do, get image extensions from wx
        registeredExtensions = '' # to do, this will be extensions added by child apps
        return 'Known Files' + ' |*.heeks;*.HEEKS;*.igs;*.IGS;*.iges;*.IGES;*.stp;*.STP;*.step;*.STEP;*.dxf;*.DXF;*.stl' + imageExtStr + registeredExtensions + '|Heeks files (*.heeks)|*.heeks;*.HEEKS|STL files (*.stl)|*.stl;*.STL|Scalar Vector Graphics files (*.svg)|*.svg;*.SVG|DXF files (*.dxf)|*.dxf;*.DXF|RS274X/Gerber files (*.gbr,*.rs274x)|*.gbr;*.GBR;*.rs274x;*.RS274X;*.pho;*.PHO|Picture files (' + imageExtStr2 + ')|' + imageExtStr

    def GetExportWildcardString(self):
        return 'Known Files |*.stl;*.dxf;*.cpp;*.py;*.obj|STL files (*.stl)|*.stl|DXF files (*.dxf)|*.dxf|CPP files (*.cpp)|*.cpp|OpenCAMLib python files (*.py)|*.py|Wavefront .obj files (*.obj)|*.obj'
                                         
    def OnImport(self, e):
        config = HeeksConfig()
        default_directory = config.Read('ImportDirectory', wx.GetApp().GetDefaultDir())
        dialog = wx.FileDialog(self, 'Import File', default_directory, '', self.GetImportWildcardString())
        dialog.CenterOnParent()
        
        if dialog.ShowModal() == wx.ID_OK:
            filepath = dialog.GetPath()
            if cad.Import(filepath):
                self.DoFileOpenViewMag()
                if wx.GetApp().filepath == None:
                    dot = filepath.rfind('.')
                    if dot != -1:
                        wx.GetApp().filepath = filepath[:dot+1] + '.heeks'
                self.SetFrameTitle()
                config.Write('ImportDirectory', dialog.GetDirectory())
                cad.Repaint()
            
    def GetPathSuffix(self, path):
        dot = path.rfind('.')
        if dot == -1:
            return ''
        return path[dot+1:].lower()
        
    def OnExport(self, e):
        config = HeeksConfig()
        default_directory = config.Read('ExportDirectory', wx.GetApp().GetDefaultDir())
        print('self.GetExportWildcardString() = ' + str(self.GetExportWildcardString()))
        dialog = wx.FileDialog(self, 'Export File', default_directory, '', self.GetExportWildcardString(), wx.FD_SAVE | wx.FD_OVERWRITE_PROMPT)
        dialog.CenterOnParent()
        
        if dialog.ShowModal() == wx.ID_OK:
            path = dialog.GetPath()
            suffix = self.GetPathSuffix(path)
            print('suffix = ' + suffix)
            if suffix == 'svg':
                import Svg
                Svg.Export(path)
            else:
                cad.SaveFile(path)
            config.Write('ExportDirectory', dialog.GetDirectory())
                
    def OnPrint(self, e):
        printDialogData = wx.PrintDialogData(wx.GetApp().printData)
        printer = wx.Printer(printDialogData)
        self.printout = Printout()
        if printer.Print(self, self.printout, True):
            theApp.printData = printer.GetPrintDialogData().GetPrintData()
        else:
            if wx.Printer.GetLastError() == wx.PRINTER_ERROR:
                wx.MessageBox('There was a problem printing.\nPerhaps your current printer is not set correctly?', 'Printing', wx.OK)
            else:
                wx.MessageBox('You canceled printing', 'Printing', wx.OK)
                
        self.printout = None
            
    def OnPageSetup(self, e):
        pageSetupData = wx.GetApp().printData
        
        pageSetupDialog = wx.PageSetupDialog(self, wx.GetApp().pageSetupData)
        pageSetupDialog.ShowModal()
        
        wx.GetApp().printData = pageSetupDialog.GetPageSetupDialogData().GetPrintData()
        wx.GetApp().pageSetupData = pageSetupDialog.GetPageSetupDialogData()
            
    def OnPrintPreview(self, e):
        printDialogData = wx.PrintDialogData(wx.GetApp().printData)
        preview = wx.PrintPreview(Printout(), Printout(), printDialogData)
        if not preview.IsOk():
            preview = None
            wx.MessageBox('There was a problem previewing.\nPerhaps your current printer is not set correctly?', 'Previewing', wx.OK)

        frame = wx.PreviewFrame(preview, self, 'Print Preview', wx.Point(100,100), wx.Size(600, 650))
        frame.Centre(wx.BOTH)
        frame.Initialize()
        frame.Show()
    
            
    def OnResetDefaults(self, e):
        wx.GetApp().RestoreDefaults()
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
        cad.Import(temp_file)
        
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
        cad.SetInputMode(wx.GetApp().select_mode)
        
    def OnMagPrevious(self, e):
        self.graphics_canvas.viewport.RestorePreviousViewPoint()
        self.graphics_canvas.Refresh()
        
    def OnMag(self, e):
        cad.SetInputMode(cad.GetMagnification())
        
    def OnMagExtents(self, e):
        self.graphics_canvas.viewport.OnMagExtents(True, 6)
        self.graphics_canvas.Refresh()
        
    def OnMagNoRot(self, e):
        self.graphics_canvas.viewport.OnMagExtents(False, 6)
        self.graphics_canvas.Refresh()
        
    def OnMagAxes(self, unitY, unitZ):
        self.graphics_canvas.viewport.ClearViewpoints()
        self.graphics_canvas.viewport.m_view_point.SetView(unitY, unitZ, 6)
        self.graphics_canvas.viewport.StoreViewPoint()
        self.Refresh()
        
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
            self.SetMenuBar(None)
            self.windows_visible = {}
            for w in wx.GetApp().hideable_windows:
                self.windows_visible[w] = self.aui_manager.GetPane(w).IsShown() and w.IsShown()
                self.aui_manager.GetPane(w).Show(False)
        else:
            self.SetMenuBar(self.menuBar)
            for w in wx.GetApp().hideable_windows:
                if w in self.windows_visible:
                    visible = self.windows_visible[w]
                    self.aui_manager.GetPane(w).Show(visible)
            
        res = super().ShowFullScreen(show, style)
        self.aui_manager.Update()
        return res
        
    def OnFullScreen(self, e):
        self.ShowFullScreen(True)
        
    def OnRedraw(self, e):
        cad.GetApp().KillGLLists()
        self.graphics_canvas.Refresh()
        
    def OnLines(self, e):
        cad.SetLineArcDrawing()
        
    def OnCircles3p(self, e):
        cad.SetCircles3pDrawing()
        
    def OnCircles2p(self, e):
        cad.SetCircles2pDrawing()
        
    def OnCircles1p(self, e):
        cad.SetCircle1pDrawing()
        
    def OnEllipse(self, e):
        cad.SetEllipseDrawing()
        
    def OnILine(self, e):
        cad.SetILineDrawing()
        
    def OnPoints(self, e):
        cad.SetInputMode(point_drawing)
        
    def OnGear(self, e):
        gear = Gear.Gear(1.0, 12)
        self.gears.append(gear)
        cad.AddUndoably(gear, None, None)
        
    def OnText(self, e):
        cad.AddUndoably(cad.NewText("text"))
        
    def OnDimensioning(self, e):
        pass
        
    def OnCoordinateSystem(self, e):
        pass
        
    def OnNewOrigin(self, e):
        pass
        
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
    
                
