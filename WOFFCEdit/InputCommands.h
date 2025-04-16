#pragma once

struct InputCommands
{
	bool forward;
	bool back;
	bool right;
	bool left;
	bool up;
	bool down;
	bool rotRight;
	bool rotLeft;

	bool mouse_Mid_Down;
	bool mouse_LB_Down;
	int mouse_X;
	int mouse_Y;

	bool ctrlDown;

	bool tabDown;
	bool tabPrevState;

	bool undoDown;
	bool undoDownPrevState;

	bool redoDown;
	bool redoDownPrevState;

	bool deleteDown;
	

	bool g_key_down;
	bool z_key_down;
	bool y_key_down;

	bool c_key_down;
	bool c_key_prev;
	bool v_key_down;
	bool v_key_prev;

	bool f_key_down;
	bool focus_prev; 
	int editMode;

};
enum EditMode
{
	CameraMove = 0,
	ModelMove = 1,
	ModelRotate = 2,
	ModelScale = 3
};