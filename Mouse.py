import wx
import cad

def MouseEventFromWx(w):
    e = cad.MouseEvent()

    if w.LeftDown():
        e.m_event_type = 1
    elif w.LeftUp():
        e.m_event_type = 2
    elif w.LeftDClick():
        e.m_event_type = 3
    elif w.RightDown():
        e.m_event_type = 4
    elif w.RightUp():
        e.m_event_type = 5
    elif w.MiddleDown():
        e.m_event_type = 6
    elif w.MiddleUp():
        e.m_event_type = 7
    elif w.Dragging() or w.Moving():
        e.m_event_type = 8
    elif w.GetWheelRotation():
        e.m_event_type = 9

    e.m_x = w.GetX()
    e.m_y = w.GetY()
    e.m_leftDown = w.LeftIsDown() 
    e.m_middleDown = w.MiddleIsDown()
    e.m_rightDown = w.RightIsDown()
    e.m_controlDown = w.ControlDown()
    e.m_shiftDown = w.ShiftDown()
    e.m_altDown = w.AltDown()
    e.m_metaDown = w.MetaDown() 
    e.m_wheelRotation = w.WheelRotation
    e.m_wheelDelta = w.WheelDelta
    e.m_linesPerAction = w.LinesPerAction

    return e

def copy(e):
    e2 = cad.MouseEvent()
    e2.m_event_type = e.m_event_type
    e2.m_x = e.m_x
    e2.m_y = e.m_y
    e2.m_leftDown = e.m_leftDown
    e2.m_middleDown = e.m_middleDown
    e2.m_rightDown = e.m_rightDown
    e2.m_controlDown = e.m_controlDown
    e2.m_shiftDown = e.m_shiftDown
    e2.m_altDown = e.m_altDown
    e2.m_metaDown = e.m_metaDown
    e2.m_wheelRotation = e.m_wheelRotation
    e2.m_wheelDelta = e.m_wheelDelta
    e2.m_linesPerAction = e.m_linesPerAction
    return e2
