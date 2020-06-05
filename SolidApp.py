from App import App
from HeeksConfig import HeeksConfig
import step
import cad
import wx
from HDialog import HDialog
    
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

    def OnRuledSurface(self, event):
        if not self.CheckForNumberOrMore(2, [cad.OBJECT_TYPE_SKETCH], 'Pick two or more sketches, to create a lofted solid between\n( hold down Ctrl key to select more than one solid )', 'Lofted Body'):
            return
        
        step.CreateRuledSurface(True)
        
    def InputExtrusionHeight(self, value, extrude_makes_a_solid, taper_angle):
        pass
#        dlg = HDialog(self.frame)
#        sizerMain = wx.BoxSizer(wx.VERTICAL)
#        static_label = wx.StaticText(dlg, label = 'Make Extrusion')

    def OnExtrude(self, event):
        if cad.GetNumSelected() == 0:
            self.PickObjects('Pick Some Sketches, Faces or Circles')
        
        if not self.CheckForNumberOrMore(2, [cad.OBJECT_TYPE_SKETCH, step.GetFaceType(), cad.OBJECT_TYPE_CIRCLE], 'Pick one or more sketches, faces or circles, to create an extruded body from\n( hold down Ctrl key to select more than one solid )', 'Extude'):
            return
        
        step.CreateExtrusion()

    def OnRevolve(self, event):
        pass

    def OnSweep(self, event):
        pass

    def OnSubtract(self, event):
        pass

    def OnFuse(self, event):
        pass

    def OnCommon(self, event):
        pass

    def OnFillet(self, event):
        pass

    def OnChamfer(self, event):
        pass

    def OnSectioning(self, event):
        pass
    
    def AddSolidMenu(self, frame):
        frame.AddMenu('&Solid')
        frame.AddMenuItem('Add a sphere', self.OnSphere, None, 'sphere')
        frame.AddMenuItem('Add a cube', self.OnCube, None, 'cube')
        frame.AddMenuItem('Add a cylinder', self.OnCyl, None, 'cyl')
        frame.AddMenuItem('Add a cone', self.OnCone, None, 'cone')
        frame.AddSeparator()
        frame.AddMenuItem('Loft two sketches', self.OnRuledSurface, None, 'ruled')
        frame.AddMenuItem('Extrude a sketch', self.OnExtrude, None, 'extrude')
        frame.AddMenuItem('Revolve a sketch', self.OnRevolve, None, 'revolve')
        frame.AddMenuItem('Sweep objects along a sketch', self.OnSweep, None, 'sweep')
        frame.AddSeparator()
        frame.AddMenuItem('Cut', self.OnSubtract, None, 'subtract')
        frame.AddMenuItem('Fuse', self.OnFuse, None, 'fuse')
        frame.AddMenuItem('Common', self.OnCommon, None, 'common')
        frame.AddSeparator()
        frame.AddMenuItem('Fillet', self.OnFillet, None, 'fillet')
        frame.AddMenuItem('Chamfer', self.OnChamfer, None, 'chamfer')
        frame.AddMenuItem('Sectioning', self.OnSectioning, None, 'section')
        frame.EndMenu()
