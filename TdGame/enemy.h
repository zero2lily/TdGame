#pragma once

#include "timer.h"
#include "animation.h"
#include "vector2.h"
#include "route.h"
#include "config_manager.h"
#include "resources_manager.h"

class Enemy
{
public:
	typedef std::function<void(Enemy* enemy)> SkillCallback;

public:
	Enemy()
	{
		const static ResourcesManager::TexturePool& tex_pool =
			ResourcesManager::instance()->get_texture_pool();

		//技能计时器 每隔一段时间触发技能
		timer_skill.set_one_shot(false);
		timer_skill.set_on_timeout([&]() { on_skill_released(this); });

		//剪影闪烁为单次触发 触发后设置为不闪烁状态
		timer_sketch.set_one_shot(true);
		timer_sketch.set_wait_time(0.075f);
		timer_sketch.set_on_timeout([&]() { is_show_sketch = false; });

		//回复原来的速度
		timer_restore_speed.set_one_shot(true);
		timer_restore_speed.set_on_timeout([&]() { speed = max_speed; });
	}

	~Enemy() = default;

	void on_update(double delta)
	{
		timer_skill.on_update(delta);
		timer_sketch.on_update(delta);
		timer_restore_speed.on_update(delta);

		Vector2 move_distance = velocity * delta;
		Vector2 target_distance = position_target - position;
		//如果这一帧的用速度的距离大于当前距离和目标点的距离 则取后者
		position += move_distance < target_distance ? move_distance : target_distance;

		if (target_distance.approx_zero())
		{
			idx_target++;
			refresh_position_target();

			//获得前进方向范围向量
			direction = (position_target - position).normalize();
		}

		//每秒移动多少个像素块
		velocity.x = direction.x * speed * SIZE_TILE;
		velocity.y = direction.y * speed * SIZE_TILE;

		//由于精度原因 可能速度接近于0 所以这样判断方向
		bool is_show_x_amin = abs(velocity.x) >= abs(velocity.y);

		if (is_show_sketch)
		{
			if (is_show_x_amin)
				anim_current = velocity.x > 0 ? &anim_right_sketch : &anim_left_sketch;
			else
				anim_current = velocity.y > 0 ? &anim_down_sketch : &anim_up_sketch;
		}
		else
		{
			if (is_show_x_amin)
				anim_current = velocity.x > 0 ? &anim_right : &anim_left;
			else
				anim_current = velocity.y > 0 ? &anim_down : &anim_up;
		}
		//更新敌人动画
		anim_current->on_update(delta);
	}

	void on_render(SDL_Renderer* renderer)
	{
		static SDL_Rect rect;                                          //血条要绘制的位置
		static SDL_Point point;                                        //血条的左上角位置 ？？？
		static const int offset_y = 2;                                 //y方向的小偏差
		static const Vector2 size_hp_bar = { 40,8 };                   //血条的宽高
		static const SDL_Color color_border = { 116,185,124,255 };     //血条边框的颜色
		static const SDL_Color color_content = { 226,255,194,255 };    //血条填充物的颜色

		//画动画时需要左上角的位置  而寻路的时候需要动画中心的位置
		point.x = (int)(position.x - size.x / 2);
		point.y = (int)(position.y - size.y / 2);
		
		//渲染敌人动画
		anim_current->on_render(renderer, point);

		//绘制血条 只有在血条不满的时候才绘制
		if (hp <= max_hp)
		{
			rect.x = (int)(position.x - size_hp_bar.x / 2);
			rect.y = (int)(position.y - size.y / 2 - size_hp_bar.y - offset_y);
			rect.w = (int)(size_hp_bar.x * (hp / max_hp));
			rect.h = (int)size_hp_bar.y;
			//画填充物
			SDL_SetRenderDrawColor(renderer, color_border.r, color_border.g, color_border.b, color_border.a);
			SDL_RenderFillRect(renderer, &rect);

			rect.w = (int)size_hp_bar.x;
			//画边框
			SDL_SetRenderDrawColor(renderer, color_content.r, color_content.g, color_content.b, color_content.a);
			SDL_RenderDrawRect(renderer, &rect);
		}
	}

	//设置敌人回调函数
	void set_on_skill_released(SkillCallback on_skill_released)
	{
		this->on_skill_released = on_skill_released;
	}

