import cad
import math
import geom
import wx

class DigitizeMode(cad.InputMode):
    def __init__(self):
        cad.InputMode.__init__(self)
        self.point_or_window = PointOrWindow(False)
        self.digitized_point = cad.DigitizedPoint()
        
    def GetTitle(self):
        s = self.prompt + '\n'
        
        if math.fabs(self.digitized_point.point.z) < 0.00000001:
            s += 'X' + '%.3f' % self.digitized_point.point.x + ' Y' + '%.3f' % self.digitized_point.point.y
        elif math.fabs(self.digitized_point.point.y) < 0.00000001:
            s += 'X' + '%.3f' % self.digitized_point.point.x + ' Z' + '%.3f' % self.digitized_point.point.z
        elif math.fabs(self.digitized_point.point.x) < 0.00000001:
            s += 'Y' + '%.3f' % self.digitized_point.point.y + ' Z' + '%.3f' % self.digitized_point.point.z
        else:
            s += 'X' + '%.3f' % self.digitized_point.point.x + ' Y' + '%.3f' % self.digitized_point.point.y + ' Z' + '%.3f' % self.digitized_point.point.z
        
        type_str = None
        if self.digitized_point.type == cad.DigitizeType.DIGITIZE_ENDOF_TYPE: type_str = 'end'
        elif self.digitized_point.type == cad.DigitizeType.DIGITIZE_INTER_TYPE: type_str = 'intersection'
        elif self.digitized_point.type == cad.DigitizeType.DIGITIZE_MIDPOINT_TYPE: type_str = 'midpoint'
        elif self.digitized_point.type == cad.DigitizeType.DIGITIZE_CENTRE_TYPE: type_str = 'centre'
        elif self.digitized_point.type == cad.DigitizeType.DIGITIZE_SCREEN_TYPE: type_str = 'screen'
        elif self.digitized_point.type == cad.DigitizeType.DIGITIZE_NEAREST_TYPE: type_str = 'nearest'
        elif self.digitized_point.type == cad.DigitizeType.DIGITIZE_TANGENT_TYPE: type_str = 'tangent'
        
        if type_str != None:
            s += ' (' + type_str + ')'
        
        return s
    
    def GetHelpText(self):
        return 'Press Esc key to cancel\nLeft button to accept position'
        
    def OnMouse( self, event ):
        if event.LeftDown():
            self.point_or_window.OnMouse(event)
            self.lbutton_point = cad.Digitize(cad.IPoint(event.x, event.y))
        elif event.LeftUp():
            if self.lbutton_point.type != cad.DigitizeType.DIGITIZE_NO_ITEM_TYPE:
                self.digitized_point = self.lbutton_point
                self.wants_to_exit_main_loop = True
        elif event.Moving():
            self.digitized_point = cad.Digitize(cad.IPoint(event.x, event.y))
            self.point_or_window.OnMouse(event)
            wx.GetApp().frame.input_mode_canvas.Refresh()
            cad.Repaint()
            
    def OnModeChange(self):
        self.point_or_window.reset()
        self.point_or_window.OnModeChange()
        
    def OnFrontRender(self):
        self.point_or_window.OnFrontRender()

class PointOrWindow(cad.InputMode):
    def __init__(self, wd):
        if wd: self.window = WindowDragging()
        else: self.window = None
        self.box_chosen = cad.IRect()
        self.reset()
        self.visible = False
        
    def reset(self):
        if self.window != None: self.window.reset()
        self.use_window = False
        
    def OnMouse(self, event):
        if event.LeftDown():
            self.mouse_down_point = cad.IPoint(event.x, event.y)
        elif event.Moving():
            if event.leftDown and not self.use_window and (self.window != None) and abs(event.x - self.mouse_down_point.x) > 2 and abs(event.y - self.mouse_down_point.y) > 2:
                self.use_window = True
                self.box_chosen.x = self.mouse_down_point.x
                self.box_chosen.y = wx.GetApp().GetViewport().GetHeight() - self.mouse_down_point.y
                self.window.window_box = self.box_chosen
    
            if self.use_window:
                self.box_chosen.width = event.x - self.box_chosen.x
                self.box_chosen.height = wx.GetApp().GetViewport().GetHeight() - event.y - self.box_chosen.y
            else:
                self.SetWithPoint(cad.IPoint(event.x, event.y))
                
    def OnModeChange(self):
        self.visible = True
        
    def SetWithPoint(self, point):
        self.box_chosen.x = point.x - 5
        self.box_chosen.y = point.y - 5
        self.box_chosen.width = 10
        self.box_chosen.height = 10
        self.use_window = False
        
    def OnFrontRender(self):
        if not self.visible: return
        cad.DrawColor(cad.Color(255, 255, 255)) 
        wx.GetApp().GetViewport().DrawWindow(self.box_chosen, False)
             
class WindowDragging(cad.InputMode):
    def __init__(self):
        self.reset()

    def reset(self):
        self.window_box = cad.IRect()
        self.box_found = False
        self.finish_dragging = False
        
    def OnMouse(self, event):
        if event.LeftDown():
            self.window_box.x = event.x
            self.window_box.y = wx.GetApp().GetViewport().GetHeight() - event.y
        elif event.LeftUp():
            self.window_box.width = event.x - self.window_box.x
            self.window_box.height = wx.GetApp().GetViewport().GetHeight() - self.window_box.y - event.y
            if abs(self.window_box.width)<4: self.box_found = False
            elif abs(self.window_box.height)<4: self.box_found = False
            else:self.box_found = True
            self.finish_dragging = True
        elif event.Moving():
            self.window_box.width = event.x - self.window_box.x
            self.window_box.height = wx.GetApp().GetViewport().GetHeight() - self.window_box.y - event.y
