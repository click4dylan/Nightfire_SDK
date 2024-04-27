#pragma once


// nightfire

class MaterialManager;
struct RenderTextureAPI;
struct d3dRender;

__declspec(align(16))
class d3dRenderFeatures
{
public:

	virtual const char* getFirstDevice(unsigned int&);
	virtual const char* getNextDevice(unsigned int&);
	virtual bool getFirstResolution(unsigned int, unsigned int&, unsigned int&, unsigned int&, unsigned int&, unsigned int&);
	virtual bool getNextResolution(unsigned int, unsigned int&, unsigned int&, unsigned int&, unsigned int&, unsigned int&);
	virtual void* getDevice();
	virtual void setDevice(unsigned int);
	virtual unsigned int getWidth();
	virtual bool setWidth(unsigned int);
	virtual unsigned int getHeight();
	virtual bool setHeight(unsigned int);
	virtual unsigned int getRefreshRate();
	virtual bool setRefreshRate(unsigned int);
	virtual bool getVSYNC();
	virtual bool setVSYNC(bool);
	virtual unsigned int getColorDepth();
	virtual bool setColorDepth(unsigned int);
	virtual unsigned int getZDepth();
	virtual bool setZDepth(unsigned int);
	virtual unsigned int getStencilBits();
	virtual bool setStencilBits(unsigned int);
	virtual bool getFullscreen();
	virtual bool setFullscreen(bool);
	virtual unsigned int getMultisampleCount();
	virtual bool setMultisampleCount(unsigned int);
	virtual bool getDumpCaps();
	virtual void setDumpCaps(bool);
	virtual void Destroy();
	virtual ~d3dRenderFeatures();

	d3dRender* render;
	char* name; //2048 length
	bool_nightfire dumpcaps;
	bool_nightfire fullscreen;
	bool_nightfire vsync;
	bool_nightfire device;
	unsigned int width;
	unsigned int height;
	unsigned int colordepth;
	unsigned int refreshrate;
	unsigned int zdepth;
	unsigned int stencilBits;
	unsigned int multisamplecount;
	unsigned int stencilbits;
};
//0x38 bytes MAC, 16 aligned 0x30 PC

//TextureManager
typedef struct triangleapi_s
{
	int version; //1
	int size;

	void (*LoadTexture_Client)(const char*);
	void (*UnloadTexture_Client)(RenderTextureAPI*);
	d3dRender* pRender;
	void* pRenderPlatformInfo;
	void* pRenderFeatures;
	void( __thiscall* pCreateRenderMaterial)(MaterialManager*, const char*);
} triangleapi_t;