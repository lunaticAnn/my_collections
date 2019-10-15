#include <vector>
#include <list>
#include <cstdio>

struct Vector2
{
    float x;
    float y;
    Vector2():x(0), y(0){}
    Vector2(float x, float y):x(x), y(y){}
    Vector2 operator+ (const Vector2& rhs) {return Vector2(x + rhs.x, y + rhs.y);}
	Vector2 operator- (const Vector2& rhs){return Vector2(x - rhs.x, y - rhs.y);}
    Vector2 operator* (float rhs) {return Vector2(x * rhs, y * rhs);}
	bool operator== (const Vector2& other) {return x == other.x && y == other.y;}
	Vector2 lerp(const Vector2& other, float t) {return Vector2(x * (1 - t) +  t * other.x, y * (1 - t) +  t *  other.y);}
	float dot(const Vector2& other){return other.x * x + other.y * y;}
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
	Edge(const Edge& other)
	:p0(other.p0), p1(other.p1), nextEdge(other.nextEdge){}
	Vector2 toDirection() {return p1->position - p0->position;}
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

static inline bool intersect(const Ray2D& ray, Vector2 v0, Vector2 v1, Intersection* intersection)
{
	// store result in intersection object if intersected
	// o + t * dir == v0 * t + v1 * ( 1 - t)
	float x01 = v0.x - v1.x;
	float y01 = v0.y - v1.y;
	float div = x01 * ray.dir.y - ray.dir.x * y01;

	// parallel
	if (div == 0) return false;
	float t = (ray.dir.x * (v1.y - ray.o.y) + ray.dir.y * (ray.o.x - v1.x)) / div;
	intersection->t = t;
	if (t >= 0.f && t <= 1.f)
	{
		intersection->p = v0 * t + v1 * (1 - t);
		return true;
	}
	return false;
}

static inline bool intersect(const Ray2D& ray, Edge* e, Intersection* intersection)
{
	return intersect(ray, e->p0->position, e->p1->position, intersection);
}

// a ray vector is a vector that represent affecting force and directions on a line segment in 2d space.
// besides all information an edge will contain, ray vector provide sample ability on its own segment
// mapping from (0, 1)
struct RayVector
{
	Vector2 start;
	Vector2 end;
	Vector2 dir; // generated in ctor
	std::vector<float> sortedKeys; // keys sorted by Key.t

	RayVector(const Vector2& start, const Vector2& end)
	: start(start)
	, end(end)
	{
		dir = Vector2(start.y - end.y, end.x - start.x);
	}

	float sample(Vector2 p)
	{
		Ray2D ray;
		ray.o = p;
		ray.dir = dir;
		Intersection intersection;
		intersect(ray, start, end, &intersection);
		return intersection.t < 0.f ? 0.f : (1.f < intersection.t) ? 1.f : intersection.t;	
	}
};

struct DirectedConvexPolygon {
	// data for vertices, in case in the future we need VBO indexing
	std::list<UIVertex>* vertexData;
	std::list<Edge>* edgeData;
	Edge* startEdge;
	DirectedConvexPolygon(const std::vector<UIVertex*>& vertexList, std::list<UIVertex>* vData, std::list<Edge>* eData)
	{
		vertexData = vData;
		edgeData = eData;
		Edge* lastEdge = nullptr;
		// build up edges
		size_t listSize = vertexList.size();
		if (listSize > 2)
		{
			for (size_t i = 0; i < listSize; ++i)
			{
				edgeData->push_back(Edge(vertexList[i], vertexList[i + 1 < listSize? i + 1 : 0]));
				Edge* e = &edgeData->back();
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
	DirectedConvexPolygon(std::list<UIVertex>* vertices, std::list<Edge>* edges, Edge* startEdge = nullptr)
	: startEdge(startEdge)
	, vertexData(vertices)
	, edgeData(edges)
	{}

	DirectedConvexPolygon(const DirectedConvexPolygon& other)
	: startEdge(other.startEdge)
	, vertexData(other.vertexData)
	, edgeData(other.edgeData){}

	//Debug use prints
	void iteratePrint()
	{
		printf("interating through polygon edges\n[%p]- (%f, %f), (%f, %f)\n", startEdge, startEdge->p0->position.x, startEdge->p0->position.y,startEdge->p1->position.x, startEdge->p1->position.y);
		Edge* edge = startEdge->nextEdge;
		while(edge != startEdge)
		{
			printf("[%p]- (%f, %f), (%f, %f)\n", edge, edge->p0->position.x, edge->p0->position.y, edge->p1->position.x, edge->p1->position.y);
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
				edgeData->emplace_back(sp0, e->p1);
				Edge* newEdge = &edgeData->back();
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
	
					edgeData->emplace_back(newVertex, e->p1);
					Edge* newEdge = &edgeData->back();
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
		edgeData->emplace_back(sp1, sp0);
		e->nextEdge = &edgeData->back();
		e->nextEdge->nextEdge = other->startEdge;

		e = next;
		while (e->p1 != sp0) e = e->nextEdge;
		edgeData->emplace_back(sp0, sp1);
		e->nextEdge = &edgeData->back();
		e->nextEdge->nextEdge = next;
		startEdge = next;
		return true;
	}
};

void generateTessellation(DirectedConvexPolygon* polygon, RayVector spacialModifier, std::vector<DirectedConvexPolygon>* polygons)
{
	DirectedConvexPolygon* current = polygon;
	Vector2 modifierDir = spacialModifier.end - spacialModifier.start;
	for(const auto& t:spacialModifier.sortedKeys)
	{
		Ray2D ray;
		ray.o = spacialModifier.start.lerp(spacialModifier.end, t);
		ray.dir = spacialModifier.dir;
		DirectedConvexPolygon other(polygon->vertexData, polygon->edgeData);
		if (current->split(ray, &other))
		{
			// determine which will be next polygon to split
			if (current->startEdge->toDirection().dot(modifierDir) < 0)
			{
				polygons->emplace_back(current->vertexData, current->edgeData, current->startEdge);
				current->startEdge = other.startEdge;
			}
			else
			{
				polygons->emplace_back(other.vertexData, other.edgeData, other.startEdge);
			}
		}
	}
	polygons->emplace_back(current->vertexData, current->edgeData, current->startEdge);
}

int main()
{
	std::list<UIVertex> vs({
		UIVertex(0,0),
		UIVertex(1,0),
		UIVertex(1,1),
		UIVertex(0,1)
	});
	std::list<Edge> es;
	std::vector<UIVertex*> vsPtr({
		&(*std::next(vs.begin() , 0)),		
		&(*std::next(vs.begin() , 1)),
		&(*std::next(vs.begin() , 2)),
		&(*std::next(vs.begin() , 3))
	});

	DirectedConvexPolygon poly(vsPtr, &vs, &es);
	std::vector<DirectedConvexPolygon> polygons;
	
	RayVector sampler(Vector2(0,0), Vector2(1, 1));
	sampler.sortedKeys.push_back(0.2);
	sampler.sortedKeys.push_back(0.5);
	sampler.sortedKeys.push_back(0.6);

	generateTessellation(&poly, sampler, &polygons);
	for(auto p:polygons)
	{
		p.iteratePrint();
	} 
	return 0;
}
