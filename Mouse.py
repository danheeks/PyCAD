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
    
    print(e.m_event_type)

    return e
