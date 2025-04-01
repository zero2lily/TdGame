#pragma once


#include<vector>

//һ����Ԫ�����48���ش�С����ԴҲ��48���ص�
#define SIZE_TILE 48

struct Tile
{
	//������ û�� �� �� �� ��
	enum class Direction
	{
		None = 0,
		Up,
		Down,
		Left,
		Right
	};

	int terrian = 0;                           //���ε����
	int decoration = -1;                       //װ���� -1Ϊû��װ����
	int special_flag = -1;                     //������ -1Ϊû��������
	bool has_tower = false;                    //����Ƭ�Ƿ��з�����
	Direction direction = Direction::None;     //����Ƭ���н�·�ߵķ���
};

typedef std::vector<std::vector<Tile>> TileMap;
