/*
-------------------------------------------------------------------------------
    Copyright (c) Charles Carley.

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------
*/
#ifndef _skBoundingBox_h_
#define _skBoundingBox_h_

#include "Math/skMath.h"
#include "Math/skRay.h"
#include "Math/skVector3.h"
#include "RenderSystem/Math/rtMath.h"
#include "RenderSystem/Math/skBoundingSphere.h"

class skBoundingBox
{
public:
    skScalar bMin[3];
    skScalar bMax[3];

public:
    skBoundingBox()
    {
        clear();
    }

    skBoundingBox(const skScalar _bMin[3], const skScalar _bMax[3])
    {
        for (int i = 0; i < 3; ++i)
        {
            bMin[i] = _bMin[i];
            bMax[i] = _bMax[i];
        }
    }

    skBoundingBox(const skVector3& extent, const skVector3& center)
    {
        const skScalar* ep = extent.ptr();
        const skScalar* cp = center.ptr();

        for (int i = 0; i < 3; ++i)
        {
            bMin[i] = cp[i] - ep[i] * skScalar(0.5);
            bMax[i] = cp[i] + ep[i] * skScalar(0.5);
        }
    }

    void clear()
    {
        for (int i = 0; i < 3; ++i)
        {
            bMin[i] = SK_INFINITY;
            bMax[i] = -SK_INFINITY;
        }
    }

    void compare(const skVector3& pt)
    {
        const skScalar* ep = pt.ptr();
        for (int i = 0; i < 3; ++i)
        {
            if (ep[i] < bMin[i])
                bMin[i] = ep[i];
            if (ep[i] > bMax[i])
                bMax[i] = ep[i];
        }
    }

    void compare(const skScalar* pt)
    {
        for (int i = 0; i < 3; ++i)
        {
            if (pt[i] < bMin[i])
                bMin[i] = pt[i];
            if (pt[i] > bMax[i])
                bMax[i] = pt[i];
        }
    }

    void merge(const skBoundingBox& bb)
    {
        compare(bb.bMin);
        compare(bb.bMax);
    }

    void scale(const skScalar& sc)
    {
        for (int i = 0; i < 3; ++i)
        {
            bMin[i] *= sc;
            bMax[i] *= sc;
        }
    }

    void translate(const skVector3& pt)
    {
        const skScalar* ep = pt.ptr();

        for (int i = 0; i < 3; ++i)
        {
            bMin[i] += ep[i];
            bMax[i] += ep[i];
        }
    }

    SK_INLINE skVector3 min() const
    {
        return skVector3(bMin[0], bMin[1], bMin[2]);
    }

    SK_INLINE skVector3 max() const
    {
        return skVector3(bMax[0], bMax[1], bMax[2]);
    }

    SK_INLINE skVector3 extent() const
    {
        return skVector3(bMax[0] - bMin[0], bMax[1] - bMin[1], bMax[2] - bMin[2]);
    }

    SK_INLINE skScalar halfLength2() const
    {
        return extent().length2() * skScalar(0.5);
    }

    SK_INLINE skScalar halfLength() const
    {
        return extent().length() * skScalar(0.5);
    }

    SK_INLINE skScalar max3() const
    {
        return skMax3(bMax[0], bMax[1], bMax[2]);
    }

    SK_INLINE skScalar min3() const
    {
        return skMax3(bMin[0], bMin[1], bMin[2]);
    }

    skVector3 center() const
    {
        return skVector3(
            skScalar(0.5) * (bMax[0] + bMin[0]),
            skScalar(0.5) * (bMax[1] + bMin[1]),
            skScalar(0.5) * (bMax[2] + bMin[2]));
    }

    void setMin(const skVector3& mi)
    {
        bMin[0] = mi.x;
        bMin[1] = mi.y;
        bMin[2] = mi.z;
    }

    void setMax(const skVector3& ma)
    {
        bMax[0] = ma.x;
        bMax[1] = ma.y;
        bMax[2] = ma.z;
    }

    SK_INLINE skScalar length() const
    {
        return extent().length();
    }

    SK_INLINE skScalar length2() const
    {
        return extent().length();
    }

    SK_INLINE skScalar signedLength() const
    {
        const skVector3 c    = center();
        skScalar        sign = 1;
        if (c.x < 0)
            sign *= -1;
        if (c.y < 0)
            sign *= -1;
        if (c.z < 0)
            sign *= -1;
        return sign * c.length();
    }

