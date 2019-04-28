#ifndef GLUTESS_H
#define GLUTESS_H
#include <windows.h>
#include "glu.h"
#include <vector>
#include "TPoint.h"

using namespace std;

struct Vertex
{
    double dpos[3];
};

struct Triangle
{
    int vidx[3]; // Indices into the vertex array for the triangle's verts.
	Triangle(int a, int b, int c)
	{
		vidx[0] = a;
		vidx[1] = b;
		vidx[2] = c;
	}
	Triangle(){}
};

class GLUTess
{
private:
    void Preprocess(const CPointsVector& sourcePoints);
	bool TessWithGlu(void);
	void Postprocess(vector<CTPoint<int>>& result);

public:
	GLenum  m_tess_type;      // Current type of tesselation: Strip, Fan, List.
    int  m_tess_idx[2];    // Keeps track of last two verts of the current triangle.
    int  m_tess_nidx;      // Number of indecies defined.
    bool m_tess_odd;       // For triangle strips, keep track of (flipping) winding order.
	int  m_num_verts;
	int	 m_vertexIndex;
    const GLubyte* m_tess_err;      // An error occured.
	vector<Vertex>		m_verts;	// All the verts that define the polygon.
	vector<Triangle>	m_trigs;	// Will be filled out by the tesselation code

	GLUTess()
	{
		m_vertexIndex = 0;
		m_tess_type = GL_INVALID_ENUM;	// No triangulation type defined.
		m_tess_nidx = 0;				// No vertex indices defined.
		m_tess_err = nullptr;			// No errors.
		m_trigs.reserve(1024 * 1024);
	}
    void GLUTessProcess(const CPointsVector& sourcePoints, vector<CTPoint<int>>& result);
};
    
#endif