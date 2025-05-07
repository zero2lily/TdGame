//#include "cJSON.h"
//
//#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <iostream>
//#include <vector>
//#include <mutex>
//#include <thread>
//
//#include "tower_type.h"
//
//#pragma comment(lib, "ws2_32.lib")
//
//#define MAX_CLIENT 2
//
//// ������������
//enum class CommandType 
//{
//    BuildTower,
//    UpgradeTower,
//    SpawnEnemy,
//    PlayerReady
//};
//
//// ��������ṹ
//struct WebCommand 
//{
//    uint32_t frame;        // �����߼�֡
//    int player_Id;     // ���ID (0��1)
//    int x;                 // ����X
//    int y;                 // ����Y
//    TowerType towerType;   // ����������
//    CommandType type;      // ��������
//};
//
//std::vector<cJSON*> towers;    //���еķ�������Ϣ
//std::vector<SOCKET> clients;   //���еĿͻ���
//std::mutex towers_mutex, login_mutex, is_start_game;
//bool is_login_player1 = false; //���һ�Ƿ��¼
//bool is_login_player2 = false; //��Ҷ��Ƿ��¼
//double pos_dargon_x = 0;       //����xλ��
//double pos_dargon_y = 0;       //����yλ��
//
//void handle_client(SOCKET client_socket)
//{
//    char buffer[4096];
//    int bytes_received;
//
//    //bytes_received����0˵�����ݽ��ճɹ�
//    //recvҲ����������
//    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)))
//    {
//        if (bytes_received <= 0) break;
//
//        cJSON* root = cJSON_Parse(buffer);
//        if (!root) continue;
//
//        char* type = cJSON_GetObjectItem(root, "type")->valuestring;
//        if (strcmp(type, "login") == 0)
//        {
//            std::lock_guard<std::mutex> lock(login_mutex);
//
//            cJSON* response = cJSON_CreateObject();
//            if (is_login_player1 && is_login_player2)
//            {
//                cJSON_AddNumberToObject(response, "type", 0);
//            }
//            if(!is_login_player1) cJSON_AddNumberToObject(response, "type", 1);
//            if(!is_login_player2) cJSON_AddNumberToObject(response, "type", 2);
//            (is_login_player1 == true) ? (is_login_player2 = true) : (is_login_player1 = true);
//            //��������
//            char* response_str = cJSON_Print(response);
//            send(client_socket, response_str, strlen(response_str), 0);
//            //�ͷ��ڴ�
//            free(response_str);
//            cJSON_Delete(response);
//        }
//        else if (strcmp(type, "get_dragon_postition") == 0)
//        {
//            cJSON* response = cJSON_CreateObject();
//            cJSON_AddStringToObject(response, "type", "dargon_postition");
//            cJSON_AddNumberToObject(response, "x", pos_dargon_x);
//            cJSON_AddNumberToObject(response, "y", pos_dargon_y);
//
//            char* response_str = cJSON_Print(response);
//            send(client_socket, response_str, strlen(response_str), 0);
//
//            free(response_str);
//            cJSON_Delete(response);
//        }
//        else if (strcmp(type, "updata_dargon_postition") == 0 )
//        {
//            pos_dargon_x = cJSON_GetObjectItem(root, "x")->valuedouble;
//            pos_dargon_y = cJSON_GetObjectItem(root, "y")->valuedouble;
//        }
//        else if (strcmp(type, "add_tower") == 0) 
//        {
//            std::lock_guard<std::mutex> lock(towers_mutex);
//
//            cJSON* data = cJSON_GetObjectItem(root, "data");
//            towers.push_back(cJSON_Duplicate(data, 1));
//        }
//        else if (strcmp(type, "upgrade_tower") == 0)
//        {
//            std::lock_guard<std::mutex> lock(towers_mutex);
//
//            int id = cJSON_GetObjectItem(root, "id")->valueint;
//            int level = cJSON_GetObjectItem(root, "level")->valueint;
//            for (auto& tower : towers)
//            {
//                int now_id = cJSON_GetObjectItem(tower, "id")->valueint;
//                if (now_id == id)
//                {
//                    cJSON* now_level = cJSON_GetObjectItem(tower, "level");
//                    cJSON_SetNumberValue(now_level, level);
//                    break;
//                }
//            }
//        }
//        else if (strcmp(type, "get_towers") == 0)
//        {
//            std::lock_guard<std::mutex> lock(towers_mutex);
//
//            cJSON* response = cJSON_CreateObject();
//            //�����������Ϊ�������б�
//            cJSON_AddStringToObject(response, "type", "tower_list");
//            cJSON* tower_array = cJSON_CreateArray();
//            
//            for (auto& tower : towers) 
//            {
//                cJSON_AddItemToArray(tower_array, cJSON_Duplicate(tower, 1));
//            }
//            //���һ������
//            cJSON_AddItemToObject(response, "data", tower_array);
//            char* response_str = cJSON_Print(response);
//            send(client_socket, response_str, strlen(response_str), 0);
//
//            free(response_str);
//            cJSON_Delete(response);
//        }
//        //�����ڴ�
//        cJSON_Delete(root);
//    }
//    //�ر��׽���
//    closesocket(client_socket);
//}
//
//int main()
//{
//    //��ʼ��
//    WSADATA wsa;
//    WSAStartup(MAKEWORD(2, 2), &wsa);
//
//    //���������׽��� AF_INET��ʹ��ipv4 ��tcp����Э��
//    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
//    if (server_socket == -1)
//    {
//        std::cout << "�����׽��ִ���ʧ�ܣ�" << std::endl;
//        return 0;
//    }
//    //�󶨱��ص�IP���˿�   
//    //INADDR_ANY�����ݱ���������ȥ��ȡ��Ӧ��ip
//    sockaddr_in server_addr;
//    server_addr.sin_family = AF_INET;
//    server_addr.sin_port = htons(25565);       //���
//    server_addr.sin_addr.s_addr = INADDR_ANY;
//
//    //���׽��ֵ���ַ�Ͷ˿�
//    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1)
//    {
//        std::cout << "�׽��ְ�ʧ�ܣ�" << std::endl;
//        return 0;
//    }
//    //��ʼ������������ �������5���ͻ����ڵȴ�����
//    if (listen(server_socket, 128) == -1)
//    {
//        std::cout << "����ʧ�ܣ�" << std::endl;
//        return 0;
//    }
//
//    std::cout << "Server running on port 25565..." << std::endl;
//    //ʹ�ö��߳�
//    //�������ȴ��ͻ������� accept��һ���������ĺ���
//    while (clients.size() < MAX_CLIENT)
//    {
//        //���ͨ���׽���
//        SOCKET client_socket = accept(server_socket, NULL, NULL);
//        //�洢�ͻ���
//        clients.push_back(client_socket);
//        //����ÿ���ͻ��˴���һ���߳� ʵ�ֲ���
//        std::thread(handle_client, client_socket).detach();
//    }
//    
//    //����Ѿ�׼���� ��������ҷ���������Ϸ��ָ��
//    std::cout << "���пͻ����Ѿ�������Ϸ������ʼ��" << std::endl;
//    const char* start_msg = "GAME_START";
//    for (SOCKET client : clients) 
//    {
//        send(client, start_msg, strlen(start_msg), 0);
//    }
//    while (1); 
//
//    closesocket(server_socket);
//    WSACleanup();
//    return 0;
//}
#define SDL_MAIN_HANDLED

