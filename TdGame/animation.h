#pragma once

#include "timer.h"

#include <SDL.h>
#include <vector>
#include <functional>

class Animation
{
public:
	typedef std::function<void()> PlayCallback;

public:
	Animation()
	{
		//动画不是单次触发 需要不断的更新动画帧
		timer.set_one_shot(false);
		timer.set_on_timeout(
			[&]()
			{
				idx_frame++;
				if (idx_frame >= rect_src_list.size())
				{
					idx_frame = is_loop ? 0 : rect_src_list.size() - 1;
					//如果不是循环动画到了最后一帧 调用动画结束回调函数
					if (!is_loop && on_finished)
						on_finished();
				}
			}
		);
	}
	~Animation() = default;

	void reset()
	{
		timer.restart();
		idx_frame = 0;
	}

	//加载动画帧
	//num_h:素材图片一行有多少个动画帧  num_v：素材图片的高(一列有几个动画帧)有多少 idx_list：向获取的目标动画序号
	void set_frame_data(SDL_Texture* texture, int num_h, int num_v, const std::vector<int>& idx_list)
	{
		//像素图片的宽高
		int width_tex, height_tex;

		this->texture = texture;
		SDL_QueryTexture(texture, nullptr, nullptr, &width_tex, &height_tex);
		width_frame = width_tex / num_h, height_frame = height_tex / num_v;

		//设置vector的大小
		rect_src_list.resize(idx_list.size());
		for (size_t i = 0; i < idx_list.size(); i++)
		{
			int idx = idx_list[i];
			SDL_Rect& rec_src = rect_src_list[i];

			rec_src.x = (idx % num_h) * width_frame;
			rec_src.y = (idx / num_h) * height_frame;
			rec_src.w = width_frame, rec_src.h = height_frame;
		}
	}

	void set_loop(bool is_loop)
	{
		this->is_loop = is_loop;
	}

	//设置动画帧和帧之间切换时间就是设置timer的间隔时间
	void set_interval(double interval)
	{
		timer.set_wait_time(interval);
	}

	void set_on_finished(PlayCallback on_finished)
	{
		this->on_finished = on_finished;
	}

	void on_update(double delta)
	{
		timer.on_update(delta);
	}

	//pos_dst:要渲染的目标区域  angle：动画旋转角度
	void on_render(SDL_Renderer* renderer, const SDL_Point& pos_dst, double angle = 0, bool is_vertical = false, bool is_horizontal = false) const
	{
		static SDL_Rect rect_dst;

		rect_dst.x = pos_dst.x, rect_dst.y = pos_dst.y;
		rect_dst.w = width_frame, rect_dst.h = height_frame;

		//SDL_RendererFlip::SDL_FLIP_NONE：图片不翻转
		if (is_vertical)
		{
			SDL_RenderCopyEx(renderer, texture, &rect_src_list[idx_frame], &rect_dst, angle, nullptr, SDL_RendererFlip::SDL_FLIP_VERTICAL);
		}
		else if (is_horizontal)
		{
			SDL_RenderCopyEx(renderer, texture, &rect_src_list[idx_frame], &rect_dst, angle, nullptr, SDL_RendererFlip::SDL_FLIP_HORIZONTAL);
		}
		else
		{
			SDL_RenderCopyEx(renderer, texture, &rect_src_list[idx_frame], &rect_dst, angle, nullptr, SDL_RendererFlip::SDL_FLIP_NONE);
		}
	}


private:
	Timer timer;                              //图片切换定时器
	bool is_loop = true;                      //动画是否循环
	size_t idx_frame = 0;                     //当前动画遍历到第几帧
	PlayCallback on_finished;                 //不循环动画的回调函数
	SDL_Texture* texture = nullptr;           //当前动画的像素图片资源
	std::vector<SDL_Rect> rect_src_list;      //需要从像素图片裁剪出来的矩形区域数组
	int width_frame = 0, height_frame = 0;    //动画每一帧的像素宽度和高度
};

