
#pragma once


class MouseEvent
{
public:
	int m_event_type;
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
};
