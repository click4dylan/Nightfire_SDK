#pragma warning(disable: 4018) //amckern - 64bit - '<' Singed/Unsigned Mismatch

//
//	Transparency Arrays for sparse and vismatrix methods
//
#include "qrad.h"

#ifdef HLRAD_HULLU

#define TRANS_LIST_GROWTH 64
#define RAW_LIST_GROWTH 2048

typedef struct {
	unsigned	p1;
	unsigned	p2;
	unsigned	data_index;
} transList_t;

static vec3_t *		s_trans_list	= NULL;
static unsigned int	s_trans_count	= 0;
static unsigned int	s_max_trans_count = 0;

static transList_t*	s_raw_list	= NULL;
static unsigned int	s_raw_count	= 0;
static unsigned int	s_max_raw_count	= 0;	// Current array maximum (used for reallocs)

static transList_t*	s_sorted_list	= NULL;	// Sorted first by p1 then p2
static unsigned int	s_sorted_count	= 0;

const vec3_t vec3_one = {1.0,1.0,1.0};

//===============================================
// AddTransparencyToRawArray
//===============================================
static unsigned AddTransparencyToDataList(const vec3_t trans)
{
	//Check if this value is in list already
	for(int i = 0; i < s_trans_count; i++)
	{
		if( VectorCompare( trans, s_trans_list[i] ) )
		{
			return i;
		}
	}
	
	//realloc if needed
	while( s_trans_count >= s_max_trans_count )
	{
		unsigned int old_max_count = s_max_trans_count;
		s_max_trans_count += TRANS_LIST_GROWTH;
		
		s_trans_list = (vec3_t *)realloc( s_trans_list, sizeof(vec3_t) * s_max_trans_count );
		
		memset( &s_trans_list[old_max_count], 0, sizeof(vec3_t) * TRANS_LIST_GROWTH );
		
		if( old_max_count == 0 )
		{
			VectorFill(s_trans_list[0], 1.0);
			s_trans_count++;
		}
	}
	
	VectorCopy(trans, s_trans_list[s_trans_count]);
	
	return ( s_trans_count++ );
}

//===============================================
// AddTransparencyToRawArray
//===============================================
void	AddTransparencyToRawArray(const unsigned p1, const unsigned p2, const vec3_t trans)
{
	//make thread safe
	ThreadLock();
	
	unsigned data_index = AddTransparencyToDataList(trans);
	
	//realloc if needed
	while( s_raw_count >= s_max_raw_count )
	{
		unsigned int old_max_count = s_max_raw_count;
		s_max_raw_count += RAW_LIST_GROWTH;
		
		s_raw_list = (transList_t *)realloc( s_raw_list, sizeof(transList_t) * s_max_raw_count );
		
		memset( &s_raw_list[old_max_count], 0, sizeof(transList_t) * RAW_LIST_GROWTH );
	}
	
	s_raw_list[s_raw_count].p1 = p1;
	s_raw_list[s_raw_count].p2 = p2;
	s_raw_list[s_raw_count].data_index = data_index;
	
	s_raw_count++;
	
	//unlock list
	ThreadUnlock();
}

//===============================================
// SortList
//===============================================
static int CDECL SortList(const void *a, const void *b)
{
	const transList_t* item1 = (transList_t *)a;
	const transList_t* item2 = (transList_t *)b;
	
	if( item1->p1 == item2->p1 )
	{
		return item1->p2 - item2->p2;
	}
	else
	{
		return item1->p1 - item2->p1;
	}
}

