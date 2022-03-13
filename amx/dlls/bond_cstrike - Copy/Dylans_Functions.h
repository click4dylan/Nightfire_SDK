#ifndef DYLANS_FUNCTIONS
#define DYLANS_FUNCTIONS
#include "engine.h"
#include "cbase.h"

// everything that blocks player movement
#define	MASK_PLAYERSOLID			(CONTENTS_SOLID)//|CONTENTS_MOVEABLE|CONTENTS_PLAYERCLIP|CONTENTS_WINDOW|CONTENTS_MONSTER|CONTENTS_GRATE)

// lifted directly from sh_list.h in amxmodx_mm

// MSVC8 fix for offsetof macro redefition warnings
#ifdef _MSC_VER 
	#if _MSC_VER >= 1400
		#undef offsetof
	#endif
#endif

#include <new>
#include <stdlib.h>

//namespace SourceHook
//{
	//This class is from CSDM for AMX Mod X
	/*
		A circular, doubly-linked list with one sentinel node

		Empty:
			m_Head = sentinel
			m_Head->next = m_Head;
			m_Head->prev = m_Head;
		One element:
			m_Head = sentinel
			m_Head->next = node1
			m_Head->prev = node1
			node1->next = m_Head
			node1->prev = m_Head
		Two elements:
			m_Head = sentinel
			m_Head->next = node1
			m_Head->prev = node2
			node1->next = node2
			node1->prev = m_Head
			node2->next = m_Head
			node2->prev = node1
	*/
	template <class T>
	class List
	{
	public:
		class iterator;
		friend class iterator;
		class ListNode
		{
		public:
			ListNode(const T & o) : obj(o) { };
			ListNode() { };
			T obj;
			ListNode *next;
			ListNode *prev;
		};
	private:
		// Initializes the sentinel node.
		// BAIL used malloc instead of new in order to bypass the need for a constructor.
		ListNode *_Initialize()
		{
			ListNode *n = (ListNode *)malloc(sizeof(ListNode));
			n->next = n;
			n->prev = n;
			return n;
		}
	public:
		List() : m_Head(_Initialize()), m_Size(0)
		{
		}
		List(const List &src) : m_Head(_Initialize()), m_Size(0)
		{
			iterator iter;
			for (iter=src.begin(); iter!=src.end(); iter++)
				push_back( (*iter) );
		}
		~List()
		{
			clear();

			// Don't forget to free the sentinel
			if (m_Head)
			{
				free(m_Head);
				m_Head = NULL;
			}
		}
		void push_back(const T &obj)
		{
			ListNode *node = new ListNode(obj);

			node->prev = m_Head->prev;
			node->next = m_Head;
			m_Head->prev->next = node;
			m_Head->prev = node;

			m_Size++;
		}
		size_t size()
		{
			return m_Size;
		}

		void clear()
		{
			ListNode *node = m_Head->next;
			ListNode *temp;
			m_Head->next = m_Head;
			m_Head->prev = m_Head;

			// Iterate through the nodes until we find g_Head (the sentinel) again
			while (node != m_Head)
			{
				temp = node->next;
				delete node;
				node = temp;
			}
			m_Size = 0;
		}
		bool empty()
		{
			return (m_Size == 0);
		}
		T & back()
		{
			return m_Head->prev->obj;
		}
	private:
		ListNode *m_Head;
		size_t m_Size;
	public:
		class iterator
		{
		friend class List;
		public:
			iterator()
			{
				m_This = NULL;
			}
			iterator(const List &src)
			{
				m_This = src.m_Head;
			}
			iterator(ListNode *n) : m_This(n)
			{
			}
			iterator(const iterator &where)
			{
				m_This = where.m_This;
			}
			//pre decrement
			iterator & operator--()
			{
				if (m_This)
					m_This = m_This->prev;
				return *this;
			}
			//post decrement
			iterator operator--(int)
			{
				iterator old(*this);
				if (m_This)
					m_This = m_This->prev;
				return old;
			}	
			
			//pre increment
			iterator & operator++()
			{
				if (m_This)
					m_This = m_This->next;
				return *this;
			}
			//post increment
			iterator operator++(int)
			{
				iterator old(*this);
				if (m_This)
					m_This = m_This->next;
				return old;
			}
			
			const T & operator * () const
			{
				return m_This->obj;
			}
			T & operator * ()
			{
				return m_This->obj;
			}
			
			T * operator -> ()
			{
				return &(m_This->obj);
			}
			const T * operator -> () const
			{
				return &(m_This->obj);
			}
			
			bool operator != (const iterator &where) const
			{
				return (m_This != where.m_This);
			}
			bool operator ==(const iterator &where) const
			{
				return (m_This == where.m_This);
			}
		private:
			ListNode *m_This;
		};
	public:
		iterator begin() const
		{
			return iterator(m_Head->next);
		}
		iterator end() const
		{
			return iterator(m_Head);
		}
		iterator erase(iterator &where)
		{
			ListNode *pNode = where.m_This;
			iterator iter(where);
			iter++;


			// Works for all cases: empty list, erasing first element, erasing tail, erasing in the middle...
			pNode->prev->next = pNode->next;
			pNode->next->prev = pNode->prev;

			delete pNode;
			m_Size--;

			return iter;
		}

		iterator insert(iterator where, const T &obj)
		{
			// Insert obj right before where

			ListNode *node = new ListNode(obj);
			ListNode *pWhereNode = where.m_This;
			
			pWhereNode->prev->next = node;
			node->prev = pWhereNode->prev;
			pWhereNode->prev = node;
			node->next = pWhereNode;

			m_Size++;

			return iterator(node);
		}

	public:
		void remove(const T & obj)
		{
			iterator b;
			for (b=begin(); b!=end(); b++)
			{
				if ( (*b) == obj )
				{
					erase( b );
					break;
				}
			}
		}
		template <typename U>
		iterator find(const U & equ)
		{
			iterator iter;
			for (iter=begin(); iter!=end(); iter++)
			{
				if ( (*iter) == equ )
					return iter;
			}
			return end();
		}
		List & operator =(const List &src)
		{
			clear();
			iterator iter;
			for (iter=src.begin(); iter!=src.end(); iter++)
				push_back( (*iter) );
			return *this;
		}
	};
//};	//NAMESPACE

// *****************************************************
// class CPlayer lifted from CMisc.h in amxmodx_mm
// *****************************************************

struct ClientCvarQuery_Info
{
	int resultFwd;
	int requestId;
	int paramLen;
	cell *params;
};

class PackedEntity;

class PackedEntities
{
public:
	PackedEntities() : entities(0) {}

	int				num_entities;
	int				max_entities;
	
	// FIXME: Can we use the packed ent dict on the client too?
	// Client only fields:
	PackedEntity	*entities;
};

// -------------------------------------------------------------------------------------------------- //
// client_frame_t
// -------------------------------------------------------------------------------------------------- //
#define MAX_EDICTS 180224 //actual nightfire value

#define PAD_NUMBER(number, boundary) \
	( ((number) + ((boundary)-1)) / (boundary) ) * (boundary)

class CFrameSnapshot;

/*class client_frame_t
{
public:

						client_frame_t();
						~client_frame_t();

	// Accessors to snapshots. The data is protected because the snapshots are reference-counted.
	CFrameSnapshot*		GetSnapshot() const;
	void				SetSnapshot( CFrameSnapshot *pSnapshot );

	int					GetNumEntities() const;


public:

	// Time world sample was sent to client.
	double				senttime;       

	// Realtime when client ack'd the update.
	float				raw_ping;

	Vector playerorigin; //Nightfire puts these here for some reason

	float unknown_animationx;

	float unknown_animationy;

	float unknown_animationz;

	int unknown_weaponanimation;

	int unknown1;
	int unknown2;
	int unknown3;
	int unknown4;
	int unknown5;
	float unknownNAN;
	int unknown6;
	int unknown7;
	float headheight;


	// Ping value adjusted for client/server framerate issues
	//float				latency;     
	 // Internal lag (i.e., local client should = 0 ping)
	//float               frame_time;    

	// State of entities this frame from the POV of the client.
	//PackedEntities		entities;       

	// Used by server to indicate if the entity was in the player's pvs
	//unsigned char 		entity_in_pvs[ PAD_NUMBER( MAX_EDICTS, 8 ) ];

//private:

	// Index of snapshot entry that stores the entities that were active and the serial numbers
	//  for the frame number this packed entity corresponds to
	//CFrameSnapshot		*m_pSnapshot;
	char unk[2948]; //full size = 0x0bd0
};*/
#define MAX_PHYSINFO_STRING 256

