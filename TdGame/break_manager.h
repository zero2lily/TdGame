#pragma once
//���ļ�Ϊ��������manger��ClientManager������������������
//����manager���ô�.h����ָ����Ϣ

#include <mutex>

#include "client_core.h"
#include "vector2.h"
#include "game_manager.h"

std::mutex mutex;

class BreakManager : public Manager<BreakManager>
{
	friend class Manager<BreakManager>;

public:
	BreakManager()
	{
		sock = ClientCore::instance()->get_sock();
		id_player = ClientCore::instance()->get_id_player();
	}

	~BreakManager()
	{
		closesocket(sock);
	}

public :

	void ready_wave_cmd()
	{
		std::lock_guard<std::mutex> lock(mutex);

		Command cmd;
		cmd.player_Id = id_player;
		cmd.type = CommandType::SpawnEnemy;
		send(sock, (char*)&cmd, sizeof(cmd), 0);

		std::cout << "����׼������һ������" << std::endl;
	}

	void decrease_coin_cmd(int decrease_coin_count)
	{
		std::lock_guard<std::mutex> lock(mutex);

		Command cmd;
		cmd.coin_count = decrease_coin_count;
		cmd.type = CommandType::DecreaseCoin;
		send(sock, (char*)&cmd, sizeof(cmd), 0);

		std::cout << "���ͼ��ٽ������" << std::endl;
	}

	//���÷���������
	void place_tower_cmd(TowerType type, const SDL_Point& idx)
	{
		std::lock_guard<std::mutex> lock(mutex);

		current_frame = ClientCore::instance()->get_current_frame();

		Command cmd{};
		cmd.frame = *current_frame + 1;
		cmd.player_Id = id_player;
		cmd.x = idx.x;
		cmd.y = idx.y;
		cmd.type = CommandType::BuildTower;
		cmd.towerType = type;

		send(sock, (char*)&cmd, sizeof(cmd), 0);

		std::cout << "���ͷ��÷��������λ�ã�" << idx.x << " " << idx.y << std::endl;
	}

	//��������������
	void upgrade_tower_cmd(TowerType type, const SDL_Point& idx)
	{
		std::lock_guard<std::mutex> lock(mutex);

		current_frame = ClientCore::instance()->get_current_frame();

		Command cmd{};
		cmd.frame = *current_frame + 1;
		cmd.player_Id = id_player;
		cmd.x = idx.x;
		cmd.y = idx.y;
		cmd.type = CommandType::UpgradeTower;
		cmd.towerType = type;

		send(sock, (char*)&cmd, sizeof(cmd), 0);

		std::cout << "�����������������λ�ã�" << idx.x << " " << idx.y << std::endl;
	}


	void dragon_move_cmd(Vector2 position)
	{
		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_move;
		cmd.type = CommandType::DragonMove;
		cmd.x = position.x;
		cmd.y = position.y;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dragon_move_left_cmd()
	{
		std::lock_guard<std::mutex> lock(mutex);

		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_move_left;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dragon_move_right_cmd()
	{
		std::lock_guard<std::mutex> lock(mutex);

		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_move_right;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dragon_move_up_cmd()
	{
		std::lock_guard<std::mutex> lock(mutex);

		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_move_up;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dragon_move_down_cmd()
	{
		std::lock_guard<std::mutex> lock(mutex);

		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_move_down;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dargon_release_skill_j_cmd()
	{
		std::lock_guard<std::mutex> lock(mutex);

		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_release_skill_j;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dargon_release_skill_K_cmd()
	{
		std::lock_guard<std::mutex> lock(mutex);

		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_release_skill_K;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dragon_stop_left_cmd()
	{
		std::lock_guard<std::mutex> lock(mutex);

		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_stop_move_left;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dragon_stop_right_cmd()
	{
		std::lock_guard<std::mutex> lock(mutex);

		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_stop_move_right;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dragon_stop_up_cmd()
	{
		std::lock_guard<std::mutex> lock(mutex);

		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_stop_move_up;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dragon_stop_down_cmd()
	{
		std::lock_guard<std::mutex> lock(mutex);

		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_stop_move_down;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dargon_stop_skill_j_cmd()
	{
		std::lock_guard<std::mutex> lock(mutex);

		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_stop_skill_j;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dargon_stop_skill_K_cmd()
	{
		std::lock_guard<std::mutex> lock(mutex);

		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_stop_skill_k;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

private:
	
	int id_player = 0;                                                  //���id
	uint32_t* current_frame = nullptr;
	SOCKET sock;                                                        //�׽���
};


