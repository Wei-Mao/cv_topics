// -----------------------------------------------------------
// raytracer.cpp
// 2004 - Jacco Bikker - jacco@bik5.com - www.bik5.com -   <><
// -----------------------------------------------------------

#include "raytracer.h"
#include "scene.h"
#include "common.h"
#include "windows.h"
#include "winbase.h"

namespace Raytracer {

Ray::Ray( vector3& a_Origin, vector3& a_Dir, int a_ID ) : 
	m_Origin( a_Origin ), 
	m_Direction( a_Dir ),
	m_ID( a_ID )
{
}

Engine::Engine()
{
	m_Scene = new Scene();
}

Engine::~Engine()
{
	delete m_Scene;
}

// -----------------------------------------------------------
// Engine::SetTarget
// Sets the render target canvas
// -----------------------------------------------------------
void Engine::SetTarget( Pixel* a_Dest, int a_Width, int a_Height )
{
	// set pixel buffer address & size
	m_Dest = a_Dest;
	m_Width = a_Width;
	m_Height = a_Height;
	// precalculate 1 / size of a cell (for x, y and z)
	m_SR.x = GRIDSIZE / m_Scene->GetExtends().GetSize().x;
	m_SR.y = GRIDSIZE / m_Scene->GetExtends().GetSize().y;
	m_SR.z = GRIDSIZE / m_Scene->GetExtends().GetSize().z;
	// precalculate size of a cell (for x, y, and z)
	m_CW = m_Scene->GetExtends().GetSize() * (1.0f / GRIDSIZE);
}

// -----------------------------------------------------------
// Engine::FindNearest
// Finds the nearest intersection in a regular gird for a ray
// -----------------------------------------------------------
int Engine::FindNearest( Ray& a_Ray, float& a_Dist, Primitive*& a_Prim )
{
	int retval = MISS;
	vector3 raydir, curpos; // floating
	Box e = m_Scene->GetExtends();
	curpos = a_Ray.GetOrigin(); // floating
	raydir = a_Ray.GetDirection();
	// setup 3DDDA (double check reusability of primary ray data)
	vector3 cb, tmax, tdelta, cell; // cb for current boundary
	cell = (curpos - e.GetPos()) * m_SR;
	int stepX, outX, X = (int)cell.x;
	int stepY, outY, Y = (int)cell.y;
	int stepZ, outZ, Z = (int)cell.z;
	if ((X < 0) || (X >= GRIDSIZE) || (Y < 0) || (Y >= GRIDSIZE) || (Z < 0) || (Z >= GRIDSIZE)) return 0;
	if (raydir.x > 0)
	{
        // outX is the max x_index of grid + 1.
		stepX = 1, outX = GRIDSIZE;
		cb.x = e.GetPos().x + (X + 1) * m_CW.x;
        // + 1  means right or upper boundary of the current cell.
	}
	else
	{
        // outX is the min x_index of grid - 1
		stepX = -1, outX = -1;
		cb.x = e.GetPos().x + X * m_CW.x; // left and lower boundaryh of current cell.
	}

	if (raydir.y > 0.0f)
	{
		stepY = 1, outY = GRIDSIZE;
		cb.y = e.GetPos().y + (Y + 1) * m_CW.y;
	}
	else
	{
		stepY = -1, outY = -1;
		cb.y = e.GetPos().y + Y * m_CW.y;
	}

	if (raydir.z > 0.0f)
	{
		stepZ = 1, outZ = GRIDSIZE;
		cb.z = e.GetPos().z + (Z + 1) * m_CW.z;
	}
	else
	{
		stepZ = -1, outZ = -1;
		cb.z = e.GetPos().z + Z * m_CW.z;
	}

    /* m_CW size of cell along x, y, and z. */
    /*
       x = x_start + t * dir
       t = ( x - x_start) / dir (perform element-wise division if no division by zero occurs.)
    */
	float rxr, ryr, rzr;
	if (raydir.x != 0) // should be if ((raydir.x - 0.0f) < EPSILON)
	{
		rxr = 1.0f / raydir.x;
		tmax.x = (cb.x - curpos.x) * rxr;  // always > 0
		tdelta.x = m_CW.x * stepX * rxr; // move along x by m_CW.x (voxel size along x)
	}
	else tmax.x = 1000000;   // ray is perpendicular to x, can not move along x

	if (raydir.y != 0)
	{
		ryr = 1.0f / raydir.y;
		tmax.y = (cb.y - curpos.y) * ryr;
		tdelta.y = m_CW.y * stepY * ryr;
	}
	else tmax.y = 1000000;

	if (raydir.z != 0)
	{
		rzr = 1.0f / raydir.z;
		tmax.z = (cb.z - curpos.z) * rzr;
		tdelta.z = m_CW.z * stepZ * rzr;
	}
	else tmax.z = 1000000;

	// start stepping
	ObjectList* list = 0;
	ObjectList** grid = m_Scene->GetGrid();
	a_Prim = 0;

	// trace primary ray
	while (1)
	{
		list = grid[X + (Y << GRIDSHFT) + (Z << (GRIDSHFT * 2))];
		while (list)
		{
			Primitive* pr = list->GetPrimitive();
			int result;
			if (pr->GetLastRayID() != a_Ray.GetID()) if (result = pr->Intersect( a_Ray, a_Dist )) 
			{
				retval = result;
				a_Prim = pr;
				goto testloop;
			}
			list = list->GetNext();
		}

		if (tmax.x < tmax.y)
		{
            // (X, Y, Z) is the current tracked voxel.
			if (tmax.x < tmax.z)
			{
				X = X + stepX;
				if (X == outX) return MISS;
				tmax.x += tdelta.x;  // tmax.x is updated for the next step
			}
			else
			{
				Z = Z + stepZ;
				if (Z == outZ) return MISS;
				tmax.z += tdelta.z;
			}
		}
		else
		{
			if (tmax.y < tmax.z)
			{
				Y = Y + stepY;
				if (Y == outY) return MISS;
				tmax.y += tdelta.y;
			}
			else
			{
				Z = Z + stepZ;
				if (Z == outZ) return MISS;
				tmax.z += tdelta.z;
			}
		}
	}
testloop:
	while (1)
	{
		list = grid[X + (Y << GRIDSHFT) + (Z << (GRIDSHFT * 2))];
		while (list)
		{
			Primitive* pr = list->GetPrimitive();
			int result;
			if (pr->GetLastRayID() != a_Ray.GetID()) if (result = pr->Intersect( a_Ray, a_Dist )) 
			{
				a_Prim = pr;
				retval = result;
			}
			list = list->GetNext();
		}
		if (tmax.x < tmax.y)
		{
			if (tmax.x < tmax.z)
			{
				if (a_Dist < tmax.x) break;
				X = X + stepX;
				if (X == outX) break;
				tmax.x += tdelta.x;
			}
			else
			{
				if (a_Dist < tmax.z) break;
				Z = Z + stepZ;
				if (Z == outZ) break;
				tmax.z += tdelta.z;
			}
		}
		else
		{
			if (tmax.y < tmax.z)
			{
				if (a_Dist < tmax.y) break;
				Y = Y + stepY;
				if (Y == outY) break;
				tmax.y += tdelta.y;
			}
			else
			{
				if (a_Dist < tmax.z) break;
				Z = Z + stepZ;
				if (Z == outZ) break;
				tmax.z += tdelta.z;
			}
		}
	}

	return retval;
}

// -----------------------------------------------------------
// Engine::Raytrace
// Naive ray tracing: Intersects the ray with every primitive
// in the scene to determine the closest intersection
// -----------------------------------------------------------
Primitive* Engine::Raytrace( Ray& a_Ray, Color& a_Acc, int a_Depth, float a_RIndex, float& a_Dist )
{
	if (a_Depth > TRACEDEPTH) return 0;
	// trace primary ray
	a_Dist = 1000000.0f;
	vector3 pi;
	Primitive* prim = 0;
	int result;
	// find the nearest intersection
	if (!(result = FindNearest( a_Ray, a_Dist, prim ))) return 0;
	// handle intersection
	if (prim->IsLight())
	{
		// we hit a light, stop tracing
		a_Acc = prim->GetMaterial()->GetColor();
	}
	else
	{
		// determine color at point of intersection
		pi = a_Ray.GetOrigin() + a_Ray.GetDirection() * a_Dist;
		// trace lights
		for ( int l = 0; l < m_Scene->GetNrLights(); l++ )
		{
			Primitive* light = m_Scene->GetLight( l );
			// handle point light source
			float shade = 1.0f;
			if (light->GetType() == Primitive::SPHERE)
			{
				vector3 L = ((Sphere*)light)->GetCentre() - pi;
				float tdist = LENGTH( L );
				L *= (1.0f / tdist);
				Primitive* pr = 0;
				FindNearest( Ray( pi + L * EPSILON, L, ++m_CurID ), tdist, pr );
				if (pr != light) shade = 0;
			}
			if (shade > 0)
			{
				// calculate diffuse shading
				vector3 L = ((Sphere*)light)->GetCentre() - pi;
				NORMALIZE( L );
				vector3 N = prim->GetNormal( pi );
				if (prim->GetMaterial()->GetDiffuse() > 0)
				{
					float dot = DOT( L, N );
					if (dot > 0)
					{
						float diff = dot * prim->GetMaterial()->GetDiffuse() * shade;
						// add diffuse component to ray color
						a_Acc += diff * prim->GetMaterial()->GetColor() * light->GetMaterial()->GetColor();
					}
				}
				// determine specular component
				if (prim->GetMaterial()->GetSpecular() > 0)
				{
					// point light source: sample once for specular highlight
					vector3 V = a_Ray.GetDirection();
					vector3 R = L - 2.0f * DOT( L, N ) * N;
					float dot = DOT( V, R );
					if (dot > 0)
					{
						float spec = powf( dot, 20 ) * prim->GetMaterial()->GetSpecular() * shade;
						// add specular component to ray color
						a_Acc += spec * light->GetMaterial()->GetColor();
					}
				}
			}
		}
		// calculate reflection
		float refl = prim->GetMaterial()->GetReflection();
		if ((refl > 0.0f) && (a_Depth < TRACEDEPTH))
		{
			vector3 N = prim->GetNormal( pi );
			vector3 R = a_Ray.GetDirection() - 2.0f * DOT( a_Ray.GetDirection(), N ) * N;
			Color rcol( 0, 0, 0 );
			float dist;
			Raytrace( Ray( pi + R * EPSILON, R, ++m_CurID ), rcol, a_Depth + 1, a_RIndex, dist );
			a_Acc += refl * rcol * prim->GetMaterial()->GetColor();
		}
		// calculate refraction
		float refr = prim->GetMaterial()->GetRefraction();
		if ((refr > 0) && (a_Depth < TRACEDEPTH))
		{
			float rindex = prim->GetMaterial()->GetRefrIndex();
			float n = a_RIndex / rindex;
			vector3 N = prim->GetNormal( pi ) * (float)result;
			float cosI = -DOT( N, a_Ray.GetDirection() );
			float cosT2 = 1.0f - n * n * (1.0f - cosI * cosI);
			if (cosT2 > 0.0f)
			{
				vector3 T = (n * a_Ray.GetDirection()) + (n * cosI - sqrtf( cosT2 )) * N;
				Color rcol( 0, 0, 0 );
				float dist;
				Raytrace( Ray( pi + T * EPSILON, T, ++m_CurID ), rcol, a_Depth + 1, rindex, dist );
				// apply Beer's law
				Color absorbance = prim->GetMaterial()->GetColor() * 0.15f * -dist;
				Color transparency = Color( expf( absorbance.r ), expf( absorbance.g ), expf( absorbance.b ) );
				a_Acc += rcol * transparency;
			}
		}
	}
	// return pointer to primitive hit by primary ray
	return prim;
}

// -----------------------------------------------------------
// Engine::InitRender
// Initializes the renderer, by resetting the line / tile
// counters and precalculating some values
// -----------------------------------------------------------
void Engine::InitRender()
{
	// set firts line to draw to
	m_CurrLine = 20;
	// set pixel buffer address of first pixel
	m_PPos = m_CurrLine * m_Width;
	// screen plane in world space coordinates
	m_WX1 = -4, m_WX2 = 4, m_WY1 = m_SY = 3, m_WY2 = -3;
	// calculate deltas for interpolation
	m_DX = (m_WX2 - m_WX1) / m_Width;
	m_DY = (m_WY2 - m_WY1) / m_Height;
	m_SY += m_CurrLine * m_DY;
	// allocate space to store pointers to primitives for previous line
	m_LastRow = new Primitive*[m_Width];
	memset( m_LastRow, 0, m_Width * 4 );
	// reset ray id counter
	m_CurID = 0;
}

// -----------------------------------------------------------
// Engine::RenderRay
// Helper function, fires one ray in the regular grid
// -----------------------------------------------------------
Primitive* Engine::RenderRay( vector3 a_ScreenPos, Color& a_Acc )
{
	Box e = m_Scene->GetExtends();
	vector3 o( 0, 0, -5 );
	vector3 dir = a_ScreenPos - o;
	NORMALIZE( dir );
	Color acc( 0, 0, 0 );
	Ray r( o, dir, ++m_CurID );
	// advance ray to scene bounding box boundary
	if (!e.Contains( o ))
	{
		float bdist = 10000.0f;
		if (e.Intersect( r, bdist )) r.SetOrigin( o + (bdist + EPSILON) * dir );
	}
	float dist;
	return Raytrace( r, a_Acc, 1, 1.0f, dist );
}

// -----------------------------------------------------------
// Engine::Render
// Fires rays in the scene one scanline at a time, from left
// to right
// -----------------------------------------------------------
bool Engine::Render()
{
	// render scene
	vector3 o( 0, 0, -5 );
	Box e = m_Scene->GetExtends();
	// initialize timer
	int msecs = GetTickCount();
	// reset last found primitive pointer
	Primitive* lastprim = 0;
	// render remaining lines
	for ( int y = m_CurrLine; y < (m_Height - 20); y++ )
	{
		m_SX = m_WX1;
		// render pixels for current line
		for ( int x = 0; x < m_Width; x++ )
		{
			// fire primary rays
			Color acc( 0, 0, 0 );
			Primitive* prim = RenderRay( vector3( m_SX, m_SY, 0 ), acc );
			int red, green, blue;
			if (prim != lastprim)
			{
				lastprim = prim;
				RenderRay( vector3( m_SX - m_DX / 2, m_SY, 0 ), acc );
				RenderRay( vector3( m_SX, m_SY - m_DY / 2, 0 ), acc );
				RenderRay( vector3( m_SX - m_DX / 2, m_SY - m_DY / 2, 0 ), acc );
				red = (int)(acc.r * 64);
				green = (int)(acc.g * 64);
				blue = (int)(acc.b * 64);
				if (red > 255) red = 255;
				if (green > 255) green = 255;
				if (blue > 255) blue = 255;
			}
			else
			{
				red = (int)(acc.r * 256);
				green = (int)(acc.g * 256);
				blue = (int)(acc.b * 256);
				if (red > 255) red = 255;
				if (green > 255) green = 255;
				if (blue > 255) blue = 255;
			}
			m_Dest[m_PPos++] = (red << 16) + (green << 8) + blue;
			m_SX += m_DX;
		}
		m_SY += m_DY;
		// see if we've been working to long already
		if ((GetTickCount() - msecs) > 100) 
		{
			// return control to windows so the screen gets updated
			m_CurrLine = y + 1;
			return false;
		}
	}
	// all done
	return true;
}

}; // namespace Raytracer
