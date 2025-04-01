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

		//���ܼ�ʱ�� ÿ��һ��ʱ�䴥������
		timer_skill.set_one_shot(false);
		timer_skill.set_on_timeout([&]() { on_skill_released(this); });

		//��Ӱ��˸Ϊ���δ��� ����������Ϊ����˸״̬
		timer_sketch.set_one_shot(true);
		timer_sketch.set_wait_time(0.075f);
		timer_sketch.set_on_timeout([&]() { is_show_sketch = false; });

		//�ظ�ԭ�����ٶ�
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
		//�����һ֡�����ٶȵľ�����ڵ�ǰ�����Ŀ���ľ��� ��ȡ����
		position += move_distance < target_distance ? move_distance : target_distance;

		if (target_distance.approx_zero())
		{
			idx_target++;
			refresh_position_target();

			//���ǰ������Χ����
			direction = (position_target - position).normalize();
		}

		//ÿ���ƶ����ٸ����ؿ�
		velocity.x = direction.x * speed * SIZE_TILE;
		velocity.y = direction.y * speed * SIZE_TILE;

		//���ھ���ԭ�� �����ٶȽӽ���0 ���������жϷ���
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
		//���µ��˶���
		anim_current->on_update(delta);
	}

	void on_render(SDL_Renderer* renderer)
	{
		static SDL_Rect rect;                                          //Ѫ��Ҫ���Ƶ�λ��
		static SDL_Point point;                                        //Ѫ�������Ͻ�λ�� ������
		static const int offset_y = 2;                                 //y�����Сƫ��
		static const Vector2 size_hp_bar = { 40,8 };                   //Ѫ���Ŀ��
		static const SDL_Color color_border = { 116,185,124,255 };     //Ѫ���߿����ɫ
		static const SDL_Color color_content = { 226,255,194,255 };    //Ѫ����������ɫ

		//������ʱ��Ҫ���Ͻǵ�λ��  ��Ѱ·��ʱ����Ҫ�������ĵ�λ��
		point.x = (int)(position.x - size.x / 2);
		point.y = (int)(position.y - size.y / 2);
		
		//��Ⱦ���˶���
		anim_current->on_render(renderer, point);

		//����Ѫ�� ֻ����Ѫ��������ʱ��Ż���
		if (hp <= max_hp)
		{
			rect.x = (int)(position.x - size_hp_bar.x / 2);
			rect.y = (int)(position.y - size.y / 2 - size_hp_bar.y - offset_y);
			rect.w = (int)(size_hp_bar.x * (hp / max_hp));
			rect.h = (int)size_hp_bar.y;
			//�������
			SDL_SetRenderDrawColor(renderer, color_border.r, color_border.g, color_border.b, color_border.a);
			SDL_RenderFillRect(renderer, &rect);

			rect.w = (int)size_hp_bar.x;
			//���߿�
			SDL_SetRenderDrawColor(renderer, color_content.r, color_content.g, color_content.b, color_content.a);
			SDL_RenderDrawRect(renderer, &rect);
		}
	}

	//���õ��˻ص�����
	void set_on_skill_released(SkillCallback on_skill_released)
	{
		this->on_skill_released = on_skill_released;
	}

	//����Ѫ��
	void increase_hp(double val)
	{
		hp += val;

		if (hp >= max_hp)
			hp = max_hp;
	}

	//����Ѫ��
	void decrease_hp(double val)
	{
		hp -= val;

		if (hp <= 0)
		{
			hp = 0;
			//����������Ч
			is_valid = false;
		}
		//���˴��� ���˼�Ӱ
		is_show_sketch = true;
		timer_sketch.restart();
	}

	//�������ٽ���
	void slow_down()
	{
		speed = max_speed - 0.5;
		//һ���ظ�ԭ�����ٶ�
		timer_restore_speed.set_wait_time(1);
		timer_restore_speed.restart();
	}

	//���õ���λ��
	void set_position(const Vector2& position)
	{
		this->position = position;
	}

	//���ô˵���·��
	void set_route(const Route* route)
	{
		this->route = route;
		//ˢ��Ŀ���
		refresh_position_target();
	}

	//�ⲿ����Ϊ��Ч �����������ݺ���˱�ɾ��
	void make_invalid()
	{
		is_valid = false;
	}

	//���ⲿ���Ѫ��
	double get_hp() const
	{
		return hp;
	}

	//�ⲿ���ͼƬ��С ��ײ��� ��Ҫ��õ��˵Ĵ�С
	const Vector2& get_size() const
	{
		return size;
	}

	//�ⲿ��õ���λ��
	const Vector2& get_position() const
	{
		return position;
	}

	//�ⲿ��õ��˵�ͼ�ٶ�
	const Vector2& get_velocity() const
	{
		return velocity;
	}

	//�ⲿ��öԷ��ݵ��˺�ֵ
	double get_damage() const
	{
		return damage;
	}

	//�ⲿ��ñ������ĸ���
	double get_reward_ratio() const
	{
		return reward_ratio;
	}

	//�ⲿ��ûظ��뾶 �ӵ�Ԫ��Χת��Ϊ���緶Χ
	double get_recover_radius() const
	{
		return SIZE_TILE * recover_range;
	}

	//�ⲿ��ûظ�ǿ��
	double get_recover_intensity() const
	{
		return recover_intensity;
	}

	//�Ƿ����ɾ��
	bool can_remove() const
	{
		return !is_valid;
	}

	//�Ƿ��Ѿ�����
	bool can_dead() const
	{
		return !is_dead;
	}

	//����ߵ����ٵ�·���̶�
	double get_route_process() const
	{
		if (route->get_idx_list().size() == 1)
			return 1;

		return (double)idx_target / (route->get_idx_list().size() - 1);
	}
