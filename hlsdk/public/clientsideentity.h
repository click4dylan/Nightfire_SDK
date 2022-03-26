#pragma once
#include <string>
#include <vector>

typedef struct
{
	char* name;
	void* (*ClientEntityHandler) (std::string const& name, std::vector<std::pair<std::string, std::string>>const& keyvalues);
}CLIENTSIDEENTITY;

/*
worldspawn
41015110

env_fog
41015790

env_sprite
41015d80

env_glow
41015d80

item_generic
41015aa0

item_breakable
41015b80

physics_lantern
41015ca0
*/