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
		// ��ȡ��������ID
		recv(sock, (char*)&id_player, 1, 0); 
	}

	ClientManager()
	{
		sock = ClientCore::instance()->get_sock();
		id_player = ClientCore::instance()->get_id_player();
	}

	~ClientManager() = default;

	//������������ʼ��Ϸָ�� ͬ����ʼ��Ϸ
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

		std::cout << "��Ϸ��ʼ��" << std::endl;
		//������Ϸ���߳�
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

#pragma region ���߳̽���
	//���0���Ʒ������ķ��� ��Ҫ��ȡ��Ҷ�������λ��
	//���1���������ƶ� ��Ҫ��ȡ���һ�����з�������ʵʱλ�á����н�ҵ�ʵʱλ��
#pragma endregion
	void main_thread()
	{
		
	}

	void simulate_frame()
	{
		// �������б�֡����
		for (auto& cmd : pendingCommands) 
		{
			execute_command(cmd);
		}
		pendingCommands.clear();

		current_frame =  ClientCore::instance()->get_current_frame();
		(*current_frame)++;
	}

private:
	int id_player = 0;                                                  //���id
	int random_seed = 1;                                                //���������
	uint32_t* current_frame = nullptr;                                  //��ǰ���߼�֡
	SOCKET sock;                                                        //�׽���
	Stage stage = Stage::Waiting;                                       //��ǰ��Ϸ�׶�
	std::string str_address;                                            //��������ַ
	std::vector<Command> pendingCommands;                               //��ǰ֡����������

private:
	//����ָ���߳�
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

	//����֡���еĲ���
	void execute_command(const Command& cmd)
	{
		static TowerManager* instance = TowerManager::instance();
		SDL_Point idx = { cmd.x,cmd.y };
		switch (cmd.type)
		{
		case CommandType::BuildTower:
			std::cout << "���ܵ��������������" << std::endl;
			instance->place_tower(cmd.towerType, idx);
			break;
		case CommandType::UpgradeTower:
			std::cout << "���ܵ���������������" << std::endl;
			instance->upgrade_tower(idx);
			break;
		case CommandType::SpawnEnemy:
			std::cout << "���ܵ�����ʼ���ɵ�������" << std::endl;
			WaveManager::instance()->set_is_wave_started(true);
			break;
		case CommandType::SpawnRand:
			ClientCore::instance()->set_random(cmd.random);
			break;
		case CommandType::DragonMove:
			//��������״̬
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
			std::cout << "���ܵ���ʼ��Ϸ����" << std::endl;
			stage = ClientManager::Stage::Racing;
			break;
		case CommandType::UpdataFrame:
			current_frame =  ClientCore::instance()->get_current_frame();
			*current_frame = cmd.current_frame;
			break;
		case CommandType::DecreaseCoin:
			std::cout << "���ܵ����ٽ������" << std::endl;
			CoinManager::instance()->decrease_coin(cmd.coin_count);
			break;
		case CommandType::increaseCoin:
			std::cout << "���ܵ����ӽ������" << std::endl;
			CoinManager::instance()->increase_coin(cmd.coin_count);
			break;
		}
	}
};
