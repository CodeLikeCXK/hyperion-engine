/* Copyright (c) 2024 No Tomorrow Games. All rights reserved. */

#include <core/math/Triangle.hpp>
#include <core/math/MathUtil.hpp>

namespace hyperion {

Triangle::Triangle()
    : points {}
{
}

Triangle::Triangle(const Vertex& v0, const Vertex& v1, const Vertex& v2)
    : points({ v0, v1, v2 })
{
}

Triangle::Triangle(const Vec3f& v0, const Vec3f& v1, const Vec3f& v2)
    : points({ Vertex(v0), Vertex(v1), Vertex(v2) })
{
}

Vertex& Triangle::Closest(const Vec3f& vec)
{
    float distances[3];
    uint32 shortestIndex = 0;

    for (uint32 i = 0; i < 3; i++)
    {
        distances[i] = points[i].GetPosition().DistanceSquared(vec);

        if (i != 0)
        {
            if (distances[i] < distances[shortestIndex])
            {
                shortestIndex = i;
            }
        }
    }

    return points[shortestIndex];
}

const Vertex& Triangle::Closest(const Vec3f& vec) const
{
    return const_cast<Triangle*>(this)->Closest(vec);
}

BoundingBox Triangle::GetBoundingBox() const
{
    return BoundingBox()
        .Union(points[0].GetPosition())
        .Union(points[1].GetPosition())
        .Union(points[2].GetPosition());
}

bool Triangle::ContainsPoint(const Vec3f& pt) const
{
    const Vec3f v0 = points[2].GetPosition() - points[0].GetPosition();
    const Vec3f v1 = points[1].GetPosition() - points[0].GetPosition();
    const Vec3f v2 = pt - points[0].GetPosition();

    const float dot00 = v0.Dot(v0);
    const float dot01 = v0.Dot(v1);
    const float dot02 = v0.Dot(v2);
    const float dot11 = v1.Dot(v1);
    const float dot12 = v1.Dot(v2);

    const float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    const float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    const float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    return (u >= 0.0f) && (v >= 0.0f) && (u + v < 1.0f);
}

Triangle operator*(const Matrix4& transform, const Triangle& triangle)
{
    const Matrix4 normalMatrix = transform.Inverted().Transposed();

    Triangle result;

    for (SizeType i = 0; i < 3; i++)
    {
        result.points[i] = triangle.points[i];
        result.points[i].SetPosition(transform * triangle.points[i].GetPosition());
        result.points[i].SetNormal((normalMatrix * Vec4f(triangle.points[i].GetNormal(), 0.0f)).GetXYZ());
        result.points[i].SetTangent((normalMatrix * Vec4f(triangle.points[i].GetTangent(), 0.0f)).GetXYZ());
        result.points[i].SetBitangent((normalMatrix * Vec4f(triangle.points[i].GetBitangent(), 0.0f)).GetXYZ());
    }

    return result;
}

} // namespace hyperion
