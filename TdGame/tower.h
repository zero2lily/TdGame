#pragma once

#include "vector2.h"
#include "animation.h"
#include "tower_type.h"
#include "bullet_manager.h"
#include "facing.h"
#include "config_manager.h"
#include "resources_manager.h"
#include "enemy_manager.h"

class Tower
{
public:
	Tower()
	{
		//计时器到时间就设置为可以开火
		timer_fire.set_one_shot(true);
		//timer_fire.set_wait_time(fire_speed);
		timer_fire.set_on_timeout(
			[&]()
			{
				can_fire = true;
			});

		anim_idle_up.set_loop(true);
		anim_idle_up.set_interval(0.2);
		anim_idle_down.set_loop(true);
		anim_idle_down.set_interval(0.2);
		anim_idle_left.set_loop(true);
		anim_idle_left.set_interval(0.2);
		anim_idle_right.set_loop(true);
		anim_idle_right.set_interval(0.2);

		//开完火立马进入待机状态并更改防御塔的方向 开火时不能修改防御塔的方向 
		anim_fire_up.set_loop(false);
		anim_fire_up.set_interval(0.2);
		anim_fire_up.set_on_finished(
			[&]()
			{
				//调用更改待机方向的函数
				update_idle_animation();
			});

		anim_fire_down.set_loop(false);
		anim_fire_down.set_interval(0.2);
		anim_fire_down.set_on_finished(
			[&]()
			{
				//调用更改待机方向的函数
				update_idle_animation();
			});

		anim_fire_left.set_loop(false);
		anim_fire_left.set_interval(0.2);
		anim_fire_left.set_on_finished(
			[&]()
			{
				//调用更改待机方向的函数
				update_idle_animation();
			});

		anim_fire_right.set_loop(false);
		anim_fire_right.set_interval(0.2);
		anim_fire_right.set_on_finished(
			[&]()
			{
				//调用更改待机方向的函数
				update_idle_animation();
			});


	}
	~Tower() = default;

	void set_position(const Vector2& position)
	{
		this->position = position;
	}

	const Vector2& get_size() const
	{
		return size;
	}

	const Vector2& get_position() const
	{
		//???????
		return position;
	}

	int get_level()
	{
		return level;
	}

	void set_level(int level)
	{
		this->level = level;
	}

	TowerType get_tower_type()
	{
		return tower_type;
	}

	void set_id(int id)
	{
		this->id = id;
	}

	const int get_id() const
	{
		return id;
	}

	void on_update(double delta)
	{
		timer_fire.on_update(delta);
		anim_current->on_update(delta);

		if (can_fire)
			on_fire();
	}

	void on_render(SDL_Renderer* renderer)
	{
		static SDL_Point point;

		point.x = (int)(position.x - size.x / 2);
		point.y = (int)(position.y - size.y / 2);

		anim_current->on_render(renderer, point);
	}

protected:
	Vector2 size;
	int level = 0;      //防御塔当前的等级
	int id = 0;         //每个防御塔的唯一id

	//防御塔待机动画和开火动画
	Animation anim_idle_up;
	Animation anim_idle_down;
	Animation anim_idle_left;
	Animation anim_idle_right;
	Animation anim_fire_up;
	Animation anim_fire_down;
	Animation anim_fire_left;
	Animation anim_fire_right;
	//防御塔类型
	TowerType tower_type = TowerType::Archer;

	double fire_speed = 0;
	BulletType bullet_type = BulletType::Arrow;

private:
	Timer timer_fire;                              //开火计时器
	Vector2 position;                              //防御塔的位置
	bool can_fire = true;                          //是否可以开火
	Facing  facing = Facing::Right;                //防御塔的朝向
	Animation* anim_current = &anim_idle_right;    //当前动画

private:
	void update_idle_animation()
	{
		switch (facing)
		{
		case Left:
			anim_current = &anim_idle_left;
			break;
		case Right:
			anim_current = &anim_idle_right;
			break;
		case Up:
			anim_current = &anim_idle_up;
			break;
		case Down:
			anim_current = &anim_idle_down;
			break;
		}
	}

	void update_fire_animation()
	{
		switch (facing)
		{
		case Left:
			anim_current = &anim_fire_left;
			break;
		case Right:
			anim_current = &anim_fire_right;
			break;
		case Up:
			anim_current = &anim_fire_up;
			break;
		case Down:
			anim_current = &anim_fire_down;
			break;
		}
	}

	Enemy* find_target_enemy()
	{
		double process = -1;
		double view_range = 0;
		Enemy* target_enemy = nullptr;

		static ConfigManager* instance = ConfigManager::instance();

		//拿到configmanager的塔的攻击范围
		switch (tower_type)
		{
		case Archer:
			view_range = instance->archer_template.view_range[level];
			break;
		case Axeman:
			view_range = instance->axeman_template.view_range[level];
			break;
		case Gunner:
			view_range = instance->gunner_template.view_range[level];
			break;
		}

		EnemyManager::EnemyList& enemy_list = EnemyManager::instance()->get_enemy_list();

		//根据每个敌人的路径进程的比列来选择最需要攻击的敌人
		for (Enemy* enemy : enemy_list)
		{
			if ((enemy->get_position() - position).length() <= view_range * SIZE_TILE)
			{
				double new_procsss = enemy->get_route_process();
				if (process < new_procsss)
				{
					target_enemy = enemy;
					process = new_procsss;
				}
			}
		}

		return target_enemy;
	}

	void on_fire()
	{
		Enemy* target_enemy = find_target_enemy();

		if (!target_enemy)
			return;

		can_fire = false;
		static ConfigManager* instance = ConfigManager::instance();
		static const ResourcesManager::SoundPool& sound_pool = ResourcesManager::instance()->get_sound_pool();

		double interval = 0, damage = 0;
		switch (tower_type)
		{
		case Archer:
			interval = instance->archer_template.interval[level];
			damage = instance->archer_template.damage[level];
			switch (rand() % 2)
			{
			case 0:
				Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ArrowFire_1)->second, 0);
				break;
			case 1:
				Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ArrowFire_2)->second, 0);
				break;
			}
			break;
		case Axeman:
			interval = instance->axeman_template.interval[level];
			damage = instance->axeman_template.damage[level];
			Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_AxeFire)->second, 0);
			break;
		case Gunner:
			interval = instance->gunner_template.interval[level];
			damage = instance->gunner_template.damage[level];
			Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_ShellFire)->second, 0);
			break;
		}
		//动态设置开火计时器的时间间隔
		timer_fire.set_wait_time(interval);
		timer_fire.restart();

		//创建一个子弹
		Vector2 direction = target_enemy->get_position() - position;
		BulletManager::instance()->fire_bullet(bullet_type, position, fire_speed, damage, target_enemy);

		//防御塔应该朝向的方向
		bool is_show_x_anim = abs(direction.x) >= abs(direction.y);
		if (is_show_x_anim)
			facing = direction.x > 0 ? Facing::Right : Facing::Left;
		else
			facing = direction.y > 0 ? Facing::Down : Facing::Up;

		update_fire_animation();
		anim_current->reset();
	}
};