typedef struct clientdata_s
{
	vec3_t				origin;
	vec3_t				velocity;

	int					viewmodel;
	vec3_t				punchangle;
	int					flags;
	int					waterlevel;
	int					watertype;
	vec3_t				view_ofs;
	float				health;

	int					bInDuck;
	int					bInLeanLeft; //dylan added
	int					bInLeanRight; //dylan added
	vec3_t				preleanangles; //dylan added

	int					weapons; // remove?
	
	int					flTimeStepSound;
	int					flDuckTime;
	int					flLeanLeftTime; //dylan added
	int					flLeanRightTime; //dylan added
	int					flSwimTime;
	int					waterjumptime;
	unsigned char		jetpackfuel; //dylan added

	float				fov;
	float				maxspeed;

	//float				fov;
	int					weaponanim;

	int					m_iId;
	int					ammo_shells;
	int					ammo_nails;
	int					ammo_cells;
	int					ammo_rockets;
	float				m_flNextAttack;
	
	int					tfstate; // dylan, remove? check m_fReloadTime in weapon_data_t

	int					pushmsec;

	int					deadflag;

	char				physinfo[ MAX_PHYSINFO_STRING ];

	// For mods
	//dylan: unknown which 2 bytes were removed
	short				iuser1;
	int					iuser2;
	int					iuser3;
	int					iuser4;
	float				fuser1;
	float				fuser2;
	float				fuser3;
	float				fuser4;
	vec3_t				vuser1;
	vec3_t				vuser2;
	vec3_t				vuser3;
	vec3_t				vuser4;
	/*
	unsigned char		iuser1;
	unsigned char		iuser2;
	float				fuser1;
	float				fuser4;
	vec3_t				vuser1; //1 and 4 may be reversed, they are in delta.lst
	vec3_t				vuser4;
	*/
} clientdata_t; //508 bytes

// Info about weapons player might have in his/her possession
typedef struct weapon_data_s
{
	int			m_iId;
	int			m_iClip;

	float		m_flNextPrimaryAttack;
	float		m_flNextSecondaryAttack;
	float		m_flTimeWeaponIdle;

	int			m_fInReload;
	int			m_fInSpecialReload;
	float		m_flNextReload;
	float		m_flPumpTime;
	//float		m_fReloadTime; //dylan, should this be here? test!!!!!!!!!! if its this, then remove tfstate from clientdata_t

	//float		m_fAimedDamage;
	//float		m_fNextAimBonus;
	int			m_iViewMode;//m_fInZoom;
	int			m_nWeaponMode; //m_iWeaponState;

	int			iuser1;
	int			iuser2;
	int			iuser3;
	int			iuser4;
	float		fuser1;
	float		fuser2;
	float		fuser3;
	float		fuser4;
	
} weapon_data_t;

// For entityType below
#define ENTITY_NORMAL		(1<<0)
#define ENTITY_BEAM			(1<<1)

// Entity state is used for the baseline and for delta compression of a packet of 
//  entities that is sent to a client.
typedef struct entity_state_s entity_state_t;

struct entity_state_s
{
// Fields which are filled in by routines outside of delta compression
	int			entityType;
	// Index into cl_entities array for this entity.
	int			number;
	float		msg_time;

	// Message number last time the player/entity state was updated.
	int			messagenum;

	// Fields which can be transitted and reconstructed over the network stream
	vec3_t		origin;
	vec3_t		angles;

	int			modelindex;
	int			sequence;
	float		frame;
	int			colormap;
	short		skin;
	short		solid;
	int			effects;
	float		scale;

	byte		eflags;
	
	// Render information
	int			rendermode;
	int			renderamt;
	color24		rendercolor;
	int			renderfx;

	int			movetype;
	float		animtime;
	float		framerate;
	int			body;
	byte		controller[4];
	byte		blending[4];
	vec3_t		velocity;

	// Send bbox down to client for use during prediction.
	vec3_t		mins;
	vec3_t		maxs;

	int			aiment;
	// If owned by a player, the index of that player ( for projectiles ).
	int			owner; 

	// Friction, for prediction.
	float		friction;
	// Gravity multiplier
	float		gravity;

// PLAYER SPECIFIC
	int			team;
	int			playerclass; // Playerclass signifies it's a decalable glass item when referring to an object
	int			health;
	qboolean	spectator;
	int			weaponmodel;
	int			gaitsequence;
	// If standing on conveyor, e.g.
	vec3_t		basevelocity;
	// Use the crouched hull, or the regular player hull.
	int			usehull;
	// Latched buttons last time state updated.
	int			oldbuttons;
	// -1 = in air, else pmove entity number
	int			onground;
	int			iStepLeft;
	// How fast we are falling
	float		flFallVelocity;

	float		fov;
	int			weaponanim;

	// Parametric movement overrides
	//vec3_t		startpos;
	vec3_t		endpos;
	float		impacttime;
	float		starttime;

	// For mods
	int			iuser1;
	int			iuser2;
	//int			iuser3;
	unsigned char			iuser4;
	float		fuser1;
	//float		fuser2;
	//float		fuser3;
	//float		fuser4;
	vec3_t		vuser1;
	vec3_t		vuser2;
	//vec3_t		vuser3;
	//vec3_t		vuser4;
	char unknown[64];
};

typedef struct packet_entities_s
{
	int num_entities;
	unsigned char flags[64]; //32 in half-life
	entity_state_t *entities; //correct size
} packet_entities_t;


typedef struct client_frame_s
{
	double senttime;
	float ping_time;
	clientdata_t clientdata; //
	weapon_data_t weapondata[32]; //hl = 64, nf = 72??
	packet_entities_t entities;
} client_frame_t;

#define MAX_CLIENTS 32

// 64 simultaneous events, max
#define MAX_EVENT_QUEUE 64
// 192 bytes of data per event max
#define EVENT_DATA_LEN_BITS	11			// ( 1<<8 bits == 256, but only using 192 below )
#define MAX_EVENT_DATA 128 //192 before dylan changed, it's just a random number..

#include "event_flags.h"

class CEventInfo
{
public:
	// 0 implies not in use
	short index;			  
	// If non-zero, the time when the event should be fired ( fixed up on the client )
	float fire_time;       
	
	// Length of data bits
	int	  bits;
	// Raw event data
	byte data[ MAX_EVENT_DATA ];
	// CLIENT ONLY Reliable or not, etc.
	int	  flags;			
};

//-----------------------------------------------------------------------------
// Used for serialization
//-----------------------------------------------------------------------------
class QAngle;

class bf_read;

class bf_write
{
public:
					bf_write();
					
					// nMaxBits can be used as the number of bits in the buffer. 
					// It must be <= nBytes*8. If you leave it at -1, then it's set to nBytes * 8.
					bf_write( void *pData, int nBytes, int nMaxBits = -1 );
					bf_write( const char *pDebugName, void *pData, int nBytes, int nMaxBits = -1 );

	// Start writing to the specified buffer.
	// nMaxBits can be used as the number of bits in the buffer. 
	// It must be <= nBytes*8. If you leave it at -1, then it's set to nBytes * 8.
	void			StartWriting( void *pData, int nBytes, int iStartBit = 0, int nMaxBits = -1 );

	// Restart buffer writing.
	void			Reset();

	// Get the base pointer.
	unsigned char*	GetBasePointer() const	{ return m_pData; }

	// Enable or disable assertion on overflow. 99% of the time, it's a bug that we need to catch,
	// but there may be the occasional buffer that is allowed to overflow gracefully.
	void			SetAssertOnOverflow( bool bAssert );

	// This can be set to assign a name that gets output if the buffer overflows.
	const char*		GetDebugName();
	void			SetDebugName( const char *pDebugName );


// Seek to a specific position.
public:
	
	void			SeekToBit( int bitPos );


// Bit functions.
public:

	void			WriteOneBit(int nValue);
	void			WriteOneBitNoCheck(int nValue);
	void			WriteOneBitAt( int iBit, int nValue );
	
	// Write signed or unsigned. Range is only checked in debug.
	void			WriteUBitLong( unsigned int data, int numbits, bool bCheckRange=true );
	void			WriteSBitLong( int data, int numbits );
	
	// Tell it whether or not the data is unsigned. If it's signed,
	// cast to unsigned before passing in (it will cast back inside).
	void			WriteBitLong(unsigned int data, int numbits, bool bSigned);

	// Write a list of bits in.
	bool			WriteBits(const void *pIn, int nBits);

	// Copy the bits straight out of pIn. This seeks pIn forward by nBits.
	// Returns an error if this buffer or the read buffer overflows.
	bool			WriteBitsFromBuffer( bf_read *pIn, int nBits );
	
