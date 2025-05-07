#pragma once

#include<string>
#include<SDL.h>
#include<SDL_ttf.h>
#include<SDL_image.h>
#include<SDL_mixer.h>
#include<fstream>
#include<sstream>
#include<thread>
#include<chrono>
#include <winsock2.h>
#include <ws2tcpip.h>

#include "manager.h"
#include "config_manager.h"
#include "enemy_manager.h"
#include "wave_manager.h"
#include "resources_manager.h"
#include "tower_manager.h"
#include "bullet_manager.h"
#include "coin_manager.h"
#include "status_bar.h"
#include "panel.h"
#include "place_panel.h"
#include "upgrade_panel.h"
#include "player_manager.h"
#include "banner.h"
#include "client_manager.h"
#include "client_core.h"

#include "httplib.h"

class GameManager : public Manager<GameManager>
{
	friend class Manager<GameManager>;

public:
	int run(int argc, char** argv)
	{
		//����bgm
		Mix_FadeInMusic(ResourcesManager::instance()->get_music_pool().find(ResID::Music_BGM)->second, -1, 1500);
		Mix_VolumeMusic(6);

		//��ȡ�߾��ȶ�ʱ���ĵ�ǰ����
		Uint64 last_counter = SDL_GetPerformanceCounter();
		//��ȡ�߽��ȶ�ʱ��Ƶ��(ÿ���Ӹ߾��ȶ�ʱ���Ĵ���);
		const Uint64 counter_freq = SDL_GetPerformanceFrequency();

		static ClientManager* instance = ClientManager::instance();

		while (!is_quit)
		{
			//�������е��߼�֡
			instance->simulate_frame();

			//SDL_PollEvent�����е�event�����������棬��eventһ��һ��ȡ����
			//�������е�ǰ֡������
			while (SDL_PollEvent(&event))
				on_input();

			Uint64 current_counuer = SDL_GetPerformanceCounter();
			//delta Ϊ���˶�����
			double delta = (double)(current_counuer - last_counter) / counter_freq;
			last_counter = current_counuer;
			//�ӳٺ�����msΪ��λ��    1000.0/60 Ϊһ��60֡�ʹ�����ms    һ֡���16ms,delta���16ms
			if (delta * 1000 < 1000.0 / 60)
				SDL_Delay((Uint32)(1000.0 / 60 - delta * 1000));

			//������
			on_update(delta);

			//������Ⱦ
			//�����renderer����պ��ĳ����ɫ
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			//��Ϸ����
			SDL_RenderClear(renderer);

			on_render();

			//��������GPU��Ⱦ�Ķ���������Ļ��CPU���� 
			//˫������� ʹ�������������ǰ�󻺳��������Ѿ����õĻ������뵱ǰ���������� ���ķ�ʽ�൱���л���ַ����
			SDL_RenderPresent(renderer);
		}
		return 0;
	}

public:               
	void link_to_server()
	{
		//�������̰�ť�Ƿ���԰���
		if (ClientCore::instance()->get_id_player() == 0)
		{
			place_panel->set_is_can_click(true);
			upgrade_panel->set_is_can_click(true);
		}

		
		//��Ϸ��ʼ
		ClientManager::instance()->start_game();
		//ClientManager::instance()->place_tower_cmd(Archer, { 9,10 });

#pragma region ɾ��
		//const int& id_player = ClientManager::instance()->get_id_player();
		//const SOCKET& sock = ClientManager::instance()->get_sock();
		////������Ϸ���߳�
		//std::thread([id_player, sock]()
		//	{
		//		int last_num_tower = 0;
		//		static TowerManager* instance = TowerManager::instance();
		//		while (true)
		//		{
		//			if (id_player == 1)
		//			{
		//				cJSON* request = cJSON_CreateObject();
		//				cJSON_AddStringToObject(request, "type", "get_dragon_postition");

		//				char* request_str = cJSON_Print(request);
		//				send(sock, request_str, strlen(request_str), 0);

		//				free(request_str);
		//				cJSON_Delete(request);

		//				char buffer[1024];
		//				int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
		//				if (bytes_received > 0)
		//				{
		//					buffer[bytes_received] = '\0';
		//					cJSON* root = cJSON_Parse(buffer);
		//					if (root)
		//					{

		//					}
		//				}

		//			}
		//			else if (id_player == 2)
		//			{
		//				cJSON* request = cJSON_CreateObject();
		//				cJSON_AddStringToObject(request, "type", "get_towers");

		//				//��������
		//				char* request_str = cJSON_Print(request);
		//				send(sock, request_str, strlen(request_str), 0);

		//				free(request_str);
		//				cJSON_Delete(request);

		//				char buffer[4096];
		//				int bytes_received = recv(sock, buffer, sizeof(buffer), 0);

		//				//����ɹ����յ�����
		//				if (bytes_received > 0)
		//				{
		//					buffer[bytes_received] = '\0';
		//					cJSON* root = cJSON_Parse(buffer);
		//					if (root)
		//					{
		//						cJSON* data = cJSON_GetObjectItem(root, "data");
		//						//������ݲ�Ϊ��
		//						if (data)
		//						{
		//							int tower_count = cJSON_GetArraySize(data);
		//							int now_count = 0;
		//							cJSON* now_tower;
		//							cJSON_ArrayForEach(now_tower, data)
		//							{
		//								//û�з��ù��ķ��������з���
		//								if (now_count >= last_num_tower)
		//								{
		//									int x = cJSON_GetObjectItem(now_tower, "x")->valueint;
		//									int y = cJSON_GetObjectItem(now_tower, "y")->valueint;
		//									int level = cJSON_GetObjectItem(now_tower, "level")->valueint;
		//									std::string type = (cJSON_GetObjectItem(now_tower, "type_tower")->valuestring);
		//									SDL_Point idx = { x,y };
		//									if (type == "Archer")
		//									{
		//										instance->place_tower(TowerType::Archer, idx, level);
		//									}
		//									else if (type == "Axeman")
		//									{
		//										instance->place_tower(TowerType::Axeman, idx, level);
		//									}
		//									else
		//									{
		//										instance->place_tower(TowerType::Gunner, idx, level);
		//									}
		//								}
		//								else
		//								{
		//									//���ù��Ľ��еȼ�����
		//									int level = cJSON_GetObjectItem(now_tower, "level")->valueint;
		//									Tower*& tower = instance->get_tower_list()[now_count];
		//									if (level != tower->get_level())
		//									{
		//										instance->upgrade_tower(tower);
		//									}
		//								}
		//								now_count++;
		//							}
		//							last_num_tower = tower_count;
		//						}
		//						cJSON_Delete(root);
		//					}
		//				}
		//			}
		//			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		//		}
		//	}).detach();
#pragma endregion

		
	}

