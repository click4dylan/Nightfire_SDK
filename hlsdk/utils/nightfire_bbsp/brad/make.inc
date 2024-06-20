HLRAD_DEFINES+=
HLRAD:=$(OUTDIR)/hlrad$(EXEEXT)
HLRAD_SRCDIR:=hlrad
HLRAD_OUTDIR:=$(HLRAD_SRCDIR)/$(OUTDIR)


HLRAD_INPUT := \
$(HLRAD_SRCDIR)/lightmap.cpp \
$(HLRAD_SRCDIR)/qrad.cpp \
$(HLRAD_SRCDIR)/qradutil.cpp \
$(HLRAD_SRCDIR)/mathutil.cpp \
$(HLRAD_SRCDIR)/trace.cpp \
$(HLRAD_SRCDIR)/transfers.cpp \
$(HLRAD_SRCDIR)/vismatrix.cpp \
$(HLRAD_SRCDIR)/vismatrixutil.cpp \
$(HLRAD_SRCDIR)/sparse.cpp \
$(HLRAD_SRCDIR)/nomatrix.cpp \
$(HLRAD_SRCDIR)/lerp.cpp \
$(HLRAD_SRCDIR)/transparency.cpp \
$(COMMON_SRCDIR)/blockmem.cpp \
$(COMMON_SRCDIR)/bspfile.cpp \
$(COMMON_SRCDIR)/cmdlib.cpp \
$(COMMON_SRCDIR)/filelib.cpp \
$(COMMON_SRCDIR)/log.cpp \
$(COMMON_SRCDIR)/mathlib.cpp \
$(COMMON_SRCDIR)/messages.cpp \
$(COMMON_SRCDIR)/resourcelock.cpp \
$(COMMON_SRCDIR)/scriplib.cpp \
$(COMMON_SRCDIR)/threads.cpp \
$(COMMON_SRCDIR)/winding.cpp \


HLRAD_OUTPUT := \
$(HLRAD_OUTDIR)/lightmap$(OBJEXT) \
$(HLRAD_OUTDIR)/qrad$(OBJEXT) \
$(HLRAD_OUTDIR)/qradutil$(OBJEXT) \
$(HLRAD_OUTDIR)/mathutil$(OBJEXT) \
$(HLRAD_OUTDIR)/trace$(OBJEXT) \
$(HLRAD_OUTDIR)/transfers$(OBJEXT) \
$(HLRAD_OUTDIR)/vismatrix$(OBJEXT) \
$(HLRAD_OUTDIR)/vismatrixutil$(OBJEXT) \
$(HLRAD_OUTDIR)/sparse$(OBJEXT) \
$(HLRAD_OUTDIR)/nomatrix$(OBJEXT) \
$(HLRAD_OUTDIR)/lerp$(OBJEXT) \
$(HLRAD_OUTDIR)/transparency$(OBJEXT) \
$(HLRAD_OUTDIR)/blockmem$(OBJEXT) \
$(HLRAD_OUTDIR)/bspfile$(OBJEXT) \
$(HLRAD_OUTDIR)/cmdlib$(OBJEXT) \
$(HLRAD_OUTDIR)/filelib$(OBJEXT) \
$(HLRAD_OUTDIR)/log$(OBJEXT) \
$(HLRAD_OUTDIR)/mathlib$(OBJEXT) \
$(HLRAD_OUTDIR)/messages$(OBJEXT) \
$(HLRAD_OUTDIR)/resourcelock$(OBJEXT) \
$(HLRAD_OUTDIR)/scriplib$(OBJEXT) \
$(HLRAD_OUTDIR)/threads$(OBJEXT) \
$(HLRAD_OUTDIR)/winding$(OBJEXT) \

