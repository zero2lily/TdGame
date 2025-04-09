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
		//播放bgm
		Mix_FadeInMusic(ResourcesManager::instance()->get_music_pool().find(ResID::Music_BGM)->second, -1, 1500);
		Mix_VolumeMusic(6);

		//获取高精度定时器的当前次数
		Uint64 last_counter = SDL_GetPerformanceCounter();
		//获取高进度定时器频率(每秒钟高精度定时器的次数);
		const Uint64 counter_freq = SDL_GetPerformanceFrequency();

		static ClientManager* instance = ClientManager::instance();

		while (!is_quit)
		{
			//处理所有的逻辑帧
			instance->simulate_frame();

			//SDL_PollEvent：所有的event都放在这里面，将event一个一个取出来
			//发送所有当前帧的命令
			while (SDL_PollEvent(&event))
				on_input();

			Uint64 current_counuer = SDL_GetPerformanceCounter();
			//delta 为过了多少秒
			double delta = (double)(current_counuer - last_counter) / counter_freq;
			last_counter = current_counuer;
			//延迟函数是ms为单位的    1000.0/60 为一个60帧率过多少ms    一帧大概16ms,delta大概16ms
			if (delta * 1000 < 1000.0 / 60)
				SDL_Delay((Uint32)(1000.0 / 60 - delta * 1000));

			//检查更新
			on_update(delta);

			//更新渲染
			//给你的renderer画笔蘸上某种颜色
			SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
			//游戏清屏
			SDL_RenderClear(renderer);

			on_render();

			//将所有在GPU渲染的东西放在屏幕（CPU）上 
			//双缓冲机制 使用这个函数交换前后缓冲区，将已经画好的缓冲区与当前缓冲区交换 换的方式相当于切换地址就行
			SDL_RenderPresent(renderer);
		}
		return 0;
	}

