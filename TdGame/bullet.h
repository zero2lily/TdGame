#pragma once

#include "animation.h"
#include "enemy.h"
#include "vector2.h"
#include "config_manager.h"

class Bullet
{
public:
	Bullet() = default;
	~Bullet() = default;

	void set_velocity(const Vector2& velocity)
	{
		this->velocity = velocity;

		if (can_rotated)
		{
			//������ת�Ƕ�
			double randian = std::atan2(velocity.y, velocity.x);
			angle_anim_rotated = randian * 180 / 3.14159265;
		}
	}

	void set_position(const Vector2& position)
	{
		this->position = position;
	}

	void set_damage(double damage)
	{
		this->damage = damage;
	}

	void set_target_enemy(Enemy*& target_enemy)
	{
		this->target_enemy = target_enemy;
	}
	
	void set_fire_speed(double val)
	{
		fire_speed = val;
	}

	const Vector2& get_size() const
	{
		return size;
	}

	const Vector2& get_position() const
	{
		return position;
	}

	double get_damage() const
	{
		return damage;
	}

	double get_damage_range() const
	{
		return damage_range;
	}

	void disable_collide()
	{
		is_collisional = false;
	}

	bool can_collide() const
	{
		return is_collisional;
	}

	//����Ǳ�ը�ӵ� ֻ�ڱ�ը��һ˲������˺� �ڲ��ű�ը�����������˺���Ҳ���ǲ�������ײ
	void make_invalid()
	{
		is_vaild = false;
		is_collisional = false;
	}
	
	bool can_remove() const 
	{
		return !is_vaild;
	}

	virtual void on_update(double delta)
	{
		if (target_enemy->get_hp() < 0)
			delete target_enemy;

		animation.on_update(delta);
		if (target_enemy != nullptr && !target_enemy->can_remove())
		{
			Vector2 direction = target_enemy->get_position() - position;
			set_velocity(direction.normalize() * fire_speed * SIZE_TILE);
		}
		position += velocity * delta;

		static const SDL_Rect& rect_map
			= ConfigManager::instance()->rect_tile_map;

		if (position.x - size.x / 2 <= rect_map.x
			|| position.x + size.x / 2 >= rect_map.x + rect_map.w
			|| position.y - size.y / 2 <= rect_map.y
			|| position.y + size.y / 2 >= rect_map.y + rect_map.h)
		{
			is_vaild = false;
		}
	}

	virtual void on_render(SDL_Renderer* renderer)
	{
		static SDL_Point point;

		point.x = (int)(position.x - size.x / 2);
		point.y = (int)(position.y - size.y / 2);

		animation.on_render(renderer, point, angle_anim_rotated);
	}

	virtual void on_collide(Enemy* enemy)
	{
		is_vaild = false;
		is_collisional = false;
	}

protected:
	Vector2 size;                        //�ӵ���С
	Vector2 velocity;                    //�ӵ��ٶ�
	Vector2 position;                    //λ�� ���Ͻ�

	Animation animation;                 //����
	bool can_rotated = false;            //�Ƿ���ת
	
	double damage = 0;                   //�˺�
	double damage_range = -1;            //�˺���Χ

private:
	bool is_vaild = true;                //�Ƿ���Ч
	bool is_collisional = true;          //�Ƿ������ײ
	double angle_anim_rotated = 0;       //ѡװ�Ƕ�
	Enemy* target_enemy = nullptr;       //�ӵ������Ŀ�����
	double fire_speed = 0;               //�������ӵ����ٶ�
};



