#pragma once
#include "vectors.h"

extern texinfo_t* TexinfoForBrushTexture(side_t* side, const vec3_t origin);
extern void TextureAxisFromPlaneHL1(const plane_t* const pln, vec3_t xv, vec3_t yv);
extern int TextureAxisFromPlane(const struct plane_s* pln);
extern bool TextureAxisFromPlane(const struct plane_s* pln, vec3_t xv, vec3_t yv);
extern void GetLightmapProjections(vec3_t out_1, face_t* face, vec3_t out_2);
extern void GetLightmapProjections_Original(double* dest_1_vec3_t, face_t* face, double* dest_2_vec3_t);
extern unsigned int GetTextureIndex(const char* src);
extern unsigned int GetMaterialIndex(const char* src);
extern unsigned int GetProjectionIndex(double s[4], double t[4]);