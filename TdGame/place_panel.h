#pragma once

#include "panel.h"
#include "resources_manager.h"
#include "tower_manager.h"
#include "coin_manager.h"

#include <SDL2_gfxPrimitives.h>

//轮盘的放置
class PlacePanel : public Panel
{
public:
	PlacePanel()
	{
		
		const ResourcesManager::TexturePool& tex_pool =
			ResourcesManager::instance()->get_texture_pool();

		tex_idle = tex_pool.find(ResID::Tex_UIPlaceIdle)->second;
		tex_hovered_top = tex_pool.find(ResID::Tex_UIPlaceHoveredTop)->second;
		tex_hovered_left = tex_pool.find(ResID::Tex_UIPlaceHoveredLeft)->second;
		tex_hovered_right = tex_pool.find(ResID::Tex_UIPlaceHoveredRight)->second;
	}

	~PlacePanel() = default;

	void show() override
	{
		if (!is_can_click)
			return;
		visible = true;
	}

	void on_update(SDL_Renderer* renderer) override
	{
		static TowerManager* instance = TowerManager::instance();

		//动态更新放置的金币
		val_top = instance->get_place_cost(TowerType::Axeman);
		val_left = instance->get_place_cost(TowerType::Archer);
		val_right = instance->get_place_cost(TowerType::Gunner);

		//动态更新放置的角色范围
		reg_top = instance->get_damage_range(TowerType::Axeman, target_tower) * SIZE_TILE;
		reg_left = instance->get_damage_range(TowerType::Archer, target_tower) * SIZE_TILE;
		reg_right = instance->get_damage_range(TowerType::Gunner, target_tower) * SIZE_TILE;

		//获得应该显示的金币
		val = -1;
		switch (hovered_target)
		{
		case Panel::HoveredTarget::Top:
			val = val_top;
			break;
		case Panel::HoveredTarget::Left:
			val = val_left;
			break;
		case Panel::HoveredTarget::Right:
			val = val_right;
			break;
		}
		level = 0;

		Panel::on_update(renderer);
	}

	void on_render(SDL_Renderer* renderer) override
	{
		if (!visible)
			return;

		int reg = 0;
		switch (hovered_target)
		{
		case Panel::HoveredTarget::Top:
			reg = reg_top;
			break;
		case Panel::HoveredTarget::Left:
			reg = reg_left;
			break;
		case Panel::HoveredTarget::Right:
			reg = reg_right;
			break;
		}
		
		if (reg > 0)
		{
			filledCircleRGBA(renderer, center_pos.x, center_pos.y, reg,
				color_region_content.r, color_region_content.g, color_region_content.b, color_region_content.a);
			aacircleRGBA(renderer, center_pos.x, center_pos.y, reg,
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
		switch (hovered_target)
		{
		case Panel::HoveredTarget::None:
			tex_panel = tex_idle;
			break;
		case Panel::HoveredTarget::Top:
			tex_panel = tex_hovered_top;
			break;
		case Panel::HoveredTarget::Left:
			tex_panel = tex_hovered_left;
			break;
		case Panel::HoveredTarget::Right:
			tex_panel = tex_hovered_right;
			break;
		}

		SDL_RenderCopy(renderer, tex_panel, nullptr, &rect_dst_panel);

		Panel::on_render(renderer);
	}

protected:
	virtual void on_click_top_area() override
	{
		CoinManager* instance = CoinManager::instance();

		if (val_top <= instance->get_current_coin_num())
		{
			//TowerManager::instance()->place_tower(TowerType::Axeman, idx_tile_selected, 0, true);
			ClientManager::instance()->place_tower_cmd(TowerType::Axeman, idx_tile_selected);
			BreakManager::instance()->decrease_coin_cmd(val_top);
			//instance->decrease_coin(val_top);
		}
	}

	virtual void on_click_left_area() override
	{
		CoinManager* instance = CoinManager::instance();

		if (val_left <= instance->get_current_coin_num())
		{
			//TowerManager::instance()->place_tower(TowerType::Archer, idx_tile_selected, 0, true);
			ClientManager::instance()->place_tower_cmd(TowerType::Archer, idx_tile_selected);
			BreakManager::instance()->decrease_coin_cmd(val_left);
			//instance->decrease_coin(val_left);
		}
	}

	virtual void on_click_right_area() override
	{
		CoinManager* instance = CoinManager::instance();

		if (val_right <= instance->get_current_coin_num())
		{
			//TowerManager::instance()->place_tower(TowerType::Gunner, idx_tile_selected, 0, true);
			ClientManager::instance()->place_tower_cmd(TowerType::Gunner, idx_tile_selected);
			BreakManager::instance()->decrease_coin_cmd(val_right);
			//instance->decrease_coin(val_right);
		}
	}
};

