#pragma once

#include "vector2.h"
#include "timer.h"
#include "tile.h"
#include "resources_manager.h"
#include "animation.h"

#include <SDL.h>

class CoinProp
{
public:
	CoinProp()
	{
		static SDL_Texture* tex_coin = ResourcesManager::instance()
			->get_texture_pool().find(ResID::Tex_Coin)->second;

		timer_jump.set_one_shot(true);
		timer_jump.set_wait_time(interval_jump);
		timer_jump.set_on_timeout(
			[&]()
			{
				is_jumping = false;
			}
		);

		timer_disappear.set_one_shot(true);
		timer_disappear.set_wait_time(interval_disappear);
		timer_disappear.set_on_timeout(
			[&]()
			{
				is_valid = false;
			}
		);

		anim_idle.set_loop(true);
		anim_idle.set_interval(0.1);
		anim_idle.set_frame_data(tex_coin, 8, 1, { 0,1,2,3,4,5,6,7 });

		//随机设置向左或者向右跳跃
		velocity.x = (rand() % 2 ? 1 : -1) * 2 * SIZE_TILE;
		velocity.y = -3 * SIZE_TILE;
	}
	~CoinProp() = default;

	void set_position(const Vector2& position)
	{
		this->position = position;
	}

	const Vector2& get_position() const
	{
		return position;
	}

	const Vector2& get_size() const
	{
		return size;
	}

	void make_invalid()
	{
		is_valid = false;
	}

	bool can_remove()
	{
		return !is_valid;
	}

	void on_update(double delta)
	{
		timer_jump.on_update(delta);
		timer_disappear.on_update(delta);
		anim_idle.on_update(delta);

		if (is_jumping)
		{
			velocity.y += gravity * delta;
		}
		else
		{
			velocity.x = 0;
			//金币跳跃完在降落点上下浮动效果
			//SDL_GetTicks64():获得的是游戏从开始到现在的时间（ms单位）  velocity.y的范围:30~-30
			velocity.y = sin(SDL_GetTicks64() / 1000.0 * 4) * 30;
		}

		position += velocity * delta;
	}

	void on_renderer(SDL_Renderer* renderer)
	{
		static SDL_Point point;

		point.x = (int)(position.x - size.x / 2);
		point.y = (int)(position.y - size.y / 2);

		anim_idle.on_render(renderer, point);
		//SDL_RenderCopy(renderer, tex_coin, nullptr, &rect);
	}


private:
	Vector2 position;                        //位置
	Vector2 velocity;                        //移动速度 模拟重力

	Timer timer_jump;                        //金币弹跳计时器
	Timer timer_disappear;                   //金币消失计时器
	Animation anim_idle;                     //金币默认动画

	bool is_valid = true;                    //金币是否有效
	bool is_jumping = true;                  //金币是否在跳跃
	
	double gravity = 490;                    //重力
	double interval_jump = 0.75;   
	Vector2 size = { 32,32 };
	double interval_disappear = 10;
};