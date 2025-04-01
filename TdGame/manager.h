#pragma once

template <typename T>
class Manager 
{
public:
	static T* instance()
	{
		if (!manager)
			manager = new T();
		
		return manager;
	}

private:
	//��̬�������������г�ʼ��
	//��Ϊ��̬��Ա���������࣬��������ĳ��������������ڳ�ʼ�����ᵼ��ÿ�����󶼰����þ�̬��Ա������ì�ܵ�
	static T* manager;

protected:
	//���캯��Ϊ���� �ⲿ����ֱ��ͨ��new������һ������
	Manager() = default;
	~Manager() = default;
	Manager(const Manager&) = delete;
	Manager& operator = (const Manager&) = delete;
};

template <typename T>
T* Manager<T>::manager = nullptr;
