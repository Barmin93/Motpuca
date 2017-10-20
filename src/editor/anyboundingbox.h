#ifndef ANYBOUNDINGBOX_H
#define ANYBOUNDINGBOX_H

#include "vector.h"
#include "transform.h"

class anyBoundingBox
{
public:
    anyTransform trans;    ///< transformation matrix: rotation + translation
    anyVector from;        ///< start point
    anyVector to;          ///< end point

    anyBoundingBox();
    void update_bounding_box_by_point(anyVector v, anyVector &u_from, anyVector &u_to, bool &first);
    virtual void update_bounding_box(anyVector &from, anyVector &to, bool &first);
    virtual bool is_point_inside(anyVector const &p, real r);
    void fix();


};

#endif // ANYBOUNDINGBOX_H
