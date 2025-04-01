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
    // 初始化 SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // 创建窗口和渲染器
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

    // 初始化 ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // 启用键盘控制

    // 设置 ImGui 样式
    ImGui::StyleColorsDark();

    // 初始化 ImGui 的 SDL2 和 SDL_Renderer 后端
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    // 应用程序状态
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f); // 背景颜色
    bool show_demo_window = true; // 是否显示 ImGui 演示窗口
    bool draw_rectangle = true;   // 是否绘制矩形
    bool draw_circle = false;     // 是否绘制圆形

    // 主循环
    bool done = false;
    while (!done)
    {
        // 处理事件
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // 开始 ImGui 帧
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // 1. 显示 ImGui 演示窗口（可选）
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. 创建一个自定义 ImGui 窗口
        {
            ImGui::Begin("Control Panel"); // 创建窗口
            ImGui::Text("Hello, world!"); // 显示文本
            ImGui::Checkbox("Draw Rectangle", &draw_rectangle); // 复选框
            ImGui::Checkbox("Draw Circle", &draw_circle);       // 复选框
            ImGui::ColorEdit3("Background Color", (float*)&clear_color); // 颜色选择器
            ImGui::End();
        }

        // 渲染
        ImGui::Render();
        SDL_RenderSetScale(renderer, io.DisplayFramebufferScale.x, io.DisplayFramebufferScale.y);
        SDL_SetRenderDrawColor(renderer, (Uint8)(clear_color.x * 255), (Uint8)(clear_color.y * 255), (Uint8)(clear_color.z * 255), (Uint8)(clear_color.w * 255));
        SDL_RenderClear(renderer);

        // 绘制矩形
        if (draw_rectangle)
        {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // 红色
            SDL_Rect rect = { 100, 100, 200, 150 }; // 矩形的位置和大小
            SDL_RenderFillRect(renderer, &rect);
        }

        // 绘制圆形（通过多个线段模拟）
        if (draw_circle)
        {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255); // 绿色
            const int center_x = 400, center_y = 300, radius = 100;
            for (int i = 0; i < 360; ++i)
            {
                float angle = i * 3.14159f / 180.0f;
                int x = center_x + (int)(radius * cosf(angle));
                int y = center_y + (int)(radius * sinf(angle));
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }

        // 渲染 ImGui
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    // 清理
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}