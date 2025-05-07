#pragma once

#include <mutex>
#include <iostream>
#include "tower_type.h"


// ������������
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


//��������
class Command
{
public:
	Command() = default;
	~Command() = default;

public:
	uint32_t frame;              // �����߼�֡
	uint32_t current_frame;      // ��ǰ����˵��߼�֡
	int player_Id;               // ���ID (0��1)
	int x;                       // ����X
	int y;                       // ����Y
	int is_wave_start;           // �Ƿ�ʼ����
	int coin_count;              // �������
	int random;                  // �����
	DargonCommandType DargonType; // ����������
	TowerType towerType;         // ����������
	CommandType type;            // ��������
};

class ClientCore : public Manager<ClientCore>
{
    friend class Manager<ClientCore>;
public:
    ClientCore()
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

		//Udp����Э��
		sock = socket(AF_INET, SOCK_STREAM, 0);
		sockaddr_in server_addr;
		server_addr.sin_family = AF_INET;
		server_addr.sin_port = htons(25565);
		inet_pton(AF_INET, str_stream.str().c_str(), &server_addr.sin_addr);
		//��������
		int succes = connect(sock, (sockaddr*)&server_addr, sizeof(server_addr));

		if (succes == -1)
		{
			std::cout << "���ӷ�����ʧ��" << std::endl;
			exit(-1);
		}

		// ��ȡ��������ID
		recv(sock, (char*)&id_player, 1, 0);

		std::cout << "�ɹ����ӷ���������Ϊ���" << id_player << "�ȴ�������Ҽ���" << std::endl;
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
    int id_player = 0;                                                  //���id
	int random = 0;                                                     //�����
	uint32_t current_frame = 0;                                         //��ǰ���߼�֡
	SOCKET sock;                                                        //�׽���
	std::string str_address;                                            //��������ַ
};
