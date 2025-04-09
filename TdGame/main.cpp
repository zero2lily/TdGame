#define SDL_MAIN_HANDLED
//#pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup")

#include "game_manager.h"

#include <iostream>

//sdl2的窗口是以左上角为原点的
//RGBA的范围是0-255

int main(int argc,char** argv)
{
	const int* a = nullptr;
	return GameManager::instance()->run(argc, argv);
}