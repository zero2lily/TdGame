#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include <SDL.h>
#include <stdio.h>

#if !SDL_VERSION_ATLEAST(2, 0, 17)
#error This backend requires SDL 2.0.17+ because of SDL_RenderGeometry() function
#endif

int main(int, char**)
{
    // ��ʼ�� SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // �������ں���Ⱦ��
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window* window = SDL_CreateWindow("SDL2 + ImGui + SDL_Renderer Example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr)
    {
        printf("Error: SDL_CreateRenderer(): %s\n", SDL_GetError());
        return -1;
    }

    // ��ʼ�� ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // ���ü��̿���

    // ���� ImGui ��ʽ
    ImGui::StyleColorsDark();

    // ��ʼ�� ImGui �� SDL2 �� SDL_Renderer ���
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // Ӧ�ó���״̬
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f); // ������ɫ
    bool show_demo_window = true; // �Ƿ���ʾ ImGui ��ʾ����
    bool draw_rectangle = true;   // �Ƿ���ƾ���
    bool draw_circle = false;     // �Ƿ����Բ��

    // ��ѭ��
    bool done = false;
    while (!done)
    {
        // �����¼�
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // ��ʼ ImGui ֡
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // 1. ��ʾ ImGui ��ʾ���ڣ���ѡ��
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. ����һ���Զ��� ImGui ����
        {
            ImGui::Begin("Control Panel"); // ��������
            ImGui::Text("Hello, world!"); // ��ʾ�ı�
            ImGui::Checkbox("Draw Rectangle", &draw_rectangle); // ��ѡ��
            ImGui::Checkbox("Draw Circle", &draw_circle);       // ��ѡ��
            ImGui::ColorEdit3("Background Color", (float*)&clear_color); // ��ɫѡ����
            ImGui::End();
        }

        // ��Ⱦ
        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(renderer);

        // ���ƾ���
        if (draw_rectangle)
        {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // ��ɫ
            SDL_Rect rect = { 100, 100, 200, 150 }; // ���ε�λ�úʹ�С
            SDL_RenderFillRect(renderer, &rect);
        }

        // ����Բ�Σ�ͨ������߶�ģ�⣩
        if (draw_circle)
        {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // ��ɫ
            const int center_x = 400, center_y = 300, radius = 100;
            for (int i = 0; i < 360; ++i)
            {
                float angle = i * 3.14159f / 180.0f;
                int x = center_x + (int)(radius * cosf(angle));
                int y = center_y + (int)(radius * sinf(angle));
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }

        // ��Ⱦ ImGui
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    // ����
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}