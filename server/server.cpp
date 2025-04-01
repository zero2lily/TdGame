#include "cJSON.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <vector>
#include <mutex>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

#define MAX_CLIENT 2

std::vector<cJSON*> towers;    //���еķ�������Ϣ
std::vector<SOCKET> clients;   //���еĿͻ���
std::mutex towers_mutex, login_mutex, is_start_game;
bool is_login_player1 = false; //���һ�Ƿ��¼
bool is_login_player2 = false; //��Ҷ��Ƿ��¼
double pos_dargon_x = 0;       //����xλ��
double pos_dargon_y = 0;       //����yλ��

void handle_client(SOCKET client_socket)
{
    char buffer[4096];
    int bytes_received;

    //bytes_received����0˵�����ݽ��ճɹ�
    //recvҲ����������
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
            //��������
            char* response_str = cJSON_Print(response);
            send(client_socket, response_str, strlen(response_str), 0);
            //�ͷ��ڴ�
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
            //�����������Ϊ�������б�
            cJSON_AddStringToObject(response, "type", "tower_list");
            cJSON* tower_array = cJSON_CreateArray();
            
            for (auto& tower : towers) 
            {
                cJSON_AddItemToArray(tower_array, cJSON_Duplicate(tower, 1));
            }
            //���һ������
            cJSON_AddItemToObject(response, "data", tower_array);
            char* response_str = cJSON_Print(response);
            send(client_socket, response_str, strlen(response_str), 0);

            free(response_str);
            cJSON_Delete(response);
        }
        //�����ڴ�
        cJSON_Delete(root);
    }
    //�ر��׽���
    closesocket(client_socket);
}

int main()
{
    //��ʼ��
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    //���������׽��� AF_INET��ʹ��ipv4 ��tcp����Э��
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1)
    {
        std::cout << "�����׽��ִ���ʧ�ܣ�" << std::endl;
        return 0;
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
        return 0;
    }
    //��ʼ������������ �������5���ͻ����ڵȴ�����
    if (listen(server_socket, 128) == -1)
    {
        std::cout << "����ʧ�ܣ�" << std::endl;
        return 0;
    }

    std::cout << "Server running on port 25565..." << std::endl;
    //ʹ�ö��߳�
    //�������ȴ��ͻ������� accept��һ���������ĺ���
    while (clients.size() < MAX_CLIENT)
    {
        //���ͨ���׽���
        SOCKET client_socket = accept(server_socket, NULL, NULL);
        //�洢�ͻ���
        clients.push_back(client_socket);
        //����ÿ���ͻ��˴���һ���߳� ʵ�ֲ���
        std::thread(handle_client, client_socket).detach();
    }
    
    //����Ѿ�׼���� ��������ҷ���������Ϸ��ָ��
    std::cout << "���пͻ����Ѿ�������Ϸ������ʼ��" << std::endl;
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