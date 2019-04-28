////////////////////////////////////////////////////////////////////////////
//
// OpenGL GLU tesselation (simple).
//  - Originaly written by Valery Moya (vturnip@yahoo.com)
//
//   The big picture is to:
//
//   - Register some callbacks with GLU so we can grab the tesselation
//     as GLU does it.
//
//   - Give GLU a list of verts that defines the perimeter (of the outside)
//     of the polygon ordered clockwise, and if any, lists of verts that
//     make holes in the polygon in counter clockwise order.
//
//   GLU will return triangles, triangle fans and/or triangle strips that
//   are the triangulation of the polygon.  This is done by GLU by first
//   calling by TessBeginCallback() to let us know the triangle (vertex)
//   arangement, followed by multiple calls to TessVertexCallback() that
//   should be interepreted as the verticies of a triangle list, triangle
//   fan or triangle strip based on what the preceding TessBeginCallback()
//   set.
//
//   For simplicity here, I assume that the polygon is flat and in the XY
//   plane (aka, +Z is the polygon's surface normal).
//
////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <cstdlib>
#include "GLUTess.h"
using namespace std;

//////////////////////////////////////////////////////////////////
//
// TessAddTrig()
//
// - Keep track of a new triangle defined by vertex indecies a,
//   b, and c.
//
//////
void TessAddTrig(int a, int b, int c, GLUTess* tessobj)
{
	// Add triangle defined by a,b and c to list...
	tessobj->m_trigs.push_back(Triangle(a, b, c));
}

// TessAddTrig() /////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// TessParseTriangleXXX()
//
// - Interpret last three vertex indices and keep track of the
//   triangle defined.
//
//////

void TessParseTriangle(int idx, GLUTess* tessobj)
{
	TessAddTrig(tessobj->m_tess_idx[0], tessobj->m_tess_idx[1], idx, tessobj);
	// Three unique verts make 1 triangle.
	tessobj->m_tess_nidx = 0;
}

void TessParseTriangleFan(int idx, GLUTess* tessobj)
{
	TessAddTrig(tessobj->m_tess_idx[0], tessobj->m_tess_idx[1], idx, tessobj);
	// First vert is anchor of fan, latest index
	// should make next triangle's first edge.
	tessobj->m_tess_idx[1] = idx;
}

void TessParseTriangleStrip(int idx, GLUTess* tessobj)
{
	if (!tessobj->m_tess_odd)
	{
		TessAddTrig(tessobj->m_tess_idx[0], tessobj->m_tess_idx[1], idx, tessobj);
	}
	else
	{
		TessAddTrig(tessobj->m_tess_idx[0], idx, tessobj->m_tess_idx[1], tessobj);
	}

	// Last two verts should make first edge of next triangle
	tessobj->m_tess_idx[0] = tessobj->m_tess_idx[1];
	tessobj->m_tess_idx[1] = idx;

	tessobj->m_tess_odd = !tessobj->m_tess_odd;
}
void APIENTRY TessBeginCallbackData(GLenum which, void* polygon_data)
{
	GLUTess* pGLUT = (GLUTess*)polygon_data;
	pGLUT->m_tess_type = which; // How next verts should be interpreted to form triangles.
	pGLUT->m_tess_nidx = 0;     //
	pGLUT->m_tess_odd = false; // Reset odd-even winding.
}
//////////////////////////////////////////////////////////////////
//
// TessVertexCallback()
//
// - GLU will first call TessBeginCallback() to let us know how
//   verticies passed to this function should be interepreted.
//   Verticies are then passed in one at a time, so this
//   function keeps state to track the last two verts that make
//   up a full triangle as well as the winding order if the
//   verticies define a triangle strip.
//
//////

void APIENTRY TessVertexCallbackData(GLvoid *vertex, void* polygon_data)
{
	// This is a bit of a hack, I assume the vertex
	// passed in is a pointer into the vertex array
	// we gave GLU, and I compute the index into
	// that array via simple pointer arithmetic.
	GLUTess* pGLUT = (GLUTess*)polygon_data;
	Vertex *dpos = (Vertex*)vertex;
	int vidx = dpos - &pGLUT->m_verts[0];
	if (pGLUT->m_tess_nidx == 2)
	{
		// If two verts were previously given, then this
		// vertex completes a triangle.
		switch (pGLUT->m_tess_type)
		{
		case GL_TRIANGLES: TessParseTriangle(vidx, pGLUT); break;
		case GL_TRIANGLE_FAN: TessParseTriangleFan(vidx, pGLUT); break;
		case GL_TRIANGLE_STRIP: TessParseTriangleStrip(vidx, pGLUT); break;
		default:
			printf("Unknown tesselasion enum: %d\n", pGLUT->m_tess_type);
			// Don't create triangles from these tesselations
			break;
		}
	}
	else
	{
		// All triangle tesselation types need at least two verts.
		pGLUT->m_tess_idx[pGLUT->m_tess_nidx] = vidx;
		pGLUT->m_tess_nidx++;
	}
}

