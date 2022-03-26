//========= Copyright © 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#if !defined ( STUDIOMODELRENDERER_H )
#define STUDIOMODELRENDERER_H
#if defined( _WIN32 )
#pragma once
#endif

/*
====================
CStudioModelRenderer

====================
*/
class CStudioModelRenderer
{
public:
	// Construction/Destruction
	CStudioModelRenderer( void );
	virtual ~CStudioModelRenderer( void );

	// Initialization
	virtual void Init( void );

public:  
	// Public Interfaces
	virtual int StudioDrawModel ( int flags );
	virtual int StudioDrawPlayer ( int flags, struct entity_state_s *pplayer );
	
	virtual void StudioGetAnimPos (cl_entity_t *,float *,float);

public:
	// Local interfaces
	//

	// Look up animation data for sequence
	virtual mstudioanim_t *StudioGetAnim ( model_t *m_pSubModel, mstudioseqdesc_t *pseqdesc );

	virtual void StudioSetUpTransform (int trivial_accept);

	// Interpolate model position and angles and set up matrices
	virtual void StudioSetUpPredictedTransform (int trivial_accept);

	// Set up model bone positions
	virtual void StudioSetupBones ( void );	
	
	virtual void StudioSetupBones ( float );	

	// Find final attachment points
	virtual void StudioCalcAttachments ( void );
	
	// Save bone matrices and names
	virtual void StudioSaveBones( void );

	// Merge cached bones with current bones for model
	virtual void StudioMergeBones ( model_t *m_pSubModel );

	// Determine interpolation fraction
	virtual float StudioEstimateInterpolant( void );

	// Determine current frame for rendering
	virtual float StudioEstimateFrame ( mstudioseqdesc_t *pseqdesc );
	
	virtual bool StudioFrameOver(mstudioseqdesc_t* pseqdesc);
	
	virtual float StudioEstimateLastFrame(mstudioseqdesc_t* pseqdesc); //double in mac?
	
	virtual float StudioEstimatePrevFrame(mstudioseqdesc_t* pseqdesc); //double in mac?
	
	virtual float StudioEstimateBlendedFrame(mstudioseqdesc_t* pseqdesc, int frame); //double in mac?

	// Apply special effects to transform matrix
	virtual void StudioFxTransform( cl_entity_t *ent, float transform[3][4] );

	// Spherical interpolation of bones
	virtual void StudioSlerpBones ( vec4_t q1[], float pos1[][3], vec4_t q2[], float pos2[][3], float s );
	
	virtual void StudioSlerpBones ( vec4_t q1[], float pos1[][3], vec4_t q2[], float pos2[][3], float s, int frame );
	
	virtual void StudioSlerpMouthBones ( vec4_t q1[], float pos1[][3], vec4_t q2[], float pos2[][3], float s, float cycle );

	// Compute bone adjustments ( bone controllers )
	virtual void StudioCalcBoneAdj ( float dadt, float *adj, const byte *pcontroller1, const byte *pcontroller2, byte mouthopen );

	// Get bone quaternions
	virtual void StudioCalcBoneQuaterion ( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *q );

	// Get bone positions
	virtual void StudioCalcBonePosition ( int frame, float s, mstudiobone_t *pbone, mstudioanim_t *panim, float *adj, float *pos );

	// Compute rotations
	virtual void StudioCalcRotations ( float pos[][3], vec4_t *q, mstudioseqdesc_t *pseqdesc, mstudioanim_t *panim, float f );

	// Send bones and verts to renderer
	virtual void StudioRenderModel ( int unknown );
	
	// Player specific data
	// Determine pitch and blending amounts for players
	virtual void StudioPlayerBlend ( mstudioseqdesc_t *pseqdesc, int *pBlend, float *pPitch );
	
	// Estimate gait frame for player
	virtual void StudioEstimateGait ( entity_state_t *pplayer );
	
	// Process movement of player
	virtual void StudioProcessGait ( entity_state_t *pplayer );
	
	virtual float GetTotalAnimationDistance(mstudioseqdesc_t* pseqdesc);
	virtual void Studio_Lod_Update();
	virtual void Studio_Lod_SetBodygroup(int, int, int);
	virtual int Studio_Lod_GetBodygroup(int);
	virtual int Studio_Lod_GetPureBodygroup(int);

public:

//NIGHTFIRE STRUCT UNFINISHED BELOW
	int version;
	int size;
	double m_clTime;
	double m_clOldTime;
	bool m_fDoInterp;
	char pad_interp[3];
	int m_fGaitEstimation;
	int m_nFrameCount;
	ConsoleVariable* m_pCvarHiModels;
	ConsoleVariable* m_pCvarDeveloper;
	ConsoleVariable* m_pCvarDrawEntities;
	cl_entity_s* m_pCurrentEntity;
	model_t* m_pRenderModel;
	player_info_t* m_pPlayerInfo;
	int m_nPlayerIndex;
	float m_flGaitMovement;
	studiohdr_t* m_pStudioHeader;
	mstudiobodyparts_t* m_pBodyPart;
	mstudiomodel_t* m_pSubModel;
	int pad[3074];
	float m_vUp[3];
	float m_vRight[3];
	float m_vNormal[3];
	float m_vRenderOrigin[3];
	int* m_pModelsDrawn;
	float(*m_protationmatrix)[3][4];
	float(*m_paliastransform)[3][4];
	float(*m_pbonetransform)[128][3][4];
	float(*m_plighttransform)[128][3][4];
};
//unknown size

#endif // STUDIOMODELRENDERER_H