	void			WriteBitAngle( float fAngle, int numbits );
	void			WriteBitCoord (const float f);
	void			WriteBitFloat(float val);
	void			WriteBitVec3Coord( const Vector& fa );
	void			WriteBitNormal( float f );
	void			WriteBitVec3Normal( const Vector& fa );
	void			WriteBitAngles( const QAngle& fa );


// Byte functions.
public:

	void			WriteChar(int val);
	void			WriteByte(int val);
	void			WriteShort(int val);
	void			WriteWord(int val);
	void			WriteLong(long val);
	void			WriteFloat(float val);
	bool			WriteBytes( const void *pBuf, int nBytes );

	// Returns false if it overflows the buffer.
	bool			WriteString(const char *pStr);


// Status.
public:

	// How many bytes are filled in?
	int				GetNumBytesWritten();
	int				GetNumBitsWritten();
	int				GetMaxNumBits();
	int				GetNumBitsLeft();
	int				GetNumBytesLeft();
	unsigned char*	GetData();

	// Has the buffer overflowed?
	bool			CheckForOverflow(int nBits);
	inline bool		IsOverflowed() {return m_bOverflow;}

	inline void		SetOverflowFlag();


public:
	// The current buffer.
	unsigned char*	m_pData;
	int				m_nDataBytes;
	int				m_nDataBits;
	
	// Where we are in the buffer.
	int				m_iCurBit;
	
private:

	// Errors?
	bool			m_bOverflow;

	bool			m_bAssertOnOverflow;
	const char		*m_pDebugName;
};


//netadr.h, net.h
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
	netadrtype_t	type;
	unsigned char	ip[4];
	unsigned char	ipx[10];
	unsigned short	port;
} netadr_t;

// This is the packet payload without any header bytes (which are attached for actual sending)
#define NET_MAX_PAYLOAD 4000
// 0 == regular, 1 == file stream
#define MAX_STREAMS			2    
#define	MAX_OSPATH		260			// max length of a filesystem pathname
#define MAX_FLOWS 2 //HALF-LIFE IS 2, NIGHTFIRE IS 3

// Size of fragmentation buffer internal buffers
#define FRAGMENT_SIZE 1400

#define	FRAG_NORMAL_STREAM	0
#define FRAG_FILE_STREAM	1

#define FLOW_OUTGOING 0
#define FLOW_INCOMING 1

inline int SV_UPDATE_BACKUP_F() {
	return (*(int*)0x4310C028);
}

inline int SV_UPDATE_MASK_F() {
	return (*(int*)0x4310C02C);
}

#define SV_UPDATE_BACKUP (*(int*)0x4310C028) //dylan added this
#define SV_UPDATE_MASK	 (*(int*)0x4310C02C) //dylan added this

// Message data
typedef struct
{
	int unknown;
	// Size of message sent/received
	int		size;
	// Time that message was sent/received
	double	time;
} flowstats_t;

#define	MAX_LATENT	32

typedef struct
{
	// Data for last MAX_LATENT messages
	flowstats_t stats[ MAX_LATENT ];
	//char unknownbytes[1];
	////////////unsigned char unknown[132];
	// Current message position
	int			current;
	// Time when we should recompute k/sec data
	double		nextcompute; 
	// Average data
	float		kbytespersec;
	float		avgkbytespersec;
	int			totalbytes;
} flow_t;

// Generic fragment structure
typedef struct fragbuf_s
{
	// Next buffer in chain
	struct fragbuf_s	*next;
	// Id of this buffer
	int					bufferid;
	// Message buffer where raw data is stored
	bf_write			frag_message;
	// The actual data sits here
	byte				frag_message_buf[ FRAGMENT_SIZE ];
	// Is this a file buffer?
	qboolean			isfile;
	// Is this file buffer from memory ( custom decal, etc. ).
	qboolean			isbuffer;
	// Name of the file to save out on remote host
	char				filename[ MAX_OSPATH ];
	// Offset in file from which to read data
	int					foffset;  
	// Size of data to read at that offset
	int					size;
} fragbuf_t;

// Waiting list of fragbuf chains
typedef struct fragbufwaiting_s
{
	// Next chain in waiting list
	struct fragbufwaiting_s		*next;
	// Number of buffers in this chain
	int							fragbufcount;
	// The actual buffers
	fragbuf_t					*fragbufs;
} fragbufwaiting_t;

typedef struct sizebuf_s
{
	const char *buffername;
	uint16 flags;
	byte *data;
	int maxsize;
	int cursize;
} sizebuf_t;

// Max length of a reliable message
#define	MAX_MSGLEN	3990 // 10 reserved for fragheader?

class netchan_t //unfinished nightfire netchan_t
{
public:
	// NS_SERVER or NS_CLIENT, depending on channel.
	netsrc_t sock;

	// Address this channel is talking to.
	netadr_t remote_address;

	int player_slot;
	// For timeouts.  Time last message was received.
	float last_received;
	// Time when channel was connected.
	float connect_time;

	int unknownint;

	// Bandwidth choke
	// Bytes per second
	double rate;
	// If realtime > cleartime, free to send next packet
	double cleartime;

	// Sequencing variables
	//
	// Increasing count of sequence numbers 
	int incoming_sequence;
	// # of last outgoing message that has been ack'd.
	int incoming_acknowledged;
	// Toggles T/F as reliable messages are received.
	int incoming_reliable_acknowledged;
	// single bit, maintained local
	int incoming_reliable_sequence;
	// Message we are sending to remote
	int outgoing_sequence;
	// Whether the message contains reliable payload, single bit
	int reliable_sequence;
	// Outgoing sequence number of last send that had reliable data
	int last_reliable_sequence;

	void *connection_status;
	int (*pfnNetchan_Blocksize)(void *);

	// Staging and holding areas
	sizebuf_t message;
	byte message_buf[MAX_MSGLEN];

	// Reliable message buffer. We keep adding to it until reliable is acknowledged. Then we clear it.
	int reliable_length; //correct +0x1008
	byte reliable_buf[MAX_MSGLEN]; //correct +0x100C

	// Waiting list of buffered fragments to go onto queue. Multiple outgoing buffers can be queued in succession.
	fragbufwaiting_t *waitlist[MAX_STREAMS];

	// Is reliable waiting buf a fragment?
	int reliable_fragment[MAX_STREAMS]; //correct +0x1FAC
	// Buffer id for each waiting fragment
	unsigned int reliable_fragid[MAX_STREAMS];

	// The current fragment being set
	fragbuf_t *fragbufs[MAX_STREAMS];
	// The total number of fragments in this stream
	int fragbufcount[MAX_STREAMS];

	// Position in outgoing buffer where frag data starts
	short int frag_startpos[MAX_STREAMS];
	// Length of frag data in the buffer
	short int frag_length[MAX_STREAMS];

	// Incoming fragments are stored here
	fragbuf_t *incomingbufs[MAX_STREAMS];
	// Set to true when incoming data is ready
	bool/*qboolean*/ incomingready[MAX_STREAMS];

	// Only referenced by the FRAG_FILE_STREAM component
	// Name of file being downloaded
	char incomingfilename[MAX_PATH]; //deleted?

	void *tempbuffer;
	//int tempbuffersize;

	// Incoming and outgoing flow metrics
	flow_t flow[MAX_FLOWS];
	//char unknown[260];
#if 0
	/*
		// Connected to remote server?  1 == yes, 0 otherwise
	//int			connected; 
	// Client's IP address
	//netadr_t	local_address;
	// Address of remote server
	//netadr_t	remote_address;
	// Packet Loss ( as a percentage )
	int			packet_loss;
	// Latency, in seconds ( multiply by 1000.0 to get milliseconds )
	double		latency;
	// Connection time, in seconds
	//double		connection_time;
	short asdf4;

	// Rate setting ( for incoming data )
	double		rate;

	// If realtime > cleartime, free to send next packet
	double		cleartime;
	
	// Sequencing variables
	//
	// Increasing count of sequence numbers 
	int			incoming_sequence;   
	// # of last outgoing message that has been ack'd.
	int			incoming_acknowledged;
	
	unsigned char unfinished[9434]; // 9444 //9500 //9501 //size of netchan, screw getting netchan's table atm
	*/
	//9472 full size


	// NS_SERVER or NS_CLIENT, depending on channel.
	netsrc_t    sock;               

	// Address this channel is talking to.
	//netadr_t	remote_address;  
	float		unknown_netchan1;
	
	// For timeouts.  Time last message was received.
	float		last_received;		
	// Time when channel was connected.
	float       connect_time;       

	// Bandwidth choke
	// Bytes per second
	double		rate;				
	// If realtime > cleartime, free to send next packet
	double		cleartime;			

