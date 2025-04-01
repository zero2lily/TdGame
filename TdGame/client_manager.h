#pragma once

#include "manager.h"

#include <string>

class TowerManager;

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
		//简历连接
		connect(sock, (sockaddr*)&server_addr, sizeof(server_addr));
	}

	void play_login_server()
	{
		cJSON* request = cJSON_CreateObject();
		cJSON_AddStringToObject(request, "type", "login");
		char* request_str = cJSON_Print(request);
		//请求数据
		send(sock, request_str, strlen(request_str), 0);
		free(request_str);
		cJSON_Delete(request);

		char buffer[4096];
		//接收到数据 成功执行时，返回接收到的字节数
		int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
		if (bytes_received < 0)
		{
			std::cerr << "登录服务器失败" << std::endl;
			exit(-1);
		}
		if (bytes_received > 0)
		{
			buffer[bytes_received] = '\0';
			cJSON* root = cJSON_Parse(buffer);
			if (root)
			{
				cJSON* id = cJSON_GetObjectItem(root, "type");
				if (id->valueint == 0)
				{
					std::cerr << "人数已满，登录服务器失败" << std::endl;
					exit(-1);
				}
				id_player = id->valueint;
				cJSON_Delete(root);
			}
		}
	}

	ClientManager()
	{
		connect_to_server();
	}

	~ClientManager() = default;

	//服务器发出开始游戏指令 同步开始游戏
	void is_game_start()
	{
		char buffer[1024];
		int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
		if (bytes_received > 0)
		{
			buffer[bytes_received] = '\0';
			if (strcmp(buffer, "GAME_START") == 0)
			{
				stage = ClientManager::Stage::Racing;
			}
		}
	}

	const int get_random_seed() const
	{
		return random_seed;
	}

	const SOCKET get_sock() const
	{
		return sock;
	}

	const int get_id_player() const
	{
		return id_player;
	}

	const Stage get_stage() const 
	{
		return stage;
	}

public:

#pragma region 主线程解释
	//玩家一控制防御塔的放置 需要获取玩家二的龙的位置
	//玩家二控制龙的移动 需要获取玩家一的所有防御塔的实时位置、所有金币的实时位置
#pragma endregion
	void main_thread()
	{
		
	}

	//放置防御塔线程
	void place_tower_thread(TowerType type, const SDL_Point& idx, int level,int id)
	{
		std::thread([type, idx, level, id, this]()
			{
				cJSON* request = cJSON_CreateObject();
				if (!request) {
					std::cerr << "失败创建root" << std::endl;
					return;
				}
				cJSON_AddStringToObject(request, "type", "add_tower");

				cJSON* data = cJSON_CreateObject();
				cJSON_AddNumberToObject(data, "x", idx.x);
				cJSON_AddNumberToObject(data, "y", idx.y);
				cJSON_AddNumberToObject(data, "level", level);
				cJSON_AddNumberToObject(data, "id", id);
				switch (type)
				{
				case Archer:

					cJSON_AddStringToObject(data, "type_tower", "Archer");
					break;
				case Axeman:
					cJSON_AddStringToObject(data, "type_tower", "Axeman");
					break;
				case Gunner:
					cJSON_AddStringToObject(data, "type_tower", "Gunner");
					break;
				}
				cJSON_AddItemToObject(request, "data", data);

				char* request_str = cJSON_Print(request);
				//请求数据
				send(sock, request_str, strlen(request_str), 0);
				//释放内存
				free(request_str);
				cJSON_Delete(request);
			}).detach();
	}

	void uograde_tower_thread(int level,int id)
	{
		std::thread([level, id, this]()
			{
				cJSON* request = cJSON_CreateObject();
				if (!request) {
					std::cerr << "失败创建root" << std::endl;
					return;
				}
				cJSON_AddStringToObject(request, "type", "upgrade_tower");
				cJSON_AddNumberToObject(request, "id", id);
				cJSON_AddNumberToObject(request, "level", level);

				char* request_str = cJSON_Print(request);
				//请求数据
				send(sock, request_str, strlen(request_str), 0);
				//释放内存
				free(request_str);
				cJSON_Delete(request);
			}).detach();
	}


private:
	int id_player = 0;     //自己是玩家1还是玩家二
	int random_seed = 1;   //随机数种子
	SOCKET sock;           //套接字
	Stage stage = Stage::Waiting;       //当前游戏阶段
	std::string str_address;            //服务器地址
};
