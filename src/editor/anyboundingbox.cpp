#include "anyboundingbox.h"

anyBoundingBox::anyBoundingBox(): from(0, 0, 0), to(0, 0, 0) {
    trans.setToIdentity();
}

bool anyBoundingBox::is_point_inside(anyVector const &p, float r)
{
    anyTransform trans_inv = trans.inverted();
    anyVector p2 = trans_inv*p;
    return p2.x + r >= from.x && p2.x - r <= to.x
        && p2.y + r >= from.y && p2.y - r <= to.y
        && p2.z + r >= from.z && p2.z - r <= to.z;
}

void anyBoundingBox::fix()
{
    if (from.x > to.x)
        from.x = to.x = (from.x + to.x)*0.5f;
    if (from.y > to.y)
        from.y = to.y = (from.y + to.y)*0.5f;
    if (from.z > to.z)
        from.z = to.z = (from.z + to.z)*0.5f;
}

void anyBoundingBox::update_bounding_box_by_point(anyVector v, anyVector &u_from, anyVector &u_to, bool &first)
{
    v = trans*v;
    if (first)
    {
        u_from = v;
        u_to = v;
        first = false;
    }
    else
    {
        if (v.x < u_from.x)
            u_from.x = v.x;
        if (v.x > u_to.x)
            u_to.x = v.x;

        if (v.y < u_from.y)
            u_from.y = v.y;
        if (v.y > u_to.y)
            u_to.y = v.y;

        if (v.z < u_from.z)
            u_from.z = v.z;
        if (v.z > u_to.z)
            u_to.z = v.z;
    }
}

void anyBoundingBox::update_bounding_box(anyVector &u_from, anyVector &u_to, bool &first)
{
    anyVector v;

    v.set(from.x, from.y, from.z);
    update_bounding_box_by_point(v, u_from, u_to, first);
    v.set(from.x, from.y, to.z);
    update_bounding_box_by_point(v, u_from, u_to, first);
    v.set(from.x, to.y,   from.z);
    update_bounding_box_by_point(v, u_from, u_to, first);
    v.set(from.x, to.y,   to.z);
    update_bounding_box_by_point(v, u_from, u_to, first);
    v.set(to.x,   from.y, from.z);
    update_bounding_box_by_point(v, u_from, u_to, first);
    v.set(to.x,   from.y, to.z);
    update_bounding_box_by_point(v, u_from, u_to, first);
    v.set(to.x,   to.y,   from.z);
    update_bounding_box_by_point(v, u_from, u_to, first);
    v.set(to.x,   to.y,   to.z);
    update_bounding_box_by_point(v, u_from, u_to, first);
}