	// Sequencing variables
	//
	// Increasing count of sequence numbers 
	int			incoming_sequence;              
	// # of last outgoing message that has been ack'd.
	int			incoming_acknowledged;          
	// Toggles T/F as reliable messages are received.
	int			incoming_reliable_acknowledged;	
	// single bit, maintained local
	int			incoming_reliable_sequence;	    
	// Message we are sending to remote
	int			outgoing_sequence;              
	// Whether the message contains reliable payload, single bit
	int			reliable_sequence;			    
	// Outgoing sequence number of last send that had reliable data
	int			last_reliable_sequence;		 

	//information below is not guaranteed

	// Staging and holding areas
	bf_write	message;
	byte		message_buf[NET_MAX_PAYLOAD];

	// Reliable message buffer.  We keep adding to it until reliable is acknowledged.  Then we clear it.
	int			reliable_length;
	byte		reliable_buf[NET_MAX_PAYLOAD];	// unacked reliable message

	// Waiting list of buffered fragments to go onto queue.
	// Multiple outgoing buffers can be queued in succession
	fragbufwaiting_t *waitlist[ MAX_STREAMS ]; 

	// Is reliable waiting buf a fragment?
	int				reliable_fragment[ MAX_STREAMS ];          
	// Buffer id for each waiting fragment
	unsigned int	reliable_fragid[ MAX_STREAMS ];

	// The current fragment being set
	fragbuf_t	*fragbufs[ MAX_STREAMS ];
	// The total number of fragments in this stream
	int			fragbufcount[ MAX_STREAMS ];

	// Position in outgoing buffer where frag data starts
	short		frag_startpos[ MAX_STREAMS ];
	// Length of frag data in the buffer
	short		frag_length[ MAX_STREAMS ];

	// Incoming fragments are stored here
	fragbuf_t	*incomingbufs[ MAX_STREAMS ];
	// Set to true when incoming data is ready
	qboolean	incomingready[ MAX_STREAMS ];

	// Only referenced by the FRAG_FILE_STREAM component
	// Name of file being downloaded
	char		incomingfilename[ MAX_OSPATH ];

	// Incoming and outgoing flow metrics
	flow_t flow[ MAX_FLOWS ];  

	char unknown[244];
#endif
};

typedef struct event_info_s event_info_t;

// Event was invoked with stated origin
#define FEVENT_ORIGIN	( 1<<0 )

// Event was invoked with stated angles
#define FEVENT_ANGLES	( 1<<1 )

typedef struct event_args_s
{
	int		flags;

	// Transmitted
	int		entindex;

	float	origin[3];
	float	angles[3];
	float	velocity[3];

	int		ducking;

	float	fparam1;
	float	fparam2;

	int		iparam1;
	int		iparam2;

	int		bparam1;
	int		bparam2;
} event_args_t;

struct event_info_s
{
	unsigned short index;			  // 0 implies not in use

	short packet_index;      // Use data from state info for entity in delta_packet .  -1 implies separate info based on event
	                         // parameter signature
	short entity_index;      // The edict this event is associated with

	float fire_time;        // if non-zero, the time when the event should be fired ( fixed up on the client )
	
	event_args_t args;

// CLIENT ONLY	
	int	  flags;			// Reliable or not, etc.

};

typedef struct event_state_s event_state_t;

struct event_state_s
{
	struct event_info_s ei[ MAX_EVENT_QUEUE ];
};

/////////////////////////////////////////
#define MAX_INFO_STRING 256
#define MAX_DATAGRAM 4000

typedef FILE * FileHandle_t;
typedef int FileFindHandle_t;
typedef int WaitForResourcesHandle_t;
////////////////////////////////////////////////////////

typedef class CPlayer 
{
public:
	// ------------------------------------------------------------ //
	// Voice stuff.
	// ------------------------------------------------------------ //
	//bool			m_bLoopback;		// Does this client want to hear his own voice?
	//unsigned long	m_VoiceStreams[32/*MAX_CLIENTS*/ / 32 + 1];	// Which other clients does this guy's voice stream go to?


	bool/*qboolean*/		active;				// false = client is free
	bool/*qboolean*/		spawned;			// false = don't send datagrams
	//unsigned char state; //can be used to freeze packet flow without timing out player
	bool fully_connected;
	bool /*qboolean*/        connected;          // On server, getting data.

	//bool uploading;
	bool hasusrmsgs;
	bool has_force_unmodified;
	char unknownct;
	//int  unknown;

//===== NETWORK ============
	//int unknownshit;
	//int unknownshit2;
	//netadr_t	remote_address;
	netchan_t		netchan;            // The client's net connection. Unfinished

	int				chokecount;         // Number of packets choked at the server because the client - server //0x2520
										//  network channel is backlogged with too much data.
	int				delta_sequence;		// -1 = no compression.  This is where the server is creating the
										// compressed info from.

	//int				acknowledged_tickcount; // tracks host_tickcount based on what client has ack'd receiving
	//int				tabledef_acknowledged_tickcount; // tracks host_tickcount based on what client has ack'd receiving
	//int				GetMaxAckTickCount() const;

	// This is used when we send out a nodelta packet to put the client in a state where we wait 
	// until we get an ack from them on this packet.
	// This is for 3 reasons:
	// 1. A client requesting a nodelta packet means they're screwed so no point in deluging them with data.
	//    Better to send the uncompressed data at a slow rate until we hear back from them (if at all).
	// 2. Since the nodelta packet deletes all client entities, we can't ever delta from a packet previous to it.
	// 3. It can eat up a lot of CPU on the server to keep building nodelta packets while waiting for
	//    a client to get back on its feet.
	//int							m_ForceWaitForAck;
	//bool						m_bResendNoDelta;

	// This is set each frame if we're using the single player optimized path so if they disable that path,
	// it knows to send an uncompressed packet.
	//bool m_bUsedLocalNetworkBackdoor;
	
	bool/*qboolean*/		fakeclient;			// JAC: This client is a fake player controlled by the game DLL

	//test this below, tested, works
	bool proxy;
	usercmd_t lastcmd; //0x252C


	int unknownInt; //asdfsjlfsjlfsjdlsjaldjlksdfjlkljskal

	//unsigned char unknownbytes[62]; //118
	

	double connecttime; //0x2568
	double cmdtime; //0x2570
	double ignorecmdtime; //0x2578
	float latency; //0x2580
	float packet_loss; //0x2584
	double localtime; //unused originally, but used in nightfire patch to store player's interp
	double nextping;
	double svtimebase; //0x2598
	//added from hl1 ^

	// The datagram is written to after every frame, but only cleared
	//  when it is sent out to the client.  overflow is tolerated.
	//bf_write		datagram; //the values inside bf_write could be wrong
	//byte			datagram_buf[4000]; //NET_MAX_PAYLOAD, this is correct for nightfire
	sizebuf_t datagram;
	//double datagram_unknown;
	byte datagram_buf[MAX_DATAGRAM];
	//unsigned char unknown_mightbe_datagram[4143];

	float unknownFloatOrInt;

	double			connection_started;	// Or time of disconnect for zombies 0x3558
	
	// Time when we should send next world state update ( datagram )
	double          next_messagetime;   
	// Default time to wait for next message
	double          next_messageinterval;  

	bool /*qboolean*/		send_message;		// Set on frames a datagram arrived on
	bool /*qboolean*/		skip_message;		// Defer message sending for one frame

	short unknown_short; //this used to be used for our interp value, but is now stored in 'localtime' instead

	client_frame_t	*frames; // updates can be deltad from here
	//int				numframes; //no idea if this is correct
	
	event_state_t events;
	//char unknownCEventState[5632]; //5628 with numframes


	// Identity information.
	edict_t			*edict;				// EDICT_NUM(clientnum+1)

	const edict_t	*pViewEntity;		// View Entity (camera or the client itself)

	int				userid;				// identifying number on server
	int				network_userid; //USERID_t, could be clientip (unsigned int), wonid, or idtype
	char			userinfo[256 /*MAX_INFO_STRING*/];		// infostring
	//int unknown3;
	bool /*qboolean*/		sendinfo;
	//unsigned char unknown4[7];
	// Client sends this during connection, so we can see if
	//  we need to send sendtable info or if the .dll matches
	//CRC32_t			sendtable_crc; //this is most likely correct

	//unsigned char unknown4[3];
	
	//char            hashedcdkey[/*SIGNED_GUID_LEN*/32 + 1]; // MD5 hash is 32 hex #'s, plus trailing 0

	//unsigned char unknown5[28];
	float sendinfo_time;
	char hashedcdkey[64];

	char			name[32];			// for printing to other people
	//int topcolor; //removed in nightfire
	//int bottomcolor; //removed in nightfire
	int entityId;

	resource_t resourcesonhand;
	resource_t resourcesneeded;
	FileHandle_t upload;
	//bool uploaddoneregistering;
	//customization_t customdata;
	int crcValue;
	int lw;
	int lc;
	char physinfo[MAX_INFO_STRING];
	//bool m_bLoopback;
	//uint32 m_VoiceStreams[2];
	//double m_lastvoicetime;
	int m_sendrescount; //removed?
	//unsigned char gearboxwastedspace[540];
		
	//int				entityId;			// ID # in save structure

	// Spray point logo
	//CRC32_t			logo;
	//bool			request_logo;		// True if logo request should be made at SV_New_f



//Invalid original table from cmisc.h
//public:
//	edict_t* pEdict;
//	
//	String name;
//	String ip;
//	String team;
//
//	bool initialized;
//	bool ingame;
//	bool authorized;
//	bool vgui;	
//
//	float time;
//	float playtime;
//	float menuexpire;
//	
//	struct
//	{
//		int ammo;
//		int clip;
//	} weapons[MAX_WEAPONS];
//	
//	int current;
//	int teamId;
//	int deaths;
//	int aiming;
//	int menu;
//	int keys;
//	int index;
//	int flags[32];
//
//	int death_headshot;
//	int death_killer;
//	int death_victim;
//	bool death_tk;
//	String death_weapon;
//	int newmenu;
//	int page;
//
//	float channels[5];
//	cell hudmap[5];
//	
//	Vector lastTrace;
//	Vector lastHit;
//	
//	List<ClientCvarQuery_Info *> queries;
//
//	void Init(edict_t* e, int i);
//	void Disconnect();
//	void PutInServer();
//	
//	bool Connect(const char* connectname, const char* ipaddress);
//
//	inline bool IsBot()
//	{
//		if ((pEdict->v.flags & FL_FAKECLIENT) == FL_FAKECLIENT)
//		{
//			return true;
//		}
//		/*
//		const char *auth = GETPLAYERAUTHID(pEdict); 	 //DYLAN TOFIX
//		if (auth && (strcmp(auth, "BOT") == 0)) 	 
//		{
//			return true;
//		}
//		*/
//		return false;
//	}
//
//	inline bool IsAlive()
//	{
//		return ((pEdict->v.deadflag == DEAD_NO) && (pEdict->v.health > 0));
//	}
//
//	inline void Authorize() { authorized = true; }
//
//	int NextHUDChannel();
//
} client_t;

















