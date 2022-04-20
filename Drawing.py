import cad
import geom
from LeftAndRight import LeftAndRight
import wx
import math
from InputMode import InputMode
import ToolBarTool
from Object import PyProperty
from HeeksConfig import HeeksConfig

class Drawing(InputMode):
    def __init__(self):
        InputMode.__init__(self)
        self.left_and_right = LeftAndRight()
        self.temp_object_in_list = []
        self.prev_object = None
        self.container = None
        self.button_down_point = None
        self.inhibit_coordinate_change = False
        self.getting_position = False
        self.draw_step = 0
        self.before_start_pos = cad.DigitizedPoint()
        self.start_pos = cad.DigitizedPoint()
        self.user_add_point = geom.Point3D(0,0,0)
        
    def GetTitle(self):
        return 'Drawing'
    
    def DragDoneWithXOR(self):
        return True

    def OnMouse(self, event):
        left_and_right_pressed, event_used = self.left_and_right.LeftAndRightPressed(event)
        
        if left_and_right_pressed:
            if self.DragDoneWithXOR():
                wx.GetApp().EndDrawFront()
            self.ClearObjectsMade()
            wx.GetApp().SetInputMode(wx.GetApp().select_mode)

        if not event_used:
            if event.middleDown or event.GetWheelRotation() != 0:
                wx.GetApp().select_mode.OnMouse(event)
            else:
                if event.LeftDown():
                    if not self.inhibit_coordinate_change:
                        self.button_down_point = cad.Digitize(cad.IPoint(event.x, event.y))
                elif event.LeftUp():
                    if self.inhibit_coordinate_change:
                        self.inhibit_coordinate_change = False
                    else:
                        self.set_digitize_plane()
                        wx.GetApp().digitizing.digitized_point = cad.DigitizedPoint(self.button_down_point)
                        if self.getting_position:
                            self.inhibit_coordinate_change = True
                            self.getting_position = False
                        else:
                            self.AddPoint(wx.GetApp().digitizing.digitized_point)
                elif event.RightUp():
                    # do context menu same as select mode
                    wx.GetApp().select_mode.OnMouse(event)
                elif event.Moving():
                    if not self.inhibit_coordinate_change:
                        end = self.RecalculateAndRedraw(cad.IPoint(event.x, event.y))
                        self.UpdateUserPointProperties(end)                       
                        
    def UpdateUserPointProperties(self, end):
        if end != None:
            p = wx.GetApp().frame.input_mode_canvas.pg.GetProperty('add point')
            if p != None:
                px = p.GetPropertyByName('X')
                if px != None:
                    px.SetValue(str(end.point.x))
                py = p.GetPropertyByName('Y')
                if py != None:
                    py.SetValue(str(end.point.y))
                pz = p.GetPropertyByName('Z')
                if pz != None:
                    pz.SetValue(str(end.point.z))
               
                self.user_add_point = geom.Point3D(end.point)
                
    def UpdatePointFromProperties(self):
        p = wx.GetApp().frame.input_mode_canvas.pg.GetProperty('add point')
        if p != None:
            px = p.GetPropertyByName('X')
            if px != None:
                self.user_add_point.x = float(px.GetValue())
            py = p.GetPropertyByName('Y')
            if py != None:
                self.user_add_point.y = float(py.GetValue())
            pz = p.GetPropertyByName('Z')
            if pz != None:
                self.user_add_point.z = float(pz.GetValue())
            
    def ClearObjectsMade(self):
        self.temp_object_in_list = []
        
    def set_digitize_plane(self):
        pass
    
    def calculate_item(self, end):
        return False
    
    def before_add_item(self):
        pass
    
    def set_previous_direction(self):
        pass
    
    def number_of_steps(self):
        return 2
    
    def step_to_go_to_after_last_step(self):
        return 0
    
    def is_an_add_level(self, level):
        return False
    
    def is_a_draw_level(self, level):
        return self.is_an_add_level(level)
    
    def TempObject(self):
        if len(self.temp_object_in_list) == 0:
            return None
        return self.temp_object_in_list[-1]
    
    def AddPoint(self, d):
        if d.type == cad.DigitizeType.DIGITIZE_NO_ITEM_TYPE:
            return 
        calculated = False
        cad.StartHistory('Add Drawing Point')
        if self.is_an_add_level(self.draw_step):
            calculated = self.calculate_item(d)
            if calculated:
                self.before_add_item()
                self.prev_object = self.TempObject()
                self.AddObjectsMade()
                self.set_previous_direction()
                
        self.ClearObjectsMade()
        self.SetStartPosUndoable(d)
        #cad.UseDigitiedPointAsReference()
        
        next_step = self.draw_step + 1
        if next_step >= self.number_of_steps():
            next_step = self.step_to_go_to_after_last_step()
            
        self.SetDrawStepUndoable(next_step)
        wx.GetApp().EndHistory()
        self.getting_position = False
        self.inhibit_coordinate_change = False
        cad.Repaint()

    def AddToTempObjects(self, object):
        self.temp_object_in_list.append(object)
                        
    def AddObjectsMade(self):
        for object in self.temp_object_in_list:
            cad.AddUndoably(object, self.GetOwnerForDrawingObjects())
        if self.DragDoneWithXOR():cad.DrawObjectsOnFront(self.temp_object_in_list, True)
        self.temp_object_in_list = []        
                
    def RecalculateAndRedraw(self, point):
        self.set_digitize_plane()
        
        end = cad.Digitize(point)
        if end.type == cad.DigitizeType.DIGITIZE_NO_ITEM_TYPE:
            return
        if self.is_a_draw_level(self.draw_step):
            if self.DragDoneWithXOR():
                wx.GetApp().EndDrawFront()
            self.calculate_item(end)
            if self.DragDoneWithXOR():wx.GetApp().DrawFront()
            else: cad.Repaint(True)
        return end
    
    def SetContainerUndoable(self, container):
        undoable = SetContainer(self, container)
        cad.PyIncref(undoable)
        cad.DoUndoable(undoable)
            
    def SetDrawStepUndoable(self, s):
        undoable = SetDrawingDrawStep(self, s)
        cad.PyIncref(undoable)
        cad.DoUndoable(undoable)

    def SetStartPosUndoable(self, pos):
        undoable = SetDrawingPosition(self, pos)
        cad.PyIncref(undoable)
        cad.DoUndoable(undoable)
        
    def OnKeyDown(self, key_code):
        if key_code == cad.KeyCode.F1 or key_code == cad.KeyCode.Return or key_code == cad.KeyCode.Escape:
            self.ClearObjectsMade()
            wx.GetApp().SetInputMode(wx.GetApp().select_mode)
            return True
        return False

    def OnStart(self):
        self.draw_step = 0
        self.ReadDefaultValues()
        if self.AddToSketch():
            cad.StartHistory('Start Sketch', False)
        
    def OnEnd(self):
        if self.AddToSketch():
            wx.GetApp().EndHistory()
        self.WriteDefaultValues()
        self.SetContainerUndoable(None)
        
    def ReadDefaultValues(self):
        pass

    def WriteDefaultValues(self):
        pass
    
    def AddToSketch(self):
        return False
            
    def GetOwnerForDrawingObjects(self):
        if self.AddToSketch():
            if self.container == None:
                self.SetContainerUndoable(cad.NewSketch())
                cad.AddUndoably(self.container)
            return self.container
        return cad.GetApp()
    
    def OnFrontRender(self):
        if self.DragDoneWithXOR() and self.draw_step:
            for object in self.temp_object_in_list:
                object.OnGlCommands(False, False, True)
                
    def GetTools(self):
        tools = []
        tools.append(ToolBarTool.CadToolBarTool('Add Point', 'add', self.AddPointToDrawing))
        tools.append(ToolBarTool.CadToolBarTool('Stop Drawing', 'enddraw', self.EndDrawing))
        tools.append(ToolBarTool.CadToolBarTool('Get Position', 'pickpos', self.PickPosition))
        return tools
    
    def AddPointToDrawing(self):
        # kill focus on control being typed into
        wx.GetApp().frame.input_mode_canvas.pg.ClearSelection()
        self.UpdatePointFromProperties()
        d = cad.DigitizedPoint(self.user_add_point, cad.DigitizeType.DIGITIZE_INPUT_TYPE)
        self.AddPoint(d)
        
    def EndDrawing(self):
        if self.DragDoneWithXOR():
            wx.GetApp().EndDrawFront()
        self.ClearObjectsMade()
        wx.GetApp().SetInputMode(wx.GetApp().select_mode)
        
    def PickPosition(self):
        self.getting_position = True
        
    def GetProperties(self):
        properties = []
        wx.GetApp().AddInputProperty(properties, PyProperty("add point", 'user_add_point', self))
        return properties
            
