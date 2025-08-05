#include "KeyboardHandler.h"
#include <iostream>

void KeyboardHandler::OnKeyDown(char key)
{
	std::cout << key;
}

void KeyboardHandler::OnKeyUp(char key)
{
	std::cout << key;
}
