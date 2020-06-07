from App import App
from HeeksConfig import HeeksConfig
import step
import cad
import wx
from HDialog import HDialog
from HDialog import control_border
from NiceTextCtrl import LengthCtrl
from NiceTextCtrl import DoubleCtrl
    
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

    def OnRuledSurface(self, event):
        if not self.CheckForNumberOrMore(2, [cad.OBJECT_TYPE_SKETCH], 'Pick two or more sketches, to create a lofted solid between\n( hold down Ctrl key to select more than one solid )', 'Lofted Body'):
            return
        
        step.CreateRuledSurface(True)
        
    def InputExtrusionHeight(self, value, extrude_makes_a_solid, taper_angle):
        dlg = HDialog('Make Extrusion')
        sizerMain = wx.BoxSizer(wx.VERTICAL)
        
        value_control = LengthCtrl(dlg)
        value_control.SetValue(value)
        dlg.AddLabelAndControl(sizerMain, 'Height', value_control)
        
        solid_check_box = None
        if extrude_makes_a_solid != None:
            solid_check_box = wx.CheckBox(dlg, label = 'Extrude makes a solid')
            solid_check_box.SetValue(extrude_makes_a_solid)
            sizerMain.Add( solid_check_box, 0, wx.ALL | wx.ALIGN_LEFT, control_border )
            
        taper_angle_control = None
        if taper_angle != None:
            taper_angle_control = DoubleCtrl(dlg)
            taper_angle_control.SetValue(taper_angle)
            dlg.AddLabelAndControl( sizerMain, 'Taper Outward Angle', taper_angle_control )
            
        dlg.MakeOkAndCancel( wx.HORIZONTAL ).AddToSizer( sizerMain )
        dlg.SetSizer( sizerMain )
        sizerMain.Fit( dlg )
        value_control.SetFocus()
        if dlg.ShowModal() == wx.ID_OK:
            value = value_control.GetValue()
            if extrude_makes_a_solid != None: extrude_makes_a_solid = solid_check_box.GetValue()
            if taper_angle != None: taper_angle = taper_angle_control.GetValue()
            return True, value, extrude_makes_a_solid, taper_angle
        return False, value, extrude_makes_a_solid, taper_angle

    def OnExtrude(self, event):
        if cad.GetNumSelected() == 0:
            self.PickObjects('Pick Some Sketches, Faces or Circles')
        
        if not self.CheckForNumberOrMore(1, [cad.OBJECT_TYPE_SKETCH, step.GetFaceType(), cad.OBJECT_TYPE_CIRCLE], 'Pick one or more sketches, faces or circles, to create an extruded body from\n( hold down Ctrl key to select more than one solid )', 'Extude'):
            return
        
        config = HeeksConfig();
        height = config.ReadFloat('ExtrusionHeight', 10.0)
        taper_angle = config.ReadFloat('ExtrusionTaperAngle', 0.0)
        extrude_makes_a_solid = config.ReadBool('ExtrudeToSolid')
        
        success, height, extrude_makes_a_solid, taper_angle = self.InputExtrusionHeight(height, extrude_makes_a_solid, taper_angle)
        
        if success:
            config.WriteFloat('ExtrusionHeight', height)
            config.WriteFloat('ExtrusionTaperAngle', taper_angle)
            config.WriteBool('ExtrudeToSolid', extrude_makes_a_solid)
            step.CreateExtrusion(height, extrude_makes_a_solid, False, taper_angle, cad.Color(128, 128, 128))
        
    def InputRevolutionAngle(self, angle, extrude_makes_a_solid):
        dlg = HDialog('Input Revolution Angle')
        sizerMain = wx.BoxSizer(wx.VERTICAL)
        
        angle_control = DoubleCtrl(dlg)
        angle_control.SetValue(angle)
        dlg.AddLabelAndControl(sizerMain, 'Angle', angle_control)
        
        solid_check_box = None
        if extrude_makes_a_solid != None:
            solid_check_box = wx.CheckBox(dlg, label = 'Extrude makes a solid')
            solid_check_box.SetValue(extrude_makes_a_solid)
            sizerMain.Add( solid_check_box, 0, wx.ALL | wx.ALIGN_LEFT, control_border )
            
        dlg.MakeOkAndCancel( wx.HORIZONTAL ).AddToSizer( sizerMain )
        dlg.SetSizer( sizerMain )
        sizerMain.Fit( dlg )
        angle_control.SetFocus()
        if dlg.ShowModal() == wx.ID_OK:
            angle = angle_control.GetValue()
            if extrude_makes_a_solid != None: extrude_makes_a_solid = solid_check_box.GetValue()
            return True, angle, extrude_makes_a_solid
        return False, angle, extrude_makes_a_solid

    def OnRevolve(self, event):
        if cad.GetNumSelected() == 0:
            self.PickObjects('Pick Some Sketches, Faces or Circles')
        
        if not self.CheckForNumberOrMore(1, [cad.OBJECT_TYPE_SKETCH, step.GetFaceType(), cad.OBJECT_TYPE_CIRCLE], 'Pick one or more sketches, faces or circles, to create an extruded body from\n( hold down Ctrl key to select more than one solid )', 'Revolve'):
            return
        
        config = HeeksConfig();
        angle = config.ReadFloat('RevolutionAngle', 360.0)
        extrude_makes_a_solid = config.ReadBool('ExtrudeToSolid')
        
        success, angle, extrude_makes_a_solid = self.InputRevolutionAngle(angle, extrude_makes_a_solid)
        
        if success:
            config.WriteFloat('RevolutionAngle', angle)
            config.WriteBool('ExtrudeToSolid', extrude_makes_a_solid)
            step.CreateExtrusion(angle, extrude_makes_a_solid, True, 0.0, cad.Color(128, 128, 150))

    def OnSweep(self, event):
        if cad.GetNumSelected() == 0:
            self.PickObjects('Pick Some Sketches, Faces or Circles')
        
        if not self.CheckForNumberOrMore(1, [cad.OBJECT_TYPE_SKETCH, step.GetFaceType(), cad.OBJECT_TYPE_CIRCLE], 'Pick one or more sketches, faces or circles, to sweep\n( hold down Ctrl key to select more than one solid )', 'Sweep'):
            return
        
        sweep_objects = cad.GetSelectedObjects()
        cad.ClearSelection(True)
        self.PickObjects('Pick a Sketch to Sweep Along')

        if not self.CheckForNumberOrMore(1, [cad.OBJECT_TYPE_SKETCH], 'Pick one sketch to sweep along', 'Sweep'):
            return
        
        sweep_profile = cad.GetSelectedObjects()[0]
        
        cad.StartHistory()
        step.CreateSweep(sweep_objects, sweep_profile, cad.Color(128, 150, 128))
        cad.EndHistory()

    def OnSubtract(self, event):
        if not self.CheckForNumberOrMore(2, [step.GetFaceType(), step.GetSolidType()], 'Pick two or more faces or solids, the first one will be cut by the others', 'Subtract Solids'):
            return
        cad.StartHistory()
        step.CutShapes()
        cad.EndHistory()

    def OnFuse(self, event):
        if not self.CheckForNumberOrMore(2, [step.GetSolidType()], 'Pick two or more solids to be fused together', 'Fuse Solids'):
            return
        cad.StartHistory()
        step.FuseShapes()
        cad.EndHistory()

    def OnCommon(self, event):
        if not self.CheckForNumberOrMore(2, [step.GetSolidType()], 'Pick two or more solids, only the shape that is contained by all of them will remain', 'Intersection of Solids'):
            return
        cad.StartHistory()
        step.CommonShapes()
        cad.EndHistory()

    def OnFillet(self, event):
        if not self.CheckForNumberOrMore(1, [step.GetEdgeType()], 'Pick one or more edges to add a fillet to', 'Edge Fillet'):
            return
        config = HeeksConfig()
        rad = config.ReadFloat('EdgeBlendRadius', 2.0)
        if self.InputLength('Enter Blend Radius', 'Radius', rad):
            cad.StartHistory()
            step.FilletOrChamferEdges(rad, False)
            config.WriteFloat('EdgeBlendRadius', rad)
            cad.ClearSelection(True)
            cad.EndHistory()

    def OnChamfer(self, event):
        if not self.CheckForNumberOrMore(1, [step.GetEdgeType()], 'Pick one or more edges to add a chamfer to', 'Edge Chamfer'):
            return
        config = HeeksConfig()
        rad = config.ReadFloat('EdgeChamferDist', 2.0)
        if self.InputLength('Enter Chamfer Distance', 'Distance', rad):
            cad.StartHistory()
            step.FilletOrChamferEdges(rad, True)
            config.WriteFloat('EdgeChamferDist', rad)
            cad.ClearSelection(True)
            cad.EndHistory()
    
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
        frame.EndMenu()
