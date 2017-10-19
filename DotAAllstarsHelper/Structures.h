
#pragma pack(push,1)


enum class RawImageEventType : unsigned int
{
	MouseUp = 1U,
	MouseDown = 2U,
	MouseClick = 4U,
	MouseEnter = 8U,
	MouseLeave = 16U,
	MouseMove = 32U,
	ALL = 63U
};


struct ClickPortrainForId
{
	int abilid;
	int keycode;
};


struct CustomHPBar
{
	int unittypeid;
	unsigned int color;
	float scalex;
	float scaley;
};

struct Matrix1//Matrix 4x4
{
	float flt1;//0
	float flt2;//4
	float flt3;//8
	float flt4;//12
	float flt5;//16
	float flt6;//20
	float flt7;//24
	float flt8;//28
	float flt9;//32
	float flt10;//36
	float flt11;//40
	float flt12;//44
	float flt13;//48
	float flt14;//52
	float flt15;//56
	float flt16;//60
};


struct BarStruct
{
	int _BarClass;		// 0
	int _unk1_flag;		// 4
	int _unk2_flag;		// 8
	int _unk3_pointer;	// C
	int _unk4;			// 10
	int _unk5;			// 14
	int _unk6;			// 18
	int _unk7;			// 1c
	int _unk8;			// 20
	int _unk9;			// 24
	int _unk10;			// 28
	int _unk11;			// 2c
	int _unk12;			// 30
	int _unk13;			// 34
	int _unk14;			// 38
	int _unk15_pointer;	// 3c
	int _unk16_pointer;	// 40
	float offset1;		// 44
	float offset2;		// 48
	float offset3;		// 4c
	float offset4;		// 50
	int _unk17_flag;	// 54
	float ScaleX;		// 58
	float ScaleY;		// 5c
	float Scale;		// 60
	int _unk18;			// 64
	int _unk19_pointer; // 68
	int _unk20;			// 6C
	int _unk21;			// 70
	int _unk22;			// 78
	int _unk23;			// 7C
	int _unk24;			// 80
	int _unk25;			// 84
	int bartype;		// 88
	int _unk26;			// 8C
	int vis1;			// 90
	int vis2;			// 94
	float offset5;		// 98
	float offset6;		// 9C
	float offset7;		// 100
	float offset8;		// 104
	float offset9;		// 108
	int _unk29;			// 10C
	int _unk30;			// 110
	int _unk31;			// 114
	int _unk32;			// 118
	int _unk33;			// 11C
	int _unk34;			// 120
	int _unk35;			// 124
	int _unk36;			// 128
	int _unk37;			// 12C
	int _unk38;			// 130
	int _unk39;			// 134
	int _unk40_pointer;	// 138
	int _unk41_pointer;	// 13C
	int _unk42;			// 140
	int _unk43_pointer;	// 144
	int _unk44_pointer;	// 148
	int _unk45;			// 14C
	int _unk46_pointer;	// 150
	int _unk47_pointer;	// 154
	int _unk48;			// 158
	int _unk49_pointer;	// 15C
	int _unk50_pointer;	// 160
	int _unk51;			// 164
	int _unk52_pointer;	// 168
	int _unk53_pointer;	// 16C
	int _unk54;			// 170
	int _unk55_pointer;	// 174
	int _unk56_pointer;	// 178
	int _unk57;			// 17C
	int _unk58_pointer;	// 180
	int _unk59_pointer;	// 184
	int _unk60;			// 188
	float offset10;		// 18C
	float offset11;		// 190
	float offset12;		// 194
	int _unk61_pointer;	// 198
	int _unk62;			// 19C
	int _unk63;			// 200
	int _unk64_pointer;	// 204
	int _unk65_pointer;	// 208
	int _unk66;			// 20C
	int _unk67;			// 210
	float offset13;		// 214
	int unitaddr;		// 218
	int _unk68;			// 21C
	int _unk69;			// 220
};

struct StrTableNode
{
	void * unknownaddr;
	struct StringRep * curr;
};

struct String2H
{
	uint32_t			hash;//0x0
	void *				unk1;//0x4
	void *				unk2;//0x8
	void *				prev;//0xc
	String2H *			next;//0x10
	char *				text;//0x14
	int					id;//0x18
};

