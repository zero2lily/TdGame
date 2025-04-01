#pragma once

#include "manager.h"
#include "config_manager.h"
#include "resources_manager.h"

class HomeManager : public Manager<HomeManager>
{
	friend class Manager<HomeManager>;

public:
	double get_current_hp_num()
	{
		return num_hp;
	}

	void decrease_hp(double val)
	{
		num_hp -= val;
		
		if (num_hp < 0)
			num_hp = 0;

		static const ResourcesManager::SoundPool sound_pool
			= ResourcesManager::instance()->get_sound_pool();

		//��Ϲܵ� ����������Ч
		Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_HomeHurt)->second, 0);
	}

protected:
	HomeManager()
	{
		num_hp = ConfigManager::instance()->num_initial_hp;
	}
	~HomeManager() = default;

private:
	//����Ѫ��
	double num_hp = 0;

};

