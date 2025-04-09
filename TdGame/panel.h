#pragma once

#include "resources_manager.h"
#include "tile.h"
#include "client_manager.h"
#include "break_manager.h"

#include <SDL.h>
#include <string>

class Panel
{
public:
	Panel()
	{
		tex_select_cursor = ResourcesManager::instance()->get_texture_pool().find(ResID::Tex_UISelectCursor)->second;
	}

	~Panel()
	{
		SDL_DestroyTexture(tex_text_background);
		SDL_DestroyTexture(tex_text_foreground);
	}

	void set_idx_tile(const SDL_Point& idx)
	{
		idx_tile_selected = idx;
	}

	void set_center_pos(const SDL_Point& center_pos)
	{
		this->center_pos = center_pos;
	}

	void set_target_tower(Tower*& target_tower)
	{
		this->target_tower = target_tower;
	}

	void set_is_can_click(bool flag)
	{
		is_can_click = flag;
	}

	virtual void show()
	{
		visible = true;
	}

	virtual void on_input(const SDL_Event& event)
	{
		if (!visible)
			return;
		
		if (is_can_click)
		{
			switch (event.type)
			{
			case SDL_MOUSEMOTION:
			{
				//pos_cursor；放的是鼠标的位置
				SDL_Point pos_cursor = { event.motion.x,event.motion.y };
				SDL_Rect rect_target = { 0,0,size_button,size_button };

				//鼠标在顶部的按钮位置 
				rect_target.x = center_pos.x - width / 2 + offset_top.x;
				rect_target.y = center_pos.y - height / 2 + offset_top.y;
				last_hovered_target = hovered_target;
				if (SDL_PointInRect(&pos_cursor, &rect_target))
				{
					hovered_target = HoveredTarget::Top;
					play_mouse_overed_icon();
					return;
				}

				//鼠标在左边的按钮位置 
				rect_target.x = center_pos.x - width / 2 + offset_left.x;
				rect_target.y = center_pos.y - height / 2 + offset_left.y;
				if (SDL_PointInRect(&pos_cursor, &rect_target))
				{
					hovered_target = HoveredTarget::Left;
					play_mouse_overed_icon();
					return;
				}

				//鼠标在右边的按钮位置 
				rect_target.x = center_pos.x - width / 2 + offset_right.x;
				rect_target.y = center_pos.y - height / 2 + offset_right.y;
				if (SDL_PointInRect(&pos_cursor, &rect_target))
				{
					hovered_target = HoveredTarget::Right;
					play_mouse_overed_icon();
					return;
				}

				hovered_target = HoveredTarget::None;
			}
			break;
			case SDL_MOUSEBUTTONUP:
			{
				//当鼠标抬起时 触发对应的点击事件
				switch (hovered_target)
				{
				case Panel::HoveredTarget::Top:
					on_click_top_area();
					break;
				case Panel::HoveredTarget::Left:
					on_click_left_area();
					break;
				case Panel::HoveredTarget::Right:
					on_click_right_area();
					break;
				}
				//无论是否按到按钮都将轮盘转为不可视的
				visible = false;
			}
			break;
			default:
				break;
			}
		}
		else
		{
			switch (event.type)
			{
			case SDL_MOUSEBUTTONUP:
			{
				visible = false;
				break;
			}
			default:
				break;
			}
		}

	}

	virtual void on_update(SDL_Renderer* renderer)
	{
		static TTF_Font* font = ResourcesManager::instance()->get_font_pool().find(ResID::Font_Main)->second;
		static TowerManager* instance = TowerManager::instance();

		//先销毁上一帧的纹理内存
		SDL_DestroyTexture(tex_text_background);
		SDL_DestroyTexture(tex_text_foreground);
		SDL_DestroyTexture(tex_level_text_background);
		SDL_DestroyTexture(tex_level_text_foreground);
		tex_text_background = nullptr;
		tex_text_foreground = nullptr;
		tex_level_text_background = nullptr;
		tex_level_text_foreground = nullptr;

		std::string str_val = val < 0 ? "MAX" : std::to_string(val);
		SDL_Surface* suf_text_background = TTF_RenderText_Blended(font, str_val.c_str(), color_text_background);
		SDL_Surface* suf_text_foreground = TTF_RenderText_Blended(font, str_val.c_str(), color_text_foreground);

		std::string str_level = level < 0 ? "level:10" : "level:" + std::to_string(level);
		SDL_Surface* suf_level_text_background = TTF_RenderText_Blended(font, str_level.c_str(), color_text_background);
		SDL_Surface* suf_level_text_foreground = TTF_RenderText_Blended(font, str_level.c_str(), color_text_foreground);

		//设置金币数字宽高和纹理
		width_text = suf_text_background->w, height_text = suf_text_background->h;
		tex_text_background = SDL_CreateTextureFromSurface(renderer, suf_text_background);
		tex_text_foreground = SDL_CreateTextureFromSurface(renderer, suf_text_foreground);

		//设置等级数字宽高和纹理
		width_level_text = suf_level_text_background->w, height_level_text = suf_level_text_background->h;
		tex_level_text_background = SDL_CreateTextureFromSurface(renderer, suf_level_text_background);
		tex_level_text_foreground = SDL_CreateTextureFromSurface(renderer, suf_level_text_foreground);

		//释放内存
		SDL_FreeSurface(suf_text_background);
		SDL_FreeSurface(suf_text_foreground);
		SDL_FreeSurface(suf_level_text_background);
		SDL_FreeSurface(suf_level_text_foreground);

		//如果当前鼠标没有悬浮到任何的按钮上 则直接退出
		if (hovered_target == HoveredTarget::None)
			return;
	}