class SetDrawingDrawStep(cad.BaseUndoable):
    def __init__(self, drawing, s):
        cad.BaseUndoable.__init__(self)
        self.drawing = drawing
        self.old_step = drawing.draw_step
        self.step = s
        
    def GetTitle(self):
        return "set_draw_step"
    
    def Run(self, redo):
        self.drawing.draw_step = self.step
        
    def RollBack(self):
        self.drawing.draw_step = self.old_step
            
class SetContainer(cad.BaseUndoable):
    def __init__(self, drawing, container):
        cad.BaseUndoable.__init__(self)
        self.drawing = drawing
        self.old_container = drawing.container
        self.container = container
        
    def GetTitle(self):
        return "set_container"
    
    def Run(self, redo):
        self.drawing.container = self.container
        
    def RollBack(self):
        self.drawing.container = self.old_container
            
class SetDrawingPosition(cad.BaseUndoable):
    def __init__(self, drawing, pos):
        cad.BaseUndoable.__init__(self)
        self.drawing = drawing
        self.old_before_pos = cad.DigitizedPoint(drawing.before_start_pos)
        self.prev_pos = cad.DigitizedPoint(drawing.start_pos)
        self.next_pos = cad.DigitizedPoint(pos)
        
    def GetTitle(self):
        return "set position"
    
    def Run(self, redo):
        self.drawing.before_start_pos = cad.DigitizedPoint(self.prev_pos)
        self.drawing.start_pos = cad.DigitizedPoint(self.next_pos)
        
    def RollBack(self):
        self.drawing.before_start_pos = cad.DigitizedPoint(self.old_before_pos)
        self.drawing.start_pos = cad.DigitizedPoint(self.prev_pos)

