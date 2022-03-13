MODNAME = singlep_mm

EXTRA_CFLAGS = 

SRCFILES = engine_api.cpp h_export.cpp log_plugin.cpp meta_api.cpp \
	sdk_util.cpp vdate.cpp \
	singlep.cpp 

INFOFILES = info_name.h vers_singlep.h
RESFILE = res_meta.rc

PLATFORM = win32-only