	GameManager()
	{
		//SDL_INIT_EVERYTHING:��ʼ��sdl������ģ��
		init_assert(!SDL_Init(SDL_INIT_EVERYTHING), u8"SDL2 ��ʼ��ʧ�ܣ�");
		init_assert(IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG), u8"SDL_iamge ��ʼ��ʧ�ܣ� ");
		init_assert(Mix_Init(MIX_INIT_MP3), u8"SDL_mixer ��ʼ��ʧ�ܣ� ");
		init_assert(!TTF_Init(), u8"SDL_ttf ��ʼ��ʧ�ܣ�");

		//��Ƶ��ʼ��
		Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

		//�����ĺ�ѡ���б�
		SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

		ConfigManager* config = ConfigManager::instance();

		//�� ��ͼ �ؿ� ��Ϸ  ���õļ���
		init_assert(config->map.load("map.csv"), u8"������Ϸ��ͼʧ�ܣ�");
		init_assert(config->load_level_config("level.json"), u8"���عؿ�����ʧ�ܣ�");
		init_assert(config->load_game_config("config.json"), u8"������Ϸ����ʧ�ܣ�");

		//������Ϸ���� ���е�sdl_creat���캯��������malloc�����ڴ棬����һ��Ҫ�ͷ��ڴ�
		window = SDL_CreateWindow(config->basic_template.window_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			config->basic_template.window_width, config->basic_template.window_height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
		init_assert(window, u8"������Ϸ����ʧ�ܣ�");

		//������Ⱦ��
		//SDL_RENDERER_ACCELERATED Ӳ������  ����GPU��Ⱦ
		//SDL_RENDERER_PRESENTVSYNC ��ֱͬ�� ��ֹ����˺��
		//SDL_RENDERER_TARGETTEXTURE ���Ƶ�һ�������� ������������ ������Ƭ��ͼ
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
		init_assert(renderer, u8"������Ⱦ��ʧ�ܣ�");

		//����Ϸ��Դ�ļ���
		init_assert(ResourcesManager::instance()->load_from_file(renderer), u8"������Ϸ��Դʧ�ܣ�");

		init_assert(generate_tile_map_texture(), u8"������Ƭ��ͼ����ʧ�ܣ�");

		//����ui����λ��
		status_bar.set_position(15, 15);

		//��ʼ������panel
		place_panel = new PlacePanel();
		upgrade_panel = new UpgradePanel();
		banner = new Banner();

		//��ʼ��������Ч��С ��������Ϊ 50
		ResourcesManager::SoundPool sound_pool = ResourcesManager::instance()->get_sound_pool();
		for (auto& pair : sound_pool)
		{
			Mix_VolumeChunk(pair.second, 5); 
		}

		//�������������
		srand(ClientManager::instance()->get_random_seed());

		link_to_server();
		//std::cout << 1 << std::endl;
	}

