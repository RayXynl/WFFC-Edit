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

	bool g_key_down;

	int editMode;

};
enum EditMode
{
	CameraMove = 0,
	ModelMove = 1,
	ModelRotate = 2,
	ModelScale = 3
};