#include "cJSON.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <mutex>
#include <thread>
#include <unordered_map>

#include "tower_type.h"
#include "SDL.h"

#pragma comment(lib, "ws2_32.lib")

#define MAX_CLIENT 2  //���ͻ�����������

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
   Dargon_stop_skill_k
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

class Server
{
public:
    Server()
    {
        //��ʼ��
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);

        //���������׽��� AF_INET��ʹ��ipv4 ��UDP����Э��
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket == -1)
        {
            std::cout << "�����׽��ִ���ʧ�ܣ�" << std::endl;
            exit(-1);
        }

        //�󶨱��ص�IP���˿�   
        //INADDR_ANY�����ݱ���������ȥ��ȡ��Ӧ��ip
        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(25565);       //���
        server_addr.sin_addr.s_addr = INADDR_ANY;

        //���׽��ֵ���ַ�Ͷ˿�
        if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        {
            std::cout << "�׽��ְ�ʧ�ܣ�" << std::endl;
            exit(-1);
        }

        //udp����Ҫ����
        //��ʼ������������ �������5���ͻ����ڵȴ�����
        if (listen(server_socket, 128) == -1)
        {
            std::cout << "����ʧ�ܣ�" << std::endl;
            exit(-1);
        }

        std::cout << "Server running on port 25565..." << std::endl;
    }

    ~Server()
    {
        closesocket(server_socket);
        WSACleanup();
    }

    void run()
    {
        //�������
        wait_client_link();

        //��ȡ�߾��ȶ�ʱ���ĵ�ǰ����
        Uint64 last_counter = SDL_GetPerformanceCounter();
        //��ȡ�߽��ȶ�ʱ��Ƶ��(ÿ���Ӹ߾��ȶ�ʱ���Ĵ���);
        const Uint64 counter_freq = SDL_GetPerformanceFrequency();

        while (!is_quit)
        {
            while (SDL_PollEvent(&event))
                on_input();

            //�ռ���֡���е�ָ��
            collect_commands(current_frame);

            //�㲥��ǰ�����
            broadcast_current_random(rand());

            //�㲥��֡���е�ָ��
            broadcastCommands(current_frame);

            //�߼�֡�ƽ�
            current_frame++;

            //�㲥��ǰ������߼�֡ ͬ��֡��
            broadcast_current_frame(current_frame);

            Uint64 current_counuer = SDL_GetPerformanceCounter();
            //delta Ϊ���˶�����
            double delta = (double)(current_counuer - last_counter) / counter_freq;
            last_counter = current_counuer;
            //�ӳٺ�����msΪ��λ��    1000.0/60 Ϊһ��60֡�ʹ�����ms    һ֡���16ms,delta���16ms
            if (delta * 1000 < 1000.0 / 60)
                SDL_Delay((Uint32)(1000.0 / 60 - delta * 1000));
        }
        return ;
    }

