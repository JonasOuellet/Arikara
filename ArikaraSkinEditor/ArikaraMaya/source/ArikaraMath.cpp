#include "../include/ArikaraMath.h"

namespace ArikaraMayaMath
{
    bool ArikaraMayaMath::BarycentricCoordinate(MPoint& A, MPoint& B, MPoint& C, MPoint& pInPoint, double& u, double &v, double &w)
    {
        // Compute the normal of the triangle
        MVector TriNorm = (B - A) ^ (C - A);

        //check collinearity of A,B,C
        //check(TriNorm.SizeSquared() > SMALL_NUMBER && "Collinear points in FMath::ComputeBaryCentric2D()");

        MVector N = TriNorm.normal();

        // Compute twice area of triangle ABC
        const double AreaABCInv = 1.0f / (N * TriNorm);

        // Compute a contribution
        const double AreaPBC = N * ((B - pInPoint) ^ (C - pInPoint));
        const double tmpX = AreaPBC * AreaABCInv;

        // Compute b contribution
        const double AreaPCA = N * ((C - pInPoint) ^ (A - pInPoint));
        const double tmpY = AreaPCA * AreaABCInv;

        /*if (tmpX < 0.0001)
            return false;
        if (tmpY < 0.0001)
            return false;

        if (tmpX + tmpY > 1.0001)
            return false;*/

        if (tmpX < 0.0 && !almost_equal(tmpX, 0.0, 5))
            return false;
        if (tmpY < 0.0 && !almost_equal(tmpY, 0.0, 5))
            return false;

        if (tmpX + tmpY > 1.0000)
            return false;

        if (tmpX > 1.0 && !almost_equal(tmpX, 1.0, 5))
            return false;
        if (tmpY > 1.0 && !almost_equal(tmpY, 1.0, 5))
            return false;

        u = tmpX;
        v = tmpY;
        w = 1.0 - tmpX - tmpY;
        return true;
    }

}
