import cad
import geom
import step
import wx
import math
from Drawing import Drawing


EllipseDrawingMode = 0
SplineDrawingMode = 1

CubicSplineMode = 0
QuarticSplineMode = 1
RationalSplineMode = 2

class EllipseDrawing(Drawing):
    def __init__(self):
        Drawing.__init__(self)
        self.spline_points = []
        self.drawing_mode = EllipseDrawingMode
        self.spline_mode = CubicSplineMode

    def number_of_steps(self):
        if self.drawing_mode == SplineDrawingMode:
            if self.spline_mode == CubicSplineMode: return 4
            elif self.spline_mode == QuarticSplineMode: return 3
            else: return 20
        elif self.drawing_mode == EllipseDrawingMode:
            return 3

    def step_to_go_to_after_last_step(self):
        if self.drawing_mode == SplineDrawingMode: return 3;
        return 0;

    def is_a_draw_level(self, level):
        if (self.drawing_mode == SplineDrawingMode) and (self.spline_mode == RationalSplineMode):
            return level >= 3
        if self.drawing_mode == EllipseDrawingMode:
            return level >= 1
        return Drawing.is_a_draw_level(self, level)

    def is_an_add_level(self, level):
        if self.drawing_mode == EllipseDrawingMode:
            return level == 2
        else: #case SplineDrawingMode:
            if self.spline_mode == CubicSplineMode:
                return level == 3
            elif self.spline_mode == QuarticSplineMode:
                return level == 2
            elif self.spline_mode == RationalSplineMode:
                return level == 20
        return level == 1

    def calculate_item(self, end):
        if self.number_of_steps() > 1 and self.start_pos.type == cad.DigitizeType.DIGITIZE_NO_ITEM_TYPE: return False
        if end.type == cad.DigitizeType.DIGITIZE_NO_ITEM_TYPE: return False

        if self.drawing_mode == EllipseDrawingMode:
            if self.draw_step == 1:
                # add a circle for graphics
                c_rad = self.start_pos.point.Dist(end.point)
                if self.TempObject() == None:
                    self.AddToTempObjects(cad.NewCircle(self.start_pos.point, geom.Point3D(0, 0, 1), c_rad))
                else:
                    self.TempObject().radius = c_rad
            else: # draw_step = 2
                if (self.TempObject() != None) and (self.TempObject().GetType() != step.GetEllipseType()):
                    self.ClearObjectsMade()
        
                if self.TempObject() == None:
                    new_object = step.Ellipse(self.before_start_pos, self.start_pos, end)
                    cad.PyIncref(new_object)
                    self.AddToTempObjects(new_object)
                else:
                    self.TempObject().SetFromDigitized(self.before_start_pos, self.start_pos, end)
            return True

        if self.drawing_mode == SplineDrawingMode:
            if (self.TempObject() != None) and (self.TempObject().GetType() != step.GetSplineType()):
                self.ClearObjectsMade()
# 
#         Handle_Geom_BSplineCurve spline;
#         switch (spline_mode)
#         {
#         case CubicSplineMode:
#             DigitizedPointGetCubicSpline(GetBeforeBeforeStartPos(), GetBeforeStartPos(), GetStartPos(), end, spline);
#             break;
#         case QuarticSplineMode:
#             DigitizedPointGetQuarticSpline(GetBeforeStartPos(), GetStartPos(), end, spline);
#             break;
#         case RationalSplineMode:
#             DigitizedPointGetRationalSpline(spline_points, end, spline);
#             break;
#         }
# 
#         if (TempObject() == NULL){
#             AddToTempObjects(new HSpline(spline, &theApp->GetCurrentColor()));
#         }
#         else{
#             ((HSpline*)TempObject())->m_spline = spline;
#         }
# 
#         return true;

        return False

    def GetTitle(self):
        if self.drawing_mode == EllipseDrawingMode:
            s = 'Ellipse drawing mode : center and 2 points mode\n'
            if self.draw_step == 0: s += 'click on center point'
            elif self.draw_step == 1:
                s += 'click on point on ellipse\n  (colinear or orthogonal to axis)'
            else: s += 'click on another point on ellipse'
        return s
#     case SplineDrawingMode:
# 
#         str_for_GetTitle = std::wstring(L"Spline drawing mode");
#         str_for_GetTitle.append(std::wstring(L" : "));
# 
#         switch (spline_mode){
#         case CubicSplineMode:
#             str_for_GetTitle.append(std::wstring(L"cubic spline mode"));
#             str_for_GetTitle.append(std::wstring(L"\n  "));
#             if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(L"click on start point"));
#             else if (GetDrawStep() == 1) str_for_GetTitle.append(std::wstring(L"click on end point"));
#             else if (GetDrawStep() == 2) str_for_GetTitle.append(std::wstring(L"click on first control point"));
#             else str_for_GetTitle.append(std::wstring(L"click on second control point"));
#             break;
#         case QuarticSplineMode:
#             str_for_GetTitle.append(std::wstring(L"quartic spline mode"));
#             str_for_GetTitle.append(std::wstring(L"\n  "));
#             if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(L"click on start point"));
#             else if (GetDrawStep() == 1) str_for_GetTitle.append(std::wstring(L"click on end point"));
#             else str_for_GetTitle.append(std::wstring(L"click on control point"));
#             break;
#         case RationalSplineMode:
#             str_for_GetTitle.append(std::wstring(L"rational spline mode"));
#             str_for_GetTitle.append(std::wstring(L"\n  "));
#             if (GetDrawStep() == 0)str_for_GetTitle.append(std::wstring(L"click on start point"));
#             else if (GetDrawStep() == 1) str_for_GetTitle.append(std::wstring(L"click on first control point"));
#             else if (GetDrawStep() == 2) str_for_GetTitle.append(std::wstring(L"click on second control point"));
#             else str_for_GetTitle.append(std::wstring(L"click on next control point or endpoint"));
# 
#             break;
#         }
    
ellipse_drawing = EllipseDrawing()
    
def SetEllipseDrawing():
    global ellipse_drawing
    ellipse_drawing.drawing_mode = EllipseDrawingMode
    wx.GetApp().SetInputMode(ellipse_drawing)    
