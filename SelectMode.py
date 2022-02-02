import cad
import wx
import geom
from LeftAndRight import LeftAndRight
from InputMode import InputMode
        
class SelectMode(InputMode):
    def __init__(self):
        InputMode.__init__(self)
        self.left_and_right = LeftAndRight()
        self.control_key_initially_pressed = False
        self.button_down = False
        self.middle_button_down = False
        self.just_one = False
        self.CurrentPoint = None
        self.button_down_point = None
        self.window_box = None
        self.mouse_move_highlighting = False
        self.highlighted_objects = []
        self.highlight_color = cad.Color(0,255,0)
        self.prompt = ''
        self.dragging_moves_objects = True
        self.drag_gripper = None
        self.ctrl_does_rotate = False
        self.filter = cad.Filter()
        
    def GetTitle(self):
        return self.prompt if self.prompt else 'Select Mode'
    
    def GetHelpText(self):
        s = 'Left button for selecting objects\n( with Ctrl key for extra objects)\n( with Shift key for similar objects)\nDrag with left button to window select'
        
        if self.dragging_moves_objects: s+='\nor to move object if on an object'
        s += '\nMouse wheel to zoom in and out'
        
        if self.ctrl_does_rotate:
            s += '\nMiddle button to pan view\n( with Ctrl key to rotate view )'
        else:
            s += '\nMiddle button to rotate view\n( with Ctrl key to pan view )'

        s += '\nRight button for object menu\nSee options window to hide this help\n("view options"->"screen text")'
        
        if wx.GetApp().inMainLoop:
            s += '\nPress Esc key to cancel'
            s += '\nPress Return key to accept selection'

        return s
    
    def OnRender(self):
        for object in self.highlighted_objects:
            cad.DrawColor(self.highlight_color)
            object.OnGlCommands(False, False, True)
                
    def OnMouse(self, event):
        left_and_right_pressed, event_used = self.left_and_right.LeftAndRightPressed(event)
        
        if left_and_right_pressed:
            if wx.GetApp().inMainLoop:
                wx.GetApp().ExitMainLoop()
                
        if event_used:
            return
        
        if event.LeftDown():
            self.button_down_point = cad.IPoint(event.x, event.y)
            self.CurrentPoint = self.button_down_point
            self.button_down = True
            
            self.highlighted_objects = []
            
            if self.dragging_moves_objects:
                objects = cad.ObjectsUnderWindow(cad.IRect(event.x, event.y), False, True, self.filter, True)
                for object in objects:
                    if object.GetType() == cad.OBJECT_TYPE_GRIPPER:
                        wx.GetApp().DrawFront()
                        self.drag_gripper = object
                        self.drag_gripper.__class__ = cad.Gripper
                        self.grip_from = cad.Digitize(self.button_down_point)
                        self.grip_to = cad.DigitizedPoint(self.grip_from)
                        self.drag_gripper.OnGripperGrabbed(True, self.grip_from.point)
                        wx.GetApp().EndDrawFront()
                        break
        elif event.LeftUp():
            if self.drag_gripper:
                self.grip_to = cad.Digitize(cad.IPoint(event.x, event.y))
                self.drag_gripper.OnGripperReleased(self.grip_from.point, self.grip_to.point)
                self.drag_gripper = None
            elif self.window_box:
                if not event.controlDown:
                    cad.ClearSelection(True)
                only_if_fully_in = True
                if self.window_box.width < 0:
                    only_if_fully_in = False
                objects = cad.ObjectsUnderWindow(self.window_box, only_if_fully_in, False, self.filter, True)
                for object in objects: cad.Select(object)
                self.window_box = None
            elif self.button_down_point != None:
                 wx.GetApp().OnLeftClick(event)
            if self.just_one and wx.GetApp().inMainLoop and cad.GetNumSelected() > 0:
                wx.GetApp().ExitMainLoop()
            else:
                wx.GetApp().frame.graphics_canvas.Refresh()
                
        elif event.MiddleDown():
            self.middle_button_down = True
            self.button_down_point = cad.IPoint(event.x, event.y)
            self.CurrentPoint = self.button_down_point
            wx.GetApp().frame.graphics_canvas.viewport.StoreViewPoint()
            
        elif event.Moving():
            if event.middleDown or (event.controlDown and event.leftDown):
                v = wx.GetApp().GetViewport()
                dm = cad.IPoint(event.x - self.CurrentPoint.x, event.y - self.CurrentPoint.y)
                if event.controlDown and not event.leftDown:
                    v.view_point.ShiftI(dm)
                else:
                    if cad.GetRotateUpright():
                        v.view_point.TurnVerticalI(dm)
                    else:
                        v.view_point.TurnI(dm)
                wx.GetApp().frame.graphics_canvas.Update()
                wx.GetApp().frame.graphics_canvas.Refresh()
            elif event.leftDown:
                if self.drag_gripper != None:
                    to = cad.Digitize(cad.IPoint(event.x, event.y))
                    self.drag_gripper.OnGripperMoved(self.grip_from.point, to.point)                    
                elif self.button_down_point != None and (abs(self.button_down_point.x - event.x) > 2 or abs(self.button_down_point.y - event.y) > 2):
                    if self.dragging_moves_objects and self.window_box == None:
                        selected_objects_dragged = []

                        if cad.GetNumSelected() > 0:
                            objects = cad.GetSelectedObjects()
                        else:
                            objects = cad.ObjectsUnderWindow(cad.IRect(self.button_down_point.x, self.button_down_point.y), False, True, self.filter, True)
                            for object in objects:
                                cad.Select(object)

                        for object in objects:
                            selected_objects_dragged.append(object)
                            
                        if len(selected_objects_dragged) > 0:
                            self.drag_gripper = cad.GetDragGripper()
                            self.grip_from = cad.Digitize(self.button_down_point)
                            self.grip_to = cad.DigitizedPoint(self.grip_from)
                            self.drag_gripper.OnGripperGrabbed(True, self.grip_from.point)
                            self.drag_gripper.OnGripperMoved(self.grip_from.point, self.grip_to.point)
                            return
                        
                    if not self.just_one:
                        v = wx.GetApp().GetViewport()
                        v.SetXOR()
                        if self.window_box:
                            v.DrawWindow(self.window_box, True) # undraw the window
                        self.window_box = cad.IRect(self.button_down_point.x, self.button_down_point.y, event.x - self.button_down_point.x, event.y - self.button_down_point.y)
                        v.DrawWindow(self.window_box, True) # draw the window
                        v.EndXOR()
            else:
                if self.window_box == None and self.mouse_move_highlighting:
                    self.highlighted_objects = cad.ObjectsUnderWindow(cad.IRect(event.x, event.y), False, True, self.filter, True)                        
                    wx.GetApp().frame.graphics_canvas.Refresh()
            self.CurrentPoint = cad.IPoint(event.x, event.y)
            
        if event.GetWheelRotation() != 0:
            wx.GetApp().GetViewport().OnWheelRotation(-event.wheelRotation, event.x, event.y)
            wx.GetApp().frame.graphics_canvas.Refresh()
            
                    
