#pragma once

#include "timer.h"
#include "manager.h"
#include "config_manager.h"
#include "enemy_manager.h"
#include "coin_manager.h"

#include <set>

class WaveManager : public Manager<WaveManager>
{
	friend class Manager<WaveManager>;

public:
	void on_update(double delta)
	{
		static ConfigManager* instance = ConfigManager::instance();
		if (instance->is_game_over)
			return;

		if (!is_wave_started)
			timer_start_wave.on_update(delta);
		else
			timer_spawn_enemy.on_update(delta);

		//��������һ�����˲��ҵ��˶�������
		if (is_spawned_last_enemy && EnemyManager::instance()->check_cleared())
		{
			//���ӽ��
			CoinManager::instance()->increase_coin(instance->wave_list[idx_wave].rawards);

			idx_wave++;

			//������һ����������
			if (idx_wave >= instance->wave_list.size())
			{
				instance->is_game_over = true;
				instance->is_game_win = true;
			}
			else
			{
				idx_spawn_event = 0;
				is_wave_started = false;
				is_spawned_last_enemy = false;

				const Wave& wave = instance->wave_list[idx_wave];
				timer_start_wave.set_wait_time(wave.interval);
				timer_start_wave.restart();
			}
		}
	}

	void on_render(SDL_Renderer* renderer)
	{
		static const ResourcesManager::TexturePool& tex_pool = ResourcesManager::instance()->get_texture_pool();
		static SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;
		static Map& map = ConfigManager::instance()->map;
		static SDL_Rect rect_dst;
		static SDL_Texture* tex_enemy_show = tex_pool.find(ResID::Tex_UIEnemyShow)->second;
		if (!is_wave_started)
		{
			
			for (auto& point : num_point)
			{
				const auto& itor = map.get_idx_spawner_pool().find(point);
				if (itor == map.get_idx_spawner_pool().end())
					return;
				
				const Route::IdxList& idx_list = itor->second.get_idx_list();
				
				rect_dst.x = rect_tile_map.x + idx_list[0].x * SIZE_TILE;
				rect_dst.y = rect_tile_map.y + idx_list[0].y * SIZE_TILE;
				rect_dst.w = 80 + sin(SDL_GetTicks64() / 1000.0 * 4) * 10;
				rect_dst.h = 80 + sin(SDL_GetTicks64() / 1000.0 * 4) * 10;
				if (idx_list[0].x == 0)
				{
					rect_dst.x += SIZE_TILE;
				}
				else if (idx_list[0].x >= map.get_width() - 1)
				{
					rect_dst.x -= SIZE_TILE;
				}
				if (idx_list[0].y == 0)
				{
					rect_dst.y += SIZE_TILE;
				}
				else if (idx_list[0].y == map.get_height() - 1)
				{
					rect_dst.x -= SIZE_TILE;
				}
				
				SDL_RenderCopy(renderer, tex_enemy_show, nullptr, &rect_dst);
			}
		}
	}

protected:
	WaveManager()
	{
		static const std::vector<Wave>& wave_list = ConfigManager::instance()->wave_list;
		for (auto& event : wave_list[0].spawn_event_list)
		{
			num_point.insert(event.spawn_point);
		}

		timer_start_wave.set_one_shot(true);
		//���κͲ���֮���ʱ����
		timer_start_wave.set_wait_time(wave_list[0].interval);
		timer_start_wave.set_on_timeout(
			[&]()
			{
				is_wave_started = true;
				//���õ��˳��ּ�ʱ��
				timer_spawn_enemy.set_wait_time(wave_list[idx_wave].spawn_event_list[0].interval);
				timer_spawn_enemy.restart();

				//�����һ���ĵ��˳����� �洢���˳��ֵ�
				if (idx_wave + 1 == wave_list.size())
				{
					return;
				}
				num_point.clear();
				for (auto& event : wave_list[idx_wave + 1].spawn_event_list)
				{
					num_point.insert(event.spawn_point);
				}
			}
		);
		//��ʱ������Ϊ���δ��� ÿ��ͨ�����ϵ�restart���ü�ʱ������������һ�����˳��ֵ�ʱ����
		timer_spawn_enemy.set_one_shot(true);
		timer_spawn_enemy.set_on_timeout(
			[&]()
			{
				//��õ�ǰ���εĽڵ�����е��˳�����Ϣ
				const std::vector<Wave::SpawnEvent>& spawn_event_list = wave_list[idx_wave].spawn_event_list;
				//��õ�ǰ������һ���ֵ���Ϣ
				const Wave::SpawnEvent spawn_event = spawn_event_list[idx_spawn_event];

				//����һ������
				EnemyManager::instance()->spawn_enemy(spawn_event.enemy_type, spawn_event.spawn_point);

				//���������Ѽ�
				idx_spawn_event++;

				if (idx_spawn_event >= spawn_event_list.size())
				{
					is_spawned_last_enemy = true;
					return;
				}
				//������һ���������ü�ʱ������Ϣ
				timer_spawn_enemy.set_wait_time(spawn_event_list[idx_spawn_event].interval);
				timer_spawn_enemy.restart();
			}
		);
	}
	~WaveManager() = default;


private:
	int idx_wave = 0;                      //��ǰ���ε�����
	int idx_spawn_event = 0;               //��ǰ���γ��ֵ�����
	Timer timer_start_wave;                //��ǰ�����Ƿ�ʼ��ʱ��
	Timer timer_spawn_enemy;               //���ε������ɼ�ʱ��
	bool is_wave_started = false;          //��ǰ�����Ƿ�ʼ
	bool is_spawned_last_enemy = false;    //��ǰ�����Ƿ�������һֱ����

	bool last_is_scale = false;            //��һ���Ƿ�Ŵ�ͼƬ
	int scale_size = 0;                    //�����ĳ̶�
	Timer timer_scale;                     //������ʱ��
	std::set<int> num_point;               //�洢��ǰ����Ҫ���ֵ��˵ĳ�����
};

