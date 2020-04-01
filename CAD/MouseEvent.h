
#pragma once

enum MouseEventType
{
	MouseEventNull,
	MouseEventLeftDown,
	MouseEventLeftUp,
	MouseEventLeftDClick,
	MouseEventRightDown,
	MouseEventRightUp,
	MouseEventMiddleDown,
	MouseEventMiddleUp,
	MouseEventMovingOrDragging,
	MouseEventWheelRotation,
};

class MouseEvent
{
public:
	MouseEventType m_event_type;

	int m_x, m_y;

	bool          m_leftDown;
	bool          m_middleDown;
	bool          m_rightDown;

	bool          m_controlDown;
	bool          m_shiftDown;
	bool          m_altDown;
	bool          m_metaDown;

	int           m_wheelRotation;
	int           m_wheelDelta;
	int           m_linesPerAction;

	int GetX(){ return m_x; }
	int GetY(){ return m_y; }
	bool LeftDown(){ return m_event_type == MouseEventLeftDown; }
	bool LeftUp(){ return m_event_type == MouseEventLeftUp; }
	bool LeftDClick(){ return m_event_type == MouseEventLeftDClick; }
	bool RightDown(){ return m_event_type == MouseEventRightDown; }
	bool RightUp(){ return m_event_type == MouseEventRightUp; }
	bool MiddleDown(){ return m_event_type == MouseEventMiddleDown; }
	bool MiddleUp(){ return m_event_type == MouseEventMiddleUp; }
	bool Moving(){ return m_event_type == MouseEventMovingOrDragging; }
	bool GetWheelRotation(){ return m_event_type == MouseEventWheelRotation; }
};
