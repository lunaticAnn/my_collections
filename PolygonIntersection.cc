#include <vector>
#include <optional>
#include <cstdio>

using namespace std;

struct Vector2
{
    Vector2() = default;
    Vector2(double _x, double _y)
    : x(_x)
    , y(_y)
    {
    }
    
    Vector2(const Vector2& other) = default;
    
    Vector2 operator+(const Vector2& other)
    {
        return {x + other.x, y + other.y};
    }

    Vector2 operator-(const Vector2& other)
    {
        return {x - other.x, y - other.y};
    }
    
    Vector2 operator-()
    {
        return {-x, -y};
    }
    
    Vector2 operator*(float rhs)
    {
        return {x * rhs, y * rhs};
    }

    double x;
    double y;
};

double cross(Vector2 v0, Vector2 v1)
{
    return v0.x * v1.y - v0.y * v1.x;
}

double dot(Vector2 v0, Vector2 v1)
{
    return v0.x * v1.x + v0.y * v1.y;
}

// hmmmmm...eyes
optional<Vector2> intersect(Vector2 p0, Vector2 p1, Vector2 q0, Vector2 q1)
{
    double denom = (p0.x - p1.x) * (q0.y - q1.y) - (p0.y - p1.y) * (q0.x - q1.x);
    
    // parallel or coincident
    if (denom == 0)
    {
        return std::nullopt;  // No intersection
    }
    
    double t = (p0.x - q0.x) * (q0.y - q1.y) - (p0.y - q0.y) * (q0.x - q1.x);
    double u = (p0.y - p1.y) * (p0.x - q0.x) - (p0.x - p1.x) * (p0.y - q0.y);
    
    t /= denom;
    u /= denom;
    
    // Check if the intersection is within the bounds of both segments
    if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
        Vector2 intersection;
        intersection.x = p0.x + t * (p1.x - p0.x);
        intersection.y = p0.y + t * (p1.y - p0.y);
        return intersection;
    }
    
    return std::nullopt; 
}

// if not, what would you do to make sure clockwise/counter-clockwise align
struct ConvexPolygon
{
    // points guaranteed to be sorted clockwise
    vector<Vector2> points;
};

optional<ConvexPolygon> intersect(ConvexPolygon p1, ConvexPolygon p2)
{
    
}

int main()
{
    return 0;
}