//Start of dylan's functions

enum TEAMCHOICE {
	TEAM_RANDOM,
	TEAM_MI6,
	TEAM_PHOENIX
};

#define PLAYER_TABLE_CELLSIZE sizeof(client_t)//20248 is the actual value for nightfire
#define svs_clients (client_t*)(*(UINT32*)0x44A86B4C)
#define svs_maxclients (*(UINT32*)0x44A86B50)
//__forceinline UINT32 GetClientTable() { return *(UINT32*)0x44A86B4C; }
#define GetClientTable() *(UINT32*)0x44A86B4C

inline client_t *GetClientPointerFromIndex( int index ) {
	//get first client ptr, add it to the size of each player struct multiplied by the index
	return (client_t*) ( GetClientTable() + (sizeof(client_t)/*PLAYER_TABLE_CELLSIZE*/ * index) );
}
#define GetClientPointerByIndex GetClientPointerFromIndex
#define GET_CLIENT_POINTER GetClientPointerFromIndex
#define INDEX_TO_CLIENT GetClientPointerFromIndex

inline edict_t *GetEdictNumFromIndex( int index ) {
	CPlayer *pPlayer = GetClientPointerFromIndex(index);
	if ( pPlayer )
		 return pPlayer->edict;
	return NULL;
}

//Dylan's Useful Functions
#include <vector> //Required by SplitChar3

//Noel's String Explode

class StringList{
	std::vector<const char*> strings;
public:
	StringList(int Length, int initialmemreserve);
	~StringList();

	inline unsigned StringList::GetSize(){
		return strings.size();
	}

	inline void StringList::Add(const char* str){ strings.push_back(str); }

	inline void StringList::AddAt(unsigned pos, const char * str){ strings.insert(strings.begin() + pos, str); }

	/*
	inline char *& StringList::operator[](int pos){
	return strings.at(pos);
	}
	*/
	inline void StringList::Remove(){ strings.pop_back(); return; }

	inline void StringList::RemoveAt(unsigned pos){ strings.erase(strings.begin() + pos); }

	inline const char * StringList::Get(unsigned pos) { return strings.at(pos); }

	inline void StringList::Set(unsigned pos, const char * replacement){ strings.at(pos) = replacement; }
	/*inline unsigned int GetSize();
	inline void Add(const char * str); // Add A String To The End Of The String List
	//char *& operator[](int pos); // testing on this later
	inline void AddAt(unsigned pos, const char * str); // Add A String And Store It At A Current Position In The String List
	inline void Remove(); // Remove A String From The End Of The String List And Returns The String
	inline void RemoveAt(unsigned pos); // Remove A String At A Current Position In The String List
	inline const char * Get(unsigned pos); // Get A String At A Current Position In The String List
	inline void Set(unsigned pos, const char * replacement); // Set A String At A Current Position In The String List With A Replacement Value*/
};

StringList * SplitChar(const char * text, char delimiters[]);


#define ADR_WEAPONSTRIP 0x420A6B40
inline void StripWeapons(edict_t *pEntity, int dukes) {
	client_t *cptr = GetClientPointerFromIndex(ENTINDEX(pEntity) - 1);
	if ( !cptr->spawned )
		return;

	int xadr = *(UINT32*)((int)pEntity + 0x11C);
	if (xadr) {
		DWORD adr = ADR_WEAPONSTRIP;
		__asm {
			push dukes //Wanna keep the dukes or not? 1 removes them, 0 keeps them
			push dukes //Don't know what this one does
			mov ecx, xadr
			call adr
		}
	}
}

inline float SV_CalcPing( client_t *cl, int numsamples /*= 1*/ )
{
	float		ping;
	int			i;
	int			count;
	int back;
	register	client_frame_t *frame;
	int idx;

	// bots don't have a real ping
	if ( cl->fakeclient )
		return 5;

	idx = cl - svs_clients;

	ping  = 0;
	count = 0;

	if ( numsamples <= 0 )
	{
		back = min( SV_UPDATE_BACKUP / 2, 16 );
	}
	else
	{
		back = min( SV_UPDATE_BACKUP, numsamples );
	}

	for ( i=0 ; i< SV_UPDATE_BACKUP ; i++)
	{
		frame = &cl->frames[ ( cl->netchan.incoming_acknowledged - i - 1) & SV_UPDATE_MASK ];
		if (frame->ping_time > 0)
		{
			ping += frame->ping_time;
			count++;
		}

		if ( count >= back )
			break;
	}

	if ( !count )
		return 0.0f;
	ping /= count;

	if ( ping < 0 )
	{
		ping = 0.0f;
	}

	return ping * 1000.0f;
}

inline float GetPlayerPing( int playerIndex, int numsamples ) {
		if ( playerIndex < 1 || playerIndex > svs_maxclients ) {
			return 0.0f;
		}

		client_t *cl = svs_clients + playerIndex - 1;
		if ( !cl->active  ) {
			return 0.0f;
		}
				
		float ping = SV_CalcPing( cl, numsamples );
		return ping;
}

inline float GetPlayerKills( edict_t *pEntity ) {
	client_t *cptr = GetClientPointerFromIndex(ENTINDEX(pEntity) - 1);
	if ( !cptr->spawned )
		return 0;

	return pEntity->v.frags;
}

inline void SetPlayerKills( edict_t *pEntity, float Kills ) {
	client_t *cptr = GetClientPointerFromIndex(ENTINDEX(pEntity) - 1);
	if ( !cptr->spawned )
		return;
	pEntity->v.frags = Kills;
}

inline int GetPlayerDeaths( edict_t *pEntity ) {
	client_t *cptr = GetClientPointerFromIndex(ENTINDEX(pEntity) - 1);
	if ( !cptr->spawned )
		return 0;

	//int adr = *(UINT32*)((int)pEntity + 0x11C);
	//return *(UINT32*)(adr + 0xB30);
	return C_BasePlayer::Instance(pEntity)->deaths;
}

inline void SetPlayerDeaths( edict_t *pEntity, int Deaths ) {
	client_t *cptr = GetClientPointerFromIndex(ENTINDEX(pEntity) - 1);
	if ( !cptr->spawned )
		return;

	//int adr = *(UINT32*)((int)pEntity + 0x11C);
	//*(UINT32*)(adr + 0xB30) = Deaths;
	C_BasePlayer::Instance(pEntity)->deaths = Deaths;
}

