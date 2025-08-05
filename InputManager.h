#pragma once
class InputManager
{
public:
	static InputManager* getInstance();
	static void initialize();
	static void destroy();


private:
	InputManager();
	~InputManager() = default;
	InputManager(InputManager const&) {}; //copy operator
	InputManager& operator=(InputManager const&) {}; //assignment operator
	static InputManager* sharedInstance;


};

