#pragma once

#include <mutex>
#include <iostream>
#include "tower_type.h"


// 网络命令类型
enum class CommandType
{
	BuildTower,
	UpgradeTower,
	SpawnEnemy,
	SpawnRand,
	StartGame,
	UpdataFrame,
	DragonMove,
	DecreaseCoin,
	increaseCoin
};

enum  class DargonCommandType
{
	Dargon_move_left,
	Dargon_move_right,
	Dargon_move_up,
	Dargon_move_down,
	Dargon_release_skill_j,
	Dargon_release_skill_K,
	Dargon_stop_move_left,
	Dargon_stop_move_right,
	Dargon_stop_move_up,
	Dargon_stop_move_down,
	Dargon_stop_skill_j,
	Dargon_stop_skill_k,
	Dargon_move,
};


//网络命令
class Command
{
public:
	Command() = default;
	~Command() = default;

public:
	uint32_t frame;              // 所属逻辑帧
	uint32_t current_frame;      // 当前服务端的逻辑帧
	int player_Id;               // 玩家ID (0或1)
	int x;                       // 坐标X
	int y;                       // 坐标Y
	int is_wave_start;           // 是否开始出怪
	int coin_count;              // 金币数量
	int random;                  // 随机数
	DargonCommandType DargonType; // 龙操作类型
	TowerType towerType;         // 防御塔类型
	CommandType type;            // 命令类型
};

class ClientCore : public Manager<ClientCore>
{
    friend class Manager<ClientCore>;
public:
    ClientCore()
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

		//Udp传输协议
		sock = socket(AF_INET, SOCK_STREAM, 0);
		sockaddr_in server_addr;
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(25565);
		inet_pton(AF_INET, str_stream.str().c_str(), &server_addr.sin_addr);
		//建立连接
		int succes = connect(sock, (sockaddr*)&server_addr, sizeof(server_addr));

		if (succes == -1)
		{
			std::cout << "连接服务器失败" << std::endl;
			exit(-1);
		}

		// 获取分配的玩家ID
		recv(sock, (char*)&id_player, 1, 0);

		std::cout << "成功连接服务器，你为玩家" << id_player << "等待其他玩家加入" << std::endl;
    }
	~ClientCore()
	{
		closesocket(sock);
	}

	const SOCKET get_sock() const
	{
		return sock;
	}

	const int get_id_player() const
	{
		return id_player;
	}

	const int get_random() const
	{
		return random;
	}

	uint32_t* get_current_frame()
	{
		return &current_frame;
	}

	void set_random(int val)
	{
		random = val;
	}

private:
    int id_player = 0;                                                  //玩家id
	int random = 0;                                                     //随机数
	uint32_t current_frame = 0;                                         //当前的逻辑帧
	SOCKET sock;                                                        //套接字
	std::string str_address;                                            //服务器地址
};
