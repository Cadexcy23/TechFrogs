#pragma once
#ifndef Artist
#include "Artist.h"
#endif
#ifndef vector
#include <vector>
#endif
#ifndef Button
#include "Button.h"
#endif

class Controller {
public:
	static bool quit;
	//Keeps track of mouse Pos
	static int mouseX, mouseY;
	static bool mouseMoved;
	static Artist::pos lastMousePos;
	static Uint32 SDLMouseStates;
	static std::vector<int> keyboardStates;
	static std::vector<int> mouseStates;
	static bool FPSLock;
	static Artist::pos mouseWheelMovment;
	static Uint32 state;
	static enum state
	{
		STATE_TITLE,
		STATE_MAP,
		STATE_COMBAT
	};
	static std::vector<Button::button> buttonList;

	bool loadController();
	//Checks for inputs from the user
	void controller();
};