struct StringRep {
	void**				vtable;		//0x0
	uint32_t			refCount;	//0x4
	uint32_t			hash;		//0x8
	StrTableNode *		table;		//0xC
	StringRep*			prev;		//0x10
	/* txtnode */
	void *				prevtxtnode;//0x14  ?
	StringRep*			next;		//0x18
	char*				text;		//0x1C
};//sizeof = 0x20

struct RCString {
	void**				vtable;		//0x0
	uint32_t			unk_4;		//0x4
	StringRep*			stringRep;	//0x8
};//sizeof = 0xC 


struct ModelCollisionFixStruct
{
	string FilePath;
	float X, Y, Z, Radius;
	ModelCollisionFixStruct( )
	{
		FilePath = string( );
		X = Y = Z = Radius = 0.0f;
	}
};
struct ModelTextureFixStruct
{
	string FilePath;
	int TextureID;
	string NewTexturePath;
	ModelTextureFixStruct( )
	{
		FilePath = string( );
		NewTexturePath = string( );
		TextureID = 0;
	}
};
struct ModelPatchStruct
{
	string FilePath;
	string patchPath;
	ModelPatchStruct( )
	{
		FilePath = string( );
		patchPath = string( );
	}
};
struct ModelRemoveTagStruct
{
	string FilePath;
	string TagName;
	ModelRemoveTagStruct( )
	{
		FilePath = string( );
		TagName = string( );
	}
};
struct ModelSequenceReSpeedStruct
{
	string FilePath;
	string AnimationName;
	float SpeedUp;
	ModelSequenceReSpeedStruct( )
	{
		FilePath = string( );
		AnimationName = string( );
		SpeedUp = 0.0f;
	}
};
struct ModelScaleStruct
{
	string FilePath;
	float Scale;
	ModelScaleStruct( )
	{
		Scale = 0.0f;
		FilePath = string( );
	}
};
struct ModelSequenceValueStruct
{
	string FilePath;
	string AnimationName;
	int Indx;
	float Value;
	ModelSequenceValueStruct( )
	{
		FilePath = string( );
		AnimationName = string( );
		Indx = 0;
		Value = 0.0f;
	}
};




struct ICONMDLCACHE
{
	uint64_t _hash;
	size_t hashlen;
	char * buf;
	size_t size;
};

struct FileRedirectStruct
{
	string NewFilePath;
	string RealFilePath;
	FileRedirectStruct( )
	{
		NewFilePath = string( );
		RealFilePath = string( );
	}
};

struct FakeFileStruct
{
	char * filename;
	BYTE * buffer;
	size_t size;
};


struct RawImageCallbackData
{
	int RawImage;
	RawImageEventType EventType;
	float mousex;
	float mousey;
	BOOL IsAltPressed;
	BOOL IsCtrlPressed;
	BOOL IsLeftButton;
	int offsetx;
	int offsety;
	int RawImageCustomId;
};

struct RawImageStruct
{
	int RawImage; // 0x0
	int width;// 0x4
	int height;// 0x8
	StormBuffer img;// 0xc
	StormBuffer ingamebuffer;// 0x10
	BOOL ingame;// 0x14
	string filename;// 0x18
	BOOL used_for_overlay;// 0x1c
	float overlay_x; // 0.0 1.0 // 0x20
	float overlay_y; // 0.0 1.0 // 0x24
	BOOL button; // 0x28 
	void * textureaddr;// 0x2C
	BOOL needResetTexture;// 0x30

	BOOL MouseCallback; // 0x34

	BOOL	MouserExecuteFuncCallback;
	BOOL	PacketCallback;
	RCString MouseActionCallback; // 0x38


	BOOL IsMouseDown; // 0x3C
	BOOL IsMouseEntered; // 0x40
	unsigned int events;// 0x44
	float overlay_x2; // 0x48
	float overlay_y2;
	float overlay_x0;
	float overlay_y0;
	DWORD MoveTime1;
	DWORD MoveTime2;
	DWORD SleepTime;
	DWORD StartTimer;

	int RawImageCustomId;