RectanglesRegularShapeMode = 0
PolygonsRegularShapeMode = 1
ObroundRegularShapeMode = 2

class RegularShapesDrawing(Drawing):
    def __init__(self):
        Drawing.__init__(self)
        self.mode = RectanglesRegularShapeMode
        self.number_of_side_for_polygon = 6
        self.rect_radius = 0.0
        self.obround_radius = 2.0
        self.polygon_inscribed = False
        
    def ClearSketch(self):
        if self.TempObject() != None:
            self.TempObject().Clear()
            
    def calculate_item(self, end):
        if end.type == cad.DigitizeType.DIGITIZE_NO_ITEM_TYPE:
            return False
        
        if (self.TempObject() != None) and (self.TempObject().GetType() != cad.OBJECT_TYPE_SKETCH):
            self.ClearObjectsMade()
            
        # make sure sketch exists
        if self.TempObject() == None:
            self.AddToTempObjects(cad.NewSketch())
            
        mat = cad.GetDrawMatrix(True)
        xdir = geom.Point3D(1,0,0).Transformed(mat)
        ydir = geom.Point3D(0,1,0).Transformed(mat)
        zdir = geom.Point3D(0,0,1).Transformed(mat)
        
        p0 = self.start_pos.point
        p2 = end.point
        
        x = p2 * xdir - p0 * xdir
        y = p2 * ydir - p0 * ydir
        
        p1 = p0 + xdir * x
        p3 = p0 + ydir * y
        
        # swap left and right, if user dragged to the left
        if x < 0:
            t = geom.Point3D(p0)
            p0 = geom.Point3D(p1)
            p1 = t
            t = geom.Point3D(p3)
            p3 = geom.Point3D(p2)
            p2 = t
            
        # swap top and bottom, if user dragged upward
        if y < 0:
            t = geom.Point3D(p0)
            p0 = geom.Point3D(p3)
            p3 = t
            t = geom.Point3D(p1)
            p1 = geom.Point3D(p2)
            p2 = t
            
        # add ( or modify ) lines and arcs
        if self.mode == RectanglesRegularShapeMode:
            self.CalculateRectangle(x, y, p0, p1, p2, p3, xdir, ydir, zdir)
        elif self.mode == PolygonsRegularShapeMode:
            self.CalculatePolygon(self.start_pos.point, end.point, zdir)
        elif self.mode == ObroundRegularShapeMode:
            self.CalculateObround(self.start_pos.point, end.point, xdir, zdir)
            
        return True
    
    def CalculateRectangle(self, x, y, p0, p1, p2, p3, xdir, ydir, zdir):
        radii_wanted = False
        x_lines_disappear = False
        y_lines_disappear = False

        if self.rect_radius > 0.0000000001:
            if math.fabs(x) - self.rect_radius*2 > -0.0000000001 and math.fabs(y) - self.rect_radius*2 > -0.0000000001:
                radii_wanted = True
            if math.fabs(math.fabs(x) - self.rect_radius*2) < 0.0000000001:
                x_lines_disappear = True
            if math.fabs(math.fabs(y) - self.rect_radius*2) < 0.0000000001:
                y_lines_disappear = True

        good_num = 4

        if radii_wanted:
            if x_lines_disappear and y_lines_disappear: good_num = 2
            elif x_lines_disappear or y_lines_disappear: good_num = 4
            else: good_num = 8

        if self.TempObject().GetNumChildren() != good_num: self.ClearSketch()
        # check first item
        elif self.TempObject().GetFirstChild().GetType() != (cad.OBJECT_TYPE_SKETCH_ARC if radii_wanted else cad.OBJECT_TYPE_SKETCH_LINE):
            self.ClearSketch()

        if radii_wanted:
            if x_lines_disappear and y_lines_disappear:
                # make two arcs, for a circle
                arcs = [None, None]
                if self.TempObject().GetNumChildren() > 0:
                    object = self.TempObject().GetFirstChild()
                    for i in range(0, 2):
                        arcs[i] = object;
                        object = self.TempObject().GetNextChild()
                else:
                    for i in range(0, 2):
                        arcs[i] = cad.NewArc(geom.Point3D(0, 0, 0), geom.Point3D(0, 0, 0), geom.Point3D(0,0,1), geom.Point3D(0,0,0))
                        self.TempObject().Add(arcs[i], None)
                arcs[0].A = p0 + xdir * self.rect_radius
                arcs[0].B = p3 + xdir * self.rect_radius
                arcs[0].C = p0 + xdir * self.rect_radius + ydir * self.rect_radius
                arcs[0].axis = zdir
                arcs[1].A = arcs[0].B
                arcs[1].B = arcs[0].A
                arcs[1].C = arcs[0].C
                arcs[1].axis = arcs[0].axis
            elif x_lines_disappear or y_lines_disappear:
                # arc-line-arc-line
                arcs = [None, None]
                lines = [None, None]
                if self.TempObject().GetNumChildren() > 0:
                    object = self.TempObject().GetFirstChild()
                    for i in range( 0, 2):
                        arcs[i] = object
                        object = self.TempObject().GetNextChild()
                        lines[i] = object
                        object = self.TempObject().GetNextChild()
                else:
                    for i in range( 0, 2):
                        arcs[i] = cad.NewArc(geom.Point3D(0, 0, 0), geom.Point3D(0, 0, 0), geom.Point3D(0,0,1), geom.Point3D(0,0,0))
                        self.TempObject().Add(arcs[i], None)
                        lines[i] = cad.NewLine(geom.Point3D(0, 0, 0), geom.Point3D(0, 0, 0))
                        self.TempObject().Add(lines[i], None)
    
                if x_lines_disappear:
                    arcs[0].A = p2 - ydir * self.rect_radius
                    arcs[0].B = p3 - ydir * self.rect_radius
                    arcs[0].C = p3 + xdir * self.rect_radius - ydir * self.rect_radius
                    arcs[0].axis = zdir
                    lines[0].A = arcs[0].B
                    lines[0].B = p0 + ydir * self.rect_radius
                    arcs[1].A = lines[0].B
                    arcs[1].B = p1 + ydir * self.rect_radius
                    arcs[1].C = p0 + xdir * self.rect_radius + ydir * self.rect_radius
                    arcs[1].axis = arcs[0].axis
                    lines[1].A = arcs[1].B
                    lines[1].B = arcs[0].A
                else:
                    arcs[0].A = p1 - xdir * self.rect_radius
                    arcs[0].B = p2 - xdir * self.rect_radius
                    arcs[0].C = p1 - xdir * self.rect_radius + ydir * self.rect_radius
                    arcs[0].axis = zdir
                    lines[0].A = arcs[0].B
                    lines[0].B = p3 + xdir * self.rect_radius
                    arcs[1].A = lines[0].B
                    arcs[1].B = p0 + xdir * self.rect_radius
                    arcs[1].C = p0 + xdir * self.rect_radius + ydir * self.rect_radius
                    arcs[1].axis = zdir
                    lines[1].A = arcs[1].B
                    lines[1].B = arcs[0].A
            else:
                # arc-line-arc-line-arc-line-arc-line
                lines = [None, None, None, None]
                arcs = [None, None, None, None]
                if self.TempObject().GetNumChildren() > 0:
                    object = self.TempObject().GetFirstChild()
                    for i in range(0, 4):
                        arcs[i] = object
                        object = self.TempObject().GetNextChild()
                        lines[i] = object
                        object = self.TempObject().GetNextChild()
                else:
                    for i in range(0, 4):
                        arcs[i] = cad.NewArc(geom.Point3D(0, 0, 0), geom.Point3D(0, 0, 0), geom.Point3D(0,0,1), geom.Point3D(0,0,0))
                        self.TempObject().Add(arcs[i], None)
                        lines[i] = cad.NewLine(geom.Point3D(0, 0, 0), geom.Point3D(0, 0, 0))
                        self.TempObject().Add(lines[i], None)
    
                arcs[0].A = p1 - xdir * self.rect_radius
                arcs[0].B = p1 + ydir * self.rect_radius
                arcs[0].C = p1 - xdir * self.rect_radius + ydir * self.rect_radius
                arcs[0].axis = zdir
                lines[0].A = arcs[0].B
                lines[0].B = p2 - ydir * self.rect_radius
                arcs[1].A = lines[0].B
                arcs[1].B = p2 - xdir * self.rect_radius
                arcs[1].C = p2 - xdir * self.rect_radius - ydir * self.rect_radius
                arcs[1].axis = zdir
                lines[1].A = arcs[1].B
                lines[1].B = p3 + xdir * self.rect_radius
                arcs[2].A = lines[1].B
                arcs[2].B = p3 - ydir * self.rect_radius
                arcs[2].C = p3 + xdir * self.rect_radius - ydir * self.rect_radius
                arcs[2].axis = zdir
                lines[2].A = arcs[2].B
                lines[2].B = p0 + ydir * self.rect_radius
                arcs[3].A = lines[2].B
                arcs[3].B = p0 + xdir * self.rect_radius
                arcs[3].C = p0 + xdir * self.rect_radius + ydir * self.rect_radius
                arcs[3].axis = zdir
                lines[3].A = arcs[3].B
                lines[3].B = arcs[0].A
        else:
            # line-line-line-line
            lines = [None, None, None, None]
            if self.TempObject().GetNumChildren() > 0:
                object = self.TempObject().GetFirstChild()
                for i in range(0, 4):
                    lines[i] = object
                    object = self.TempObject().GetNextChild()
            else:
                for i in range(0, 4):
                    lines[i] = cad.NewLine(geom.Point3D(0, 0, 0), geom.Point3D(0, 0, 0))
                    self.TempObject().Add(lines[i])
    
            lines[0].A = p0
            lines[0].B = p1
            lines[1].A = p1
            lines[1].B = p2
            lines[2].A = p2
            lines[2].B = p3
            lines[3].A = p3
            lines[3].B = p0
    
    def CalculatePolygon(self, p0, p1, zdir):
        if p0 == p1:
            return
        
        if self.TempObject() == None:
            return

        if self.TempObject().GetNumChildren() != self.number_of_side_for_polygon:
            self.ClearSketch()
            
        lines = []

        if self.TempObject().GetNumChildren() > 0:
            object = self.TempObject().GetFirstChild()
            for i in range(0, self.number_of_side_for_polygon):
                lines.append(object)
                object = self.TempObject().GetNextChild()
        else:
            for i in range(0, self.number_of_side_for_polygon):
                line = cad.NewLine(geom.Point3D(0, 0, 0), geom.Point3D(0, 0, 0))
                lines.append(line)
                self.TempObject().Add(line)

        radius = p0.Dist(p1)
        sideAngle=0
        
        if self.polygon_inscribed:
            # inscribed circle
            sideAngle = 2.0 * math.pi / self.number_of_side_for_polygon
            radius = radius/math.cos(sideAngle/2)
            for i in range(0, self.number_of_side_for_polygon):
                xdir = p1 - p0
                xdir.Normalize()
                ydir = zdir ^ xdir
                angle0 = (sideAngle * i)+(sideAngle/2)
                angle1 = (sideAngle * (i+1))+(sideAngle/2)
                lines[i].A = p0 + xdir * ( math.cos(angle0) * radius ) + ydir * ( math.sin(angle0) * radius )
                if i == self.number_of_side_for_polygon - 1: lines[i].B = lines[0].A
                lines[i].B = p0 + xdir * ( math.cos(angle1) * radius ) + ydir * ( math.sin(angle1) * radius )

        else:
            # excribed circle
            for i in range(0, self.number_of_side_for_polygon):
                xdir = p1 - p0
                xdir.Normalize()
                ydir = zdir ^ xdir
                angle0 = 2.0 * math.pi / self.number_of_side_for_polygon * i
                angle1 = 2.0 * math.pi / self.number_of_side_for_polygon * (i+1)
                lines[i].A = p0 + xdir * ( math.cos(angle0) * radius ) + ydir * ( math.sin(angle0) * radius )
                if i == self.number_of_side_for_polygon - 1: lines[i].B = lines[0].A
                lines[i].B = p0 + xdir * ( math.cos(angle1) * radius ) + ydir * ( math.sin(angle1) * radius )
    
    def CalculateObround(self, p0, p1, xdir, zdir):
        lines_disappear = False

        if self.obround_radius > 0.0000000001:
            if p0 == p1: lines_disappear = True
        else:
            return

        good_num = 4
        if lines_disappear: good_num = 2

        if self.TempObject().GetNumChildren() != good_num:
            self.ClearSketch()
            
        if lines_disappear:
            # make two arcs, for a circle
            arcs = [None, None]
            if self.TempObject().GetNumChildren() > 0:
                object = self.TempObject().GetFirstChild()
                for i in range(0, 2):
                    arcs[i] = object
                    object = self.TempObject().GetNextChild()
            else:
                for i in range(0, 2):
                    arcs[i] = cad.NewArc(geom.Point3D(0, 0, 0), geom.Point3D(0, 0, 0), geom.Point3D(0,0,1), geom.Point3D(0,0,0))
                    self.TempObject().Add(arcs[i])

            arcs[0].A = p0 + xdir * self.obround_radius
            arcs[0].B = p0 - xdir * self.obround_radius
            arcs[0].C = p0
            arcs[0].axis = zdir
            arcs[1].A = arcs[0].B
            arcs[1].B = arcs[0].A
            arcs[1].C = arcs[0].C
            arcs[1].axis = arcs[0].axis
        else:
            # arc-line-arc-line
            arcs = [None, None]
            lines = [None, None]
            if self.TempObject().GetNumChildren() > 0:
                object = self.TempObject().GetFirstChild()
                for i in range(0, 2):
                    arcs[i] = object
                    object = self.TempObject().GetNextChild()
                    lines[i] = object
                    object = self.TempObject().GetNextChild()
            else:
                for i in range(0, 2):
                    arcs[i] = cad.NewArc(geom.Point3D(0, 0, 0), geom.Point3D(0, 0, 0), geom.Point3D(0,0,1), geom.Point3D(0,0,0))
                    self.TempObject().Add(arcs[i])
                    lines[i] = cad.NewLine(geom.Point3D(0, 0, 0), geom.Point3D(0, 0, 0))
                    self.TempObject().Add(lines[i])
    
            along_dir = p1 - p0
            along_dir.Normalize()
            right_dir = along_dir ^ zdir
    
            arcs[0].A = p1 + right_dir * self.obround_radius
            arcs[0].B = p1 - right_dir * self.obround_radius
            arcs[0].C = p1
            arcs[0].axis = zdir
            lines[0].A = arcs[0].B
            lines[0].B = p0 - right_dir * self.obround_radius
            arcs[1].A = lines[0].B
            arcs[1].B = p0 + right_dir * self.obround_radius
            arcs[1].C = p0
            arcs[1].axis = zdir
            lines[1].A = arcs[1].B
            lines[1].B = arcs[0].A
    
    def number_of_steps(self):
        return 2
    
    def step_to_go_to_after_last_step(self):
        return 0
    
    def is_an_add_level(self, level):
        return level == 1
    
    def GetTitle(self):
        if self.mode == RectanglesRegularShapeMode:
            return 'Rectangle drawing'
        elif self.mode == PolygonsRegularShapeMode:
            return 'Polygon drawing'
        elif self.mode == ObroundRegularShapeMode:
            return 'Obround drawing'
        
        return 'Regular shapes drawing'
    
regular_shapes_drawing = RegularShapesDrawing()
    
def SetRectanglesDrawing():
    global regular_shapes_drawing
    regular_shapes_drawing.mode = RectanglesRegularShapeMode
    wx.GetApp().SetInputMode(regular_shapes_drawing)    
    
def SetObroundsDrawing():
    global regular_shapes_drawing
    regular_shapes_drawing.mode = ObroundRegularShapeMode
    wx.GetApp().SetInputMode(regular_shapes_drawing)    
    
def SetPolygonsDrawing():
    global regular_shapes_drawing
    regular_shapes_drawing.mode = PolygonsRegularShapeMode
    wx.GetApp().SetInputMode(regular_shapes_drawing)
