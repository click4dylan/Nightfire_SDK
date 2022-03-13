#include "sdk/amxxmodule.h"
#include "ns.h"

#include "CVector.h"
#include "CString.h"

#include "ParticleManager.h"


void ParticleManager::ReadFile(void)
{
	this->Prune();
	if (m_iFileLoaded!=0)
	{
		return;
	}
	m_iFileLoaded=1;

	char FileName[256];

	snprintf(FileName,sizeof(FileName)-1,"%s/ns.ps",MF_GetModname());
	FILE *fp=fopen(FileName,"r");

	if (!fp)
	{
		MF_Log("ParticleManager: Cannot open \"%s\" for reading!",FileName);
		return;
	}

	// Since I don't care about the actual parameters of each
	// particle system, just their order and name
	// I only have to scan for "start pSystemName NAME"

	char Buffer[1024];

	char *Start;
	char *End;

	int Count=0;

	memset(&Buffer[0],0x0,sizeof(Buffer));

	while (!feof(fp))
	{
		Buffer[0]='\0';

		fgets(Buffer,1023,fp);

		Start=&Buffer[0];

		// strip whitespace from the front
		while (*Start==' ' ||
			*Start=='\t')
		{
			++Start;
		}

		// if the first character is ' ignore it
		if (*Start=='\'')
		{
			continue;
		}

		// if the first word is "start" then this is a line we want

		if (strncmp("start ",Start,6)!=0)
		{
			continue;
		}

		// Move up past 2 space blocks

		while (*Start!='\0' &&
			*Start!=' ' &&
			*Start!='\t')
		{
			++Start;
		}
		while (*Start==' ' ||
			*Start=='\t')
		{
			++Start;
		}

		while (*Start!='\0' &&
			*Start!=' ' &&
			*Start!='\t')
		{
			++Start;
		}
		while (*Start==' ' ||
			*Start=='\t')
		{
			++Start;
		}

		// now strip whitespace from the end

		End=Start+strlen(Start)-1;

		while (*End=='\n' ||
			*End=='\r' ||
			*End==' ' ||
			*End=='\t')
		{
			*End--='\0';
		}

		// "Start" should now point to the name of this particle system

		//printf("Particle system %d = \"%s\"\n",Count,Start);


		this->Add(Start,1);


		++Count;
	}

	fclose(fp);
};
