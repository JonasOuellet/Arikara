#pragma once
#include <maya/MPoint.h>
#include <maya/MVector.h>

namespace ArikaraMayaMath
{

    /**
    * Find the intersection of a line and an offset plane. Assumes that the
    * line and plane do indeed intersect; you must make sure they're not
    * parallel before calling.
    *
    * @param Point1 the first point defining the line
    * @param Point2 the second point defining the line
    * @param PlaneOrigin the origin of the plane
    * @param PlaneNormal the normal of the plane
    *
    * @return The point of intersection between the line and the plane.
    */
    inline MPoint LinePlaneIntersection
    (
        MPoint& Point1,
        MPoint& Point2,
        MPoint& pPlane1,
        MPoint& pPlane2,
        MPoint& pPlane3
    )
    {
        // to find the normal we have to do the cross product of 2 vector on plane
        //optened with the 3 point
        MVector PlaneNormal = (pPlane1 - pPlane2) ^ (pPlane1 - pPlane3);

        return
            Point1
            + (Point2 - Point1)
            *	(((pPlane1 - Point1) * PlaneNormal) / ((Point2 - Point1) * PlaneNormal));
    };

    inline MPoint LinePlaneIntersection
    (
        MPoint& Point1,
        MVector& P1Normal,
        MPoint& pPlane1,
        MPoint& pPlane2,
        MPoint& pPlane3
    )
    {
        // to find the normal we have to do the cross product of 2 vector on plane
        //optened with the 3 point
        MVector PlaneNormal = (pPlane1 - pPlane2) ^ (pPlane1 - pPlane3);

        return
            Point1
            + P1Normal
            *	(((pPlane1 - Point1) * PlaneNormal) / (P1Normal * PlaneNormal));
    };

    /**
    * Find the barycentricCoordinate of a point on a given triangle.
    *
    * @param A the frist point to define the triangle
    * @param B the second point to define the triangle
    * @param C the third point to define the triangle
    * @param pInPoint the point to find the coord for.
    *
    * @param u out weight for the first point
    * @param v out weight for the second point
    * @param 2 out weight for the third point
    *
    * @return bool if the point is on the triangle.
    */
    bool BarycentricCoordinate(MPoint& A, MPoint& B, MPoint& C, MPoint& pInPoint, double& u, double& v, double& w);

    template<class T>
    typename std::enable_if<!std::numeric_limits<T>::is_integer, bool>::type
        almost_equal(T x, T y, int ulp)
    {
        // the machine epsilon has to be scaled to the magnitude of the values used
        // and multiplied by the desired precision in ULPs (units in the last place)
        return std::abs(x - y) < std::numeric_limits<T>::epsilon() * std::abs(x + y) * ulp
            // unless the result is subnormal
            || std::abs(x - y) < std::numeric_limits<T>::min();
    }
}
