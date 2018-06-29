
#pragma once


class MouseEvent
{
public:
	int m_event_type;
	// 1 LeftDown
	// 2 LeftUp
	// 3 LeftDClick
	// 4 RightDown
	// 5 RightUp
	// 6 MiddleDown
	// 7 MiddleUp
	// 8 Moving or Dragging
	// 9 WheelRotation

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
	bool LeftDown(){ return m_event_type == 1; }
	bool LeftUp(){ return m_event_type == 2; }
	bool LeftDClick(){ return m_event_type == 3; }
	bool RightDown(){ return m_event_type == 4; }
	bool RightUp(){ return m_event_type == 5; }
	bool MiddleDown(){ return m_event_type == 6; }
	bool MiddleUp(){ return m_event_type == 7; }
	bool Moving(){ return m_event_type == 8; }
	bool GetWheelRotation(){ return m_event_type == 9; }
};
