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
				//pos_cursor���ŵ�������λ��
				SDL_Point pos_cursor = { event.motion.x,event.motion.y };
				SDL_Rect rect_target = { 0,0,size_button,size_button };

				//����ڶ����İ�ťλ�� 
				rect_target.x = center_pos.x - width / 2 + offset_top.x;
				rect_target.y = center_pos.y - height / 2 + offset_top.y;
				last_hovered_target = hovered_target;
				if (SDL_PointInRect(&pos_cursor, &rect_target))
				{
					hovered_target = HoveredTarget::Top;
					play_mouse_overed_icon();
					return;
				}

				//�������ߵİ�ťλ�� 
				rect_target.x = center_pos.x - width / 2 + offset_left.x;
				rect_target.y = center_pos.y - height / 2 + offset_left.y;
				if (SDL_PointInRect(&pos_cursor, &rect_target))
				{
					hovered_target = HoveredTarget::Left;
					play_mouse_overed_icon();
					return;
				}

				//������ұߵİ�ťλ�� 
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
				//�����̧��ʱ ������Ӧ�ĵ���¼�
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
				//�����Ƿ񰴵���ť��������תΪ�����ӵ�
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

		//��������һ֡�������ڴ�
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

		//���ý�����ֿ�ߺ�����
		width_text = suf_text_background->w, height_text = suf_text_background->h;
		tex_text_background = SDL_CreateTextureFromSurface(renderer, suf_text_background);
		tex_text_foreground = SDL_CreateTextureFromSurface(renderer, suf_text_foreground);

		//���õȼ����ֿ�ߺ�����
		width_level_text = suf_level_text_background->w, height_level_text = suf_level_text_background->h;
		tex_level_text_background = SDL_CreateTextureFromSurface(renderer, suf_level_text_background);
		tex_level_text_foreground = SDL_CreateTextureFromSurface(renderer, suf_level_text_foreground);

		//�ͷ��ڴ�
		SDL_FreeSurface(suf_text_background);
		SDL_FreeSurface(suf_text_foreground);
		SDL_FreeSurface(suf_level_text_background);
		SDL_FreeSurface(suf_level_text_foreground);

		//�����ǰ���û���������κεİ�ť�� ��ֱ���˳�
		if (hovered_target == HoveredTarget::None)
			return;
	}

	virtual void on_render(SDL_Renderer* renderer)
	{
		//���panel��������ֱ�ӷ���
		if (!visible)
			return;

		//����Ⱦѡ��ͼ�� ѡ��ͼ�����һ�����ظ���
		SDL_Rect rect_dst_cursor =
		{
			center_pos.x - SIZE_TILE / 2,
			center_pos.y - SIZE_TILE / 2,
			SIZE_TILE,
			SIZE_TILE
		};
		SDL_RenderCopy(renderer, tex_select_cursor, nullptr, &rect_dst_cursor);

		SDL_Rect rect_dst_text;
		//���ȼ���ʾ�ڽ�ҵ����·�
		rect_dst_text.x = center_pos.x - width_level_text / 2 + offset_shadow.x;
		rect_dst_text.y = center_pos.y + height / 2 + height_text + offset_shadow.y;
		rect_dst_text.w = width_level_text, rect_dst_text.h = height_level_text;
		SDL_RenderCopy(renderer, tex_level_text_background, nullptr, &rect_dst_text);

		rect_dst_text.x -= offset_shadow.x;
		rect_dst_text.y -= offset_shadow.y;
		SDL_RenderCopy(renderer, tex_level_text_foreground, nullptr, &rect_dst_text);

		//�����ǰ���û���������κεİ�ť�� ������ʾ���
		if (hovered_target == HoveredTarget::None)
			return;

		//�������ʾ�����̵����·�
		rect_dst_text.x = center_pos.x - width_text / 2 + offset_shadow.x;
		rect_dst_text.y = center_pos.y + height / 2 + offset_shadow.y;
		rect_dst_text.w = width_text, rect_dst_text.h = height_text;
		SDL_RenderCopy(renderer, tex_text_background, nullptr, &rect_dst_text);

		rect_dst_text.x -=  offset_shadow.x;
		rect_dst_text.y -=  offset_shadow.y;
		SDL_RenderCopy(renderer, tex_text_foreground, nullptr, &rect_dst_text);

	}

