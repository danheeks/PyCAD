import wx
import cad

def MouseEventFromWx(w):
    e = cad.MouseEvent()

    if w.LeftDown():
        e.event_type = cad.MouseEventType.MouseEventLeftDown
    elif w.LeftUp():
        e.event_type = cad.MouseEventType.MouseEventLeftUp
    elif w.LeftDClick():
        e.event_type = cad.MouseEventType.MouseEventLeftDClick
    elif w.RightDown():
        e.event_type = cad.MouseEventType.MouseEventRightDown
    elif w.RightUp():
        e.event_type = cad.MouseEventType.MouseEventRightUp
    elif w.MiddleDown():
        e.event_type = cad.MouseEventType.MouseEventMiddleDown
    elif w.MiddleUp():
        e.event_type = cad.MouseEventType.MouseEventMiddleUp
    elif w.Dragging() or w.Moving():
        e.event_type = cad.MouseEventType.MouseEventMovingOrDragging
    elif w.GetWheelRotation():
        e.event_type = cad.MouseEventType.MouseEventWheelRotation

    e.x = w.GetX()
    e.y = w.GetY()
    e.leftDown = w.LeftIsDown() 
    e.middleDown = w.MiddleIsDown()
    e.rightDown = w.RightIsDown()
    e.controlDown = w.ControlDown()
    e.shiftDown = w.ShiftDown()
    e.altDown = w.AltDown()
    e.metaDown = w.MetaDown() 
    e.wheelRotation = w.WheelRotation
    e.wheelDelta = w.WheelDelta
    e.linesPerAction = w.LinesPerAction

    return e

def copy(e):
    e2 = cad.MouseEvent()
    e2.event_type = e.event_type
    e2.x = e.x
    e2.y = e.y
    e2.leftDown = e.leftDown
    e2.middleDown = e.middleDown
    e2.rightDown = e.rightDown
    e2.controlDown = e.controlDown
    e2.shiftDown = e.shiftDown
    e2.altDown = e.altDown
    e2.metaDown = e.metaDown
    e2.wheelRotation = e.wheelRotation
    e2.wheelDelta = e.wheelDelta
    e2.linesPerAction = e.linesPerAction
    return e2
