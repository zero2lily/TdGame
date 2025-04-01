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
	//静态变量不能在类中初始化
	//因为静态成员属于整个类，而不属于某个对象，如果在类内初始化，会导致每个对象都包含该静态成员，这是矛盾的
	static T* manager;

protected:
	//构造函数为保护 外部不能直接通过new来创建一个对象
	Manager() = default;
	~Manager() = default;
	Manager(const Manager&) = delete;
	Manager& operator = (const Manager&) = delete;
};

template <typename T>
T* Manager<T>::manager = nullptr;
