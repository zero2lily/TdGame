#pragma once

#include "enemy.h"
#include "manager.h"
#include "config_manager.h"
#include "home_manager.h"
#include "slim_enemy.h"
#include "king_slim_enemy.h"
#include "skeleton_enemy.h"
#include "goblin_enemy.h"
#include "goblin_priest_enemy.h"
#include "coin_manager.h"
#include "bullet_manager.h"

#include <vector>
#include <SDL.h>

class EnemyManager : public Manager<EnemyManager>
{
	friend class Manager<EnemyManager>;

public:
	//�����е��˵ĵ�ַ�ŵ��б�����
	typedef std::vector<Enemy*> EnemyList;

public:
	void on_update(double delta)
	{
		//ÿ�����˽��и���
		for (Enemy* enemy : enemy_list)
			enemy->on_update(delta);

		process_home_collsion();
		process_bullet_collision();

		remove_invalid_enemy();
	}

	void on_renderer(SDL_Renderer* renderer)
	{
		for (Enemy* enemy : enemy_list)
			enemy->on_render(renderer);
	}

	void spawn_enemy(EnemyType type, int idx_spawn_point)
	{

		static Vector2 position;
		static const SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;
		//·����
		static const Map::SpawnerRouterPool& spawner_route_pool
			= ConfigManager::instance()->map.get_idx_spawner_pool();

		const auto& itor = spawner_route_pool.find(idx_spawn_point);
		if (itor == spawner_route_pool.end())
			return;

		//����һ��enemy����
		Enemy* enemy = nullptr;

		switch (type)
		{
		case EnemyType::Slim:
			enemy = new SlimEnemy();
			break;
		case EnemyType::KingSlim:
			enemy = new KingSlimeEnemy();
			break;
		case EnemyType::Skeleton:
			enemy = new SkeletonEnemy();
			break;
		case EnemyType::Goblin:
			enemy = new GoblinEnemy();
			break;
		case EnemyType::GoblinPriest:
			enemy = new GoblinPriestEnemy();
			break;
		default:
			enemy = new SlimEnemy();
			break;
		}

		//���ü��ܻص�����
		enemy->set_on_skill_released(
			[&](Enemy* enemy_src)
			{
				double recover_radius = enemy_src->get_recover_radius();
				if (recover_radius < 0) return;

				const Vector2 pos_src = enemy_src->get_position();
				//�������е����Ƿ��ڷ�Χ��
				for (Enemy* enemy_dst : enemy_list)
				{
					const Vector2& pos_dst = enemy_dst->get_position();
					double distance = (pos_dst - pos_src).length();
					if (distance <= recover_radius)
						enemy_dst->increase_hp(enemy_src->get_recover_intensity());
				}
			});

		const Route::IdxList& idx_list = itor->second.get_idx_list();

		position.x = rect_tile_map.x + idx_list[0].x * SIZE_TILE + SIZE_TILE / 2;
		position.y = rect_tile_map.y + idx_list[0].y * SIZE_TILE + SIZE_TILE / 2;

		enemy->set_position(position);
		enemy->set_route(&itor->second);

		enemy_list.push_back(enemy);
	}

	bool check_cleared()
	{
		return enemy_list.empty();
	}

	EnemyManager::EnemyList& get_enemy_list()
	{
		return enemy_list;
	}
	

protected:
	EnemyManager() = default;

	~EnemyManager()
	{
		//�ͷ����е���ָ��
		for (Enemy* enemy : enemy_list)
			delete enemy;
	}

private:
	//�洢���е��˵�vector
	EnemyList enemy_list;

private:
	//���˺ͷ��ݵ���ײ���
	void process_home_collsion()
	{
		static const SDL_Point& idx_home = ConfigManager::instance()->map.get_idx_home();
		static const SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;
		//�����ڵ�ͼ�ϵ����Ͻǵ�����
		static const Vector2 position_home_tile =
		{
			(double)rect_tile_map.x + idx_home.x * SIZE_TILE,
			(double)rect_tile_map.y + idx_home.y * SIZE_TILE
		};
		
		for (Enemy* enemy : enemy_list)
		{
			if (enemy->can_remove()) continue;

			const Vector2& position = enemy->get_position();
			
			if (position.x >= position_home_tile.x
				&& position.y >= position_home_tile.y
				&& position.x <= position_home_tile.x + SIZE_TILE
				&& position.y <= position_home_tile.y + SIZE_TILE)
			{
				enemy->make_invalid();
				//�������˼�Ѫ
				HomeManager::instance()->decrease_hp(enemy->get_damage());
			}
		}
	}

	//�ӵ��͵��˵���ײ
	void process_bullet_collision()
	{
		static BulletManager::BulletList& bullet_list
			= BulletManager::instance()->get_bullet_list();

		for (Enemy* enemy : enemy_list)
		{
			if (enemy->can_remove())
				continue;

			const Vector2& size_enemy = enemy->get_size();
			const Vector2& pos_enemy = enemy->get_position();

			for (Bullet* bullet : bullet_list)
			{
				//���˿���ͬʱ��ײ�����ӵ� ����ײ��һ���ӵ����˿����Ѿ������������Ͳ��ڼ��˵����������ӵ���ײ
				if (!bullet->can_collide() || enemy->can_remove())
					continue;

				const Vector2& pos_bullet = bullet->get_position();

				//���ӵ������ĵ�ȥ��ײ���˵ľ�������
				if (pos_bullet.x >= pos_enemy.x - size_enemy.x / 2
					&& pos_bullet.y >= pos_enemy.y - size_enemy.y / 2
					&& pos_bullet.x <= pos_enemy.x + size_enemy.x / 2
					&& pos_bullet.y <= pos_enemy.y + size_enemy.y / 2)
				{
					double damage = bullet->get_damage();
					double damage_range = bullet->get_damage_range();
					if (damage_range < 0)
					{
						enemy->decrease_hp(damage);
						if (enemy->can_remove())
						{
							try_spawn_coin_prop(pos_enemy, enemy->get_reward_ratio());
						}
					}
					else
					{
						for (Enemy* target_enemy : enemy_list)
						{
							if (enemy->can_remove())
								continue;

							const Vector2& pos_target_enemy = target_enemy->get_position();
							if ((pos_target_enemy - pos_bullet).length() <= damage_range)
							{
								target_enemy->decrease_hp(damage);
								if (target_enemy->can_remove())
									try_spawn_coin_prop(pos_target_enemy, target_enemy->get_reward_ratio());
							}
						}
					}
					bullet->on_collide(enemy);
				}
			}
		}

	}

	//�Ƴ�Ҫ�����ĵ���
	void remove_invalid_enemy()
	{
		enemy_list.erase(std::remove_if(enemy_list.begin(), enemy_list.end(),
			[](const Enemy* enemy)
			{
				bool deletable = enemy->can_remove();
				//�ͷ�ָ��
				if (deletable) delete enemy;
				return deletable;
			}), enemy_list.end());
	}

	//�������ɽ��
	void try_spawn_coin_prop(const Vector2& position, double ratio)
	{
		static CoinManager* instance = CoinManager::instance();

		if ((double)(rand() % 100 / 100) <= ratio)
		{
			instance->spawn_coin_prop(position);
		}
	}
};