	RawImageStruct( )
	{
		width = 0;
		height = 0;
		RawImageCustomId = 0;
		img = StormBuffer( );
		ingamebuffer = StormBuffer( );
		ingame = FALSE;
		filename = string( );
		used_for_overlay = FALSE;
		overlay_x = overlay_y = overlay_x0 = overlay_y0 = overlay_y2 = overlay_x2 = 0.0f;
		textureaddr = NULL;
		needResetTexture = FALSE;
		RawImage = 0;
		events = 0;
		IsMouseDown = FALSE;
		IsMouseEntered = FALSE;
		MouseCallback = FALSE;
		MouseActionCallback = RCString( );
		MouserExecuteFuncCallback = FALSE;
		PacketCallback = FALSE;
		button = FALSE;
		MoveTime1 = 0;
		MoveTime2 = 0;
		SleepTime = 0;
		StartTimer = 0;
	}
};


struct waroffsetdata
{
	int offaddr;
	int offdata;
	bool newdataapp;
	int offnewdata;
	unsigned int FeatureFlag;
};


struct FeatureRestorer
{
	unsigned int FeatureFlag;
	int Bytes;
	int Address;
};





struct Mdx_Texture        //NrOfTextures = ChunkSize / 268
{
	int ReplaceableId;
	CHAR FileName[ 260 ];
	unsigned int Flags;                       //#1 - WrapWidth
											  //#2 - WrapHeight
};

struct Mdx_FLOAT3
{
	float x;
	float y;
	float z;
};

struct Mdx_Sequence      //NrOfSequences = ChunkSize / 132
{
	CHAR Name[ 80 ];

	int IntervalStart;
	int IntervalEnd;
	FLOAT MoveSpeed;
	unsigned int Flags;                       //0 - Looping
											  //1 - NonLooping
	FLOAT Rarity;
	unsigned int SyncPoint;

	FLOAT BoundsRadius;
	Mdx_FLOAT3 MinimumExtent;
	Mdx_FLOAT3 MaximumExtent;
};




struct Mdx_SequenceTime
{
	int * IntervalStart;
	int * IntervalEnd;
};



struct Mdx_Track
{
	int NrOfTracks;
	int InterpolationType;             //0 - None
									   //1 - Linear
									   //2 - Hermite
									   //3 - Bezier
	unsigned int GlobalSequenceId;
};

struct Mdx_Tracks
{
	int NrOfTracks;
	unsigned int GlobalSequenceId;

};
struct Mdx_Node
{
	unsigned int InclusiveSize;

	CHAR Name[ 80 ];

	unsigned int ObjectId;
	unsigned int ParentId;
	unsigned int Flags;                         //0        - Helper
												//#1       - DontInheritTranslation
												//#2       - DontInheritRotation
												//#4       - DontInheritScaling
												//#8       - Billboarded
												//#16      - BillboardedLockX
												//#32      - BillboardedLockY
												//#64      - BillboardedLockZ
												//#128     - CameraAnchored
												//#256     - Bone
												//#512     - Light
												//#1024    - EventObject
												//#2048    - Attachment
												//#4096    - ParticleEmitter
												//#8192    - CollisionShape
												//#16384   - RibbonEmitter
												//#32768   - Unshaded / EmitterUsesMdl
												//#65536   - SortPrimitivesFarZ / EmitterUsesTga
												//#131072  - LineEmitter
												//#262144  - Unfogged
												//#524288  - ModelSpace
												//#1048576 - XYQuad
};

struct Mdx_GeosetAnimation
{
	unsigned int InclusiveSize;

	FLOAT Alpha;
	unsigned int Flags;                       //#1 - DropShadow
											  //#2 - Color
	Mdx_FLOAT3 Color;

	unsigned int GeosetId;

};




struct DelayedPress
{
	DWORD NeedPressMsg;
	WPARAM NeedPresswParam;
	LPARAM NeedPresslParam;
	DWORD TimeOut;
	BOOL ISNULL;
	BOOL IsAlt;
	BOOL IsCtrl;
	BOOL IsShift;
	BOOL IsCustom;

	BOOL IsNull( )
	{
		return ISNULL;
	}

	void Null( BOOL yes )
	{
		ISNULL = yes;
	}

	DelayedPress( )
	{
		ISNULL = NeedPressMsg
			= NeedPresswParam = NeedPresslParam
			= TimeOut = IsAlt = IsCtrl = IsShift = IsCustom = 0;
	}
};

struct KeyActionStruct
{
	int VK;
	int btnID;
	int altbtnID;
	BOOL IsSkill;
	BOOL IsShift;
	BOOL IsCtrl;
	BOOL IsAlt;
	BOOL IsRightClick;
};


#pragma pack(pop)