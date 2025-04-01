#pragma once

#include "tile.h"
#include "route.h"

#include<SDL.h>
#include<string>
#include<fstream>
#include<sstream>
#include<unordered_map>
#include<iostream>

class Map
{
public:
	//路径池
	typedef std::unordered_map<int, Route> SpawnerRouterPool;

public:
	Map() = default;
	~Map() = default;

	bool load(const std::string& path)
	{
		std::ifstream file(path);
		if (!file.good()) return false;

		//临时存储瓦片信息
		TileMap tile_map_temp;

		int idx_x = -1, idx_y = -1;

		//读取每一行
		std::string str_line;
		while (std::getline(file, str_line))
		{
			//对这一行去除前后空格 一般是空行情况
			str_line = trim_str(str_line);
			if (str_line.empty())
				continue;

			idx_x = -1, idx_y++;
			tile_map_temp.emplace_back();

			std::string str_tile;
			std::stringstream str_stream(str_line);
			while (std::getline(str_stream, str_tile, ','))
			{
				idx_x++;
				tile_map_temp[idx_y].emplace_back();
				Tile& tile = tile_map_temp[idx_y].back();
				load_tile_from_string(tile, str_tile);
			}
		}

		file.close();

		if (tile_map_temp.empty() || tile_map_temp[0].empty())
			return false;

		tile_map = tile_map_temp;

		generate_map_cache();

		return true;
	}

	size_t get_width() const
	{
		if (tile_map.empty())
			return 0;

		return tile_map[0].size();
	}

	size_t get_height() const
	{
		return tile_map.size();
	}

	const TileMap& get_tile_map() const
	{
		return tile_map;
	}

	const SDL_Point& get_idx_home() const
	{
		return idx_home;
	}

	const SpawnerRouterPool& get_idx_spawner_pool() const
	{
		return spwaner_route_pool;
	}

	void place_tower(const SDL_Point& idx_tile)
	{
		tile_map[idx_tile.y][idx_tile.x].has_tower = true;
	}

private:
	TileMap tile_map;                       //存储瓦片信息
	SDL_Point idx_home = { 0 };             //房子位置
	SpawnerRouterPool spwaner_route_pool;   //所有路径

private:
	//去掉前后的空格
	//"   1/1/1/1    "  ->  "1/1/1/1"
	std::string trim_str(const std::string& str)
	{
		size_t begin_idx = str.find_first_not_of(" \t");
		if (begin_idx == std::string::npos)
			return "";
		size_t end_idx = str.find_last_not_of(" \t");
		size_t idx_range = end_idx - begin_idx + 1;
		
		return str.substr(begin_idx, idx_range);
	}

	//加载每个单元的数据
	void load_tile_from_string(Tile& tile, const std::string& str)
	{
		//去掉每个单元格的前后空格
		std::string str_tidy = trim_str(str);

		std::string str_value;
		std::vector<int> values;
		std::stringstream str_stream(str_tidy);

		//分割'\'
		while (std::getline(str_stream, str_value, '\\'))
		{
			int value;
			try
			{
				//从字符转化为数字
				value = std::stoi(str_value);
			}
			catch (const std::invalid_argument&)
			{
				value = -1;
			}
			values.push_back(value);
		}
		tile.terrian = (values.size() < 1 || values[0] < 0) ? 0 : values[0];
		tile.decoration = (values.size() < 2) ? -1 : values[1];
		tile.direction = (Tile::Direction)((values.size() < 3 || values[2] < 0) ? 0 : values[2]);
		tile.special_flag = (values.size() <= 3) ? -1 : values[3];
	}

	//地图缓存
	void generate_map_cache()
	{
		//找到房屋位置
		for (int y = 0; y < get_height(); y++)
		{
			for (int x = 0; x < get_width(); x++)
			{
				const Tile& tile = tile_map[y][x];
				if (tile.special_flag < 0)
					continue;

				if (tile.special_flag == 0)
				{
					//房屋点
					idx_home.x = x;
					idx_home.y = y;
				}
				else
				{
					//刷怪点更新路径
					spwaner_route_pool[tile.special_flag] = Route(tile_map, { x,y });
				}
			}
		}
	}
};