//===============================================
// CreateFinalTransparencyArrays
//===============================================
void	CreateFinalTransparencyArrays(const char *print_name)
{
	if( s_raw_count == 0 )
	{
		s_raw_list = NULL;
		s_raw_count = s_max_raw_count = 0;
		return;
	}

	//double sized (faster find function for sorted list)
	s_sorted_count = s_raw_count * 2;
	s_sorted_list = (transList_t *)malloc( sizeof(transList_t) * s_sorted_count );
	
	//First half have p1>p2
	for( unsigned int i = 0; i < s_raw_count; i++ )
	{
		s_sorted_list[i].p1 		= s_raw_list[i].p2;
		s_sorted_list[i].p2 		= s_raw_list[i].p1;
		s_sorted_list[i].data_index	= s_raw_list[i].data_index;
	}
	//Second half have p1<p2
	memcpy( &s_sorted_list[s_raw_count], s_raw_list, sizeof(transList_t) * s_raw_count );
	
	//free old array
	free( s_raw_list );
	s_raw_list = NULL;
	s_raw_count = s_max_raw_count = 0;
	
	//need to sorted for fast search function
	qsort( s_sorted_list, s_sorted_count, sizeof(transList_t), SortList );
	
	unsigned size = s_sorted_count * sizeof(transList_t) + s_max_trans_count * sizeof(vec3_t);
	if ( size > 1024 * 1024 )
        	Log("%-20s: %5.1f megs \n", print_name, size / (1024 * 1024.0));
        else if ( size > 1024 )
        	Log("%-20s: %5.1f kilos\n", print_name, size / 1024.0);
        else
        	Log("%-20s: %5.1f bytes\n", print_name, size);
        	
#if 0
        int total_1 = 0;
        for(int i = 0; i < s_sorted_count; i++)
        {
        	Log("a: %7i b: %7i di: %10i r: %3.1f g: %3.1f b: %3.1f\n", 
        		s_sorted_list[i].p1, 
        		s_sorted_list[i].p2, 
        		s_sorted_list[i].data_index,
        		s_trans_list[s_sorted_list[i].data_index][0],
        		s_trans_list[s_sorted_list[i].data_index][1],
        		s_trans_list[s_sorted_list[i].data_index][2]
        	);
        	total_1++;
        }
        
        vec3_t rgb;
        int total_2 = 0;
        for(unsigned int next_index = 0, a = 0; a < g_num_patches; a++)
        {
        	for(unsigned int b = 0; b < g_num_patches; b++)
        	{
        		GetTransparency(a, b, rgb, next_index);
        		
        		if(!VectorCompare(rgb,vec3_one))
        		{
        			Log("a: %7i b: %7i ni: %10i r: %3.1f g: %3.1f b: %3.1f\n", 
        				a, 
        				b, 
        				next_index,
        				rgb[0],
        				rgb[1],
        				rgb[2]
        			);
        			total_2++;
        		}
        	}
        }
        
        Log("total1: %i\ntotal2: %i\n",total_1,total_2);
#endif
}

//===============================================
// FreeTransparencyArrays
//===============================================
void	FreeTransparencyArrays( )
{
	if (s_sorted_list) free(s_sorted_list);
	if (s_trans_list)  free(s_trans_list);
	
	s_trans_list = NULL;
	s_sorted_list = NULL;
	
	s_max_trans_count = s_trans_count = s_sorted_count = 0;
}

//===============================================
// GetTransparency -- find transparency from list. remembers last location
//===============================================
void GetTransparency(const unsigned p1, const unsigned p2, vec3_t &trans, unsigned int &next_index)
{
	VectorFill( trans, 1.0 );
	
	for( unsigned i = next_index; i < s_sorted_count; i++ )
	{
		if ( s_sorted_list[i].p1 < p1 )
		{
			continue;
		}
		else if ( s_sorted_list[i].p1 == p1 )
		{
			if ( s_sorted_list[i].p2 < p2 )
			{
				continue;
			}
			else if ( s_sorted_list[i].p2 == p2 )
			{
				VectorCopy( s_trans_list[s_sorted_list[i].data_index], trans );
				next_index = i + 1;
			
				return;
			}
			else //if ( s_sorted_list[i].p2 > p2 )
			{
				next_index = i;
			
				return;
			}
		}
		else //if ( s_sorted_list[i].p1 > p1 )
		{
			next_index = i;
			
			return;
		}
	}
	
	next_index = s_sorted_count;
}


#endif /*HLRAD_HULLU*/




