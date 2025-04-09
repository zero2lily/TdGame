#pragma once

#include "tower.h"
#include "manager.h"
#include "archer_tower.h"
#include "axeman_tower.h"
#include "gunner_tower.h"
#include "config_manager.h"
#include "resources_manager.h"
#include "httplib.h"


#include <vector>

class TowerManager : public Manager<TowerManager>
{
	friend class Manager<TowerManager>;

public :
	typedef std::vector<Tower*> TowerList;

public:
	void on_update(double delta)
	{
		for (Tower* tower : tower_list)
			tower->on_update(delta);
	}

	void on_render(SDL_Renderer* renderer)
	{
		for (Tower* tower : tower_list)
			tower->on_render(renderer);
	}

	//返回每个防御塔放置的费用
	double get_place_cost(TowerType type)
	{
		static	ConfigManager* instance = ConfigManager::instance();

		switch (type)
		{
		case Archer:
			return instance->archer_template.cost[0];
			break;
		case Axeman:
			return instance->axeman_template.cost[0];
			break;
		case Gunner:
			return instance->gunner_template.cost[0];
			break;
		}
		
		return 0;
	}

	//返回防御塔升级的费用 如果满级则返回-1否则返回费用
	double get_upgrade_cost(TowerType type, Tower*& target_tower)
	{
		if (!target_tower)
			return 0;

		static	ConfigManager* instance = ConfigManager::instance();
		int level = target_tower->get_level();

		switch (type)
		{
		case Archer:
			return level == 9 ? -1 :
				instance->archer_template.upgrade_cost[level];
			break;
		case Axeman:
			return level == 9 ? -1 :
				instance->axeman_template.upgrade_cost[level];
			break;
		case Gunner:
			return level == 9 ? -1 :
				instance->gunner_template.upgrade_cost[level];
			break;
		}
		return 0;
	}

	//返回攻击的范围
	double get_damage_range(TowerType type, Tower* &target_tower)
	{
		static	ConfigManager* instance = ConfigManager::instance();

		switch (type)
		{
		case Archer:
			return instance->archer_template.view_range[target_tower == nullptr ? 0 : target_tower->get_level()];
			break;
		case Axeman:
			return instance->axeman_template.view_range[target_tower == nullptr ? 0 : target_tower->get_level()];
			break;
		case Gunner:
			return instance->gunner_template.view_range[target_tower == nullptr ? 0 : target_tower->get_level()];
			break;
		}
		return 0;
	}

	//返回下一个等级的攻击范围
	double get_next_grade_damage_range(TowerType type, Tower*& target_tower)
	{
		if (!target_tower)
			return 0;

		static	ConfigManager* instance = ConfigManager::instance();
		int level = target_tower->get_level();

		switch (type)
		{
		case Archer:
			return level == 9 ? -1 :
				instance->archer_template.view_range[level + 1];
			break;
		case Axeman:
			return  level == 9 ? -1 :
				instance->axeman_template.view_range[level + 1];
			break;
		case Gunner:
			return  level == 9 ? -1 :
				instance->gunner_template.view_range[level + 1];
			break;
		}
		return 0;
	}

	int get_target_tower_level(Tower*& target_tower)
	{
		return target_tower->get_level();
	}
	
	//放置一个防御塔 线程默认不开启 防御塔的等级默认为0
	void place_tower(TowerType type, const SDL_Point& idx, int level = 0, bool is_start_thread = false)
	{
		Tower* tower = nullptr;

		switch (type)
		{
		case Archer:
			tower = new ArcherTower();
			break;
		case Axeman:
			tower = new AxemanTower();
			break;
		case Gunner:
			tower = new GunnerTower();
			break;
		default:
			tower = new ArcherTower();
			break;
		}

		//设置坐标
		static Vector2 position;
		static const SDL_Rect& rect = ConfigManager::instance()->rect_tile_map;

		position.x = rect.x + idx.x * SIZE_TILE + SIZE_TILE / 2;
		position.y = rect.y + idx.y * SIZE_TILE + SIZE_TILE / 2;
		tower->set_position(position);
		tower->set_id(give_tower_id);give_tower_id++;
		tower_list.push_back(tower);

		//此地方设置为已经有防御塔
		ConfigManager::instance()->map.place_tower(idx);

		//播放放置音效
		static const ResourcesManager::SoundPool& sound_pool
			= ResourcesManager::instance()->get_sound_pool();

		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_PlaceTower)->second, 0);
	}

	//升级防御塔 第一种方式
	void upgrade_tower(Tower*& target_tower, bool is_start_thread = false)
	{
		if (!target_tower)
			return;

		static	ConfigManager* instance = ConfigManager::instance();
		target_tower->set_level(target_tower->get_level() == 9 ? 9 : target_tower->get_level() + 1);

		//播放升级音效
		static const ResourcesManager::SoundPool& sound_pool
			= ResourcesManager::instance()->get_sound_pool();

		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_TowerLevelUp)->second, 0);

	}

	//升级防御塔 第二种方式
	void upgrade_tower(const SDL_Point& idx, bool is_start_thread = false)
	{
		static Vector2 position;
		static const SDL_Rect& rect = ConfigManager::instance()->rect_tile_map;
		position.x = rect.x + idx.x * SIZE_TILE + SIZE_TILE / 2;
		position.y = rect.y + idx.y * SIZE_TILE + SIZE_TILE / 2;

		//寻找塔并且更新等级
		for (auto tower : tower_list)
		{
			if (tower->get_position() == position)
			{
				tower->set_level(tower->get_level() == 9 ? 9 : tower->get_level() + 1);
				break;
			}
		}

		//播放升级音效
		static	ConfigManager* instance = ConfigManager::instance();
		static const ResourcesManager::SoundPool& sound_pool
			= ResourcesManager::instance()->get_sound_pool();

		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_TowerLevelUp)->second, 0);
	}

	TowerList& get_tower_list() 
	{
		return tower_list;
	}

protected:
	TowerManager() = default;
	~TowerManager()
	{
		for (Tower* tower : tower_list)
			delete tower;
	}


private:
	//所有的防御塔
	TowerList tower_list;
	int give_tower_id = 0;
};