public:
	const SDL_Color color_region_frame = { 30,80,162,55 };    //������������Χ��ɫ
	const SDL_Color color_region_content = { 0,149,217,55 };  //������������Χ�߿���ɫ

protected:
	enum class HoveredTarget
	{
		None,
		Top,
		Left,
		Right
	};

protected:
	bool visible = false;                                               //panel�Ƿ�ɼ�
	SDL_Point idx_tile_selected;                                        //�������һ����Ԫ��                              
	Tower* target_tower = nullptr;                                      //������ʾ�������panelָ����ĸ�������
	int reg_top = 0, reg_left = 0, reg_right = 0;                       //������������Χ
	const int size_button = 48;                                         //panel�����ѡ���������ťΪ48*48��
	const int width = 144, height = 144;                                //panel��СΪ144*144
	const SDL_Point offset_top = { 48,6 };                              //panel�����������ť�����Ͻ�λ��
	const SDL_Point offset_left = { 8,80 };                             //panel��߷�������ť�����Ͻ�λ��
	const SDL_Point offset_right = { 90,80 };                           //panel��߷�������ť�����Ͻ�λ��

	SDL_Point center_pos = { 0 };                                       //panel���ĵ�ͼ��λ��
	SDL_Texture* tex_idle = nullptr;                                    //���û����panel�ϵ�panelĬ��״̬����
	SDL_Texture* tex_hovered_top = nullptr;                             //�������panel�ϵĶ�����ťpanelĬ��״̬����
	SDL_Texture* tex_hovered_left = nullptr;                            //�������panel�ϵ����°�ťpanelĬ��״̬����
	SDL_Texture* tex_hovered_right = nullptr;                           //�������panel�ϵ����°�ťpanelĬ��״̬����
	SDL_Texture* tex_select_cursor = nullptr;                           //ѡ�е�ͼ������
	int val_top = 0, val_left = 0, val_right = 0;                       //Ҫ������������Ľ�Ҽ۸�
	int val = 0;                                                        //Ӧ����ʾ�Ľ������
	int level = 0;                                                      //��ʾ�ĵȼ�
	HoveredTarget hovered_target = HoveredTarget::None;                 //ѡ�����ĸ���ť
	HoveredTarget last_hovered_target = HoveredTarget::None;            //��һ֡ѡ�����ĸ���ť

	bool is_can_click = false;                                          //���̰�ť�Ƿ��ܵ��

protected:
	//������д����¼�
	virtual void on_click_top_area() = 0;
	virtual void on_click_left_area() = 0;
	virtual void on_click_right_area() = 0;

private:
	const SDL_Point offset_shadow = { 3,3 };                            //panel�Ͻ�ҵ���Ӱ��ƫ��
	const SDL_Color color_text_background = { 175,175,175,255 };        //�����Ӱ��ɫ
	const SDL_Color color_text_foreground = { 255,255,255,255 };        //���������ɫ
	
	int width_text = 0, height_text = 0;                                //������ֿ��
	SDL_Texture* tex_text_background = nullptr;                         //���������Ӱ����
	SDL_Texture* tex_text_foreground = nullptr;                         //���������Ӱ����

	int width_level_text = 0, height_level_text = 0;                    //�ȼ����ֿ��
	SDL_Texture* tex_level_text_background = nullptr;                   //�ȼ�������Ӱ����
	SDL_Texture* tex_level_text_foreground = nullptr;                   //�ȼ�������Ӱ����

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
