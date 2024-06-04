#pragma once
#include "bsp_structs.h"
#include "globals.h"

//extern plane_s gMappedPlanes[MAX_MAP_PLANES];
//extern plane_s sorted_planes[MAX_MAP_PLANES * 2];

extern unsigned int ChooseMidPlaneFromListHL1(node_t* node, face_t* original_face, const vec3_t mins, const vec3_t maxs);
extern unsigned int ChoosePlaneFromList(node_t* node, face_t* list);
extern unsigned int ChooseMidPlaneFromList(node_t* node, int axis);
extern unsigned int FindIntPlane(const vec_t* const normal, const vec_t dist);
extern unsigned int FindIntPlane(const vec_t* const normal, const vec_t* const origin);
extern int PlaneTypeForNormal(const vec3_t normal);
extern int IsDifferentPlane(const struct plane_s* a1, const struct plane_s* a2);
extern unsigned int PlaneFromPoints(const vec3_t const p0, const vec3_t const p1, const vec3_t const p2);
extern vec_t SnapPlaneDist(const vec_t dist, const vec_t epsilon = ON_EPSILON);
extern bool SnapNormal(vec3_t normal, vec_t epsilon = NORMAL_EPSILON);
extern bool PlaneEqual(const plane_s* plane1, const plane_s* plane2, double onepsilon = ON_EPSILON, double normalepsilon = NORMAL_EPSILON);