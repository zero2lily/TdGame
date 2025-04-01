#include "cJSON.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <mutex>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

#define MAX_CLIENT 2

std::vector<cJSON*> towers;    //所有的防御塔信息
std::vector<SOCKET> clients;   //所有的客户端
std::mutex towers_mutex, login_mutex, is_start_game;
bool is_login_player1 = false; //玩家一是否登录
bool is_login_player2 = false; //玩家二是否登录
double pos_dargon_x = 0;       //龙的x位置
double pos_dargon_y = 0;       //龙的y位置

void handle_client(SOCKET client_socket)
{
    char buffer[4096];
    int bytes_received;

    //bytes_received大于0说明数据接收成功
    //recv也是阻塞函数
    while ((bytes_received = recv(client_socket, buffer, sizeof(buffer), 0)))
    {
        if (bytes_received <= 0) break;

        cJSON* root = cJSON_Parse(buffer);
        if (!root) continue;

        char* type = cJSON_GetObjectItem(root, "type")->valuestring;
        if (strcmp(type, "login") == 0)
        {
            std::lock_guard<std::mutex> lock(login_mutex);

            cJSON* response = cJSON_CreateObject();
            if (is_login_player1 && is_login_player2)
            {
                cJSON_AddNumberToObject(response, "type", 0);
            }
            if(!is_login_player1) cJSON_AddNumberToObject(response, "type", 1);
            if(!is_login_player2) cJSON_AddNumberToObject(response, "type", 2);
            (is_login_player1 == true) ? (is_login_player2 = true) : (is_login_player1 = true);
            //发送数据
            char* response_str = cJSON_Print(response);
            send(client_socket, response_str, strlen(response_str), 0);
            //释放内存
            free(response_str);
            cJSON_Delete(response);
        }
        else if (strcmp(type, "get_dragon_postition") == 0)
        {
            cJSON* response = cJSON_CreateObject();
            cJSON_AddStringToObject(response, "type", "dargon_postition");
            cJSON_AddNumberToObject(response, "x", pos_dargon_x);
            cJSON_AddNumberToObject(response, "y", pos_dargon_y);

            char* response_str = cJSON_Print(response);
            send(client_socket, response_str, strlen(response_str), 0);

            free(response_str);
            cJSON_Delete(response);
        }
        else if (strcmp(type, "updata_dargon_postition") == 0 )
        {
            pos_dargon_x = cJSON_GetObjectItem(root, "x")->valuedouble;
            pos_dargon_y = cJSON_GetObjectItem(root, "y")->valuedouble;
        }
        else if (strcmp(type, "add_tower") == 0) 
        {
            std::lock_guard<std::mutex> lock(towers_mutex);

            cJSON* data = cJSON_GetObjectItem(root, "data");
            towers.push_back(cJSON_Duplicate(data, 1));
        }
        else if (strcmp(type, "upgrade_tower") == 0)
        {
            std::lock_guard<std::mutex> lock(towers_mutex);

            int id = cJSON_GetObjectItem(root, "id")->valueint;
            int level = cJSON_GetObjectItem(root, "level")->valueint;
            for (auto& tower : towers)
            {
                int now_id = cJSON_GetObjectItem(tower, "id")->valueint;
                if (now_id == id)
                {
                    cJSON* now_level = cJSON_GetObjectItem(tower, "level");
                    cJSON_SetNumberValue(now_level, level);
                    break;
                }
            }
        }
        else if (strcmp(type, "get_towers") == 0)
        {
            std::lock_guard<std::mutex> lock(towers_mutex);

            cJSON* response = cJSON_CreateObject();
            //添加数据类型为防御塔列表
            cJSON_AddStringToObject(response, "type", "tower_list");
            cJSON* tower_array = cJSON_CreateArray();
            
            for (auto& tower : towers) 
            {
                cJSON_AddItemToArray(tower_array, cJSON_Duplicate(tower, 1));
            }
            //添加一个数组
            cJSON_AddItemToObject(response, "data", tower_array);
            char* response_str = cJSON_Print(response);
            send(client_socket, response_str, strlen(response_str), 0);

            free(response_str);
            cJSON_Delete(response);
        }
        //清理内存
        cJSON_Delete(root);
    }
    //关闭套接字
    closesocket(client_socket);
}

int main()
{
    //初始化
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    //创建监听套接字 AF_INET：使用ipv4 ，tcp传输协议
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        std::cout << "监听套接字创建失败！" << std::endl;
        return 0;
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
        return 0;
    }
    //开始监听连接请求 最多允许5个客户端在等待连接
    if (listen(server_socket, 128) == -1)
    {
        std::cout << "监听失败！" << std::endl;
        return 0;
    }

    std::cout << "Server running on port 25565..." << std::endl;
    //使用多线程
    //阻塞并等待客户端连接 accept是一个会阻塞的函数
    while (clients.size() < MAX_CLIENT)
    {
        //变成通信套接字
        SOCKET client_socket = accept(server_socket, NULL, NULL);
        //存储客户端
        clients.push_back(client_socket);
        //对于每个客户端创建一个线程 实现并发
        std::thread(handle_client, client_socket).detach();
    }
    
    //玩家已经准备好 对所有玩家发布开启游戏的指令
    std::cout << "所有客户端已就绪，游戏即将开始！" << std::endl;
    const char* start_msg = "GAME_START";
    for (SOCKET client : clients) 
    {
        send(client, start_msg, strlen(start_msg), 0);
    }
    while (1); 

    closesocket(server_socket);
    WSACleanup();
    return 0;
}