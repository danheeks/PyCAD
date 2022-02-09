import cad
import geom
import wx
import math
from Drawing import Drawing
from Object import PyChoiceProperty
from Object import PyProperty

LineDrawingMode = 0
ArcDrawingMode = 1
ILineDrawingMode = 2
CircleDrawingMode = 3

CentreAndPointCircleMode = 0
ThreePointsCircleMode = 1
TwoPointsCircleMode = 2
CentreAndRadiusCircleMode = 3 # only one click needed ( edit radius in the properties before clicking)
        
class SetPreviousDirection(cad.BaseUndoable):
    def __init__(self, drawing, n):
        cad.BaseUndoable.__init__(self)
        self.drawing = drawing
        self.old_direction = drawing.previous_direction
        self.new_direction = n
        
    def GetTitle(self):
        return "set previous direction"
    
    def Run(self, redo):
        self.drawing.previous_direction = self.new_direction
        
    def RollBack(self):
        self.drawing.previous_direction = self.old_direction

class LineArcDrawing(Drawing):
    def __init__(self):
        Drawing.__init__(self)
        self.previous_direction = None
        self.drawing_mode = LineDrawingMode
        self.radius_for_circle = 5.0
        self.circle_mode = ThreePointsCircleMode
        self.save_drawing_mode = []

    def set_previous_direction(self):
        if self.prev_object == None: return

        if self.prev_object.GetType() == cad.OBJECT_TYPE_SKETCH_LINE:
            undoable = SetPreviousDirection(self, self.prev_object.B - self.prev_object.A)
            cad.PyIncref(undoable)
            cad.DoUndoable(undoable)
        elif self.prev_object.GetType() == cad.OBJECT_TYPE_SKETCH_ARC:
            undoable = SetPreviousDirection(self, (self.prev_object.axis ^ (self.prev_object.B - self.prev_object.C)).Normalized())
            cad.PyIncref(undoable)
            cad.DoUndoable(undoable)

    
    def number_of_steps(self):
        if self.drawing_mode == CircleDrawingMode:
            if self.circle_mode == ThreePointsCircleMode:
                return 3
            elif self.circle_mode == CentreAndRadiusCircleMode:
                return 1
        return 2

    def step_to_go_to_after_last_step(self):
        if self.drawing_mode == LineDrawingMode or self.drawing_mode == ArcDrawingMode:
            return 1
        return 0

    def is_a_draw_level(self, level):
        if self.drawing_mode == CircleDrawingMode:
            if self.circle_mode == ThreePointsCircleMode:
                return level > 0
        return self.is_an_add_level(level)
    
    def is_an_add_level(self, level):
        if self.drawing_mode == CircleDrawingMode:
            if self.circle_mode == ThreePointsCircleMode:
                return level == 2
            elif self.circle_mode == CentreAndRadiusCircleMode:
                return level == 0
        return level == 1
    
    def AddPoint(self, d):
        if (self.drawing_mode == LineDrawingMode) or (self.drawing_mode == ArcDrawingMode):
            # edit the end of the previous item to be the start of the arc
            # this only happens if we are drawing tangents to other objects
            # really need to fill the gap with whatever we are tangent around
            # ellipse,arc,spline or whatever
            if (self.TempObject() != None) and (self.prev_object != None):
                if (self.prev_object != None):
                    spos = self.TempObject().GetStartPoint()
                    epos = self.prev_object.GetEndPoint()
                    tanobject = self.start_pos.GetObject1()
                    if self.start_pos.type == cad.DigitizeType.DIGITIZE_TANGENT_TYPE and (tanobject != None):
                        if tanobject.GetType() == cad.OBJECT_TYPE_SKETCH_LINE:
                            #((HLine*)prev_object)->B = p;
                            pass
                        elif tanobject.GetType() == cad.OBJECT_TYPE_SKETCH_ARC:
                            tanobject.A = geom.Point3D(spos)
                            tanobject.B = geom.Point3D(epos)
                            self.AddToTempObjects(tanobject)
                        elif tanobject.GetType() == cad.OBJECT_TYPE_CIRCLE:
                            arc = cad.NewArc(spos, epos, tanobject.axis, tanobject.C)
                            arc.A = geom.Point3D(spos)
                            arc.B = geom.Point3D(epos)
                            self.AddToTempObjects(arc)
        Drawing.AddPoint(self, d)
    
    def calculate_item(self, end):
        if self.number_of_steps() > 1 and self.start_pos.type == cad.DigitizeType.DIGITIZE_NO_ITEM_TYPE:return False
        if end.type == cad.DigitizeType.DIGITIZE_NO_ITEM_TYPE: return False
    
        if self.drawing_mode == LineDrawingMode:
            if self.TempObject() != None and self.TempObject().GetType() != cad.OBJECT_TYPE_SKETCH_LINE:
                self.ClearObjectsMade()
            #DigitizeMode::GetLinePoints(GetStartPos(), end, p1, p2);
            p1 = self.start_pos.point
            p2 = end.point
            if p1 == p2: return False
            end.point = p2
            if self.TempObject() == None:
                self.AddToTempObjects(cad.NewLine(p1, p2))
            else:
                self.TempObject().A = p1
                self.TempObject().B = p2
            return True
    
        elif self.drawing_mode == ArcDrawingMode:
            # tangential arcs
            if (self.TempObject() != None) and (self.TempObject().GetType() != cad.OBJECT_TYPE_SKETCH_ARC):
                self.ClearObjectsMade()

            #Point3d centre;
            #Point3d axis;
            #Point3d p1, p2;
            #bool arc_found = DigitizeMode::GetArcPoints(GetStartPos(), m_previous_direction_set ? (&m_previous_direction) : NULL, end, p1, p2, centre, axis);
            arc_found = True
            p1 = self.start_pos.point
            p2 = end.point
            if p1 == p2: return False
            
            if self.previous_direction == None:
                return False
            
            if arc_found:
                tangent_arc_found, centre, axis = cad.TangentialArc(p1, self.previous_direction, p2)
                if tangent_arc_found:
                    # arc
                    if self.TempObject() == None:
                        self.AddToTempObjects(cad.NewArc(p1, p2, axis, centre))
                    else:
                        self.TempObject().axis = axis
                        self.TempObject().C = centre
                        self.TempObject().A = p1
                        self.TempObject().B = p2
                else:
                    # line
                    if (self.TempObject() != None) and self.TempObject().GetType() != cad.OBJECT_TYPE_SKETCH_LINE:
                        self.ClearObjectsMade()
                    if self.TempObject() == None:
                        self.AddToTempObjects(cad.NewLine(p1, p2))
                    else:
                        self.TempObject().A = p1
                        self.TempObject().B = p2
            return True
    
        elif self.drawing_mode == ILineDrawingMode:
            if self.TempObject() and self.TempObject().GetType() != cad.OBJECT_TYPE_ILINE:
                self.ClearObjectsMade()
            #DigitizeMode::GetLinePoints(GetStartPos(), end, p1, p2);
            p1 = self.start_pos.point
            p2 = end.point
            if p1 == p2: return False
            if self.TempObject() == None:
                new_object = cad.ILine(p1, p2)
                cad.PyIncref(new_object)
                self.AddToTempObjects(new_object)
            else:
                self.TempObject().A = p1
                self.TempObject().B = p2
            return True
    
        elif self.drawing_mode == CircleDrawingMode:
            if self.TempObject() and self.TempObject().GetType() != cad.OBJECT_TYPE_CIRCLE:
                self.ClearObjectsMade()
                
            if self.circle_mode == CentreAndPointCircleMode:
