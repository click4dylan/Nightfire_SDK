#pragma once
#include <bool_nightfire.h>
#include "rendercolor.h"

// nightfire

class MaterialManager;
class RenderTextureAPI;
class d3dRender;

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
	virtual ~d3dRenderFeatures(); //27

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

class RenderTextureParametersAPI
{
public:

	virtual void Destroy() = 0;
	virtual ~RenderTextureParametersAPI();
};

class RenderTextureParametersContainer
{
public:
	RenderTextureParametersContainer() { render_texture_parameters_api = nullptr; }


	virtual bool attachParameters(RenderTextureParametersAPI* api) { render_texture_parameters_api = api;  return true; };
	virtual RenderTextureParametersAPI* getParameters() { return render_texture_parameters_api; }

private:
	RenderTextureParametersAPI* render_texture_parameters_api;
};

class RenderTextureAPI : public RenderTextureParametersContainer
{
public:
	struct TextureInfo
	{
		int size; //44 bytes on mac
		bool_nightfire has_unknown9;
		bool_nightfire has_unknown10;
		bool_nightfire has_colorformat;
		bool_nightfire is_empty;
		int unknown6;
		int unknown7;
		int unknown8;
		int unknown9;
		int unknown10;
		int colorformat;

		TextureInfo()
		{
			size = sizeof(TextureInfo);
			has_unknown9 = false;
			has_unknown10 = false;
			has_colorformat = false;
			is_empty = 0;
			unknown6 = 0;
			unknown7 = 0;
			unknown8 = 0;
			unknown9 = 0;
			unknown10 = 0;
			colorformat = 0;
		}
	};
	//44 bytes MAC

	virtual ~RenderTextureAPI();
	virtual bool loadHeader() = 0;
	virtual bool unloadHeader() = 0;
	virtual bool loadImage() = 0;
	virtual bool unloadImage() = 0;
	virtual bool setFormat(RenderTextureAPI::TextureInfo const& src) = 0;
	virtual bool getFormat(RenderTextureAPI::TextureInfo& dest) = 0;
	virtual const char* getName() = 0;
	virtual const char* getOwner() = 0;
	virtual void* getPixels() = 0;
};

class d3dFrameBufferTexture : public RenderTextureParametersContainer //RenderTextureAPI?
{
public:

	d3dFrameBufferTexture()
	{
		RenderTextureParametersContainer();
		textureinfo = RenderTextureAPI::TextureInfo();
	}

	virtual void Destroy();
	virtual ~d3dFrameBufferTexture();
	virtual bool loadHeader() { return true; }
	virtual bool unloadHeader() { return true; }
	virtual bool loadImage() { return image != nullptr; }
	virtual bool unloadImage() { if (image) delete image; image = nullptr; return true; }
	virtual bool setFormat(RenderTextureAPI::TextureInfo const& src) { return false; }
	virtual bool getFormat(RenderTextureAPI::TextureInfo& dest)
	{
		if (image)
		{
			if (dest.has_unknown9)
				dest.unknown9 = textureinfo.unknown9;
			if (dest.has_unknown10)
				dest.unknown10 = textureinfo.unknown10;
			if (dest.has_colorformat)
				dest.colorformat = textureinfo.colorformat;
			dest.is_empty = 0;
			return true;
		}
		return false;
	}
	virtual const char* getName() { return "screenshot"; }
	virtual const char* getOwner() { return "d3dRender"; }
	virtual void* getPixels() { return image; }

	RenderTextureAPI::TextureInfo textureinfo;
	void* image;
}; //0x38 bytes in MAC

struct display_t
{
	char data[40000];
};

class d3dRenderable
{
public:
	virtual bool setDisplayListCount(unsigned int);
	virtual bool setDisplayListViewport(unsigned int, RenderViewportAPI*);
	virtual bool addMaterial(unsigned int, RenderMaterialAPI*);
	virtual void Render();
	virtual void ResetAllDisplayLists();
	virtual void ResetDisplayList(unsigned int);
	virtual void setClearColor(ColorBGRA const&);
	virtual void setClearState(bool clear);
	virtual d3dFrameBufferTexture* getScreenshot();
	virtual void setWireFrame(bool wireframe);
	virtual ~d3dRenderable();
	virtual bool doClear() { return ClearBuffers(clear_to_color, true, clear_to_stencil, clear_color, 1.0f, clear_stencil); }
	virtual bool doRender();
	virtual bool doFlip();
	virtual int doPresent();
	virtual void* getSurfaceForScreenshot() = 0;

	bool ClearBuffers(bool clear_to_color, bool clear_to_z, bool clear_to_stencil, ColorBGRA color, float z, unsigned long stencil);
	
private:
	d3dRender* pRender;
	unsigned int clear_stencil; //?
	bool_nightfire clear_to_stencil;
	ColorBGRA clear_color;
	bool_nightfire clear_to_color;
	int unknown;
	unsigned int num_displays;
	void** pDisplays;
	void** pDisplayUnknown;
	display_t** pDisplayData;
	unsigned int num_times_rendered;
	char unknown_1[1952];
	char unknown_2[1952];
	char unknown_what[88];
	unsigned int num_times_flipped;
	unsigned int num_times_device_lost;
	char unknown_what3[1952];
};

class RenderAPI
{

};

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