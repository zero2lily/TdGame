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

	//���ûص�����
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
			//������ǵ��δ��� || �ǵ��δ�������û�д�����   ��˵����ο��Դ����ص�����
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
	bool shotted = false;               //�Ƿ��Ѿ�������
	bool one_shot = false;             //�Ƿ��ǵ��δ���
	std::function<void()> on_timeout;  //�ص�����   <void()>  void������ֵΪ��  ():����û�в���
};