#                        Point3d p1, p2, centre;
#                        Point3d axis;
#                        DigitizeMode::GetArcPoints(GetStartPos(), NULL, end, p1, p2, centre, axis);
                p1 = self.start_pos.point
                p2 = end.point
                self.radius_for_circle = p1.Dist(p2)
                if self.TempObject() == None:
                    self.AddToTempObjects(cad.NewCircle(p1, geom.Point3D(0, 0, 1), self.radius_for_circle))
                else:
                    self.TempObject().C = p1
                    self.TempObject().radius = self.radius_for_circle
                return True
    
            elif self.circle_mode == ThreePointsCircleMode:
                if self.draw_step == 1:
                    # add a line for graphics
                    if self.TempObject() == None:
                        self.AddToTempObjects(cad.NewLine(self.start_pos.point, end.point))
                    else:
                        self.TempObject().A = self.start_pos.point
                        self.TempObject().B = end.point
                else: # draw_step = 2
                    # add the circle
                    found, c = cad.GetTangentCircle(self.before_start_pos, self.start_pos, end)
                    if self.TempObject() == None:
                        self.AddToTempObjects(cad.NewCircle(geom.Point3D(c.c.x, c.c.y, 0.0), geom.Point3D(0, 0, 1), c.radius))
                    else:
                        self.TempObject().SetCircle(c)
                return True
            elif self.circle_mode == TwoPointsCircleMode:
                found, c = cad.GetCircleBetween(self.start_pos, end)
                if found:
                    if self.TempObject() == None:
                        self.AddToTempObjects(cad.NewCircle(geom.Point3D(c.c.x, c.c.y, 0.0), geom.Point3D(0, 0, 1), c.radius))
                    else:
                        self.TempObject().SetCircle(c)
                return True
            elif self.circle_mode == CentreAndRadiusCircleMode:
                if self.TempObject() == None:
                    self.AddToTempObjects(cad.NewCircle(end.point, geom.Point3D(0, 0, 1), self.radius_for_circle))
                else:
                    self.TempObject().C = end.point
                    self.TempObject().radius = self.radius_for_circle
                return True

    def GetTitle(self):
        s = 'unknown'
        if self.drawing_mode == LineDrawingMode:
            s = "Line drawing mode : "
            if self.draw_step == 0: s += "click on start point"
            else: s+="click on end of line"
        elif self.drawing_mode == ArcDrawingMode:
            s = "Arc drawing mode : "
            if self.draw_step == 0: s += "click on start point"
            else: s+="click on end of arc"
        elif self.drawing_mode == ILineDrawingMode:
            s = "Infinite line drawing : "
            if self.draw_step == 0: s += "click on first point"
            else: s += "click on second point"
        elif self.drawing_mode == CircleDrawingMode:
            s = "Circle drawing mode : "
            if self.circle_mode == CentreAndPointCircleMode:
                s += "centre and point mode\n"
                if self.draw_step == 0: s += "click on centre point"
                else: s+="click on point on circle"
            elif self.circle_mode == ThreePointsCircleMode:
                s += "three points mode\n"
                if self.draw_step == 0: s += "click on first point"
                elif self.draw_step == 1: s+="click on second point"
                else: s+="click on third point"
            elif self.circle_mode == TwoPointsCircleMode:
                s += "two points mode\n"
                if self.draw_step == 0: s += "click on first point"
                else: s+="click on second point"
            elif self.circle_mode == CentreAndRadiusCircleMode:
                s += "centre with radius mode\n  click on centre point"
        return s
    
    def OnKeyUp(self, key_code):
        if key_code == ord('A'):
            if self.drawing_mode == ArcDrawingMode:
                # switch back to previous drawing mode
                if len(self.save_drawing_mode) > 0:
                    self.drawing_mode = self.save_drawing_mode[-1]
                    self.save_drawing_mode.pop()
            else:
                # switch to arc drawing mode until a is released
                self.save_drawing_mode.append(self.drawing_mode)
                self.drawing_mode = ArcDrawingMode
            wx.GetApp().frame.input_mode_canvas.Refresh()
            return True
        return Drawing.OnKeyUp(self, key_code)
    
    def OnModeChange(self):
        # on start of drawing mode
        Drawing.OnModeChange(self)
        if self.container:
            self.container = None
        self.prev_object = None
        self.previous_direction = None
        
    def GetProperties(self):
        properties = []
        if self.drawing_mode == CircleDrawingMode:
            if self.circle_mode == CentreAndRadiusCircleMode:
                wx.GetApp().AddInputProperty(properties, PyProperty("radius", 'radius_for_circle', self))
        properties += Drawing.GetProperties(self)
        return properties
    
line_arc_drawing = LineArcDrawing()
    
def SetLineArcDrawing():
    global line_arc_drawing
    line_arc_drawing.drawing_mode = LineDrawingMode
    wx.GetApp().SetInputMode(line_arc_drawing)    
    
def SetCircles3pDrawing():
    global line_arc_drawing
    line_arc_drawing.drawing_mode = CircleDrawingMode
    line_arc_drawing.circle_mode = ThreePointsCircleMode
    wx.GetApp().SetInputMode(line_arc_drawing)    
    
def SetCircles2pDrawing():
    global line_arc_drawing
    line_arc_drawing.drawing_mode = CircleDrawingMode
    line_arc_drawing.circle_mode = CentreAndPointCircleMode
    wx.GetApp().SetInputMode(line_arc_drawing)    
    
def SetCircle1pDrawing():
    global line_arc_drawing
    line_arc_drawing.drawing_mode = CircleDrawingMode
    line_arc_drawing.circle_mode = CentreAndRadiusCircleMode
    wx.GetApp().SetInputMode(line_arc_drawing)    
    
def SetILineDrawing():
    global line_arc_drawing
    line_arc_drawing.drawing_mode = ILineDrawingMode
    wx.GetApp().SetInputMode(line_arc_drawing)    

