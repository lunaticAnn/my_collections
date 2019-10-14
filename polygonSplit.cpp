#include <vector>
#include <cstdio>

struct Vector2
{
    float x;
    float y;
    Vector2():x(0), y(0){}
    Vector2(float x, float y):x(x), y(y){}
    Vector2 operator+ (const Vector2& rhs) {return Vector2(x + rhs.x, y + rhs.y);}
    Vector2 operator* (float rhs) {return Vector2(x * rhs, y * rhs);}
};

struct UIVertex
{
	Vector2 position;
	UIVertex(float x, float y):position(x, y){}
	// this will also conain information like uv/color
};

struct Edge {
	UIVertex* p0;
	UIVertex* p1;
	Edge* nextEdge;
	Edge(UIVertex* p0, UIVertex* p1)
	: p0(p0), p1(p1), nextEdge(nullptr){}
};

struct Ray2D
{
	Vector2 o;
	Vector2 dir;
};

struct Intersection
{
	float t; // for samples
	Vector2 p; // intersection point
};

static inline bool intersect(Ray2D ray, Edge* e, Intersection* intersection)
{
    // store result in intersection object if intersected
    // o + t * dir == v0 * t + v1 * ( 1 - t)
    Vector2 v0 = e->p0->position;
    Vector2 v1 = e->p1->position;
    float x01 = v0.x - v1.x;
    float y01 = v0.y - v1.y;
    float div = x01 * ray.dir.y - ray.dir.x * y01;

    // parallel
    if (div == 0) return false;
    float t = (ray.dir.x * (v1.y - ray.o.y) + ray.dir.y * (ray.o.x - v1.x)) / div;
    if (t >= 0.f && t <= 1.f)
    {
        intersection->p = v0 * t + v1 * (1 - t);
        intersection->t = t;
        return true;
    }
    return false;
}

struct DirectedConvexPolygon {
	// data for vertices
	std::vector<UIVertex>* vertexData;
	Edge* startEdge;
	DirectedConvexPolygon(const std::vector<UIVertex*>& vertexList, std::vector<UIVertex>* data)
	{
		vertexData = data;
		Edge* lastEdge = nullptr;
		// build up edges
		size_t listSize = vertexList.size();
		if (listSize > 2)
		{
			for (size_t i = 0; i < listSize; ++i)
			{
				Edge* e = new Edge(vertexList[i], vertexList[i + 1 < listSize? i + 1 : 0]);
				if (lastEdge != nullptr)
				{
					lastEdge->nextEdge = e;
				}
				else
				{
					startEdge = e;
				}
				lastEdge = e;
			}
			lastEdge->nextEdge = startEdge;
		}	
	}
	
	// ctor for empty polygon
	DirectedConvexPolygon(std::vector<UIVertex>* data, Edge* startEdge = nullptr)
	: startEdge(startEdge)
	, vertexData(data)
	{}

	//Debug use prints
	void iteratePrint()
	{
		printf("interating through polygon edges\n- (%f, %f), (%f, %f)\n", startEdge->p0->position.x, startEdge->p0->position.y,startEdge->p1->position.x, startEdge->p1->position.y);
		Edge* edge = startEdge->nextEdge;
		while(edge != startEdge)
		{
			printf("- (%f, %f), (%f, %f)\n", edge->p0->position.x, edge->p0->position.y, edge->p1->position.x, edge->p1->position.y);
			edge = edge->nextEdge;
		}
	}

	/*
		split 2d convex polygons into polygons with 2d ray. 
		generated polygons will be saved to this polygon/other.
	*/
	bool split(Ray2D ray, DirectedConvexPolygon* other)
	{
		// it will split current polygon into 2 if there are intersections on 2 edges
		Edge* e = startEdge;
		Edge* next = startEdge->nextEdge;

		// if intersect, then split current edge into 2, and point one to another
		Intersection intersection;
		UIVertex *sp0 = nullptr, *sp1 = nullptr;
		if (intersect(ray, e, &intersection))
		{
			// should have a threshold to generate new vertex data
			if (intersection.t < 1e-4)
			{
				sp0 = e->p1;
			}
			else if (1 - intersection.t < 1e-4)
			{
				sp0 = e->p0;
			}
			else
			{
				// generate new vertex and 2 edges, adjusting the polygon mesh
				vertexData->emplace_back(intersection.p.x, intersection.p.y);
				sp0 = &vertexData->back();
				Edge* newEdge = new Edge(sp0, e->p1);
				newEdge->nextEdge = e->nextEdge;
				e->p1 = sp0;
				e->nextEdge = newEdge;
			}
		}
		e = next;

		while (e != startEdge)
		{
			next = e->nextEdge;
			if (intersect(ray, e, &intersection))
        	{
            	// should have a threshold to generate new vertex data
            	if (intersection.t < 1e-4)
            	{
                	sp0 = sp0 == nullptr ? e->p1 : sp0;
					sp1 = sp0 == e->p1 ? sp1 : e->p1;
            	}
            	else if (1 - intersection.t < 1e-4)
            	{
                	sp0 = sp0 == nullptr ? e->p0 : sp0;
                	sp1 = sp0 == e->p0 ? sp1 : e->p0;
            	}
            	else
            	{
                	// generate new vertex and 2 edges, adjusting the polygon mesh
                	vertexData->emplace_back(intersection.p.x, intersection.p.y);
					UIVertex* newVertex;
					if (sp0 == nullptr)
					{	
                		sp0 = &vertexData->back();
						newVertex = sp0;
					}
					else
					{
						sp1 = &vertexData->back();
						newVertex = sp1;
					}
					
                	Edge* newEdge = new Edge(newVertex, e->p1);
                	newEdge->nextEdge = e->nextEdge;
                	e->p1 = newVertex;
                	e->nextEdge = newEdge;
				}
			}
			e = next;
		}

		if (sp0 == nullptr || sp1 == nullptr || sp0 == sp1)
			return false;
	
		// if 2 different intersections
		// find edge start with sp0
		while (e->p0 != sp0) e = e->nextEdge;
	
		//this will be start edge of the other polygon
		other->startEdge = e;
		while (e->p1 != sp1) e = e->nextEdge;

		// new start edge of this polygon
		next = e->nextEdge;
		e->nextEdge = new Edge(sp1, sp0);
		e->nextEdge->nextEdge = other->startEdge;

		e = next;
		while (e->p1 != sp0) e = e->nextEdge;
		e->nextEdge = new Edge(sp0, sp1);
		e->nextEdge->nextEdge = next;
		startEdge = e;
		return true;
	}
};

int main()
{
	std::vector<UIVertex> vs({
		UIVertex(0,0),
		UIVertex(1,0),
		UIVertex(1,1),
		UIVertex(0,1)
	});

	std::vector<UIVertex*> vsPtr({&vs[0], &vs[1], &vs[2], &vs[3]});

	DirectedConvexPolygon poly(vsPtr, &vs);
	poly.iteratePrint();
	Ray2D ray;
	ray.o = Vector2(0.5, 0);
	ray.dir = Vector2(1, 2);
	DirectedConvexPolygon other(&vs);
	poly.split(ray, &other);
	poly.iteratePrint();
	other.iteratePrint();
    return 0;
}