protected:
	//���˴�С
	Vector2 size;

	//������ȴ��ʱ��
	Timer timer_skill;

	Animation anim_up;
	Animation anim_down;
	Animation anim_left;
	Animation anim_right;
	//�������˼�Ӱ
	Animation anim_up_sketch;
	Animation anim_down_sketch;
	Animation anim_left_sketch;
	Animation anim_right_sketch;
	//������Ч
	Animation anim_dead;

	double hp = 0;       
	double max_hp = 0;
	double speed = 0;
	double max_speed = 0;
	double damage = 0;                 //���������� �۳����ݵ�����ֵ
	double reward_ratio = 0;           //����ҵĸ���
	double recover_interval = 0;       //�ظ�����ʱ��
	double recover_range = 0;          //�ظ��İ뾶
	double recover_intensity = 0;      //�ظ���ǿ��

private:
	Vector2 position;                     //�������ĵ�ĵ�ͼλ�� �������Ͻ�
	Vector2 velocity;
	Vector2 direction;
	
	bool is_valid = true;                 //�����Ƿ����ɾ��
	bool is_dead = false;                 //�����Ƿ��Ѿ�����

	Timer timer_sketch;                   //���˼�Ӱ��ʱ��
	bool is_show_sketch = false;          //�Ƿ�������״̬

	Animation* anim_current = nullptr;   //Ŀǰ�ĵ��˶���

	SkillCallback on_skill_released;      //�����ͷ���Ļص�����

	Timer timer_restore_speed;            //���������»ظ��ٶȼ�ʱ��

	const Route* route = nullptr;         //�����н�·��
	int idx_target = 0;                   //Ŀ��·���±����
	Vector2 position_target;              //Ŀ��λ��(�����ĵ��λ�� �������Ͻ�)

private:
	//�����һ��Ŀ��������
	void refresh_position_target()
	{
		const Route::IdxList& idx_list = route->get_idx_list();

		if (idx_target < idx_list.size())
		{
			const SDL_Point& point = idx_list[idx_target];
			static const SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;

			//����ͼ������ת����������
			position_target.x = rect_tile_map.x + point.x * SIZE_TILE + SIZE_TILE / 2;
			position_target.y = rect_tile_map.y + point.y * SIZE_TILE + SIZE_TILE / 2;
		}
	}
};

