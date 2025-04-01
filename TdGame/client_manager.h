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
		Waiting, //�ȴ���Ҽ���
		Ready,   //��ʼ����ʱ
		Racing   //������Ϸ��
	};

public:

	void connect_to_server()
	{
		//��ʼ��
		WSADATA wsa;
		WSAStartup(MAKEWORD(2, 2), &wsa);

		//���ļ�
		std::ifstream file("config.cfg");
		//�ļ���ʧ��
		if (!file.good())
		{
			std::cerr << "config.cfg�ļ���ʧ��" << std::endl;
			exit(-1);
		}

		std::stringstream str_stream;
		str_stream << file.rdbuf();
		file.close();

		str_address = str_stream.str();

		//TCP����Э��
		sock = socket(AF_INET, SOCK_STREAM, 0);
		sockaddr_in server_addr;
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(25565);
		inet_pton(AF_INET, str_stream.str().c_str(), &server_addr.sin_addr);
		//��������
		connect(sock, (sockaddr*)&server_addr, sizeof(server_addr));
	}

	void play_login_server()
	{
		cJSON* request = cJSON_CreateObject();
		cJSON_AddStringToObject(request, "type", "login");
		char* request_str = cJSON_Print(request);
		//��������
		send(sock, request_str, strlen(request_str), 0);
		free(request_str);
		cJSON_Delete(request);

		char buffer[4096];
		//���յ����� �ɹ�ִ��ʱ�����ؽ��յ����ֽ���
		int bytes_received = recv(sock, buffer, sizeof(buffer), 0);
		if (bytes_received < 0)
		{
			std::cerr << "��¼������ʧ��" << std::endl;
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
					std::cerr << "������������¼������ʧ��" << std::endl;
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

	//������������ʼ��Ϸָ�� ͬ����ʼ��Ϸ
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

#pragma region ���߳̽���
	//���һ���Ʒ������ķ��� ��Ҫ��ȡ��Ҷ�������λ��
	//��Ҷ����������ƶ� ��Ҫ��ȡ���һ�����з�������ʵʱλ�á����н�ҵ�ʵʱλ��
#pragma endregion
	void main_thread()
	{
		
	}

	//���÷������߳�
	void place_tower_thread(TowerType type, const SDL_Point& idx, int level,int id)
	{
		std::thread([type, idx, level, id, this]()
			{
				cJSON* request = cJSON_CreateObject();
				if (!request) {
					std::cerr << "ʧ�ܴ���root" << std::endl;
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
				//��������
				send(sock, request_str, strlen(request_str), 0);
				//�ͷ��ڴ�
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
					std::cerr << "ʧ�ܴ���root" << std::endl;
					return;
				}
				cJSON_AddStringToObject(request, "type", "upgrade_tower");
				cJSON_AddNumberToObject(request, "id", id);
				cJSON_AddNumberToObject(request, "level", level);

				char* request_str = cJSON_Print(request);
				//��������
				send(sock, request_str, strlen(request_str), 0);
				//�ͷ��ڴ�
				free(request_str);
				cJSON_Delete(request);
			}).detach();
	}


private:
	int id_player = 0;     //�Լ������1������Ҷ�
	int random_seed = 1;   //���������
	SOCKET sock;           //�׽���
	Stage stage = Stage::Waiting;       //��ǰ��Ϸ�׶�
	std::string str_address;            //��������ַ
};
