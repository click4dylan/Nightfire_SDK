#pragma once

#include "vectors.h"
//#include "bsp_structs.h"

#define	SIDE_FRONT		0
#define	SIDE_ON			2
#define	SIDE_BACK		1
#define	SIDE_CROSS		-2
#define	BOGUS_RANGE	32768.0  // was this in hl1: 10000.0
#define MAX_POINTS_ON_WINDING 128

class Winding
{
public:
    virtual ~Winding();
    unsigned int m_NumPoints;
    vec3_t* m_Points;
    unsigned int m_MaxPoints;

    bool HasPoints() { return m_NumPoints >= 3 && m_Points; }
#if 0
    void Divide(bool unknown, vec_t* normal, Winding** front_face, Winding** back_face, double dist)
    {
        reinterpret_cast<void(__stdcall*)(Winding*, bool, vec_t*, Winding**, Winding**, double)>(0x413D90)(this, unknown, normal, front_face, back_face, dist);
    }
#endif
    bool Divide(const vec3_t normal, const vec_t dist, Winding** front, Winding** back);
    bool Divide(const struct plane_s& split, Winding** front, Winding** back);
    void RemoveColinearPoints();
    void shiftPoints(unsigned int offset, unsigned int start_index);
    void initFromPlane(const vec3_t normal, const vec_t dist);
    bool Clip(const vec3_t normal, const vec_t dist, bool keepon);
    bool Clip(const struct plane_s& split, bool keepon);
    bool Clip(const struct plane_s& plane, Winding** front, Winding** back);
    bool Clip(const vec3_t normal, const vec_t dist, Winding** front, Winding** back);
private:
    bool Clip(const bool divide, const vec3_t normal, const vec_t dist, Winding** front, Winding** back);
public:
    void getPlane(struct plane_s& dest) const;
    void getPlane(vec3_t& normal, vec_t& dist) const;
    void addPoint(const vec3_t newpoint);
    void insertPoint(const vec3_t newpoint, const unsigned int offset);
    void resize(UINT32 newsize);
    bool Chop(const vec3_t normal, const vec_t dist, bool keep_front = true);
    void ClearNumPoints() { m_NumPoints = 0; }
    int WindingOnPlaneSide(const vec3_t normal, const vec_t dist);
    int classifyPointAgainstPlaneEdges(char* sides_buffer, const int max_sides, const struct plane_s& plane, const vec3_t point);
    Winding(const Winding& other);
    Winding(const struct dface_s& face);
    Winding();
    Winding(const struct plane_s& plane);
    Winding(UINT32 numpoints);
    void getCenter(vec3_t& center) const;
    vec_t getArea() const;
    void Invert();
};

//todo: check inside RemoveColinearPoints 
//extern void shiftWindingPoints(char* start_point, int offset, Winding* winding);