inline void SayTextAll ( const char *str ) {
	MESSAGE_BEGIN(MSG_ALL, GET_USER_MSG_ID(PLID, "SayText", NULL)); //0x73 for GUI messages, is 0x75 used for centered screen messages?
		WRITE_BYTE( 0 );
		WRITE_STRING( str );
		WRITE_STRING ( 0 ); //send 0 to not show (TEAM)
	MESSAGE_END();
}

//-----------------------------------------------------------------------------
// Converts an entity to a player
//-----------------------------------------------------------------------------
//inline CBasePlayer *ToBasePlayer( CBaseEntity *pEntity ) //CBasePlayer //CBaseEntity
//{
//	if ( !pEntity  )/* || !pEntity->IsPlayer()*/
//		return NULL;
//#if _DEBUG
//	return dynamic_cast<CBasePlayer  *>( pEntity ); //CBasePlayer
//#else
//	return static_cast<CBasePlayer  *>( pEntity ); //CBasePlayer
//#endif
//}
/* //already in cbase.h
static CBaseEntity *Instance( edict_t *pent ) { 
	if ( !pent )
		pent = ENT(0);
	CBaseEntity *pEnt = (CBaseEntity *)GET_PRIVATE(pent); 
	return pEnt; 
}*/

__forceinline void FireBullets( entvars_t2 *vars, ULONG	cShots, Vector  vecSrc, Vector	vecDirShooting,	Vector	vecSpread, float flDistance, int iBulletType, int iTracerFreq = 4, int iDamage = 0, entvars_t2 *pevAttacker = NULL  ) {
	//__asm mov ecx, vars
	//doesn't work
	//((void(__stdcall*)(ULONG, Vector, Vector, Vector, float, int, int iTracerFreq, int iDamage, entvars_t2 *))0x42042DE0)(cShots, vecSrc, vecDirShooting, vecSpread, flDistance, iBulletType, iTracerFreq, iDamage, pevAttacker); //Drop all powerups
}

__forceinline Vector GetGunPosition (C_BasePlayer *pPlayer) {
	Vector origin;
	origin = pPlayer->pev->origin + pPlayer->pev->view_ofs;
	return origin;
}

__forceinline __declspec (naked) void DropFlag ( C_BasePlayer *pPlayer ) {
	__asm {
		mov dword ptr ds:[esp - 4], edi
		mov dword ptr ds:[esp - 0x8], edx
		mov dword ptr ds:[esp - 0xc], esi
		sub esp, 0xc
		mov esi, dword ptr ss:[esp + 0x10]
		lea edi, dword ptr ds:[esi + 0x0b7c]
		mov ecx, edi
		mov eax, 0x42032AD0 //GETACTIVATOR
		call eax 
		test eax, eax
		jz invalid
		push esi
		mov ecx, edi
		mov eax, 0x42032AD0 //GETACTIVATOR
		call eax
		mov ecx, eax
		mov eax, 0x4201E660 //DropCTFFlag
		call eax
invalid:
		add esp, 0xc
		mov edi, dword ptr ds:[esp - 4]
		mov edx, dword ptr ds:[esp - 8]
		mov esi, dword ptr ds:[esp - 0xc]

		retn
	}
}

__forceinline void DropPowerups ( C_BasePlayer *pPlayer ) {
	if ( pPlayer->powerup_flags & 0xF8 ) { // If player has powerup?
		((void(__cdecl*)(C_BasePlayer *))0x42020A30)(pPlayer); //Drop all powerups
	}
}

__forceinline int GetTeamchoice( C_BasePlayer* pPlayer ) {
	return pPlayer->team;
}

__forceinline int GetPlayerTeam( C_BasePlayer *pPlayer ) {
	//usage: GetPlayerTeam((CBasePlayer*)Instance(pEdict));
	return pPlayer->team;
}

__forceinline void SetPlayerTeam( C_BasePlayer *pPlayer, int team ) {
	pPlayer->team = team; //this doesn't do anything visible
}

__forceinline void SetPlayerHudColor( edict_t *pEntity, int team ) {
	//Note: Does not work even though the game does this, because the client overrides with colors.txt
	//Left here for documentation
		if ( team >= 2 ) {
			//PHOENIX
			MESSAGE_BEGIN(MSG_ONE, GET_USER_MSG_ID(PLID, "HudColor", NULL), 0, pEntity);
				WRITE_BYTE(255);
				WRITE_BYTE(160);
				WRITE_BYTE(0);
			MESSAGE_END();
		} else {
			//MI6
			MESSAGE_BEGIN(MSG_ONE, GET_USER_MSG_ID(PLID, "HudColor", NULL), 0, pEntity);
				WRITE_BYTE(128);
				WRITE_BYTE(128);
				WRITE_BYTE(128);
			MESSAGE_END();
		}
}

__forceinline void UTIL_HudText ( edict_t *pEntity, const char *pText ) {
	MESSAGE_BEGIN(MSG_ONE, GET_USER_MSG_ID(PLID, "HudText", NULL), 0, pEntity);
		WRITE_STRING( pText );
	MESSAGE_END();
}

__forceinline void UpdatePlayerTeam ( edict_t *pEntity, const char *pTeamName, int team ) {
	MESSAGE_BEGIN(MSG_ALL, GET_USER_MSG_ID(PLID, "TeamInfo", NULL), 0, (edict_t *)0);
		WRITE_BYTE(ENTINDEX(pEntity)); //playerindex
		WRITE_STRING(pTeamName); //Set blank for white name, Anything other than MI6 is red
		WRITE_BYTE(team); //0 = deathmatch, 1 = mi6, 2 = phoenix
	MESSAGE_END();
}

__forceinline bool IsOddjob( C_BasePlayer* pPlayer ) {
	if ( pPlayer->IsOddjob )
		return true;
	else if ( pPlayer->edict()->v.flags & 32768 )
		return true;
}

__forceinline void EnableOddjob ( C_BasePlayer* pPlayer ) {
	//i messed something up here, it semi works but you have no darts
	pPlayer->IsOddjob = true;
	pPlayer->edict()->v.flags |= 32768;
}

__forceinline void DisableOddjob ( C_BasePlayer* pPlayer ) {
	//i messed something up here, it semi works but you have no darts
	pPlayer->IsOddjob = false;
	pPlayer->edict()->v.flags &= ~32768;
}

__forceinline void GearboxSpectatorJoin ( edict_t *pEntity ) {
	//we only give it enough arguments to make the function work, and found that gearbox broke everything.
	//left here for experimentation
	int *arg1 = (int*)(CBasePlayer*)CBaseEntity::Instance(pEntity);
	int *arg2 = 0;
	int *arg3 = 0;
	int *arg4 = 0;
	int arg5 = 0;
	int *arg6 = 0;
	int arg7 = 0;
	int *arg8 = 0;
	int *arg9 = 0;
	int *arg10 = 0;
	int adr = 0x42046E00; //SpectatorJoin
	__asm {
		push arg10
		push arg9
		push arg8
		push arg7
		push arg6
		push arg5
		push arg4
		push arg3
		push arg2
		push arg1
		mov eax, adr
		call adr
		add esp, 0x18
	}
}

#define EDICT_NUM(i) ((edict_t*(*)(int))0x4306FA00)(i);

__forceinline __declspec(naked) edict_t* __stdcall EDICT_NUM_NAKED(int index) {
	__asm {
		mov eax, 0x4306FA00
		mov ecx, dword ptr ss:[esp + 4]
		mov dword ptr ss:[esp], ecx
		call eax
		retn 4
	}
}

__forceinline __declspec(naked) client_t* __stdcall EDICT_TO_CLIENT(edict_t *pEntity) {
	__asm{
		MOV DWORD PTR SS:[ESP-4],EDX
		MOV EAX,DWORD PTR SS:[ESP+4]
		MOV EDX, 0x421085
		SUB EAX,DWORD PTR DS:[0x44A7B9A4] //sv.edicts
		Js ZeroReturn
			MUL EDX
			DEC EDX
			CMP EDX,DWORD PTR DS:[0x44A86B50] //svs.maxclients
		Jge ZeroReturn
			MOV EAX, 0x4F18
			MUL EDX
			ADD EAX,DWORD PTR DS:[0x44A86B4C] //svs.clients
			MOV EDX,DWORD PTR SS:[ESP-4]
			RETN 4
		ZeroReturn: 
		MOV EDX,DWORD PTR SS:[ESP-4]
		MOV EAX, 0
		RETN 4
	}
}

BOOL IsCTF(void);

enum Skill {
	SK_UNUSED,
	SK_OPERATIVE,
	SK_AGENT,
	SK_00AGENT
};

