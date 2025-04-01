#pragma once

#include "bullet.h"
#include "arrow_bullet.h"
#include "axe_bullet.h"
#include "shell_bullet.h"
#include "bullet_type.h"

#include <vector>

class BulletManager : public Manager<BulletManager>
{
	friend class Manager<BulletManager>;

public:
	typedef std::vector<Bullet*> BulletList;

public:
	void on_update(double delta)
	{
		for (Bullet* bullet : bullet_list)
			bullet->on_update(delta);

		//移除要清除的子弹
		bullet_list.erase(std::remove_if(
			bullet_list.begin(), bullet_list.end(),
			[](const Bullet* bullet)
			{
				bool deletable = bullet->can_remove();
				if (deletable) delete bullet;
				return deletable;
			}), bullet_list.end());
	}

	void on_render(SDL_Renderer* renderer)
	{
		for (Bullet* bullet : bullet_list)
			bullet->on_render(renderer);
	}

	//返回引用好处
	//1.避免拷贝开销    如果返回的是一个较大的对象（如数组、结构体或类实例），返回引用可以避免拷贝整个对象的开销。
	//2.允许修改返回值  返回引用后，调用者可以直接修改返回值，从而修改原始数据。
	BulletList& get_bullet_list()
	{
		return bullet_list;
	}

	//创建一个子弹函数
	void fire_bullet(BulletType type, const Vector2& position,double fire_speed, double damage, Enemy*& target_enemy)
	{
		Bullet* bullet = nullptr;

		switch (type)
		{
		case Arrow:
			bullet = new ArrowBullet();
			break;
		case Axe:
			bullet = new AxeBullet();
			break;
		case Shell:
			bullet = new ShellBullet();
			break;
		default:
			bullet = new ArrowBullet();
			break;
		}

		bullet->set_position(position);
		bullet->set_velocity((target_enemy->get_position() -  position).normalize() * fire_speed * SIZE_TILE);
		bullet->set_damage(damage);
		bullet->set_fire_speed(fire_speed);
		bullet->set_target_enemy(target_enemy);
		bullet_list.push_back(bullet);
	}


protected:
	BulletManager() = default;
	~BulletManager()
	{
		for (Bullet* bullet : bullet_list)
			delete bullet;
	}

private:
	BulletList bullet_list;

};