	//增加血量
	void increase_hp(double val)
	{
		hp += val;

		if (hp >= max_hp)
			hp = max_hp;
	}

	//减少血量
	void decrease_hp(double val)
	{
		hp -= val;

		if (hp <= 0)
		{
			hp = 0;
			//播放死亡特效
			is_valid = false;
		}
		//受伤触发 受伤剪影
		is_show_sketch = true;
		timer_sketch.restart();
	}

	//敌人移速降低
	void slow_down()
	{
		speed = max_speed - 0.5;
		//一秒后回复原来的速度
		timer_restore_speed.set_wait_time(1);
		timer_restore_speed.restart();
	}

	//设置敌人位置
	void set_position(const Vector2& position)
	{
		this->position = position;
	}

	//设置此敌人路线
	void set_route(const Route* route)
	{
		this->route = route;
		//刷新目标点
		refresh_position_target();
	}

	//外部设置为无效 比如碰到房屋后敌人被删除
	void make_invalid()
	{
		is_valid = false;
	}

	//从外部获得血量
	double get_hp() const
	{
		return hp;
	}

	//外部获得图片大小 碰撞检查 需要获得敌人的大小
	const Vector2& get_size() const
	{
		return size;
	}

	//外部获得敌人位置
	const Vector2& get_position() const
	{
		return position;
	}

	//外部获得敌人地图速度
	const Vector2& get_velocity() const
	{
		return velocity;
	}

	//外部获得对房屋的伤害值
	double get_damage() const
	{
		return damage;
	}

	//外部获得爆进步的概率
	double get_reward_ratio() const
	{
		return reward_ratio;
	}

	//外部获得回复半径 从单元格范围转化为世界范围
	double get_recover_radius() const
	{
		return SIZE_TILE * recover_range;
	}

	//外部获得回复强度
	double get_recover_intensity() const
	{
		return recover_intensity;
	}

	//是否可以删除
	bool can_remove() const
	{
		return !is_valid;
	}

	//是否已经死亡
	bool can_dead() const
	{
		return !is_dead;
	}

	//获得走到多少的路径程度
	double get_route_process() const
	{
		if (route->get_idx_list().size() == 1)
			return 1;

		return (double)idx_target / (route->get_idx_list().size() - 1);
	}
protected:
	//敌人大小
	Vector2 size;

	//技能冷却计时器
	Timer timer_skill;

	Animation anim_up;
	Animation anim_down;
	Animation anim_left;
	Animation anim_right;
	//敌人受伤剪影
	Animation anim_up_sketch;
	Animation anim_down_sketch;
	Animation anim_left_sketch;
	Animation anim_right_sketch;
	//死亡特效
	Animation anim_dead;

	double hp = 0;       
	double max_hp = 0;
	double speed = 0;
	double max_speed = 0;
	double damage = 0;                 //碰触到房屋 扣除房屋的生命值
	double reward_ratio = 0;           //爆金币的概率
	double recover_interval = 0;       //回复持续时间
	double recover_range = 0;          //回复的半径
	double recover_intensity = 0;      //回复的强度

private:
	Vector2 position;                     //动画中心点的地图位置 不是左上角
	Vector2 velocity;
	Vector2 direction;
	
	bool is_valid = true;                 //敌人是否可以删除
	bool is_dead = false;                 //敌人是否已经死亡

	Timer timer_sketch;                   //敌人剪影计时器
	bool is_show_sketch = false;          //是否处于受伤状态

	Animation* anim_current = nullptr;   //目前的敌人动画

	SkillCallback on_skill_released;      //技能释放完的回调函数

	Timer timer_restore_speed;            //被减速重新回复速度计时器

	const Route* route = nullptr;         //敌人行进路线
	int idx_target = 0;                   //目标路径下标序号
	Vector2 position_target;              //目标位置(是中心点的位置 不是左上角)

private:
	//获得下一个目标点的坐标
	void refresh_position_target()
	{
		const Route::IdxList& idx_list = route->get_idx_list();

		if (idx_target < idx_list.size())
		{
			const SDL_Point& point = idx_list[idx_target];
			static const SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;

			//将地图的坐标转到像素坐标
			position_target.x = rect_tile_map.x + point.x * SIZE_TILE + SIZE_TILE / 2;
			position_target.y = rect_tile_map.y + point.y * SIZE_TILE + SIZE_TILE / 2;
		}
	}
};