	~GameManager()
	{
		//�������³�ʼ�� ��Ҫ���������ͷ��ڴ�
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);

		TTF_Quit();
		Mix_Quit();
		IMG_Quit();
		SDL_Quit();
	}

public:
	

private:
	SDL_Event event;
	bool is_quit = false;

	StatusBar status_bar;

	SDL_Window* window = nullptr;
	//�൱һ������ ��ͼ��
	SDL_Renderer* renderer = nullptr;

	//����һ���µĻ��� ����ͼ����������ȫ��������������ϣ��ٷŵ���Ϸ������
	SDL_Texture* tex_tile_map = nullptr;

	//������������
	Panel* place_panel = nullptr;
	Panel* upgrade_panel = nullptr;
	Banner* banner = nullptr;                                    
	
private:
	void init_assert(bool flag, const char* err_msg)
	{
		if (flag) return;

		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, u8"��Ϸ����ʧ��", err_msg, window);
		exit(-1);
	}



	void on_input()
	{
		//��Ԫ������
		static SDL_Point idx_tile_selected;
		static ConfigManager* instance = ConfigManager::instance();
		static ResourcesManager::SoundPool sound_pool = ResourcesManager::instance()->get_sound_pool();
		const int& id_player = ClientCore::instance()->get_id_player();

		//��ǰ��������ĸ�������
		Tower* tower = nullptr;
		static SDL_Point pos_center;

		switch (event.type)
		{
		case SDL_QUIT:
			is_quit = true;
			break;
		case SDL_MOUSEBUTTONDOWN:
			if (instance->is_game_over)
				break;
			if (get_cursor_idx_tile(idx_tile_selected, event.motion.x, event.motion.y))
			{
				get_selected_tile_center_pos(pos_center, idx_tile_selected);
				//�������˷��������������� ����򿪷������
				if (check_tower(pos_center,tower))
				{
					upgrade_panel->set_idx_tile(idx_tile_selected);
					upgrade_panel->set_center_pos(pos_center);
					upgrade_panel->set_target_tower(tower);
					upgrade_panel->show();
					//������ͷŴ�panel��Ч
					Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_TowerOpenMenu)->second, 0);
				}
				else if (can_place_tower(idx_tile_selected))
				{
					place_panel->set_idx_tile(idx_tile_selected);
					place_panel->set_center_pos(pos_center);
					place_panel->show();
					//������ͷŴ�panel��Ч
					Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_TowerOpenMenu)->second, 0);
				}
				
			}
			break;
		default:
			break;
		}

		if (!instance->is_game_over)
		{
			place_panel->on_input(event);
			upgrade_panel->on_input(event);
			//�������̰�ť�Ƿ���԰���
			if (ClientCore::instance()->get_id_player() == 1)
			{
				PlayerManager::instance()->on_input(event);
			}
			
		}
	}

	void on_update(double delta)
	{
		//��һ֡�Ƿ����
		static bool is_game_over_last_tick = false;
		static ConfigManager* instance = ConfigManager::instance();
		static HomeManager* home_instance = HomeManager::instance();

		if (home_instance->get_current_hp_num() <= 0)
		{
			instance->is_game_over = true;
		}

		if (!instance->is_game_over)
		{
			status_bar.on_update(renderer);
			place_panel->on_update(renderer);
			upgrade_panel->on_update(renderer);
			WaveManager::instance()->on_update(delta);
			EnemyManager::instance()->on_update(delta);
			TowerManager::instance()->on_update(delta);
			BulletManager::instance()->on_update(delta);
			CoinManager::instance()->on_update(delta);
			PlayerManager::instance()->on_update(delta);

			return;
		}

		//�����Ϸ������
		if (!is_game_over_last_tick && instance->is_game_over)
		{
			static const ResourcesManager::SoundPool& sound_pool
				= ResourcesManager::instance()->get_sound_pool();

			//��Ϸbgm����Ч��
			Mix_FadeOutMusic(1500);
			Mix_PlayChannel(-1, sound_pool.find(instance->is_game_win ? ResID::Sound_Win : ResID::Sound_Loss)->second, 0);
		}

		is_game_over_last_tick = instance->is_game_over;

		banner->on_update(delta);
		if (banner->check_end_dispaly())
			is_quit = true;
	}

	void on_render()
	{
		static ConfigManager* instance = ConfigManager::instance();
		static SDL_Rect& rect_dst = instance->rect_tile_map;
		SDL_RenderCopy(renderer, tex_tile_map, nullptr, &rect_dst);

		EnemyManager::instance()->on_renderer(renderer);
		TowerManager::instance()->on_render(renderer);
		BulletManager::instance()->on_render(renderer);
		CoinManager::instance()->on_render(renderer);
		PlayerManager::instance()->on_render(renderer);
		WaveManager::instance()->on_render(renderer);

		if (!instance->is_game_over)
		{
			place_panel->on_render(renderer);
			upgrade_panel->on_render(renderer);
			status_bar.on_render(renderer);

			return;
		}

		int width_screen, height_screen;
		SDL_GetWindowSizeInPixels(window, &width_screen, &height_screen);
		banner->set_center_position({ (double)width_screen / 2, (double)height_screen / 2 });
		banner->on_render(renderer);
	}

	bool generate_tile_map_texture()
	{
		const Map& map = ConfigManager::instance()->map;
		const TileMap& tile_map = map.get_tile_map();
		SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;
		SDL_Texture* tex_tile_set = ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_Tileset)->second; //ͼƬ�ز�

		//�ز�ͼƬ�Ŀ��
		int width_tex_tile_set, height_tex_tile_set;
		//���ͼƬ�Ŀ��
		SDL_QueryTexture(tex_tile_set, nullptr, nullptr, &width_tex_tile_set, &height_tex_tile_set);
		//�õ��ز�ͼƬһ���ж��ٸ���Ƭ
		int num_tile_single_line = (int)std::ceil((double)width_tex_tile_set / SIZE_TILE);

		//��ͼ�Ŀ�� 
		int width_tex_tile_map, height_tex_tile_map;
		width_tex_tile_map = (int)map.get_width() * SIZE_TILE;
		height_tex_tile_map = (int)map.get_height() * SIZE_TILE;
		//������ʼ�� SDL_PIXELFORMAT_ABGR8888������aRPG������ÿ������8������λ   
		//SDL_TEXTUREACCESS_TARGET���������������Ϊһ���������������滭��
		tex_tile_map = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
			SDL_TEXTUREACCESS_TARGET, width_tex_tile_map, height_tex_tile_map);
		if (!tex_tile_map) return false;

		//��ֵ����λ�þ���
		ConfigManager* config = ConfigManager::instance();
		rect_tile_map.x = (config->basic_template.window_width - width_tex_tile_map) / 2;
		rect_tile_map.y = (config->basic_template.window_height - height_tex_tile_map) / 2;
		rect_tile_map.w = width_tex_tile_map;
		rect_tile_map.h = height_tex_tile_map;

		//���û��͸��ģʽ
		SDL_SetTextureBlendMode(tex_tile_map, SDL_BLENDMODE_BLEND);
		//������ȾĿ��
		SDL_SetRenderTarget(renderer, tex_tile_map);
		//std::cout << map.get_width() << std::endl;
		for (int y = 0; y < map.get_height(); y++)
		{
			for (int x = 0; x < map.get_width(); x++)
			{
				//�ü�����Ŀ��
				SDL_Rect rec_src;
				const Tile& tile = tile_map[y][x];

				//������Ŀ������
				const SDL_Rect& rect_dst =
				{
					x * SIZE_TILE,y * SIZE_TILE,
					SIZE_TILE,SIZE_TILE
				};

				rec_src =
				{
					(tile.terrian % num_tile_single_line) * SIZE_TILE,
					(tile.terrian / num_tile_single_line) * SIZE_TILE,
					SIZE_TILE,SIZE_TILE
				};
				SDL_RenderCopy(renderer, tex_tile_set, &rec_src, &rect_dst);

				if (tile.decoration >= 0)
				{
					//std::cout << tile.decoration << std::endl;
					rec_src =
					{
						(tile.decoration % num_tile_single_line) * SIZE_TILE,
						(tile.decoration / num_tile_single_line) * SIZE_TILE,
						SIZE_TILE,SIZE_TILE
					};
					SDL_RenderCopy(renderer, tex_tile_set, &rec_src, &rect_dst);
				}
			}
		}

		const SDL_Point& idx_home = map.get_idx_home();
		const SDL_Rect rect_dst =
		{
			idx_home.x * SIZE_TILE,idx_home.y * SIZE_TILE,
			SIZE_TILE,SIZE_TILE
		};
		SDL_RenderCopy(renderer, ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_Home)->second, nullptr, &rect_dst);

		//����ȾĿ�����û���Ϸ����
		SDL_SetRenderTarget(renderer, nullptr);

		return true;
	}

	//����Ƿ����˷�����
	bool check_tower(const SDL_Point& center_pos,Tower* &target_tower)
	{
		static Map& map = ConfigManager::instance()->map;
		const TowerManager::TowerList& tower_list = TowerManager::instance()->get_tower_list();

		for (Tower* tower : tower_list)
		{
			Vector2 pos_tower = tower->get_position();
			if (center_pos.x == pos_tower.x && center_pos.y == pos_tower.y)
			{
				target_tower = tower;
				return true;
			}
		}
		return false;
	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//��������������ʵ����һ������ֵ ע��& �޸���ֵ��

	//����������������� Ҫ����������ת��Ϊ��Ԫ��λ��
	bool get_cursor_idx_tile(SDL_Point& idx_tile_selected, int sceen_x, int sceen_y) const
	{
		static const Map& map = ConfigManager::instance()->map;
		static SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;

		//����㵽�˵�ͼ����
		if (sceen_x < rect_tile_map.x || sceen_x > rect_tile_map.x + rect_tile_map.w
			|| sceen_y < rect_tile_map.y || sceen_y > rect_tile_map.y + rect_tile_map.h)
			return false;

		//��õ�ͼ�ĵ�Ԫ�� Ҫ���Ǳ߽�����
		idx_tile_selected.x = (std::min)((sceen_x - rect_tile_map.x) / SIZE_TILE, (int)map.get_width() - 1);
		idx_tile_selected.y = (std::min)((sceen_y - rect_tile_map.y) / SIZE_TILE, (int)map.get_height() - 1);

		return true;
	}

	//�����Ԫ���Ƿ��ܹ�����
	bool can_place_tower(const SDL_Point& idx_tile_selected) const
	{
		static const Map& map = ConfigManager::instance()->map;
		//ע����һ�������Ͻ�Ϊԭ�� ����Ϊx�������� ����Ϊy�������������ϵ
		//���Ŀ�굥Ԫ�����Ƭ
		const Tile& tile = map.get_tile_map()[idx_tile_selected.y][idx_tile_selected.x];

		//����ں��󷵻�false
		if (tile.terrian == 1 || tile.terrian == 8 || tile.terrian == 9 || tile.terrian == 10 || tile.terrian == 11)
			return false;

		//�˵ط�������װ���� �������н�·�� �����Ѿ����ù�������
		return (tile.decoration < 0 && tile.direction == Tile::Direction::None && !tile.has_tower &&
			!((idx_tile_selected.x == map.get_idx_home().x) && (idx_tile_selected.y == map.get_idx_home().y)));
	}

	//��÷�����������λ��
	void get_selected_tile_center_pos(SDL_Point& pos, const SDL_Point& idx_tile_map) const
	{
		static SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;

		pos.x = rect_tile_map.x + idx_tile_map.x * SIZE_TILE + SIZE_TILE / 2;
		pos.y = rect_tile_map.y + idx_tile_map.y * SIZE_TILE + SIZE_TILE / 2;
	}
};

