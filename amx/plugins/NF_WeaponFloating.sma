#include < amxmodx >

#include < engine >
#include < hamsandwich >

#pragma semicolon 1

/*
new const gHalfLifeItemNames[ ][ ] =
{
	"item_battery",
	"item_armor_vest",
	"item_armor_plate",
	"item_longjump",
	"item_suit"
};
*/

new const gHalfLifeWeaponEntitiesNames[ ][ ] = 
{
	"weapon_dukes", "weapon_pp9", "weapon_kowloon", "weapon_raptor", "weapon_mp9", "weapon_mp9_silenced",
	"weapon_commando", "weapon_pdw90", "weapon_frinesi", "weapon_up11", "weapon_l96a1", "weapon_l96a1_winter",
	"weapon_smokegrenade", "weapon_flashgrenade", "weapon_fraggrenade", "weapon_bondmine", "weapon_ronin",
	"weapon_grenadelauncher", "weapon_rocketlauncher", "weapon_watch", "weapon_taser", "weapon_pen", "weapon_pda",
	"weapon_lighter", "weapon_grapple", "weapon_qworm", "weapon_laserrifle", "gadget_nightvision", "ammo_commando",
	"ammo_shotgun", "ammo_grenadelauncher", "ammo_kowloon", "ammo_mini", "ammo_mp9", "ammo_pdw90", "ammo_darts",
	"ammo_p99", "ammo_raptor", "ammo_rocketlauncher", "ammo_sniper", "ammo_up11"
};

new gFloatingWeaponsEnabled;
//new gFloatingWeaponsGlow;
new gFloatingWeaponsSpeed;
//new gFloatingWeaponsGlowThickness;

public plugin_init( )
{
	register_plugin( "NF Rotating Weapons", "2.1.0", "tuty/2GoldenBullet$" );

	new i;

	for( i = 0; i < sizeof( gHalfLifeWeaponEntitiesNames ); i++ )
	{
		register_touch( gHalfLifeWeaponEntitiesNames[ i ], "worldspawn", "WeaponsTouchTheGround" );
		register_touch( gHalfLifeWeaponEntitiesNames[ i ], "func_wall", "WeaponsTouchTheGround" );
	}
/*
	for( i = 0; i < sizeof( gHalfLifeItemNames ); i++ )
	{
		//RegisterHam( Ham_ItemRespawn, gHalfLifeItemNames[ i ], "bacon_ItemsRespawned" );
	}
*/
	gFloatingWeaponsEnabled = register_cvar( "rotating_weapons", "1" );
	//gFloatingWeaponsGlow = register_cvar( "nf_fw_glow", "1" );
	gFloatingWeaponsSpeed = register_cvar( "rotating_weapons_speed", "25.0" );
	//gFloatingWeaponsGlowThickness = register_cvar( "nf_fw_glow_thickness", "10.0" );
}

public WeaponsTouchTheGround( iEntity, iWorldspawn )
{
	if( get_pcvar_num( gFloatingWeaponsEnabled ) != 0 
	&& is_valid_ent( iEntity ) )
	{
		FloatAndSpinWeapons( iEntity );
	}
}

public bacon_ItemsRespawned( iEntity )
{
	console_print(0, "NF_WeaponFloating.amxx: bacon_ItemsRespawned called with value %d", iEntity);
	/*
	if( get_pcvar_num( gFloatingWeaponsEnabled ) != 0 )
	{
		console_print(0, "NF_WeaponFloating.amxx: bacon_ItemsRespawned called with value %d", iEntity);
		entity_set_int( iEntity, EV_INT_movetype, MOVETYPE_TOSS );
		drop_to_floor( iEntity );
		
		FloatAndSpinWeapons( iEntity );
	}
	*/
}

public FloatAndSpinWeapons( iEntity )
{
	//new Float:flWeaponOrigin[ 3 ];
	//entity_get_vector( iEntity, EV_VEC_origin, flWeaponOrigin );
		
	//flWeaponOrigin[ 2 ] += 30.0;
	//entity_set_origin( iEntity, flWeaponOrigin );

	//new Float:flAngles[ 3 ];
	//entity_get_vector( iEntity, EV_VEC_angles, flAngles );
	
	//flAngles[ 0 ] -= float( random( 80 ) );
	//flAngles[ 1 ] += float( random( 50 ) );
	
	//entity_set_vector( iEntity, EV_VEC_angles, flAngles );
	entity_set_int( iEntity, EV_INT_movetype, MOVETYPE_NOCLIP );

	set_task( 0.9, "WeaponSpinGoGo", iEntity );
		
	/*if( get_pcvar_num( gFloatingWeaponsGlow ) == 1 )
	{
		new Float:flRenderColor[ 3 ];
			
		flRenderColor[ 0 ] = random_float( 0.0, 255.0 );
		flRenderColor[ 1 ] = random_float( 0.0, 255.0 );
		flRenderColor[ 2 ] = random_float( 0.0, 255.0 );

		entity_set_int( iEntity, EV_INT_renderfx, kRenderFxGlowShell );
		entity_set_vector( iEntity, EV_VEC_rendercolor, flRenderColor );
		entity_set_float( iEntity, EV_FL_renderamt, get_pcvar_float( gFloatingWeaponsGlowThickness ) );
	}
	*/
}

public WeaponSpinGoGo( iEntity )
{
	if( is_valid_ent( iEntity ) )
	{
		new Float:flAvelocity[ 3 ];
		
		flAvelocity[ 0 ] = 0.0;
		flAvelocity[ 1 ] = get_pcvar_float( gFloatingWeaponsSpeed );
		flAvelocity[ 2 ] = 0.0;
		
		entity_set_vector( iEntity, EV_VEC_avelocity, flAvelocity );
	}
}
