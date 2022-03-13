/***
 *
 *	Copyright (c) 2009, Valve LLC. All rights reserved.
 *	
 *	This product contains software technology licensed from Id 
 *	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc. 
 *	All Rights Reserved.
 *
 *   Use, distribution, and modification of this source code and/or resulting
 *   object code is restricted to non-commercial enhancements to products from
 *   Valve LLC.  All other use, distribution, or modification is prohibited
 *   without written permission from Valve LLC.
 *
 ****/

#ifndef ENUMS_H
#define ENUMS_H

typedef enum netsrc_s
	{
		NS_CLIENT,
		NS_SERVER,
		NS_MULTICAST	// xxxMO
	} netsrc_t;

typedef enum
{
	NA_UNUSED,
	NA_LOOPBACK,
	NA_BROADCAST,
	NA_IP,
	NA_IPX,
	NA_BROADCAST_IPX,
} netadrtype_t;

typedef struct netadr_s
{
	netadrtype_t type;
	unsigned char ip[4];
	unsigned char ipx[10];
	unsigned short port;
} netadr_t;
	
#endif

