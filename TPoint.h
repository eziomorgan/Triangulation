#pragma once

#include <vector>
using namespace std;

template<typename T>
class CTPoint
{
public:
	T x;
	T y;

public:
	CTPoint(): x(T()), y(T()) {}
	CTPoint(T _x, T _y) : x(_x), y(_y) {}

	CTPoint& operator=(const CTPoint& p) { x = p.x; y = p.y; return *this; }

	bool operator==(const CTPoint& p) const { return x == p.x && y == p.y; }
	bool operator!=(const CTPoint& p) const { return !(*this == p); }

	CTPoint& operator+=(const CTPoint& p) { x += p.x; y += p.y; return *this; }
	CTPoint& operator-=(const CTPoint& p) { x -= p.x; y -= p.y; return *this; }

	CTPoint operator+(const CTPoint& p) const { return CTPoint(x + p.x, y + p.y); }
	CTPoint operator-(const CTPoint& p) const { return CTPoint(x - p.x, y - p.y); }

	static T& X(CTPoint& p) { return p.x; }
	static T& Y(CTPoint& p) { return p.y; }
};

//==============================================================================

typedef CTPoint<double> CDataPoint;
typedef vector<CDataPoint> CPointsVector;

class CPolygonContainer
{
public:
	CPolygonContainer() :m_Size(0){}
	void push_back(const CPointsVector& polygon)
	{
		if (m_Size == m_Polygons.size()){
			m_Polygons.resize(m_Size + 256);
		}
		m_Polygons[m_Size++] = polygon;
	}
	size_t size() const	{ return m_Size; }
	void clear(){ m_Size = 0; }
	void resize(size_t size)
	{
		if (size > m_Polygons.size()){
			m_Polygons.resize(size);
		}
		m_Size = size;
	}

	CPointsVector& operator[](int index){ return m_Polygons[index]; }
	const CPointsVector& operator[](int index) const{ return m_Polygons[index]; }

	CPointsVector& back(){ return m_Polygons[m_Size - 1]; }
	const CPointsVector& back() const{ return m_Polygons[m_Size - 1]; }

	vector<CPointsVector> getPolygons(){ return m_Polygons; }
private:
	size_t						m_Size;
	vector<CPointsVector>	m_Polygons;
};

