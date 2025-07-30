//
// Created by asice-cloud on 24-8-29.
// raii = resource acquisition is initialization
// exception saft
#include <fmt/format.h>
#include <numeric>
#include <ranges>
#include <vector>

// raii

/*
如果一个类定义了解构函数，那么您必须同时定义或删除拷贝构造函数和拷贝赋值函数，否则出错。
如果一个类定义了拷贝构造函数，那么您必须同时定义或删除拷贝赋值函数，否则出错，删除可导致低效。
如果一个类定义了移动构造函数，那么您必须同时定义或删除移动赋值函数，否则出错，删除可导致低效。
如果一个类定义了拷贝构造函数或拷贝赋值函数，那么您必须最好同时定义移动构造函数或移动赋值函数，否则低效。
 */
// 拷贝赋值函数≈解构函数+拷贝构造函数
/*
内存的销毁重新分配可以通过realloc，从而就地利用当前现有的m_data，避免重新分配。
因此拷贝赋值函数还是自定义下比较好
 */

/* 这些情况下编译器会调用移动：
return v2                                // v2 作返回值
v1 = std::vector<int>(200)               // 就地构造的 v2
v1 = std::move(v2)                       // 显式地移动
这些情况下编译器会调用拷贝：
return std::as_const(v2)                // 显式地拷贝
v1 = v2                                 // 默认拷贝
注意，以下语句没有任何作用：
std::move(v2)             // 不会清空 v2，需要清空可以用 v2 = {} 或 v2.clear()
std::as_const(v2)            // 不会拷贝 v2，需要拷贝可以用 { auto _ = v2; }
这两个函数只是负责转换类型，实际产生移动/拷贝效果的是在类的构造/赋值函数里。
 */
/*
移动构造≈拷贝构造+他解构+他默认构造
移动赋值≈拷贝赋值+他解构+他默认构造
移动赋值≈解构+移动构造
其实：如果你的类已经实现了移动赋值函数，那么为了省力你可以删除拷贝赋值函数
 */

/*
unique_ptr：当该对象仅仅属于我时。比如：父窗口中指向子窗口的指针。
原始指针：当该对象不属于我，但他释放前我必然被释放时。有一定风险。比如：子窗口中指向父窗口的指针。
shared_ptr：当该对象由多个对象共享时，或虽然该对象仅仅属于我，但有使用 weak_ptr 的需要。
weak_ptr：当该对象不属于我，且他释放后我仍可能不被释放时。比如：指向窗口中上一次被点击的元素*/
/*
shared_ptr 需要维护一个 atomic 的引用计数器，效率低，需要额外的一块管理内存，访问实际对象需要二级指针，
而且 deleter 使用了类型擦除技术。全部用 shared_ptr，可能出现循环引用之类的问题，导致内存泄露，
依然需要使用不影响计数的原始指针或者 weak_ptr 来避免
*/
struct Pig
{
	std::string m_name;
	int m_weight{0};

	/*Pig()
	{
		m_name = "Piqi";
		m_weight = 100;
	}*/
	// or
	Pig() = default;
	// explict: it can't be used for implicit conversion,such as Pig pig = 100, or{"name",100}
	explicit Pig(int weight) : m_name("Piqi"), m_weight(weight) {}
	Pig(std::string name, int weight) : m_name(std::move(name)), m_weight(weight) {}

	// copy constructor
	Pig(const Pig &other) = default;
	// Pig(const Pig&) = delete;

	// move constructor
	Pig(Pig &&other) noexcept : m_name(std::move(other.m_name)), m_weight(other.m_weight) {}

	// copy evaluation
	Pig &operator=(Pig const &) = default;
	// Pig& operator=(Pig const&) = delete;

	// move evaluation
	Pig &operator=(Pig &&other) = default;

	~Pig() { fmt::print("resume this object"); }
};

void show(Pig pig)
{
	fmt::print("name = {}, ", pig.m_name);
	fmt::print("weight = {}\n", pig.m_weight);
}

struct v_Vector
{
	size_t _size;
	int *_data;

	explicit v_Vector(size_t n)
	{
		_size = n;
		_data = static_cast<int *>(malloc(n * sizeof(int)));
	}
	~v_Vector() { free(_data); }

	v_Vector(const v_Vector &other)
	{
		_size = other._size;
		_data = static_cast<int *>(malloc(_size * sizeof(int)));
		memcpy(_data, other._data, _size * sizeof(int));
	}

	v_Vector &operator=(const v_Vector &other)
	{
		_size = other._size;
		_data = static_cast<int *>(realloc(_data, _size * sizeof(int)));
		memcpy(_data, other._data, _size * sizeof(int));
		return *this;
	}

	v_Vector &operator=(const v_Vector &&other) noexcept
	{
		this->~v_Vector();
		new (this) v_Vector(std::move(other));
		return *this;
	}

	size_t size() const { return _size; }
	void resize(size_t size)
	{
		_size = size;
		_data = static_cast<int *>(realloc(_data, size * sizeof(int)));
	}

	int &operator[](size_t index) const { return _data[index]; }

	void move_to(v_Vector &other)
	{
		other._size = _size;
		other._data = _data;
		_size = 0;
		_data = nullptr;
	}

	void swap_with(v_Vector &other)
	{
		std::swap(_size, other._size);
		std::swap(_data, other._data);
	}
};

int main()
{
	{
		[[maybe_unused]] int sum = 0;
		std::vector vec{5, 4, 8, 9, 1, 6, -4, -6, 1};
		// std::for_each(vec.begin(),vec.end(),[&sum](auto x){sum+=x;});
		sum = std::reduce(vec.begin(), vec.end());
		// range
		for (auto &&vi : vec | std::views::filter([](auto &&x) { return x > 0; }) |
				 std::views::transform([](auto &&x) { return x * 2; }))
		{
			fmt::print("{}, ", vi);
		}

		fmt::print("result sum: {}\n", sum);
	}

	// raii
	{ // constructor without parameters
		Pig pig;
		show(pig);
		show({"hw", 101});
	}
	{ // constructor with multiply parameter
		Pig pig{"Gorge", 200};
		show(pig);
	}
	{ // constructor with one parameter
		// trap: Pig pig = 100;
		Pig pig(300);
		show(pig);
	}
	{
		v_Vector v(5);
		v_Vector v1 = v;
	}
}