__forceinline void Bond_HudMsg(edict_t *pEntity, char *text, int duration, int skill) {
	if ( pEntity )
		MESSAGE_BEGIN(MSG_ONE_UNRELIABLE, 0x73, 0, pEntity);
	else
		MESSAGE_BEGIN(MSG_BROADCAST, 0x73, 0, pEntity);

	WRITE_STRING(text); //text
	WRITE_BYTE(1); //spawnflags
	WRITE_BYTE(skill); //show on this minimum skill level: 1 = operative, 2 = agent, 3 = 00agent
	WRITE_BYTE(duration); //duration of hudmsg
	MESSAGE_END();
}

__forceinline void  Bond_TextMsg(edict_t *pEntity, int location, const char *str, const char *str2, const char *str3, const char *str4, const char *str5) {
	if ( pEntity ) {
		MESSAGE_BEGIN(MSG_ONE, GET_USER_MSG_ID(PLID, "TextMsg", NULL), 0, pEntity);
			WRITE_BYTE(location);
			WRITE_STRING(str);
			if ( str2 )
				WRITE_STRING(str2);
			if ( str3 )
				WRITE_STRING(str3);
			if ( str4 )
				WRITE_STRING(str4);
			if ( str5 )
				WRITE_STRING(str5);
		MESSAGE_END();		
	} else {
		MESSAGE_BEGIN(MSG_ALL, GET_USER_MSG_ID(PLID, "TextMsg", NULL), 0, (edict_t *)0);
			WRITE_BYTE(location);
			WRITE_STRING(str);
			if ( str2 )
				WRITE_STRING(str2);
			if ( str3 )
				WRITE_STRING(str3);
			if ( str4 )
				WRITE_STRING(str4);
			if ( str5 )
				WRITE_STRING(str5);
		MESSAGE_END();
	}
}

__forceinline void ChangePlayerTeam ( C_BasePlayer *pPlayer, const char *pTeamName, BOOL bKill, BOOL bGib ) {
	//crashes for some reason, use the one below instead
	((void(__stdcall*)(C_BasePlayer *, const char *, BOOL, BOOL))0x420CB750)(pPlayer, pTeamName, bKill, bGib);
}

__forceinline void ChangePlayerTeamCTF ( C_BasePlayer *pPlayer, const char *pCurrentTeamName, int iTeamToSwitchTo ) {
	((void(__stdcall*)(C_BasePlayer *, const char *, int))0x42044B00)(pPlayer, pCurrentTeamName, iTeamToSwitchTo - 1);
	//this function uses 0 for MI6, 1 for Phoenix
	
	if ( IsCTF() ) {
		//Send a message in the chat since it doesn't do it in CTF mode for some reason
		MESSAGE_BEGIN(MSG_ALL, GET_USER_MSG_ID(PLID, "SysMessage", NULL), 0, (edict_t *)0);
			WRITE_STRING("#TEAMChange");
			WRITE_STRING(EDICT_TO_CLIENT(pPlayer->edict())->name);
			WRITE_STRING(iTeamToSwitchTo <= 1 ? "MI6" : "Phoenix");
		MESSAGE_END();
	} else if ( CVAR_GET_INT("mp_teamplay") == 0 ) { 
		char *name = EDICT_TO_CLIENT(pPlayer->edict())->name;
		char buffer[256];
		sprintf(buffer, "%s has left team %s", name, pCurrentTeamName);
		SayTextAll(buffer);
	}
}//0 is MI6, 1 is Phoenix
//Example:
//CBaseEntity *pent = Instance(pEntity); //pEntity is an edict_t
//ChangePlayerTeamCTF((CBasePlayer*)pent, "MI6", 1);



//Old split character functions. Warning, they leak memory!

//char ** SplitChar( char * text, char delimiters[]){
//	/*
//	 example:
//	 	char *test = "test1 test2 test3";
//		char **split = SplitChar(test, " ");
//		char * firsttoken = split[0];
//		char * secondtoken = split[1];
//		char * thirdtoken = split[2];
//	*/
//	char * editabletext = new char[strlen(text)];
//	strcpy(editabletext, text);
//	std::vector<char*> * tokens = new std::vector<char*>();
//	tokens->reserve(5); //optional speed increase
//	char *test = strtok(editabletext, delimiters);
//	tokens->push_back(test);
//	char * token;
//	while(69){
//		token = strtok(0, delimiters);
//		if(token == nullptr)
//			break;
//		tokens->push_back(token);
//	}
//	return &tokens->at(0);
//}
//
//char ** SplitChar( char * text, char delimiters[], int * length){
//	/*
//	 example:
//	 	char *test = "test1 test2 test3";
//		int numberofstrings;
//		char **split = SplitChar(test, " ", &numberofstrings);
//		char * firsttoken = split[0];
//		char * secondtoken = split[1];
//		char * thirdtoken = split[2];
//	*/
//	char * editabletext = new char[strlen(text)];
//	strcpy(editabletext, text);
//	std::vector<char*> * tokens = new std::vector<char*>();
//	tokens->reserve(2); //optional speed increase
//	char *test = strtok(editabletext, delimiters);
//	tokens->push_back(test);
//	char * token;
//	*length = 1;
//	while(69){
//		token = strtok(0, delimiters);
//		if(token == nullptr)
//			break;
//		tokens->push_back(token);
//		(*length)++;
//	}
//	return &tokens->at(0);
//}

__forceinline int CVAR_GETDIRECT(int pointer) {
	return *(int*)(pointer + 0x14);
}

__forceinline void CVAR_SETDIRECT(int pointer, int value) {
	*(int*)(pointer + 0x14) = value;
}

__forceinline int CVAR_GETDIRECT(int* pointer) {
	return *(int*)((int)pointer + 0x14);
}

__forceinline void CVAR_SETDIRECT(int* pointer, int value) {
	*(int*)((int)pointer + 0x14) = value;
}

__forceinline int CVAR_GETDIRECTINT(int pointer) {
	return *(int*)(pointer + 0x14);
}

__forceinline void CVAR_SETDIRECTINT(int pointer, int value) {
	*(int*)(pointer + 0x14) = value;
}

__forceinline float CVAR_GETDIRECT(float pointer) {
	return *(float*)((int)pointer + 0x14);
}

__forceinline void CVAR_SETDIRECT(float pointer, float value) {
	*(float*)((int)pointer + 0x14) = value;
}

__forceinline float CVAR_GETDIRECT(float* pointer) {
	return *(float*)((int)pointer + 0x14);
}

__forceinline void CVAR_SETDIRECT(float* pointer, float value) {
	*(float*)((int)pointer + 0x14) = value;
}

__forceinline float CVAR_GETDIRECTFLOAT(int pointer) {
	return *(float*)(pointer + 0x14);
}

__forceinline void CVAR_SETDIRECTFLOAT(int pointer, float value) {
	*(float*)(pointer + 0x14) = value;
}

__forceinline bool CVAR_GETDIRECT(bool* pointer) {
	return *(bool*)(pointer + 0x14);
}

__forceinline void CVAR_SETDIRECT(bool* pointer, bool value) {
	*(bool*)(pointer + 0x14) = value;
}

__forceinline bool CVAR_GETDIRECT(bool pointer) {
	return *(bool*)(pointer + 0x14);
}

__forceinline void CVAR_SETDIRECT(bool pointer, bool value) {
	*(bool*)(pointer + 0x14) = value;
}

__forceinline bool CVAR_GETDIRECTBOOL(int pointer) {
	return *(bool*)(pointer + 0x14);
}

__forceinline void CVAR_SETDIRECTBOOL(int pointer, bool value) {
	*(bool*)(pointer + 0x14) = value;
}

__forceinline const char* CVAR_GETDIRECT(const char *pointer) {
	__asm {
		mov eax, pointer
		mov eax, dword ptr ds:[eax + 0x1c];
	}
}

__forceinline void CVAR_SETDIRECT(const char *pointer, char *value) {
	char *temp;
	__asm {
		mov eax, pointer
		mov eax, dword ptr ds:[eax + 0x1c];
		mov temp, eax
	}
	strcpy(temp, value);
}

__inline int GetCTFCaptures(int teamid) {
	if ( teamid == TEAM_MI6 )
		return *(int*)0x4215B7C8;
	else
		return *(int*)0x4215B7CC;
}

__inline void SetCTFCaptures(int teamid, int captures) {
	if ( teamid == TEAM_MI6 )
		*(int*)0x4215B7C8 = captures;
	else
		*(int*)0x4215B7CC = captures;

	MESSAGE_BEGIN(MSG_ALL, GET_USER_MSG_ID(PLID, "FlagIcon", NULL));
		WRITE_BYTE(0); //flag icon
		WRITE_BYTE(teamid);
		WRITE_BYTE(captures);
	MESSAGE_END();
}

