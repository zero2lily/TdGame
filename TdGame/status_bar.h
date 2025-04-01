#pragma once

#include "resources_manager.h"
#include "coin_manager.h"
#include "home_manager.h"
#include "player_manager.h"
#include "animation.h"

#include <SDL.h>
#include <string>
#include <SDL2_gfxPrimitives.h>

class StatusBar
{
public:
	StatusBar() = default;
	~StatusBar() = default;

	//血条 金币数 龙的技能时间都放在这个位置 以一个窗口为描点，这个动则所有的ui都会动
	void set_position(int x, int y)
	{
		position.x = x, position.y = y;
	}

	//金币数更新的时候需要renderer，则要传入renderer
	void on_update(SDL_Renderer* renderer)
	{
		static TTF_Font* font = ResourcesManager::instance()->get_font_pool().find(ResID::Font_Main)->second;

		//销毁上一帧的指针 防止内存泄漏
		SDL_DestroyTexture(tex_text_background);
		tex_text_background = nullptr;
		SDL_DestroyTexture(tex_text_foreground);
		tex_text_foreground = nullptr;
		
		//每帧获取金币数
		std::string str_val = std::to_string((int)CoinManager::instance()->get_current_coin_num());
		SDL_Surface* suf_text_backgorund = TTF_RenderText_Blended(font, str_val.c_str(), color_text_background);
		SDL_Surface* suf_text_foregorund = TTF_RenderText_Blended(font, str_val.c_str(), color_text_foreground);

		width_text = suf_text_backgorund->w, height_text = suf_text_backgorund->h;

		//创建纹理从Surface中
		tex_text_background = SDL_CreateTextureFromSurface(renderer, suf_text_backgorund);
		tex_text_foreground = SDL_CreateTextureFromSurface(renderer, suf_text_foregorund);
	
		//释放内存
		SDL_FreeSurface(suf_text_backgorund);
		SDL_FreeSurface(suf_text_foregorund);
	}

	void on_render(SDL_Renderer* renderer)
	{
		static SDL_Rect rect_dst;
		static const ResourcesManager::TexturePool& tex_pool = ResourcesManager::instance()->get_texture_pool();
		static SDL_Texture* tex_coin = tex_pool.find(ResID::Tex_UICoin)->second;
		static SDL_Texture* tex_heart = tex_pool.find(ResID::Tex_UIHeart)->second;
		static SDL_Texture* tex_home_avatar = tex_pool.find(ResID::Tex_UIHomeAvatar)->second;
		static SDL_Texture* tex_player_avatar = tex_pool.find(ResID::Tex_UIPlayerAvatar)->second;

		rect_dst.x = position.x, rect_dst.y = position.y;
		rect_dst.w = 78, rect_dst.h = 78;
		SDL_RenderCopy(renderer, tex_home_avatar, nullptr, &rect_dst);

		for (int i = 0; i < (int)HomeManager::instance()->get_current_hp_num(); i++)
		{
			rect_dst.x = position.x + 78 + 15 + i * (32 + 2);
			rect_dst.y = position.y;
			rect_dst.w = 32, rect_dst.h = 32;
			SDL_RenderCopy(renderer, tex_heart, nullptr, &rect_dst);
		}

		rect_dst.x = position.x + 78 + 15;
		rect_dst.y = position.y + 78 - 32;
		rect_dst.w = 32, rect_dst.h = 32;
		SDL_RenderCopy(renderer, tex_coin, nullptr, &rect_dst);

		rect_dst.x += 32 + 10 + offset_shadow.x;
		rect_dst.y = rect_dst.y + (32 - height_text) / 2 + offset_shadow.y;
		rect_dst.w = width_text, rect_dst.h = height_text;
		SDL_RenderCopy(renderer, tex_text_background, nullptr, &rect_dst);

		rect_dst.x -= offset_shadow.x;
		rect_dst.y -= offset_shadow.y;
		SDL_RenderCopy(renderer, tex_text_foreground, nullptr, &rect_dst);

		rect_dst.x = position.x + (78 - 65) / 2;
		rect_dst.y = position.y + 78 + 5;
		rect_dst.w = 65, rect_dst.h = 65;
		SDL_RenderCopy(renderer, tex_player_avatar, nullptr, &rect_dst);

		rect_dst.x = position.x + 78 + 15;
		rect_dst.y += 10;
		roundedBoxRGBA(renderer, rect_dst.x, rect_dst.y, rect_dst.x + width_mp_bar, rect_dst.y + height_mp_bar, 4,
			color_mp_bar_background.r, color_mp_bar_background.g, color_mp_bar_background.b, color_mp_bar_background.a);

		rect_dst.x += width_border_mp_bar;
		rect_dst.y += width_border_mp_bar;
		rect_dst.w = width_mp_bar - 2 * width_border_mp_bar;
		rect_dst.h = height_mp_bar - 2 * width_border_mp_bar;
		double process = PlayerManager::instance()->get_current_mp() / 100;
		roundedBoxRGBA(renderer, rect_dst.x, rect_dst.y, rect_dst.x + (rect_dst.w * process), rect_dst.y + rect_dst.h, 2,
			color_mp_bar_foredground.r, color_mp_bar_foredground.g, color_mp_bar_foredground.b, color_mp_bar_foredground.a);
	}

private:
	const int size_heart = 32;                                     //心心的size
	const int width_mp_bar = 200;                                  //龙技能条的宽高
	const int height_mp_bar = 20;
	const int width_border_mp_bar = 4;                             //龙技能条的边框宽度
	const SDL_Point offset_shadow = { 2,2 };                       //字体偏移阴影的大小
	const SDL_Color color_text_background = { 175,175,175,255 };
	const SDL_Color color_text_foreground = { 255,255,255,255 };
	const SDL_Color color_mp_bar_background = { 48,40,51,255 };
	const SDL_Color color_mp_bar_foredground = { 144,121,173,255 };

private:
	SDL_Point position = { 0 };                      //窗口位置
	int width_text = 0, height_text = 0;              //金币数字的宽度和高度
	SDL_Texture* tex_text_background = nullptr;      //金币文字渲染的阴影纹理
	SDL_Texture* tex_text_foreground = nullptr;      //金币文字渲染的字体纹理

};