// TessVertexCallback() //////////////////////////////////////////
void APIENTRY TessCombineCallbackData(const GLdouble newVertex[3], const GLdouble *neighborVertex[4],
	const GLfloat neighborWeight[4], GLdouble **outData, void* polygon_data)
{
	GLUTess* pGLUT = (GLUTess*)polygon_data;
	pGLUT->m_verts[pGLUT->m_num_verts + pGLUT->m_vertexIndex].dpos[0] = newVertex[0];
	pGLUT->m_verts[pGLUT->m_num_verts + pGLUT->m_vertexIndex].dpos[1] = newVertex[1];
	pGLUT->m_verts[pGLUT->m_num_verts + pGLUT->m_vertexIndex].dpos[2] = newVertex[2];
	*outData = (GLdouble*)&pGLUT->m_verts[pGLUT->m_num_verts + pGLUT->m_vertexIndex];
	pGLUT->m_vertexIndex++;
}
// TessBeginCallback() ///////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// TessErrorCallback()
//
// - Error callback handler if some problem occured while
//   tesselating.
//
//////

void APIENTRY TessErrorCallbackData(GLenum errorCode, void* polygon_data)
{
	((GLUTess*)polygon_data)->m_tess_err = gluErrorString(errorCode);
}

// TessErrorCallback() ///////////////////////////////////////////
// TessParseTriangleXXX() ////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//
// TessWithGlu()
//
// - Function that uses GLU to triangulate a polygon defined by
//   lists of countours of its perimeter and any hole(s) it may
//   have.
//
//////



// TessWithGlu() /////////////////////////////////////////////////

void GLUTess::GLUTessProcess(const CPointsVector& sourcePoints, vector<CTPoint<int>>& result)
{
	if (sourcePoints.size() < 4){
		return;
	}
	Preprocess(sourcePoints);
	TessWithGlu();
	Postprocess(result);
}

void GLUTess::Preprocess(const CPointsVector& sourcePoints)
{
	m_num_verts = sourcePoints.size() - 1;
	m_verts.resize(m_num_verts * 2);
	m_trigs.resize(0);
	int count = 0;
	for (int i = 1; i < sourcePoints.size(); i++)
	{
		m_verts[count].dpos[0] = sourcePoints[i].x;
		m_verts[count].dpos[1] = sourcePoints[i].y;
		m_verts[count].dpos[2] = 1;
		count++;
	}
}

bool GLUTess::TessWithGlu(void)
{
	// Allocate space for the triangle data. A polygon with N
	// verts will have N-2 trigs in the triangulation.  If the

	// Setup triangluation state


	// Create a GLU tesselator and setup our callbacks.
	GLUtesselator* tess = gluNewTess();
	gluTessCallback(tess, GLU_TESS_BEGIN_DATA, (GLvoid(APIENTRY *) ()) &TessBeginCallbackData);
	gluTessCallback(tess, GLU_TESS_VERTEX_DATA, (GLvoid(APIENTRY *) ()) &TessVertexCallbackData);
	gluTessCallback(tess, GLU_TESS_COMBINE_DATA, (GLvoid(APIENTRY *) ())&TessCombineCallbackData);
	gluTessCallback(tess, GLU_TESS_ERROR_DATA, (GLvoid(APIENTRY *) ()) &TessErrorCallbackData);
	gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
	gluTessNormal(tess, 0, 0, 1); // Polygon in XY-plane, +Z axis is 'up'.

	// Tesselate the polygon.  First countour is outside perimeter of
	// polygon (clockwise vertex order), and all others define holes
	// in the polygon (counter clockwise vertex order).
	gluTessBeginPolygon(tess, this);
	gluTessBeginContour(tess);
	for (int i = 0; i < m_num_verts; i++)
	{
		gluTessVertex(tess, m_verts[i].dpos, m_verts[i].dpos);
	}
	gluTessEndContour(tess);
	gluTessEndPolygon(tess);

	gluDeleteTess(tess);

	// Handle errors
	if (m_tess_err)
	{
		fprintf(stderr, "Tessellation Error: %s\n", m_tess_err);
		return false;
	}

	return true;
}

void GLUTess::Postprocess(vector<CTPoint<int>>& result)
{
	size_t start = result.size();
	result.resize(start + m_trigs.size() * 3);

	for (unsigned int i = 0; i < m_trigs.size(); i++)
	{
		if (m_verts[m_trigs[i].vidx[0]].dpos[0] == m_verts[m_trigs[i].vidx[1]].dpos[0]
			&& m_verts[m_trigs[i].vidx[0]].dpos[0] == m_verts[m_trigs[i].vidx[2]].dpos[0]
			&& m_verts[m_trigs[i].vidx[1]].dpos[0] == m_verts[m_trigs[i].vidx[2]].dpos[0])
		{
			continue;
		}
		else if (m_verts[m_trigs[i].vidx[0]].dpos[1] == m_verts[m_trigs[i].vidx[1]].dpos[1]
			&& m_verts[m_trigs[i].vidx[0]].dpos[1] == m_verts[m_trigs[i].vidx[2]].dpos[1]
			&& m_verts[m_trigs[i].vidx[1]].dpos[1] == m_verts[m_trigs[i].vidx[2]].dpos[1])
		{
			continue;
		}
		result[start].x = (int)m_verts[m_trigs[i].vidx[0]].dpos[0];
		result[start].y = (int)m_verts[m_trigs[i].vidx[0]].dpos[1];
		start++;
		result[start].x = (int)m_verts[m_trigs[i].vidx[1]].dpos[0];
		result[start].y = (int)m_verts[m_trigs[i].vidx[1]].dpos[1];
		start++;
		result[start].x = (int)m_verts[m_trigs[i].vidx[2]].dpos[0];
		result[start].y = (int)m_verts[m_trigs[i].vidx[2]].dpos[1];
		start++;
	}
	m_vertexIndex = 0;
}
