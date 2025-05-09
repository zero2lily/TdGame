#pragma once

#include <functional>

class Timer
{
public:
	Timer() = default;
	~Timer() = default;

	void restart()
	{
		pass_time = 0;
		shotted = false;
	}

	void set_wait_time(double val)
	{
		wait_time = val;
	}

	void set_one_shot(bool flag)
	{
		one_shot = flag;
	}

	//设置回调函数
	void set_on_timeout(std::function<void()> on_timeout)
	{
		this->on_timeout = on_timeout;
	}

	void pause()
	{
		paused = true;
	}

	void resume()
	{
		paused = false;
	}

	void on_update(double delta)
	{
		if (paused) return;

		pass_time += delta;
		if (pass_time >= wait_time)
		{
			//如果不是单次触发 || 是单次触发但是没有触发过   就说明这次可以触发回调函数
			bool can_shot = (!one_shot || (one_shot && !shotted));
			shotted = true;
			if (can_shot && on_timeout)
				on_timeout();

			pass_time -= wait_time;
		}
	}


private:
	double pass_time = 0;
	double wait_time = 0;
	bool paused = false;
	bool shotted = false;               //是否已经触发过
	bool one_shot = false;             //是否是单次触发
	std::function<void()> on_timeout;  //回调函数   <void()>  void：返回值为空  ():函数没有参数
};