	virtual void on_render(SDL_Renderer* renderer)
	{
		//如果panel不可视则直接返回
		if (!visible)
			return;

		//先渲染选中图标 选中图标就是一个像素格子
		SDL_Rect rect_dst_cursor =
		{
			center_pos.x - SIZE_TILE / 2,
			center_pos.y - SIZE_TILE / 2,
			SIZE_TILE,
			SIZE_TILE
		};
		SDL_RenderCopy(renderer, tex_select_cursor, nullptr, &rect_dst_cursor);

		SDL_Rect rect_dst_text;
		//讲等级显示在金币的正下方
		rect_dst_text.x = center_pos.x - width_level_text / 2 + offset_shadow.x;
		rect_dst_text.y = center_pos.y + height / 2 + height_text + offset_shadow.y;
		rect_dst_text.w = width_level_text, rect_dst_text.h = height_level_text;
		SDL_RenderCopy(renderer, tex_level_text_background, nullptr, &rect_dst_text);

		rect_dst_text.x -= offset_shadow.x;
		rect_dst_text.y -= offset_shadow.y;
		SDL_RenderCopy(renderer, tex_level_text_foreground, nullptr, &rect_dst_text);

		//如果当前鼠标没有悬浮到任何的按钮上 则不用显示金币
		if (hovered_target == HoveredTarget::None)
			return;

		//将金币显示在轮盘的正下方
		rect_dst_text.x = center_pos.x - width_text / 2 + offset_shadow.x;
		rect_dst_text.y = center_pos.y + height / 2 + offset_shadow.y;
		rect_dst_text.w = width_text, rect_dst_text.h = height_text;
		SDL_RenderCopy(renderer, tex_text_background, nullptr, &rect_dst_text);

		rect_dst_text.x -=  offset_shadow.x;
		rect_dst_text.y -=  offset_shadow.y;
		SDL_RenderCopy(renderer, tex_text_foreground, nullptr, &rect_dst_text);

	}

public:
	const SDL_Color color_region_frame = { 30,80,162,55 };    //防御塔攻击范围颜色
	const SDL_Color color_region_content = { 0,149,217,55 };  //防御塔攻击范围边框颜色

protected:
	enum class HoveredTarget
	{
		None,
		Top,
		Left,
		Right
	};

protected:
	bool visible = false;                                               //panel是否可见
	SDL_Point idx_tile_selected;                                        //点击了哪一个单元格                              
	Tower* target_tower = nullptr;                                      //用来表示这个升级panel指向的哪个防御塔
	int reg_top = 0, reg_left = 0, reg_right = 0;                       //防御塔攻击范围
	const int size_button = 48;                                         //panel上面的选择防御塔按钮为48*48的
	const int width = 144, height = 144;                                //panel大小为144*144
	const SDL_Point offset_top = { 48,6 };                              //panel上面防御塔按钮的左上角位置
	const SDL_Point offset_left = { 8,80 };                             //panel左边防御塔按钮的左上角位置
	const SDL_Point offset_right = { 90,80 };                           //panel左边防御塔按钮的左上角位置

	SDL_Point center_pos = { 0 };                                       //panel中心地图的位置
	SDL_Texture* tex_idle = nullptr;                                    //鼠标没悬浮panel上的panel默认状态纹理
	SDL_Texture* tex_hovered_top = nullptr;                             //鼠标悬浮panel上的顶部按钮panel默认状态纹理
	SDL_Texture* tex_hovered_left = nullptr;                            //鼠标悬浮panel上的左下按钮panel默认状态纹理
	SDL_Texture* tex_hovered_right = nullptr;                           //鼠标悬浮panel上的友下按钮panel默认状态纹理
	SDL_Texture* tex_select_cursor = nullptr;                           //选中地图的纹理
	int val_top = 0, val_left = 0, val_right = 0;                       //要建造或者升级的金币价格
	int val = 0;                                                        //应该显示的金币数字
	int level = 0;                                                      //显示的等级
	HoveredTarget hovered_target = HoveredTarget::None;                 //选择了哪个按钮
	HoveredTarget last_hovered_target = HoveredTarget::None;            //上一帧选择了哪个按钮

	bool is_can_click = false;                                          //轮盘按钮是否能点击

protected:
	//子类重写点击事件
	virtual void on_click_top_area() = 0;
	virtual void on_click_left_area() = 0;
	virtual void on_click_right_area() = 0;

private:
	const SDL_Point offset_shadow = { 3,3 };                            //panel上金币的阴影的偏差
	const SDL_Color color_text_background = { 175,175,175,255 };        //金币阴影颜色
	const SDL_Color color_text_foreground = { 255,255,255,255 };        //金币数字颜色
	
	int width_text = 0, height_text = 0;                                //金币数字宽高
	SDL_Texture* tex_text_background = nullptr;                         //金币数字阴影纹理
	SDL_Texture* tex_text_foreground = nullptr;                         //金币数字阴影纹理

	int width_level_text = 0, height_level_text = 0;                    //等级数字宽高
	SDL_Texture* tex_level_text_background = nullptr;                   //等级数字阴影纹理
	SDL_Texture* tex_level_text_foreground = nullptr;                   //等级数字阴影纹理

private:

	void play_mouse_overed_icon()
	{
		if (last_hovered_target == HoveredTarget::None)
		{
			static ResourcesManager::SoundPool sound_pool = ResourcesManager::instance()->get_sound_pool();
			Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_TowerMouseOveredIcon)->second, 0);
		}
		
	}
};