    SK_INLINE bool less(const skBoundingBox& bb) const
    {
        return signedLength() < bb.signedLength();
    }

    bool contains(const skBoundingBox& bb) const
    {
        const skVector3 tmi = min();
        const skVector3 tma = max();
        const skVector3 bmi = bb.min();
        const skVector3 bma = bb.max();

        bool res = bmi.x >= tmi.x && bmi.y >= tmi.y && bmi.x >= tmi.z;
        if (res)
            res = bma.x <= tma.x && bma.y <= tma.y && bma.x <= tma.z;
        return res;
    }

    static void majorAxis(skVector3 & dest, const skVector3& src)
    {
        const skScalar m = skMax3(src.x, src.y, src.z);

        if (skEq(m, src.x))
            dest = skVector3::UnitX;
        else if (skEq(m, src.y))
            dest = skVector3::UnitY;
        else
            dest = skVector3::UnitZ;
        if (m < 0)
            dest *= -1;
    }

    bool hit(const skRay& ray, const skVector2& limit) const
    {
        const skScalar* origP = ray.origin.ptr();
        const skScalar* dirP  = ray.direction.ptr();

        skScalar tMin = limit.x;
        skScalar tMax = limit.y;

        for (int i = 0; i < 3; ++i)
        {
            skScalar t0 = 0, t1 = 0;
            if (!skEq(dirP[i], 0))
            {
                const skScalar t2 = 1 / dirP[i];

                t0 = (bMin[i] - origP[i]) * t2;
                t1 = (bMax[i] - origP[i]) * t2;

                if (t2 < skScalar(0.0))
                {
                    const skScalar t = t0;

                    t0 = t1;
                    t1 = t;
                }
            }

            tMin = t0 > tMin ? t0 : tMin;
            tMax = t1 < tMax ? t1 : tMax;
            if (tMax <= tMin)
                return false;
        }
        return true;
    }

    bool hit(skScalar & r0, skScalar & r1, const skRay& ray, const skVector2& limit) const
    {
        const skScalar* origP = ray.origin.ptr();
        const skScalar* dirP  = ray.direction.ptr();

        r0 = limit.x;
        r1 = limit.y;

        for (int i = 0; i < 3; ++i)
        {
            skScalar t0 = 0, t1 = 0;
            if (!skEq(dirP[i], 0))
            {
                const skScalar t2 = 1 / dirP[i];

                t0 = (bMin[i] - origP[i]) * t2;
                t1 = (bMax[i] - origP[i]) * t2;

                if (t2 < skScalar(0.0))
                {
                    const skScalar t = t0;

                    t0 = t1;
                    t1 = t;
                }
            }

            r0 = t0 > r0 ? t0 : r0;
            r1 = t1 < r1 ? t1 : r1;
            if (r1 < r0)
                return false;
        }
        return true;
    }

    bool hit(skRayHitTest & dest, const skRay& ray, const skVector2& limit) const
    {
        const skScalar* origP = ray.origin.ptr();
        const skScalar* dirP  = ray.direction.ptr();

        skScalar tMin = limit.x;
        skScalar tMax = limit.y;

        for (int i = 0; i < 3; ++i)
        {
            skScalar t0 = 0, t1 = 0;

            if (!skEq(dirP[i], 0))
            {
                const skScalar t2 = 1 / dirP[i];

                t0 = (bMin[i] - origP[i]) * t2;
                t1 = (bMax[i] - origP[i]) * t2;

                if (t2 < skScalar(0.0))
                {
                    const skScalar t = t0;

                    t0 = t1;
                    t1 = t;
                }
            }

            tMin = t0 > tMin ? t0 : tMin;
            tMax = t1 < tMax ? t1 : tMax;
            if (tMax <= tMin)
                return false;
        }

        dest.distance = tMin;
        dest.point    = ray.at(tMin);

        const skVector3 p1 = dest.point - center();
        const skVector3 p2 = extent() * skScalar(0.5);
        const skVector3 p3 = p1 / p2;
        const skVector3 p4 = p3.abs();
        const skScalar  m3 = p4.max3();
        dest.normal        = {0, 0, 0};
        if (skEq(m3, p4.x))
            dest.normal.x = skSign(p3.x);
        else if (skEq(m3, p4.y))
            dest.normal.y = skSign(p3.y);
        else
            dest.normal.z = skSign(p3.z);
        return true;
    }
};

#endif  //_skBoundingBox_h_
