#pragma once


#include<vector>

//一个单元格就是48像素大小，资源也是48像素的
#define SIZE_TILE 48

struct Tile
{
	//方向类 没有 上 下 左 右
	enum class Direction
	{
		None = 0,
		Up,
		Down,
		Left,
		Right
	};

	int terrian = 0;                           //地形的序号
	int decoration = -1;                       //装饰物 -1为没有装饰物
	int special_flag = -1;                     //特殊标记 -1为没有特殊标记
	bool has_tower = false;                    //该瓦片是否有防御塔
	Direction direction = Direction::None;     //该瓦片的行进路线的方向
};

typedef std::vector<std::vector<Tile>> TileMap;
