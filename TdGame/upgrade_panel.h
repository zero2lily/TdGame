#pragma once

#include "panel.h"
#include "resources_manager.h"
#include "tower_manager.h"

class UpgradePanel : public Panel
{
public:
	UpgradePanel()
	{
		const ResourcesManager::TexturePool& tex_pool =
			ResourcesManager::instance()->get_texture_pool();

		tex_idle = tex_pool.find(ResID::Tex_UIUpgradeIdle)->second;
		tex_hovered_top = tex_pool.find(ResID::Tex_UIUpgradeHoveredTop)->second;
		tex_hovered_left = tex_pool.find(ResID::Tex_UIUpgradeHoveredLeft)->second;
		tex_hovered_right = tex_pool.find(ResID::Tex_UIUpgradeHoveredRight)->second;
		tex_axeman = tex_pool.find(ResID::Tex_UIUpgradeAxeman)->second;
		tex_archer = tex_pool.find(ResID::Tex_UIUpgradeArcher)->second;
		tex_gunner = tex_pool.find(ResID::Tex_UIUpgradeGunner)->second;
		tex_hovered_axeman = tex_pool.find(ResID::Tex_UIUpgradeHoveredAxeman)->second;
		tex_hovered_archer = tex_pool.find(ResID::Tex_UIUpgradeHoveredArcher)->second;
		tex_hovered_gunner = tex_pool.find(ResID::Tex_UIUpgradeHoveredGunner)->second;
	}
	~UpgradePanel() = default;

	virtual void on_input(const SDL_Event& event) override
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
					if (last_hovered_target == HoveredTarget::None)
					{
						static ResourcesManager::SoundPool sound_pool = ResourcesManager::instance()->get_sound_pool();
						Mix_PlayChannel(-1, sound_pool.find(ResID::Sound_TowerMouseOveredIcon)->second, 0);
					}

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

	virtual void on_update(SDL_Renderer* renderer) override
	{
		static TowerManager* instance = TowerManager::instance();

		//动态更新放置的对应防御塔范围
		if (target_tower != nullptr)
		{
			reg_top = instance->get_damage_range(target_tower->get_tower_type(), target_tower) * SIZE_TILE;
			upgrade_reg_top = instance->get_next_grade_damage_range(target_tower->get_tower_type(), target_tower) * SIZE_TILE;
			level = instance->get_target_tower_level(target_tower);

			val = -1;
			val_top = instance->get_upgrade_cost(target_tower->get_tower_type(), target_tower);
			val = hovered_target == HoveredTarget::Top ? val_top : -1;
		}

		Panel::on_update(renderer);
	}

	virtual void on_render(SDL_Renderer* renderer) override
	{
		if (!visible)
			return;

		//下个等级的范围
		int next_reg = hovered_target == Panel::HoveredTarget::Top ? upgrade_reg_top : 0;

		//根据防御塔的类型画出对应的范围
		TowerType type = target_tower->get_tower_type();
		filledCircleRGBA(renderer, center_pos.x, center_pos.y,reg_top,
			color_region_content.r, color_region_content.g, color_region_content.b, color_region_content.a);
		aacircleRGBA(renderer, center_pos.x, center_pos.y, reg_top,
			color_region_frame.r, color_region_frame.g, color_region_frame.b, color_region_frame.a);
	
		//画出下一个等级的范围
		if (next_reg > 0)
		{
			filledCircleRGBA(renderer, center_pos.x, center_pos.y, next_reg,
				color_region_content.r, color_region_content.g, color_region_content.b, color_region_content.a);
			aacircleRGBA(renderer, center_pos.x, center_pos.y, next_reg,
				color_region_frame.r, color_region_frame.g, color_region_frame.b, color_region_frame.a);
			
		}

		//选择应该渲染哪个panel
		SDL_Rect rect_dst_panel =
		{
			center_pos.x - width / 2,
			center_pos.y - height / 2,
			width,
			height
		};

		SDL_Texture* tex_panel = nullptr;
		switch (target_tower->get_tower_type())
		{
		case TowerType::Axeman:
			tex_panel = hovered_target == Panel::HoveredTarget::Top ? tex_hovered_axeman : tex_axeman;
			break;
		case TowerType::Archer:
			tex_panel = hovered_target == Panel::HoveredTarget::Top ? tex_hovered_archer : tex_archer;
			break;
		case TowerType::Gunner:
			tex_panel = hovered_target == Panel::HoveredTarget::Top ? tex_hovered_gunner : tex_gunner;
			break;
		}
		SDL_RenderCopy(renderer, tex_panel, nullptr, &rect_dst_panel);

		Panel::on_render(renderer);
	}

	void set_target_tower(Tower*& target_tower)
	{
		this->target_tower = target_tower;
	}

protected:
	virtual void on_click_top_area() override
	{
		CoinManager* instance = CoinManager::instance();

		if (val_top > 0 &&  val_top <= instance->get_current_coin_num())
		{
			BreakManager::instance()->upgrade_tower_cmd(target_tower->get_tower_type(), idx_tile_selected);
			//TowerManager::instance()->upgrade_tower(target_tower,true);
			//instance->decrease_coin(val_top);
			BreakManager::instance()->decrease_coin_cmd(val_top);
		}
	}

	virtual void on_click_left_area() override
	{
		
	}

	virtual void on_click_right_area() override
	{

	}

private:
	int upgrade_reg_top = 0, upgrade_reg_left = 0, upgrade_reg_right = 0;;  //下一个等级的防御塔攻击范围
	SDL_Texture* tex_axeman = nullptr;                                      //升级axeman轮盘
	SDL_Texture* tex_archer = nullptr;                                      //升级axeman轮盘
	SDL_Texture* tex_gunner = nullptr;                                      //升级axeman轮盘
	SDL_Texture* tex_hovered_axeman = nullptr;                              //升级axeman轮盘
	SDL_Texture* tex_hovered_archer = nullptr;                              //升级axeman轮盘
	SDL_Texture* tex_hovered_gunner = nullptr;                              //升级axeman轮盘
};
