#include < amxmodx >

#include < engine >

#include < hamsandwich >

#pragma semicolon 1


public plugin_init( )
{
	register_plugin( "NF Test", "1.0.0", "ignacio" );

	for( new i = 0; i < 2; i++ )
	{
		server_print( "NF_TEST.amxx: TEST TEST TEST\n" );
	}
}