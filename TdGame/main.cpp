#define SDL_MAIN_HANDLED
//#pragma comment(linker, "/subsystem:windows /entry:mainCRTStartup")

#include "game_manager.h"

#include <iostream>

//sdl2�Ĵ����������Ͻ�Ϊԭ���
//RGBA�ķ�Χ��0-255

int main(int argc,char** argv)
{
	const int* a = nullptr;
	return GameManager::instance()->run(argc, argv);
}