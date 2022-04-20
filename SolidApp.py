import os
import sys

this_dir = os.path.dirname(os.path.realpath(__file__))
sys.path.append(this_dir)

from App import App
from HeeksConfig import HeeksConfig
import step
import geom
import cad
import wx
from HDialog import HDialog
from HDialog import control_border
from NiceTextCtrl import LengthCtrl
from NiceTextCtrl import DoubleCtrl
from Ribbon import RB
from Ribbon import Ribbon
import ContextTool
import EllipseDrawing
    
def ImportSolidsFile():
    step.ImportSolidsFile(cad.GetFilePathForImportExport())
    
def ExportSolidsFile():
    step.ExportSolidsFile(cad.GetFilePathForImportExport())

class SolidApp(App):
    def __init__(self):
        App.__init__(self)

    def GetAppTitle(self):
        return 'Solid CAD ( Computer Aided Design )'
       
    def GetAppConfigName(self):
        return 'HeeksCADSolid'
    
    def LoadConfig(self):
        App.LoadConfig(self)
        config = HeeksConfig()
        step.SetShowFaceNormals(config.ReadBool('ShowFaceNormals', False))

    def RegisterObjectTypes(self):
        step.SetResPath(self.cad_dir)
        step.SetApp(cad.GetApp())
        App.RegisterObjectTypes(self)
        cad.RegisterOnEndXmlWrite(step.WriteSolids)
        step.SetStepFileObjectType(cad.RegisterObjectType("STEP_file", step.CreateStepFileObject, add_to_filter = False))
        step.SetSolidType(cad.RegisterObjectType("Solid", None))
        step.SetFaceType(cad.RegisterObjectType("Face", None))
        step.SetEdgeType(cad.RegisterObjectType("Edge", None))
        step.SetEllipseType(cad.RegisterObjectType("Ellipse", step.CreateEllipse))
        step.SetSplineType(cad.RegisterObjectType("Spline", step.CreateSpline))
        self.RegisterImportFileTypes(['step', 'stp'], 'STEP Files', ImportSolidsFile)
        self.RegisterImportFileTypes(['iges', 'igs'], 'IGES Files', ImportSolidsFile)
        self.RegisterExportFileTypes(['step', 'stp'], 'STEP Files', ExportSolidsFile)
        self.RegisterExportFileTypes(['iges', 'igs'], 'IGES Files', ExportSolidsFile)
        
    def GetObjectTools(self, object, control_pressed, from_tree_canvas = False):
        tools = App.GetObjectTools(self, object, control_pressed, from_tree_canvas)
        t = object.GetType()
        if t == step.GetEdgeType():
            tools.append(ContextTool.CADObjectContextTool(object, "Blend", "fillet", self.BlendEdge))
            tools.append(ContextTool.CADObjectContextTool(object, "Chamfer", "chamfer", self.ChamferEdge))
        elif t == step.GetFaceType():
            tools.append(ContextTool.CADObjectContextTool(object, "Make Sketch From Face", "face2sketch", self.FaceToSketch))
            self.context_face_plane = object.GetPlane()
            if self.context_face_plane != None:
                tools.append(ContextTool.CADObjectContextTool(object, "Rotate To Face", "rotface", self.RotateToFace))
            tools.append(ContextTool.CADObjectContextTool(object, "(to do ) Make Face Radius 1mm smaller", "facerch", self.FaceRadiusChange))
        return tools
    
    def FaceToSketch(self, object):
        sketch = step.NewSketchFromFace(object)
        cad.AddUndoably(sketch)
        cad.Select(sketch)
    
    def RotateToFace(self, object):
        n = self.context_face_plane.normal
        p = n * (-self.context_face_plane.d)
        x,y = n.ArbitraryAxes()
        face_matrix = geom.Matrix(p, x, y)
        inv_matrix = face_matrix.Inverse()
        cad.StartHistory('Rotate To Face')
        selected = cad.GetSelectedObjects()
        # if any objects are selected, move them
        if len(selected)>0:
            for object in selected:
                cad.TransformUndoably(object, inv_matrix)
        else:
            # move the solid
            parent_body = object.GetParentBody()
            cad.TransformUndoably(parent_body, inv_matrix)
        self.EndHistory()
    
    def FaceRadiusChange(self, object):
        sketch = step.NewSketchFromFace(object)
        cad.AddUndoably(sketch)
        cad.Select(sketch)
    
    def BlendEdge(self, object):
        cad.ClearSelection()
        cad.Select(object)
        self.OnFillet(None)

    def ChamferEdge(self, object):
        cad.ClearSelection()
        cad.Select(object)
        self.OnChamfer(None)

    def IsSolidApp(self):
        return True
    
    def AddObjectFromButton(self, new_object):
        cad.StartHistory('Add Object From Button')
        cad.AddUndoably(new_object,None, None)
        cad.ClearSelection(True)
        cad.Select(new_object)
        self.SetInputMode(self.select_mode)
        self.EndHistory()
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
        
        cad.StartHistory('Sweep')
        step.CreateSweep(sweep_objects, sweep_profile, cad.Color(128, 150, 128))
        self.EndHistory()

    def OnSubtract(self, event):
        if not self.CheckForNumberOrMore(2, [step.GetFaceType(), step.GetSolidType()], 'Pick two or more faces or solids, the first one will be cut by the others', 'Subtract Solids'):
            return
        cad.StartHistory('Subtract')
        step.CutShapes()
        self.EndHistory()

    def OnFuse(self, event):
        if not self.CheckForNumberOrMore(2, [step.GetSolidType()], 'Pick two or more solids to be fused together', 'Fuse Solids'):
            return
        cad.StartHistory('Fuse')
        step.FuseShapes()
        self.EndHistory()

    def OnCommon(self, event):
        if not self.CheckForNumberOrMore(2, [step.GetSolidType()], 'Pick two or more solids, only the shape that is contained by all of them will remain', 'Intersection of Solids'):
            return
        cad.StartHistory('Common')
        step.CommonShapes()
        self.EndHistory()
        
    def OnShadow(self, event):
        if not self.CheckForNumberOrMore(1, [step.GetSolidType(), cad.OBJECT_TYPE_STL_SOLID], 'Pick one or more solids to make a shadow sketch from', 'Shadow Sketch of Solids'):
            return
        accuracy = self.InputLength('Enter Shadow Accuracy', 'Accuracy', geom.get_accuracy())
        if accuracy:
            cad.StartHistory('Shadow')
            geom.set_accuracy(accuracy)
            for object in cad.GetSelectedObjects():
                stl = object.GetTris(accuracy)
                mat = geom.Matrix()
                shadow = stl.Shadow(mat, False)
                #shadow.Reorder()
                sketch = cad.NewSketchFromArea(shadow)
                cad.AddUndoably(sketch)
            self.EndHistory()

    def OnFillet(self, event):
        if not self.CheckForNumberOrMore(1, [step.GetEdgeType()], 'Pick one or more edges to add a fillet to', 'Edge Fillet'):
            return
        config = HeeksConfig()
        rad = config.ReadFloat('EdgeBlendRadius', 2.0)
        rad = self.InputLength('Enter Blend Radius', 'Radius', rad)
        if rad:
            cad.StartHistory('Fillet')
            step.FilletOrChamferEdges(rad, False)
            config.WriteFloat('EdgeBlendRadius', rad)
            cad.ClearSelection(True)
            self.EndHistory()

    def OnChamfer(self, event):
        if not self.CheckForNumberOrMore(1, [step.GetEdgeType()], 'Pick one or more edges to add a chamfer to', 'Edge Chamfer'):
            return
        config = HeeksConfig()
        rad = config.ReadFloat('EdgeChamferDist', 2.0)
        rad = self.InputLength('Enter Chamfer Distance', 'Distance', rad)
        if rad:
            cad.StartHistory('Chamfer')
            step.FilletOrChamferEdges(rad, True)
            config.WriteFloat('EdgeChamferDist', rad)
            cad.ClearSelection(True)
            self.EndHistory()
        
    def AddExtraRibbonPages(self, ribbon):        
        page = RB.RibbonPage(ribbon, wx.ID_ANY, 'Solid', ribbon.Image('solids'))
        page.Bind(wx.EVT_KEY_DOWN, ribbon.OnKeyDown)

        panel = RB.RibbonPanel(page, wx.ID_ANY, 'Primitives', ribbon.Image('solids'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar, 'Sphere', 'sphere', 'Add a sphere', self.OnSphere)
        Ribbon.AddToolBarTool(toolbar, 'Cube', 'cube', 'Add a cube', self.OnCube)
        Ribbon.AddToolBarTool(toolbar, 'Cylinder', 'cyl', 'Add a cylinder', self.OnCyl)
        Ribbon.AddToolBarTool(toolbar, 'Cone', 'cone', 'Add a cone', self.OnCone)

        panel = RB.RibbonPanel(page, wx.ID_ANY, 'Sketch Conversions', ribbon.Image('ruled'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar, 'Ruled', 'ruled', 'Loft multiple sketches', self.OnRuledSurface)
        Ribbon.AddToolBarTool(toolbar, 'Extrude', 'extrude', 'Extrude a sketch', self.OnExtrude)
        Ribbon.AddToolBarTool(toolbar, 'Revolve', 'revolve', 'Revolve a sketch', self.OnRevolve)
        Ribbon.AddToolBarTool(toolbar, 'Sweep', 'sweep', 'Sweep objects along a sketch', self.OnSweep)

        panel = RB.RibbonPanel(page, wx.ID_ANY, 'Operations', ribbon.Image('subtract'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar, 'Cut', 'subtract', 'Cut solid with solids', self.OnSubtract)
        Ribbon.AddToolBarTool(toolbar, 'Fuse', 'fuse', 'Join solids', self.OnFuse)
        Ribbon.AddToolBarTool(toolbar, 'Common', 'common', 'Leave intersection of solids', self.OnCommon)
        Ribbon.AddToolBarTool(toolbar, 'Shadow', 'shadow', 'Make shadow sketch of solid', self.OnShadow)

        panel = RB.RibbonPanel(page, wx.ID_ANY, 'Edge Modifiers', ribbon.Image('subtract'))
        toolbar = RB.RibbonButtonBar(panel)
        Ribbon.AddToolBarTool(toolbar, 'Round', 'fillet', 'Make Edges Rounded', self.OnFillet)
        Ribbon.AddToolBarTool(toolbar, 'Chamfer', 'chamfer', 'Make Edges Chamfered', self.OnChamfer)

        page.Realize()
        
        Ribbon.AddToolBarTool(ribbon.other_drawing_toolbar, 'Ellipses', 'circles', 'Draw Ellipses', self.OnEllipse)
        Ribbon.AddToolBarTool(ribbon.other_drawing_toolbar,'Spline', 'splpts', 'Spline Through Points', self.OnSpline)
        
    def AddOptionsRibbonPanels(self, ribbon):
        App.AddOptionsRibbonPanels(self, ribbon)
        
        from Ribbon import ModeButton
        ModeButton( step.GetShowFaceNormals, step.SetShowFaceNormals, 'showfacenormals', 'hidefacenormals', "ShowFaceNormals", 'Show Face Normals', 'Hide Face Normals').AddToToolbar(self.view_toolbar)
       
    def OnFaceNormalCheck(self, e):
        config = HeeksConfig()
        step.SetShowFaceNormals(not step.GetShowFaceNormals())
        self.FaceNormalCheck.SetValue(step.GetShowFaceNormals())
        config.WriteBool('ShowFaceNormals', step.GetShowFaceNormals())
        self.frame.graphics_canvas.Refresh()
        
    def OnEllipse(self, e):
        EllipseDrawing.SetEllipseDrawing()
        
    def OnSpline(self, e):
        if not self.CheckForNumberOrMore(2, [cad.OBJECT_TYPE_POINT], 'Pick one or more points to make a spline through', 'Spline Through Points'):
            return
        cad.StartHistory('Spline')
        points = []
        for object in cad.GetSelectedObjects():
            points.append(object.GetStartPoint())
        new_object = step.NewSplineFromPoints(points)
        cad.AddUndoably(new_object)
        self.EndHistory()

    def GetSelectionFilterTools(self, filter):
        tools = App.GetSelectionFilterTools(self, filter)
        
        objects = cad.GetSelectedObjects()
                
        if filter.IsTypeInFilter(cad.OBJECT_TYPE_SKETCH) or self.SketchChildTypeInFilter(filter):
            tools.append(ContextTool.CADContextTool('Sketch to Face', 'la2face', step.SketchToFace))
        
        return tools

    def SketchChildTypeInFilter(self, filter):
        if App.SketchChildTypeInFilter(self, filter): return True
        if filter.IsTypeInFilter(step.GetEllipseType()): return True
        if filter.IsTypeInFilter(step.GetSplineType()): return True
        return False
