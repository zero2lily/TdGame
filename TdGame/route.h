#pragma once

#include"tile.h"

#include<SDL.h>
#include<vector>

class Route
{
public:
	typedef std::vector<SDL_Point> IdxList;

public:
	Route() = default;

	Route(const TileMap& map, const SDL_Point& idx_origin)
	{
		size_t width_map = map[0].size();
		size_t height_map = map.size();
		SDL_Point idx_next = idx_origin;

		while (true)
		{
			//路径越界
			if (idx_next.x >= width_map || idx_next.y >= height_map)
				break;

			//检查路径重复
			if (check_duplicate_idx(idx_next))
				break;
			else
				idx_list.push_back(idx_next);

			//下一个单元格有没有路标
			bool is_next_dir_exist = true;
			const Tile& tile = map[idx_next.y][idx_next.x];

			//是房屋则退出
			if (tile.special_flag == 0)
				break;
			switch (tile.direction)
			{
			case Tile::Direction::Up:
				idx_next.y--;
				break;
			case Tile::Direction::Down:
				idx_next.y++;
				break;
			case Tile::Direction::Left:
				idx_next.x--;
				break;
			case Tile::Direction::Right:
				idx_next.x++;
				break;
			default:
				is_next_dir_exist = false;
				break;
			}

			if (!is_next_dir_exist)
				break;
		}
	}

	~Route() = default;

	const IdxList& get_idx_list() const
	{
		return idx_list;
	}

private:
	IdxList idx_list;           //存储已经找到的路径点

private:
	//检查有没有重复路径点
	bool check_duplicate_idx(const SDL_Point& target_idx)
	{
		for (const SDL_Point& idx : idx_list)
		{
			if (idx.x == target_idx.x && idx.y == target_idx.y)
				return true;
		}

		return false;
	}
};
