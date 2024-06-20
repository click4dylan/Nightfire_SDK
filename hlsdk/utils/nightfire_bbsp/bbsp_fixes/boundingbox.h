#pragma once

struct BoundingBox
{
    vec3_t m_Mins{};
    vec3_t m_Maxs{};

    void add(const vec3_t point)
    {
        m_Mins[0] = min(m_Mins[0], point[0]);
        m_Maxs[0] = max(m_Maxs[0], point[0]);
        m_Mins[1] = min(m_Mins[1], point[1]);
        m_Maxs[1] = max(m_Maxs[1], point[1]);
        m_Mins[2] = min(m_Mins[2], point[2]);
        m_Maxs[2] = max(m_Maxs[2], point[2]);
    }

    void add(const BoundingBox& other)
    {
        add(other.m_Mins);
        add(other.m_Maxs);
    }

    void reset()
    {
#ifdef VEC_T_IS_FLOAT
        VectorFill(m_Mins, 1000000000.0);
        VectorFill(m_Maxs, -1000000000.0);
#else
        VectorFill(m_Mins, 999999999.999);
        VectorFill(m_Maxs, -999999999.999);
#endif
    }
};