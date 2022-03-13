#ifndef _INCLUDE_GLB_H
#define _INCLUDE_GLB_H

enum glb_pointers
{
	glb_start_int = 0, 
	trace_hitgroup, 
	trace_flags, 
	msg_entity, 
	cdAudioTrack, 
	maxClients, 
	maxEntities, 
	glb_end_int, 
	glb_start_float, 
	gl_time, 
	frametime, 
	force_retouch, 
	deathmatch, 
	coop, 
	teamplay, 
	serverflags, 
	found_secrets, 
	trace_allsolid, 
	trace_startsolid, 
	trace_fraction, 
	trace_plane_dist, 
	trace_inopen, 
	trace_inwater, 
	glb_end_float, 
	glb_start_edict, 
	trace_ent, 
	glb_end_edict, 
	glb_start_vector, 
	v_forward, 
	v_up, 
	v_right, 
	trace_endpos, 
	trace_plane_normal, 
	vecLandmarkOffset,
	glb_end_vector,
	glb_start_string, 
	mapname, 
	startspot, 
	glb_end_string, 
	glb_start_pchar, 
	pStringBase, 
	glb_end_pchar
};

void initialize_glb_offsets();

#endif /* _INCLUDE_GLB_H */

