from App import App
from HeeksConfig import HeeksConfig
import step
import cad
import wx
    
def ImportSTEPFile():
    step.ImportSTEPFile(cad.GetFilePathForImportExport())

class SolidApp(App):
    def __init__(self):
        App.__init__(self)

    def GetAppName(self):
        return 'Solid CAD ( Computer Aided Design )'

    def RegisterObjectTypes(self):
        step.SetResPath(self.cad_dir)
        step.SetApp(cad.GetApp())
        App.RegisterObjectTypes(self)
        cad.RegisterOnEndXmlWrite(step.WriteSolids)
        step.SetStepFileObjectType(cad.RegisterObjectType("STEP_file", step.CreateStepFileObject))
        cad.RegisterImportFileType("step", ImportSTEPFile)
        cad.RegisterImportFileType("stp", ImportSTEPFile)
        
    def GetObjectTools(self, object, control_pressed, from_tree_canvas = False):
        tools = App.GetObjectTools(self, object, control_pressed, from_tree_canvas)
#        if object.GetType() == Profile.type:
#            tools.append(CamContextTool.CamObjectContextTool(object, "Add Tags", "addtag", self.AddTags))
        return tools
    
    def IsSolidApp(self):
        return True
    
    def AddObjectFromButton(self, new_object):
        cad.StartHistory()
        cad.AddUndoably(new_object,None, None)
        cad.ClearSelection(True)
        cad.Select(new_object)
        cad.SetInputMode(self.select_mode)
        cad.EndHistory()
        cad.Repaint()
    
    def OnSphere(self, event):
        self.AddObjectFromButton(step.NewSphere())

    def OnCube(self, event):
        self.AddObjectFromButton(step.NewCuboid())

    def OnCyl(self, event):
        self.AddObjectFromButton(step.NewCyl())

    def OnCone(self, event):
        self.AddObjectFromButton(step.NewCone())
    
    def AddSolidMenu(self, frame):
        frame.AddMenu('&Solid')
        frame.AddMenuItem('Add a sphere', self.OnSphere, None, 'sphere')
        frame.AddMenuItem('Add a cube', self.OnCube, None, 'cube')
        frame.AddMenuItem('Add a cylinder', self.OnCyl, None, 'cyl')
        frame.AddMenuItem('Add a cone', self.OnCone, None, 'cone')
#        self.AddSeparator()
#        self.AddMenuItem('Add a sphere', self.OnSphere, None, 'sphere')
#        self.AddMenuItem('Add a sphere', self.OnSphere, None, 'sphere')
#        self.AddMenuItem('Add a sphere', self.OnSphere, None, 'sphere')
#        self.AddMenuItem('Add a sphere', self.OnSphere, None, 'sphere')
#        self.AddSeparator()
#        self.AddMenuItem('Add a sphere', self.OnSphere, None, 'sphere')
#        self.AddMenuItem('Add a sphere', self.OnSphere, None, 'sphere')
#        self.AddMenuItem('Add a sphere', self.OnSphere, None, 'sphere')
#        self.AddSeparator()
#        self.AddMenuItem('Add a sphere', self.OnSphere, None, 'sphere')
#        self.AddMenuItem('Add a sphere', self.OnSphere, None, 'sphere')
#        self.AddMenuItem('Add a sphere', self.OnSphere, None, 'sphere')
        frame.EndMenu()