private:
    SDL_Event event;                                                    //sdl�¼�
    SOCKET server_socket;                                               //���������׽���
    std::vector<SOCKET> clients;                                        //�洢���еĿͻ���
    std::unordered_map<uint32_t, std::vector<Command>>  frame_commands; //��¼ÿһ֡������
    bool is_quit = false;                                               //�Ƿ��˳�
    uint32_t current_frame = 0;                                         //��ǰ���˵ڼ�֡

    bool is_ready_play_zero = false;                                    //���0��û��׼���ÿ�ʼ����
    bool is_ready_play_one = false;                                     //���һ��û��׼���ÿ�ʼ����

private:

    void on_input()
    {
        switch (event.type)
        {
        case SDL_QUIT:
            is_quit = true;
            break;
        default:
            break;
        }
    }

    //�ȴ����������ͻ���
    void wait_client_link()
    {
        std::cout << "�ȴ��ͻ�������" << std::endl;
        //�������ȴ��ͻ������� accept��һ���������ĺ���
        while (clients.size() < MAX_CLIENT)
        {
            //���ͨ���׽���
            SOCKET client_socket = accept(server_socket, NULL, NULL);

            // ���÷�����ģʽ
            u_long mode = 1;
            ioctlsocket(client_socket, FIONBIO, &mode);

            //�洢�ͻ����׽���
            clients.emplace_back(client_socket);

            //�������id �Ƚ�������Ϊ0������Ϊ1
            char player_id = static_cast<char>(clients.size() - 1);
            send(client_socket, &player_id, 1, 0);

            std::cout << "���" << clients.size() << "���ӳɹ�" << std::endl;
        }

        std::cout << "��Ҷ�׼��������ʼ��Ϸ"  << std::endl;

        auto& play_1_client = clients[0];
        auto& play_2_client = clients[1];
        
        //���Ϳ�ʼ��Ϸָ��
         Command cmd{};
         cmd.type = CommandType::StartGame;
         send(play_1_client, (char*)&cmd, sizeof(cmd), 0);
         send(play_2_client, (char*)&cmd, sizeof(cmd), 0);
        
        
    }

    //�ռ���֡��ָ��
    void collect_commands(uint32_t current_frame)
    {
        for (auto& client : clients)
        {
            Command cmd;
            //���ܿͻ��˵�ǰ֡������
            while (recv(client, (char*)&cmd, sizeof(cmd), MSG_PEEK) > 0)
            {
                //�Բ���ָ����������
                if (cmd.type == CommandType::SpawnEnemy)
                {
                    recv(client, (char*)&cmd, sizeof(cmd), 0);
                    if (cmd.player_Id == 0)
                    {
                        is_ready_play_zero = true;
                    }
                    else
                    {
                        is_ready_play_one = true;
                    }
                    if (is_ready_play_one && is_ready_play_zero)
                    {
                        Command cmd;
                        cmd.is_wave_start = true;
                        cmd.type = CommandType::SpawnEnemy;
                        frame_commands[current_frame].push_back(cmd);

                        is_ready_play_one = false;
                        is_ready_play_zero = false;

                        std::cout << "����˽��ܵ���һ�����˳�������" << std::endl;
                    }
                    continue;
                }
                else if (cmd.type == CommandType::DragonMove)
                {
                    recv(client, (char*)&cmd, sizeof(cmd), 0);
                    frame_commands[current_frame].push_back(cmd);
                    continue;
                }
                if (cmd.frame + 100 >= current_frame)
                {
                    switch (cmd.type)
                    {
                    case CommandType::BuildTower:
                        std::cout << "����˽��ܵ��������������" << std::endl;
                        break;
                    case CommandType::UpgradeTower:
                        std::cout << "����˽��ܵ���������������" << std::endl;
                        break;
                    case CommandType::SpawnEnemy:
                        break;
                    case CommandType::SpawnRand:
                        break;
                    case CommandType::DragonMove:
                        break;
                    case CommandType::StartGame:
                        break;
                    case CommandType::UpdataFrame:
                        break;
                    case CommandType::DecreaseCoin:
                        std::cout << "����˽��ܵ����ٽ������" << std::endl;
                        break;
                    case CommandType::increaseCoin:
                        std::cout << "����˽��ܵ����ӽ������" << std::endl;
                       
                        break;
                    }
                    recv(client, (char*)&cmd, sizeof(cmd), 0);
                    frame_commands[current_frame].push_back(cmd);
                }
                else 
                {
                    recv(client, (char*)&cmd, sizeof(cmd), 0);
                    break;
                }
            }
            
        }
    }

    void broadcast_current_frame(uint32_t current_frame)
    {
        for (auto& client : clients)
        {
            Command cmd_time;
            cmd_time.current_frame = current_frame;
            cmd_time.type = CommandType::UpdataFrame;
            send(client, (char*)&cmd_time, sizeof(cmd_time), 0);
           
        }
    }

    void  broadcast_current_random(int random)
    {
        //ÿ2s����һ�������
        if (current_frame % 120 == 0)
        {
            for (auto& client : clients)
            {
                Command cmd_time;
                cmd_time.random = random;
                cmd_time.type = CommandType::SpawnRand;
                send(client, (char*)&cmd_time, sizeof(cmd_time), 0);

            }
        }
    
    }
    
    //�㲥��֡��ָ��
    void broadcastCommands(uint32_t current_frame)
    {
        auto& commands = frame_commands[current_frame];
        auto& play_1_client = clients[0];
        auto& play_2_client = clients[1];
        for (auto& cmd : commands) 
        {
            send(play_1_client, (char*)&cmd, sizeof(cmd), 0);
            send(play_2_client, (char*)&cmd, sizeof(cmd), 0);
        }
        
    }
};

int main()
{
    Server* server = new Server();
    server->run();
    return 0;
}