__inline int GetCTFScore(C_BasePlayer* pPlayer) {
	return pPlayer->ctf_score;
}

__inline void SetCTFScore(C_BasePlayer* pPlayer, int score) {
	pPlayer->ctf_score = score;
	MESSAGE_BEGIN(MSG_ALL, GET_USER_MSG_ID(PLID, "CTFScore", NULL));
		WRITE_BYTE(ENTINDEX(pPlayer->edict()));
		WRITE_SHORT(score);
	MESSAGE_END();
}

__inline bool IsPlayerSpectating(edict_t *pEntity) {
	if ( pEntity->v.movetype == MOVETYPE_NOCLIP && pEntity->v.renderamt == 0 )
		return true;
	return false;
}

//Retrieves the current think function for the specified entity.
__forceinline __declspec (naked) void* GET_THINK(C_BaseEntity *pEntity) {
	__asm {
		mov eax, dword ptr ss:[esp + 4] //pEntity
		mov eax, dword ptr ds:[eax + 0x10]
		retn
	}
}

//Sets the think function for the specified entity. The Think function needs to be __stdcall
__forceinline __declspec (naked) void SET_THINK(C_BaseEntity *pEntity, void *function) {
	__asm {
		mov eax, dword ptr ss:[esp + 8] //function
		mov ecx, dword ptr ss:[esp + 4] //pEntity
		mov dword ptr ds:[ecx + 0x10], eax //pEntity + Think = function
		retn
	}
}

//Sets the touch function for the specified entity. The Touch function needs to be __stdcall
__forceinline __declspec (naked) void SET_TOUCH(C_BaseEntity *pEntity, void *function) {
	__asm {
		mov eax, dword ptr ss:[esp + 8] //function
		mov ecx, dword ptr ss:[esp + 4] //pEntity
		mov dword ptr ds:[ecx + 0x14], eax //pEntity + Touch = function
		retn
	}
}

//Gets the touch function for the specified entity
__forceinline __declspec (naked) void *GET_TOUCH(C_BaseEntity *pEntity) {
	__asm {
		mov eax, dword ptr ss:[esp + 4] //pEntity
		mov eax, dword ptr ds:[eax + 0x14]//pEntity + Touch
		retn
	}
}

//Sets the use function for the specified entity. The Use function needs to be __stdcall
__forceinline __declspec (naked) void SET_USE(C_BaseEntity *pEntity, void *function) {
	//m_pfnUse
	__asm {
		mov eax, dword ptr ss:[esp + 8] //function
		mov ecx, dword ptr ss:[esp + 4] //pEntity
		mov dword ptr ds:[ecx + 0x18], eax //pEntity + Use = function
		retn
	}
}

//Gets the use function for the specified entity.
__forceinline __declspec (naked) void* GET_USE(C_BaseEntity *pEntity) {
	__asm {
		mov eax, dword ptr ss:[esp + 4] //pEntity
		mov eax, dword ptr ds:[eax + 0x18] //pEntity + Use
		retn
	}
}

//Sets the next think time for the specified entity.
__forceinline void SET_NEXT_THINK(C_BaseEntity *pEntity, float time) {
	pEntity->pev->nextthink = time;
}

//Sets the next think time for the specified entity.
__forceinline __declspec (naked) void SET_NEXT_THINK(entvars_t2 *pev, float time) {
	__asm {
		mov eax, dword ptr ss:[esp + 8] //time
		mov ecx, dword ptr ss:[esp + 4] //pev
		mov dword ptr ds:[ecx + 0x104], eax //pev + nextthink = time
		retn
	}
}

__forceinline __declspec (naked) void SET_ACTIVATOR(C_BaseEntity *pEntity, C_BaseEntity *pActivator) {
	__asm {
		mov eax, dword ptr ss:[esp + 8] //pActivator
		mov ecx, dword ptr ss:[esp + 4] //pEntity
		mov dword ptr ds:[ecx + 0x100], eax //pEntity + Activator = pActivator
		retn
	}
}

#define SUB_DoNothing (void*)0x420C7C30

//-----------------------------------------------------------------------------
// Purpose: Fade an entity (player) screen to a specified color.
// Input  : *pEntity - Entity to tell to screen fade on.
// Input  : &color  - Color to fade to
// Input  : fadeTime - Time it takes to fade
// Input  : fadeHold - Time the fade holds for
// Input  : flags - Fade in, Fade Out, Fade_Modulate (don't blend), Fade_StayOut, Fade_Purge (Replaces others)
// Output : 
void UTIL_ScreenFade( edict_t *pEntity, byte r, byte g, byte b, byte a, float fadeTime, float fadeHold, int flags );

void UTIL_ScreenFadeAll( byte r, byte g, byte b, byte a, float fadeTime, float fadeHold, int flags );

void UTIL_ScreenFadeOne( edict_t *pEntity, byte r, byte g, byte b, byte a, float fadeTime, float fadeHold, int flags );

__inline void Create_TE_EXPLOSION( edict_t *pEntity, Vector origin, int iSprite, byte scale, byte frameRate, byte flags ) {
	if ( pEntity )
		MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, NULL, (edict_t*)NULL /*pEntity*/ );
	else
		MESSAGE_BEGIN(MSG_ALL, SVC_TEMPENTITY, NULL, (edict_t*)NULL );
	WRITE_BYTE( TE_EXPLOSION );
	WRITE_COORD( origin.x );		// position (X)
	WRITE_COORD( origin.y );		// position (Y)
	WRITE_COORD( origin.z	);		// position (Z)
	WRITE_SHORT( iSprite );			// sprite index
	WRITE_BYTE( scale );			// scale in 0.1's
	WRITE_BYTE( frameRate );		// framerate
	WRITE_BYTE( flags );			// flags
	MESSAGE_END();
	//#define TE_EXPLFLAG_NONE		0	// all flags clear makes default Half-Life explosion
	//#define TE_EXPLFLAG_NOADDITIVE	1	// sprite will be drawn opaque (ensure that the sprite you send is a non-additive sprite)
	//#define TE_EXPLFLAG_NODLIGHTS	2	// do not render dynamic lights
	//#define TE_EXPLFLAG_NOSOUND		4	// do not play client explosion sound
	//#define TE_EXPLFLAG_NOPARTICLES	8	// do not draw particles
}

#define ADR_SETQUERYVAR	0x430BD020
#define ADR_MASTERLISTDD 0x448BE1C8 //(Needed for sending query vars)
__inline void SetQueryVar(const char *param1, const char *param2) { //Var and description for that var
	DWORD adr;
	adr = ADR_SETQUERYVAR;
	//adr2 = ADR_MASTERLISTDD;
	DWORD masterlistdd = *(UINT32*)(ADR_MASTERLISTDD);
	__asm push param2
	__asm push param1
	__asm push 1
	__asm mov ecx, masterlistdd
	__asm call adr
}

C_BasePlayer* PLAYER_BY_INDEX(int index);

//void UTIL_SET_ORIGIN(C_BaseEntity *entity, const Vector &vecOrigin, bool bFireTriggers);
__forceinline void SV_LinkEdict(edict_t *pEdict, qboolean bFireTriggers) {
	((void(__cdecl*)(edict_t*, qboolean))0x430994D0)(pEdict, bFireTriggers);
}
__forceinline void UTIL_SET_ORIGIN(edict_t *pEdict, const Vector &vecOrigin, qboolean bFireTriggers) {
	pEdict->v.origin = vecOrigin;
	SV_LinkEdict(pEdict, bFireTriggers);
}

inline void VectorLerp(const Vector& src1, const Vector& src2, vec_t t, Vector& dest )
{
	//CHECK_VALID(src1);
	//CHECK_VALID(src2);
	dest.x = src1.x + (src2.x - src1.x) * t;
	dest.y = src1.y + (src2.y - src1.y) * t;
	dest.z = src1.z + (src2.z - src1.z) * t;
}

/* <474dc> ../engine/mathlib.c:465 */
inline int VectorCompare(const vec_t *v1, const vec_t *v2)
{
	for (int i = 0; i < 3; i++)
	{
		if (v1[i] != v2[i]) return 0;
	}

	return 1;
}

/* <47524> ../engine/mathlib.c:476 */
inline void VectorMA(const vec_t *veca, float scale, const vec_t *vecm, vec_t *out)
{
	out[0] = scale * vecm[0] + veca[0];
	out[1] = scale * vecm[1] + veca[1];
	out[2] = scale * vecm[2] + veca[2];
}

#define realtime (*(double*)0x448BE138) //Gets absolute realtime from engine

#endif
