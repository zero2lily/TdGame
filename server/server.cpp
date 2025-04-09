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
//// 网络命令类型
//enum class CommandType 
//{
//    BuildTower,
//    UpgradeTower,
//    SpawnEnemy,
//    PlayerReady
//};
//
//// 网络命令结构
//struct WebCommand 
//{
//    uint32_t frame;        // 所属逻辑帧
//    int player_Id;     // 玩家ID (0或1)
//    int x;                 // 坐标X
//    int y;                 // 坐标Y
//    TowerType towerType;   // 防御塔类型
//    CommandType type;      // 命令类型
//};
//
//std::vector<cJSON*> towers;    //所有的防御塔信息
//std::vector<SOCKET> clients;   //所有的客户端
//std::mutex towers_mutex, login_mutex, is_start_game;
//bool is_login_player1 = false; //玩家一是否登录
//bool is_login_player2 = false; //玩家二是否登录
//double pos_dargon_x = 0;       //龙的x位置
//double pos_dargon_y = 0;       //龙的y位置
//
//void handle_client(SOCKET client_socket)
//{
//    char buffer[4096];
//    int bytes_received;
//
//    //bytes_received大于0说明数据接收成功
//    //recv也是阻塞函数
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
//            //发送数据
//            char* response_str = cJSON_Print(response);
//            send(client_socket, response_str, strlen(response_str), 0);
//            //释放内存
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
//            //添加数据类型为防御塔列表
//            cJSON_AddStringToObject(response, "type", "tower_list");
//            cJSON* tower_array = cJSON_CreateArray();
//            
//            for (auto& tower : towers) 
//            {
//                cJSON_AddItemToArray(tower_array, cJSON_Duplicate(tower, 1));
//            }
//            //添加一个数组
//            cJSON_AddItemToObject(response, "data", tower_array);
//            char* response_str = cJSON_Print(response);
//            send(client_socket, response_str, strlen(response_str), 0);
//
//            free(response_str);
//            cJSON_Delete(response);
//        }
//        //清理内存
//        cJSON_Delete(root);
//    }
//    //关闭套接字
//    closesocket(client_socket);
//}
//
//int main()
//{
//    //初始化
//    WSADATA wsa;
//    WSAStartup(MAKEWORD(2, 2), &wsa);
//
//    //创建监听套接字 AF_INET：使用ipv4 ，tcp传输协议
//    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
//    if (server_socket == -1)
//    {
//        std::cout << "监听套接字创建失败！" << std::endl;
//        return 0;
//    }
//    //绑定本地的IP，端口   
//    //INADDR_ANY：根据本机的网卡去读取对应的ip
//    sockaddr_in server_addr;
//    server_addr.sin_family = AF_INET;
//    server_addr.sin_port = htons(25565);       //大端
//    server_addr.sin_addr.s_addr = INADDR_ANY;
//
//    //绑定套接字到地址和端口
//    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1)
//    {
//        std::cout << "套接字绑定失败！" << std::endl;
//        return 0;
//    }
//    //开始监听连接请求 最多允许5个客户端在等待连接
//    if (listen(server_socket, 128) == -1)
//    {
//        std::cout << "监听失败！" << std::endl;
//        return 0;
//    }
//
//    std::cout << "Server running on port 25565..." << std::endl;
//    //使用多线程
//    //阻塞并等待客户端连接 accept是一个会阻塞的函数
//    while (clients.size() < MAX_CLIENT)
//    {
//        //变成通信套接字
//        SOCKET client_socket = accept(server_socket, NULL, NULL);
//        //存储客户端
//        clients.push_back(client_socket);
//        //对于每个客户端创建一个线程 实现并发
//        std::thread(handle_client, client_socket).detach();
//    }
//    
//    //玩家已经准备好 对所有玩家发布开启游戏的指令
//    std::cout << "所有客户端已就绪，游戏即将开始！" << std::endl;
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

#define MAX_CLIENT 2  //最大客户端连接数量

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
   Dargon_stop_skill_k
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

class Server
{
public:
    Server()
    {
        //初始化
        WSADATA wsa;
        WSAStartup(MAKEWORD(2, 2), &wsa);

        //创建监听套接字 AF_INET：使用ipv4 ，UDP传输协议
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket == -1)
        {
            std::cout << "监听套接字创建失败！" << std::endl;
            exit(-1);
        }

