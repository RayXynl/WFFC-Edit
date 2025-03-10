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

	bool middleMouseDown;
	bool mouse_LB_Down;
	int mouse_X;
	int mouse_Y;

	bool ctrlDown;
};
