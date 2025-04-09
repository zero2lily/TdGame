#pragma once
//此文件为打破其他manger和ClientManager互相包含的问题二存在
//其他manager调用此.h发送指令信息

#include "client_core.h"

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
		Command cmd;
		cmd.player_Id = id_player;
		cmd.type = CommandType::SpawnEnemy;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void decrease_coin_cmd(int decrease_coin_count)
	{
		Command cmd;
		cmd.coin_count = decrease_coin_count;
		cmd.type = CommandType::DecreaseCoin;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dragon_move_left_cmd()
	{
		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_move_left;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dragon_move_right_cmd()
	{
		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_move_right;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dragon_move_up_cmd()
	{
		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_move_up;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dragon_move_down_cmd()
	{
		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_move_down;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dargon_release_skill_j_cmd()
	{
		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_release_skill_j;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dargon_release_skill_K_cmd()
	{
		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_release_skill_K;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dragon_stop_left_cmd()
	{
		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_stop_move_left;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dragon_stop_right_cmd()
	{
		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_stop_move_right;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dragon_stop_up_cmd()
	{
		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_stop_move_up;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dragon_stop_down_cmd()
	{
		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_stop_move_down;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dargon_stop_skill_j_cmd()
	{
		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_stop_skill_j;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

	void dargon_stop_skill_K_cmd()
	{
		Command cmd;
		cmd.DargonType = DargonCommandType::Dargon_stop_skill_k;
		cmd.type = CommandType::DragonMove;
		send(sock, (char*)&cmd, sizeof(cmd), 0);
	}

private:
	int id_player = 0;                                                  //玩家id
	SOCKET sock;                                                        //套接字
};