        //绑定本地的IP，端口   
        //INADDR_ANY：根据本机的网卡去读取对应的ip
        sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(25565);       //大端
        server_addr.sin_addr.s_addr = INADDR_ANY;

        //绑定套接字到地址和端口
        if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == -1)
        {
            std::cout << "套接字绑定失败！" << std::endl;
            exit(-1);
        }

        //udp不需要监听
        //开始监听连接请求 最多允许5个客户端在等待连接
        if (listen(server_socket, 128) == -1)
        {
            std::cout << "监听失败！" << std::endl;
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
        //玩家连接
        wait_client_link();

        //获取高精度定时器的当前次数
        Uint64 last_counter = SDL_GetPerformanceCounter();
        //获取高进度定时器频率(每秒钟高精度定时器的次数);
        const Uint64 counter_freq = SDL_GetPerformanceFrequency();

        while (!is_quit)
        {
            while (SDL_PollEvent(&event))
                on_input();

            //收集本帧所有的指令
            collect_commands(current_frame);

            //广播当前随机数
            broadcast_current_random(rand());

            //广播本帧所有的指令
            broadcastCommands(current_frame);

            //逻辑帧推进
            current_frame++;

            //广播当前服务端逻辑帧 同步帧率
            broadcast_current_frame(current_frame);

            Uint64 current_counuer = SDL_GetPerformanceCounter();
            //delta 为过了多少秒
            double delta = (double)(current_counuer - last_counter) / counter_freq;
            last_counter = current_counuer;
            //延迟函数是ms为单位的    1000.0/60 为一个60帧率过多少ms    一帧大概16ms,delta大概16ms
            if (delta * 1000 < 1000.0 / 60)
                SDL_Delay((Uint32)(1000.0 / 60 - delta * 1000));
        }
        return ;
    }

private:
    SDL_Event event;                                                    //sdl事件
    SOCKET server_socket;                                               //网络连接套接字
    std::vector<SOCKET> clients;                                        //存储所有的客户端
    std::unordered_map<uint32_t, std::vector<Command>>  frame_commands; //记录每一帧的命令
    bool is_quit = false;                                               //是否退出
    uint32_t current_frame = 0;                                         //当前到了第几帧

    bool is_ready_play_zero = false;                                    //玩家0有没有准备好开始出怪
    bool is_ready_play_one = false;                                     //玩家一有没有准备好开始出怪

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

    //等待连接两个客户端
    void wait_client_link()
    {
        std::cout << "等待客户端连接" << std::endl;
        //阻塞并等待客户端连接 accept是一个会阻塞的函数
        while (clients.size() < MAX_CLIENT)
        {
            //变成通信套接字
            SOCKET client_socket = accept(server_socket, NULL, NULL);

            // 设置非阻塞模式
            u_long mode = 1;
            ioctlsocket(client_socket, FIONBIO, &mode);

            //存储客户端套接字
            clients.emplace_back(client_socket);

            //分配玩家id 先进入的玩家为0，后者为1
            char player_id = static_cast<char>(clients.size() - 1);
            send(client_socket, &player_id, 1, 0);

            std::cout << "玩家" << clients.size() << "连接成功" << std::endl;
        }

        std::cout << "玩家都准备就绪开始游戏"  << std::endl;

        auto& play_1_client = clients[0];
        auto& play_2_client = clients[1];
        
        //发送开始游戏指令
         Command cmd{};
         cmd.type = CommandType::StartGame;
         send(play_1_client, (char*)&cmd, sizeof(cmd), 0);
         send(play_2_client, (char*)&cmd, sizeof(cmd), 0);
        
        
    }

    //收集本帧的指令
    void collect_commands(uint32_t current_frame)
    {
        for (auto& client : clients)
        {
            Command cmd;
            //接受客户端当前帧的请求
            while (recv(client, (char*)&cmd, sizeof(cmd), MSG_PEEK) > 0)
            {
                //对波次指令的特殊控制
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

                        std::cout << "服务端接受到下一波敌人出现命令" << std::endl;
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
                        std::cout << "服务端接受到建造防御塔命令" << std::endl;
                        break;
                    case CommandType::UpgradeTower:
                        std::cout << "服务端接受到升级防御塔命令" << std::endl;
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
                        std::cout << "服务端接受到减少金币命令" << std::endl;
                        break;
                    case CommandType::increaseCoin:
                        std::cout << "服务端接受到增加金币命令" << std::endl;
                       
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
        //每2s更新一次随机数
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
    
    //广播本帧的指令
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