public:               
	void link_to_server()
	{
		//设置轮盘按钮是否可以按下
		if (ClientCore::instance()->get_id_player() == 0)
		{
			place_panel->set_is_can_click(true);
			upgrade_panel->set_is_can_click(true);
		}

		
		//游戏开始
		ClientManager::instance()->start_game();
		//ClientManager::instance()->place_tower_cmd(Archer, { 9,10 });

#pragma region 删除
		//const int& id_player = ClientManager::instance()->get_id_player();
		//const SOCKET& sock = ClientManager::instance()->get_sock();
		////开启游戏主线程
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

		//				//请求数据
		//				char* request_str = cJSON_Print(request);
		//				send(sock, request_str, strlen(request_str), 0);

		//				free(request_str);
		//				cJSON_Delete(request);

		//				char buffer[4096];
		//				int bytes_received = recv(sock, buffer, sizeof(buffer), 0);

		//				//如果成功接收到数据
		//				if (bytes_received > 0)
		//				{
		//					buffer[bytes_received] = '\0';
		//					cJSON* root = cJSON_Parse(buffer);
		//					if (root)
		//					{
		//						cJSON* data = cJSON_GetObjectItem(root, "data");
		//						//如果数据不为空
		//						if (data)
		//						{
		//							int tower_count = cJSON_GetArraySize(data);
		//							int now_count = 0;
		//							cJSON* now_tower;
		//							cJSON_ArrayForEach(now_tower, data)
		//							{
		//								//没有放置过的防御塔进行放置
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
		//									//放置过的进行等级更新
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
		//SDL_INIT_EVERYTHING:初始化sdl的所有模块
		init_assert(!SDL_Init(SDL_INIT_EVERYTHING), u8"SDL2 初始化失败！");
		init_assert(IMG_Init(IMG_INIT_JPG | IMG_INIT_PNG), u8"SDL_iamge 初始化失败！ ");
		init_assert(Mix_Init(MIX_INIT_MP3), u8"SDL_mixer 初始化失败！ ");
		init_assert(!TTF_Init(), u8"SDL_ttf 初始化失败！");

		//音频初始化
		Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

		//打开中文侯选词列表
		SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

		ConfigManager* config = ConfigManager::instance();

		//对 地图 关卡 游戏  配置的加载
		init_assert(config->map.load("map.csv"), u8"加载游戏地图失败！");
		init_assert(config->load_level_config("level.json"), u8"加载关卡配置失败！");
		init_assert(config->load_game_config("config.json"), u8"加载游戏配置失败！");

		//创建游戏窗口 所有的sdl_creat创造函数都用了malloc分配内存，所有一定要释放内存
		window = SDL_CreateWindow(config->basic_template.window_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
			config->basic_template.window_width, config->basic_template.window_height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
		init_assert(window, u8"创建游戏窗口失败！");

		//创建渲染器
		//SDL_RENDERER_ACCELERATED 硬件加速  就是GPU渲染
		//SDL_RENDERER_PRESENTVSYNC 垂直同步 防止画面撕裂
		//SDL_RENDERER_TARGETTEXTURE 绘制到一个纹理上 在贴到窗口上 用于瓦片地图
		renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_TARGETTEXTURE);
		init_assert(renderer, u8"创建渲染器失败！");

		//对游戏资源的加载
		init_assert(ResourcesManager::instance()->load_from_file(renderer), u8"加载游戏资源失败！");

		init_assert(generate_tile_map_texture(), u8"加载瓦片地图纹理失败！");

		//设置ui条的位置
		status_bar.set_position(15, 15);

		//初始化两个panel
		place_panel = new PlacePanel();
		upgrade_panel = new UpgradePanel();
		banner = new Banner();

		//初始化所有音效大小 设置音量为 50
		ResourcesManager::SoundPool sound_pool = ResourcesManager::instance()->get_sound_pool();
		for (auto& pair : sound_pool)
		{
			Mix_VolumeChunk(pair.second, 5); 
		}

		//设置随机数种子
		srand(ClientManager::instance()->get_random_seed());

		link_to_server();
		//std::cout << 1 << std::endl;
	}

	~GameManager()
	{
		//从上往下初始化 就要从下往上释放内存
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
	//相当一个画笔 画图像
	SDL_Renderer* renderer = nullptr;

	//创建一个新的画布 将地图的所有内容全部画在这个画布上，再放到游戏窗口上
	SDL_Texture* tex_tile_map = nullptr;

	//两个放置轮盘
	Panel* place_panel = nullptr;
	Panel* upgrade_panel = nullptr;
	Banner* banner = nullptr;                                    
	
private:
	void init_assert(bool flag, const char* err_msg)
	{
		if (flag) return;

		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, u8"游戏启动失败", err_msg, window);
		exit(-1);
	}



	void on_input()
	{
		//单元格坐标
		static SDL_Point idx_tile_selected;
		static ConfigManager* instance = ConfigManager::instance();
		static ResourcesManager::SoundPool sound_pool = ResourcesManager::instance()->get_sound_pool();
		const int& id_player = ClientCore::instance()->get_id_player();

		//当前点击到了哪个防御塔
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
				//如果点击了防御塔则打开升级面板 否则打开放置面板
				if (check_tower(pos_center,tower))
				{
					upgrade_panel->set_idx_tile(idx_tile_selected);
					upgrade_panel->set_center_pos(pos_center);
					upgrade_panel->set_target_tower(tower);
					upgrade_panel->show();
					//鼠标点击释放打开panel特效
					Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_TowerOpenMenu)->second, 0);
				}
				else if (can_place_tower(idx_tile_selected))
				{
					place_panel->set_idx_tile(idx_tile_selected);
					place_panel->set_center_pos(pos_center);
					place_panel->show();
					//鼠标点击释放打开panel特效
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
			//设置轮盘按钮是否可以按下
			if (ClientCore::instance()->get_id_player() == 1)
			{
				PlayerManager::instance()->on_input(event);
			}
			
		}
	}

	void on_update(double delta)
	{
		//上一帧是否结束
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

		//如果游戏结束了
		if (!is_game_over_last_tick && instance->is_game_over)
		{
			static const ResourcesManager::SoundPool& sound_pool
				= ResourcesManager::instance()->get_sound_pool();

			//游戏bgm淡出效果
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
		SDL_Texture* tex_tile_set = ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_Tileset)->second; //图片素材

		//素材图片的宽高
		int width_tex_tile_set, height_tex_tile_set;
		//获得图片的宽高
		SDL_QueryTexture(tex_tile_set, nullptr, nullptr, &width_tex_tile_set, &height_tex_tile_set);
		//得到素材图片一行有多少个瓦片
		int num_tile_single_line = (int)std::ceil((double)width_tex_tile_set / SIZE_TILE);

		//地图的宽高 
		int width_tex_tile_map, height_tex_tile_map;
		width_tex_tile_map = (int)map.get_width() * SIZE_TILE;
		height_tex_tile_map = (int)map.get_height() * SIZE_TILE;
		//画布初始化 SDL_PIXELFORMAT_ABGR8888：按照aRPG排序并且每个都是8个比特位   
		//SDL_TEXTUREACCESS_TARGET：将此纹理可以作为一个画布可以在上面画画
		tex_tile_map = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
			SDL_TEXTUREACCESS_TARGET, width_tex_tile_map, height_tex_tile_map);
		if (!tex_tile_map) return false;

		//赋值中心位置矩阵
		ConfigManager* config = ConfigManager::instance();
		rect_tile_map.x = (config->basic_template.window_width - width_tex_tile_map) / 2;
		rect_tile_map.y = (config->basic_template.window_height - height_tex_tile_map) / 2;
		rect_tile_map.w = width_tex_tile_map;
		rect_tile_map.h = height_tex_tile_map;

		//设置混合透明模式
		SDL_SetTextureBlendMode(tex_tile_map, SDL_BLENDMODE_BLEND);
		//设置渲染目标
		SDL_SetRenderTarget(renderer, tex_tile_map);
		//std::cout << map.get_width() << std::endl;
		for (int y = 0; y < map.get_height(); y++)
		{
			for (int x = 0; x < map.get_width(); x++)
			{
				//裁剪区域目标
				SDL_Rect rec_src;
				const Tile& tile = tile_map[y][x];

				//画到的目标区域
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

		//将渲染目标设置回游戏窗口
		SDL_SetRenderTarget(renderer, nullptr);

		return true;
	}

	//检查是否点击了防御塔
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
	//下面三个函数其实都有一个返回值 注意& 修改了值的

	//鼠标点击的是像素坐标 要将像素坐标转化为单元格位置
	bool get_cursor_idx_tile(SDL_Point& idx_tile_selected, int sceen_x, int sceen_y) const
	{
		static const Map& map = ConfigManager::instance()->map;
		static SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;

		//如果点到了地图外面
		if (sceen_x < rect_tile_map.x || sceen_x > rect_tile_map.x + rect_tile_map.w
			|| sceen_y < rect_tile_map.y || sceen_y > rect_tile_map.y + rect_tile_map.h)
			return false;

		//获得地图的单元格 要考虑边界问题
		idx_tile_selected.x = (std::min)((sceen_x - rect_tile_map.x) / SIZE_TILE, (int)map.get_width() - 1);
		idx_tile_selected.y = (std::min)((sceen_y - rect_tile_map.y) / SIZE_TILE, (int)map.get_height() - 1);

		return true;
	}

	//这个单元格是否能够放置
	bool can_place_tower(const SDL_Point& idx_tile_selected) const
	{
		static const Map& map = ConfigManager::instance()->map;
		//注意是一个以左上角为原点 向右为x的正方向 向下为y的正方向的坐标系
		//获得目标单元格的瓦片
		const Tile& tile = map.get_tile_map()[idx_tile_selected.y][idx_tile_selected.x];

		//如果在海洋返回false
		if (tile.terrian == 1 || tile.terrian == 8 || tile.terrian == 9 || tile.terrian == 10 || tile.terrian == 11)
			return false;

		//此地方不能有装饰物 不能是行进路线 不能已经放置过防御塔
		return (tile.decoration < 0 && tile.direction == Tile::Direction::None && !tile.has_tower &&
			!((idx_tile_selected.x == map.get_idx_home().x) && (idx_tile_selected.y == map.get_idx_home().y)));
	}

	//获得防御塔的中心位置
	void get_selected_tile_center_pos(SDL_Point& pos, const SDL_Point& idx_tile_map) const
	{
		static SDL_Rect& rect_tile_map = ConfigManager::instance()->rect_tile_map;

		pos.x = rect_tile_map.x + idx_tile_map.x * SIZE_TILE + SIZE_TILE / 2;
		pos.y = rect_tile_map.y + idx_tile_map.y * SIZE_TILE + SIZE_TILE / 2;
	}
};

