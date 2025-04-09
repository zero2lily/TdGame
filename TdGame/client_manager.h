#pragma once

#include "manager.h"
#include "client_core.h"
#include "tower_manager.h"
#include "wave_manager.h"
#include "coin_manager.h"
#include "player_manager.h"

#include <string>

class WaveManager;

class ClientManager : public Manager<ClientManager>
{
	friend class Manager<ClientManager>;

public :
	enum class Stage
	{
		Waiting, //等待玩家加入
		Ready,   //开始倒计时
		Racing   //正在游戏中
	};

public:

	void connect_to_server()
	{
		//初始化
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);

		//打开文件
		std::ifstream file("config.cfg");
		//文件打开失败
		if (!file.good())
		{
			std::cerr << "config.cfg文件打开失败" << std::endl;
			exit(-1);
		}

		std::stringstream str_stream;
		str_stream << file.rdbuf();
		file.close();

		str_address = str_stream.str();

		//TCP传输协议
		sock = socket(AF_INET, SOCK_STREAM, 0);
		sockaddr_in server_addr;
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(25565);
		inet_pton(AF_INET, str_stream.str().c_str(), &server_addr.sin_addr);
		//建立连接
		connect(sock, (sockaddr*)&server_addr, sizeof(server_addr));
		// 获取分配的玩家ID
		recv(sock, (char*)&id_player, 1, 0); 
	}

	ClientManager()
	{
		sock = ClientCore::instance()->get_sock();
		id_player = ClientCore::instance()->get_id_player();
	}

	~ClientManager() = default;

	//服务器发出开始游戏指令 同步开始游戏
	void start_game()
	{
		Command cmd;
		if (recv(sock, (char*)&cmd, sizeof(cmd), MSG_PEEK) > 0)
		{
			recv(sock, (char*)&cmd, sizeof(cmd), 0);
			if (cmd.type == CommandType::StartGame)
			{
				//std::cout << 165156168 << std::endl;
				stage = ClientManager::Stage::Racing;
			}
		}

		std::cout << "游戏开始！" << std::endl;
		//开启游戏主线程
		std::thread receiver(&ClientManager::receiveThread, this);
		receiver.detach();
	}

	const int get_random_seed() const
	{
		return random_seed;
	}

	const Stage get_stage() const 
	{
		return stage;
	}

public:

#pragma region 主线程解释
	//玩家0控制防御塔的放置 需要获取玩家二的龙的位置
	//玩家1控制龙的移动 需要获取玩家一的所有防御塔的实时位置、所有金币的实时位置
#pragma endregion
	void main_thread()
	{
		
	}

	void simulate_frame()
	{
		// 处理所有本帧命令
		for (auto& cmd : pendingCommands) 
		{
			execute_command(cmd);
		}
		pendingCommands.clear();

		current_frame =  ClientCore::instance()->get_current_frame();
		(*current_frame)++;
	}

private:
	int id_player = 0;                                                  //玩家id
	int random_seed = 1;                                                //随机数种子
	uint32_t* current_frame = nullptr;                                  //当前的逻辑帧
	SOCKET sock;                                                        //套接字
	Stage stage = Stage::Waiting;                                       //当前游戏阶段
	std::string str_address;                                            //服务器地址
	std::vector<Command> pendingCommands;                               //当前帧的所有命令

private:
	//接收指令线程
	void receiveThread() 
	{
		while (true)
		{
			Command cmd;
			if (recv(sock, (char*)&cmd, sizeof(cmd), 0) > 0)
			{
				pendingCommands.push_back(cmd);
			}
			//std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	//处理本帧所有的操作
	void execute_command(const Command& cmd)
	{
		static TowerManager* instance = TowerManager::instance();
		SDL_Point idx = { cmd.x,cmd.y };
		switch (cmd.type)
		{
		case CommandType::BuildTower:
			std::cout << "接受到建造防御塔命令" << std::endl;
			instance->place_tower(cmd.towerType, idx);
			break;
		case CommandType::UpgradeTower:
			std::cout << "接受到升级防御塔命令" << std::endl;
			instance->upgrade_tower(idx);
			break;
		case CommandType::SpawnEnemy:
			std::cout << "接受到建开始生成敌人命令" << std::endl;
			WaveManager::instance()->set_is_wave_started(true);
			break;
		case CommandType::SpawnRand:
			ClientCore::instance()->set_random(cmd.random);
			break;
		case CommandType::DragonMove:
			//更新龙的状态
			{
				PlayerManager* instance = PlayerManager::instance();
				switch (cmd.DargonType)
				{
				case DargonCommandType::Dargon_move:
					instance->set_positon(Vector2(cmd.x, cmd.y));
					break;
				case DargonCommandType::Dargon_move_left:
					instance->set_is_move_left(true);
					break;
				case DargonCommandType::Dargon_move_right:
					instance->set_is_move_right(true);
					break;
				case DargonCommandType::Dargon_move_up:
					instance->set_is_move_up(true);
					break;
				case DargonCommandType::Dargon_move_down:
					instance->set_is_move_down(true);
					break;
				case DargonCommandType::Dargon_release_skill_j:
					instance->on_release_flash();
					break;
				case DargonCommandType::Dargon_release_skill_K:
					instance->on_release_impact();
					break;
				case DargonCommandType::Dargon_stop_move_left:
					instance->set_is_move_left(false);
					break;
				case DargonCommandType::Dargon_stop_move_right:
					instance->set_is_move_right(false);
					break;
				case DargonCommandType::Dargon_stop_move_up:
					instance->set_is_move_up(false);
					break;
				case DargonCommandType::Dargon_stop_move_down:
					instance->set_is_move_down(false);
					break;
				}
			}
			break;
		case CommandType::StartGame:
			std::cout << "接受到开始游戏命令" << std::endl;
			stage = ClientManager::Stage::Racing;
			break;
		case CommandType::UpdataFrame:
			current_frame =  ClientCore::instance()->get_current_frame();
			*current_frame = cmd.current_frame;
			break;
		case CommandType::DecreaseCoin:
			std::cout << "接受到减少金币命令" << std::endl;
			CoinManager::instance()->decrease_coin(cmd.coin_count);
			break;
		case CommandType::increaseCoin:
			std::cout << "接受到增加金币命令" << std::endl;
			CoinManager::instance()->increase_coin(cmd.coin_count);
			break;
		}
	}
};
