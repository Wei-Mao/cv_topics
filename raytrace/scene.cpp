// -----------------------------------------------------------
// scene.cpp
// 2004 - Jacco Bikker - jacco@bik5.com - www.bik5.com -   <><
// -----------------------------------------------------------

#include "common.h"
#include "string.h"
#include "scene.h"
#include "raytracer.h"

namespace Raytracer {

// -----------------------------------------------------------
// Primitive class implementation
// -----------------------------------------------------------

void Primitive::SetName( char* a_Name )
{
	delete m_Name; 
	m_Name = new char[strlen( a_Name ) + 1]; 
	strcpy( m_Name, a_Name ); 
}

// -----------------------------------------------------------
// Material class implementation
// -----------------------------------------------------------

Material::Material() :
	m_Color( Color( 0.2f, 0.2f, 0.2f ) ),
	m_Refl( 0 ), m_Diff( 0.2f ), m_Spec( 0.8f ), m_RIndex( 1.5f )
{
}

// -----------------------------------------------------------
// Sphere primitive methods
// -----------------------------------------------------------

int Sphere::Intersect( Ray& a_Ray, float& a_Dist )
{
	vector3 v = a_Ray.GetOrigin() - m_Centre;
	float b = -DOT( v, a_Ray.GetDirection() );
	float det = (b * b) - DOT( v, v ) + m_SqRadius;
	int retval = MISS;
	if (det > 0)
	{
		det = sqrtf( det );
		float i1 = b - det;
		float i2 = b + det;
		if (i2 > 0)
		{
			if (i1 < 0) 
			{
				if (i2 < a_Dist) 
				{
					a_Dist = i2;
					retval = INPRIM;
				}
			}
			else
			{
				if (i1 < a_Dist)
				{
					a_Dist = i1;
					retval = HIT;
				}
			}
		}
	}
	return retval;
}

bool Sphere::IntersectBox( aabb& a_Box )
{
	float dmin = 0;
	vector3 v1 = a_Box.GetPos(), v2 = a_Box.GetPos() + a_Box.GetSize();
	if (m_Centre.x < v1.x) 
	{
		dmin = dmin + (m_Centre.x - v1.x) * (m_Centre.x - v1.x);
	}
	else if (m_Centre.x > v2.x)
	{
		dmin = dmin + (m_Centre.x - v2.x) * (m_Centre.x - v2.x);
	}
	if (m_Centre.y < v1.y)
	{
		dmin = dmin + (m_Centre.y - v1.y) * (m_Centre.y - v1.y);
	}
	else if (m_Centre.y > v2.y)
	{
		dmin = dmin + (m_Centre.y - v2.y) * (m_Centre.y - v2.y);
	}
	if (m_Centre.z < v1.z)
	{
		dmin = dmin + (m_Centre.z - v1.z) * (m_Centre.z - v1.z);
	}
	else if (m_Centre.z > v2.z)
	{
		dmin = dmin + (m_Centre.z - v2.z) * (m_Centre.z - v2.z);
	}
	return (dmin <= m_SqRadius);
}

aabb Sphere::GetAABB()
{ 
	vector3 size( m_Radius, m_Radius, m_Radius );
	return aabb( m_Centre - size, size * 2 );
}

// -----------------------------------------------------------
// Plane primitive class implementation
// -----------------------------------------------------------

int PlanePrim::Intersect( Ray& a_Ray, float& a_Dist )
{
	float d = DOT( m_Plane.N, a_Ray.GetDirection() );
	if (d < 0)
	{
		float dist = -(DOT( m_Plane.N, a_Ray.GetOrigin() ) + m_Plane.D) / d;
		if (dist < a_Dist) 
		{
			a_Dist = dist;
			return HIT;
		}
	}
	return MISS;
}

bool PlanePrim::IntersectBox( aabb& a_Box )
{
	vector3 v[2];
	v[0] = a_Box.GetPos(), v[1] = a_Box.GetPos() + a_Box.GetSize();
	for ( int side1 = 0, side2 = 0, i = 0; i < 8; i++ )
	{
		vector3 p( v[i & 1].x, v[(i >> 1) & 1].y, v[(i >> 2) & 1].z );
		if ((DOT( p, m_Plane.N ) + m_Plane.D) < 0) side1++; else side2++;
	}
	if ((side1 == 0) || (side2 == 0)) return false; else return true;
}

vector3 PlanePrim::GetNormal( vector3& a_Pos )
{
	return m_Plane.N;
}

// -----------------------------------------------------------
// Axis aligned box primitive class implementation
// -----------------------------------------------------------

Box::Box() : 
	m_Box( vector3( 0, 0, 0 ), vector3( 0, 0, 0 ) ), 
	m_Grid( 0 ) 
{
}

Box::Box( aabb& a_Box ) : 
	m_Box( a_Box ), 
	m_Grid( 0 ) 
{
}

int Box::Intersect( Ray& a_Ray, float& a_Dist )
{
	m_RayID = a_Ray.GetID();
	float dist[6];
	vector3 ip[6], d = a_Ray.GetDirection(), o = a_Ray.GetOrigin();
	bool retval = MISS;
	for ( int i = 0; i < 6; i++ ) dist[i] = -1;
	vector3 v1 = m_Box.GetPos(), v2 = m_Box.GetPos() + GetSize();
	if (d.x) 
	{
		float rc = 1.0f / d.x;
		dist[0] = (v1.x - o.x) * rc;
		dist[3] = (v2.x - o.x) * rc;
	}
	if (d.y) 
	{
		float rc = 1.0f / d.y;
		dist[1] = (v1.y - o.y) * rc;
		dist[4] = (v2.y - o.y) * rc;
	}
	if (d.z) 
	{
		float rc = 1.0f / d.z;
		dist[2] = (v1.z - o.z) * rc;
		dist[5] = (v2.z - o.z) * rc;
	}
	for ( i = 0; i < 6; i++ ) if (dist[i] > 0)
	{
		ip[i] = o + dist[i] * d;
		if ((ip[i].x > (v1.x - EPSILON)) && (ip[i].x < (v2.x + EPSILON)) && 
			(ip[i].y > (v1.y - EPSILON)) && (ip[i].y < (v2.y + EPSILON)) &&
			(ip[i].z > (v1.z - EPSILON)) && (ip[i].z < (v2.z + EPSILON)))
		{
			if (dist[i] < a_Dist) 
			{
				a_Dist = dist[i];
				retval = HIT;
			}
		}
	}
	return retval;
}

vector3 Box::GetNormal( vector3& a_Pos )
{
	float dist[6];
	dist[0] = (float)fabs( m_Box.GetSize().x - m_Box.GetPos().x );
	dist[1] = (float)fabs( m_Box.GetSize().x + m_Box.GetSize().x - m_Box.GetPos().x );
	dist[2] = (float)fabs( m_Box.GetSize().y - m_Box.GetPos().y );
	dist[3] = (float)fabs( m_Box.GetSize().y + m_Box.GetSize().y - m_Box.GetPos().y );
	dist[4] = (float)fabs( m_Box.GetSize().z - m_Box.GetPos().z );
	dist[5] = (float)fabs( m_Box.GetSize().z + m_Box.GetSize().z - m_Box.GetPos().z );
	int best = 0;
	float bdist = dist[0];
	for ( int i = 1 ; i < 6; i++ ) if (dist[i] < bdist) 
	{ 
		bdist = dist[i]; 
		best = i;
	}
	if (best == 0) return vector3( -1, 0, 0 );
	else if (best == 1) return vector3( 1, 0, 0 );
	else if (best == 2) return vector3( 0, -1, 0 );
	else if (best == 3)  return vector3( 0, 1, 0 );
	else if (best == 4) return vector3( 0, 0, -1 );
	else return vector3( 0, 0, 1 );
}

void Box::Light( bool a_Light )
{
	m_Light = a_Light;
	if (!m_Grid)
	{
		m_Grid = new float[32];
		m_Grid[ 0] = 1, m_Grid[ 1] = 2;
		m_Grid[ 2] = 3, m_Grid[ 3] = 3;
		m_Grid[ 4] = 2, m_Grid[ 5] = 0;
		m_Grid[ 6] = 0, m_Grid[ 7] = 1;
		m_Grid[ 8] = 2, m_Grid[ 9] = 3;
		m_Grid[10] = 0, m_Grid[11] = 3;
		m_Grid[12] = 0, m_Grid[13] = 0;
		m_Grid[14] = 2, m_Grid[15] = 2;
		m_Grid[16] = 3, m_Grid[17] = 1;
		m_Grid[18] = 1, m_Grid[19] = 3;
		m_Grid[20] = 1, m_Grid[21] = 0;
		m_Grid[22] = 3, m_Grid[23] = 2;
		m_Grid[24] = 2, m_Grid[25] = 1;
		m_Grid[26] = 3, m_Grid[27] = 0;
		m_Grid[28] = 1, m_Grid[29] = 1;
		m_Grid[30] = 0, m_Grid[31] = 2;
		for ( int i = 0; i < 16; i++ )
		{
			m_Grid[i * 2] = m_Grid[i * 2] * m_Box.GetSize().x / 4 + m_Box.GetPos().x;
			m_Grid[i * 2 + 1] = m_Grid[i * 2 + 1] * m_Box.GetSize().z / 4 + m_Box.GetPos().z;
		}
	}
}

// -----------------------------------------------------------
// Scene class implementation
// -----------------------------------------------------------

Scene::~Scene()
{
	delete m_Primitive;
}

void Scene::InitScene()
{
	m_Primitive = new Primitive*[500];
	// ground plane
	m_Primitive[0] = new PlanePrim( vector3( 0, 1, 0 ), 4.4f );
	m_Primitive[0]->SetName( "plane" );
	m_Primitive[0]->GetMaterial()->SetReflection( 0.0f );
	m_Primitive[0]->GetMaterial()->SetRefraction( 0.0f );
	m_Primitive[0]->GetMaterial()->SetDiffuse( 1.0f );
	m_Primitive[0]->GetMaterial()->SetColor( Color( 0.4f, 0.3f, 0.3f ) );
	// big sphere
	m_Primitive[1] = new Sphere( vector3( 2, 0.8f, 3 ), 2.5f );
	m_Primitive[1]->SetName( "big sphere" );
	m_Primitive[1]->GetMaterial()->SetReflection( 0.2f );
	m_Primitive[1]->GetMaterial()->SetRefraction( 0.8f );
	m_Primitive[1]->GetMaterial()->SetRefrIndex( 1.3f );
	m_Primitive[1]->GetMaterial()->SetColor( Color( 0.7f, 0.7f, 1.0f ) );
	// small sphere
	m_Primitive[2] = new Sphere( vector3( -5.5f, -0.5, 7 ), 2 );
	m_Primitive[2]->SetName( "small sphere" );
	m_Primitive[2]->GetMaterial()->SetReflection( 0.5f );
	m_Primitive[2]->GetMaterial()->SetRefraction( 0.0f );
	m_Primitive[2]->GetMaterial()->SetRefrIndex( 1.3f );
	m_Primitive[2]->GetMaterial()->SetDiffuse( 0.1f );
	m_Primitive[2]->GetMaterial()->SetColor( Color( 0.7f, 0.7f, 1.0f ) );
	// light source 1
	m_Primitive[3] = new Sphere( vector3( 0, 5, 5 ), 0.1f );
	m_Primitive[3]->Light( true );
	m_Primitive[3]->GetMaterial()->SetColor( Color( 0.4f, 0.4f, 0.4f ) );
	// light source 2
	m_Primitive[4] = new Sphere( vector3( -3, 5, 1 ), 0.1f );
	m_Primitive[4]->Light( true );
	m_Primitive[4]->GetMaterial()->SetColor( Color( 0.6f, 0.6f, 0.8f ) );
	// extra sphere
	m_Primitive[5] = new Sphere( vector3( -1.5f, -3.8f, 1 ), 1.5f );
	m_Primitive[5]->SetName( "extra sphere" );
	m_Primitive[5]->GetMaterial()->SetReflection( 0.0f );
	m_Primitive[5]->GetMaterial()->SetRefraction( 0.8f );
	m_Primitive[5]->GetMaterial()->SetColor( Color( 1.0f, 0.4f, 0.4f ) );
	// back plane
	m_Primitive[6] = new PlanePrim( vector3( 0.4f, 0, -1 ), 12 );
	m_Primitive[6]->SetName( "back plane" );
	m_Primitive[6]->GetMaterial()->SetReflection( 0.0f );
	m_Primitive[6]->GetMaterial()->SetRefraction( 0.0f );
	m_Primitive[6]->GetMaterial()->SetSpecular( 0 );
	m_Primitive[6]->GetMaterial()->SetDiffuse( 0.6f );
	m_Primitive[6]->GetMaterial()->SetColor( Color( 0.5f, 0.3f, 0.5f ) );
	// ceiling plane
	m_Primitive[7] = new PlanePrim( vector3( 0, -1, 0 ), 7.4f );
	m_Primitive[7]->SetName( "back plane" );
	m_Primitive[7]->GetMaterial()->SetReflection( 0.0f );
	m_Primitive[7]->GetMaterial()->SetRefraction( 0.0f );
	m_Primitive[7]->GetMaterial()->SetSpecular( 0 );
	m_Primitive[7]->GetMaterial()->SetDiffuse( 0.5f );
	m_Primitive[7]->GetMaterial()->SetColor( Color( 0.4f, 0.7f, 0.7f ) );
	// grid
	int prim = 8;
	for ( int x = 0; x < 8; x++ ) for ( int y = 0; y < 7; y++ )
	{
		m_Primitive[prim] = new Sphere( vector3( -4.5f + x * 1.5f, -4.3f + y * 1.5f, 10 ), 0.3f );
		m_Primitive[prim]->SetName( "grid sphere" );
		m_Primitive[prim]->GetMaterial()->SetReflection( 0 );
		m_Primitive[prim]->GetMaterial()->SetRefraction( 0 );
		m_Primitive[prim]->GetMaterial()->SetSpecular( 0.6f );
		m_Primitive[prim]->GetMaterial()->SetDiffuse( 0.6f );
		m_Primitive[prim]->GetMaterial()->SetColor( Color( 0.3f, 1.0f, 0.4f ) );
		prim++;
	}
	for ( x = 0; x < 8; x++ ) for ( int y = 0; y < 8; y++ )
	{
		m_Primitive[prim] = new Sphere( vector3( -4.5f + x * 1.5f, -4.3f, 10.0f - y * 1.5f ), 0.3f );
		m_Primitive[prim]->SetName( "grid sphere" );
		m_Primitive[prim]->GetMaterial()->SetReflection( 0 );
		m_Primitive[prim]->GetMaterial()->SetRefraction( 0 );
		m_Primitive[prim]->GetMaterial()->SetSpecular( 0.6f );
		m_Primitive[prim]->GetMaterial()->SetDiffuse( 0.6f );
		m_Primitive[prim]->GetMaterial()->SetColor( Color( 0.3f, 1.0f, 0.4f ) );
		prim++;
	}
	for ( x = 0; x < 16; x++ ) for ( int y = 0; y < 8; y++ )
	{
		m_Primitive[prim] = new Sphere( vector3( -8.5f + x * 1.5f, 4.3f, 10.0f - y ), 0.3f );
		m_Primitive[prim]->SetName( "grid sphere" );
		m_Primitive[prim]->GetMaterial()->SetReflection( 0 );
		m_Primitive[prim]->GetMaterial()->SetRefraction( 0 );
		m_Primitive[prim]->GetMaterial()->SetSpecular( 0.6f );
		m_Primitive[prim]->GetMaterial()->SetDiffuse( 0.6f );
		m_Primitive[prim]->GetMaterial()->SetColor( Color( 0.3f, 1.0f, 0.4f ) );
		prim++;
	}
	// set number of primitives
	m_Primitives = prim;
	// build the regular gird
	BuildGrid();
}

void Scene::BuildGrid()
{
	// initialize regular grid
	m_Grid = new ObjectList*[GRIDSIZE * GRIDSIZE * GRIDSIZE];
	memset( m_Grid, 0, GRIDSIZE * GRIDSIZE * GRIDSIZE * 4 );
	vector3 p1(-14, -5, -6), p2( 14, 8, 30 );
	// calculate cell width, height and depth
	float dx = (p2.x - p1.x) / GRIDSIZE, dx_reci = 1.0f / dx;
	float dy = (p2.y - p1.y) / GRIDSIZE, dy_reci = 1.0f / dy;
	float dz = (p2.z - p1.z) / GRIDSIZE, dz_reci = 1.0f / dz;
	m_Extends = aabb( p1, p2 - p1 );
	m_Light = new Primitive*[MAXLIGHTS];
	m_Lights = 0;
	// store primitives in the grid cells
	for ( int p = 0; p < m_Primitives; p++ )
	{
		if (m_Primitive[p]->IsLight()) m_Light[m_Lights++] = m_Primitive[p];
		aabb bound = m_Primitive[p]->GetAABB();
		vector3 bv1 = bound.GetPos(), bv2 = bound.GetPos() + bound.GetSize();
		// find out which cells could contain the primitive (based on aabb)
		int x1 = (int)((bv1.x - p1.x) * dx_reci), x2 = (int)((bv2.x - p1.x) * dx_reci) + 1;
		x1 = (x1 < 0)?0:x1, x2 = (x2 > (GRIDSIZE - 1))?GRIDSIZE - 1:x2;
		int y1 = (int)((bv1.y - p1.y) * dy_reci), y2 = (int)((bv2.y - p1.y) * dy_reci) + 1;
		y1 = (y1 < 0)?0:y1, y2 = (y2 > (GRIDSIZE - 1))?GRIDSIZE - 1:y2;
		int z1 = (int)((bv1.z - p1.z) * dz_reci), z2 = (int)((bv2.z - p1.z) * dz_reci) + 1;
		z1 = (z1 < 0)?0:z1, z2 = (z2 > (GRIDSIZE - 1))?GRIDSIZE - 1:z2;
		// loop over candidate cells
		for ( int x = x1; x < x2; x++ ) for ( int y = y1; y < y2; y++ ) for ( int z = z1; z < z2; z++ )
		{
			// construct aabb for current cell
			int idx = x + y * GRIDSIZE + z * GRIDSIZE * GRIDSIZE;
			vector3 pos( p1.x + x * dx, p1.y + y * dy, p1.z + z * dz );
			aabb cell( pos, vector3( dx, dy, dz ) );
			// do an accurate aabb / primitive intersection test
			if (m_Primitive[p]->IntersectBox( cell ))
			{
				// object intersects cell; add to object list
				ObjectList* l = new ObjectList();
				l->SetPrimitive( m_Primitive[p] );
				l->SetNext( m_Grid[idx] );
				m_Grid[idx] = l;
			}
		}
	}
}

}; // namespace Raytracer
