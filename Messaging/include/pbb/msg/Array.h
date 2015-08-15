#include <pbb/pbb.h>

namespace pbb {
namespace msg {

template<typename T>
class Array
{
public:

	Array()
		: m_Items(0)
		, m_Length(0)
	{
		Resize(10);
	}

	Array(uint32_t len)
		: m_Items(0)
	{
		Resize(len);
		m_Length = len;
	}
	~Array()
	{
		delete(m_Items);
	}

	void Resize(uint32_t len)
	{
		m_Items    = (T*)realloc(m_Items, sizeof(T)*len);
		m_Capacity = len;
	}

	T& operator[](size_t index)
	{
		return m_Items[index];
	}

	uint32_t Length()
	{
		return m_Length;
	}
protected:
	T*       m_Items;
	uint32_t m_Length;
	uint32_t m_Capacity;
private:
};

template<typename T, uint32_t L>
class StaticArray
{
public:
	StaticArray()
	{
		m_Length = L;
	}
	~StaticArray()
	{
		delete(m_Items);
	}

	T& operator[](size_t index)
	{
		return m_Items[index];
	}

	uint32_t Length()
	{
		return m_Length;
	}
protected:

	T m_Items[L];

	uint32_t m_Length;
private:
};

}
}