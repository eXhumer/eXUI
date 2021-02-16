ifndef DEVKITPRO
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
else
include $(DEVKITPRO)/libnx/switch_rules
endif

LIB_NANOVG	:=	libs/nanovg
LIB_LRC		:=	libs/libretro-common
LIB_FMT		:=	libs/fmt

LIB			:=	lib
TARGET		:=	libeXUI
SOURCES		:=	source $(LIB_FMT)/src
INCLUDES	:=	include

ARCH	:=	-march=armv8-a -mtune=cortex-a57 -mtp=soft -fPIE

DEFINES	:=	-D__SWITCH__
# DEFINES	+=	-DDEBUG_NXLINK

CFLAGS	:=	-Wall -O3 -ffunction-sections \
			$(ARCH) $(DEFINES)

CXXFLAGS	:= -std=gnu++2a -fno-rtti

LIBDIRS	:= $(PORTLIBS) $(LIBNX) $(LIB_NANOVG) $(LIB_LRC) $(LIB_FMT)

OUTPUT	:=	$(LIB)/$(TARGET)
LIB_OUTPUT	:=	$(OUTPUT).a

CFILES		:=	$(foreach SOURCE,$(SOURCES),$(wildcard $(SOURCE)/*.c)) \
				$(LIB_LRC)/compat/compat_strl.c \
				$(LIB_LRC)/encodings/encoding_utf.c \
				$(LIB_LRC)/features/features_cpu.c
CCFILES		:=	$(foreach SOURCE,$(SOURCES),$(wildcard $(SOURCE)/*.cc))
CPPFILES	:=	$(foreach SOURCE,$(SOURCES),$(wildcard $(SOURCE)/*.cpp))

ifeq ($(strip $(CPPFILES)),)
	LD	:=	$(CC)
else
	LD	:=	$(CXX)
endif

OFILES 	:=	$(CFILES:.c=.o) $(CCFILES:.cc=.o) $(CPPFILES:.cpp=.o)
DEPENDS	:=	$(OFILES:.o=.d)

INCFLAGS	:=	$(foreach INCLUDE,$(INCLUDES),-I$(INCLUDE)) \
				$(foreach LIBDIR,$(LIBDIRS),-I$(LIBDIR)/include)

.PHONY: library clean

library: $(LIB_OUTPUT)
$(LIB_OUTPUT): $(LIB) $(OFILES)

$(LIB):
	[ -d $@ ] || mkdir -p $@

%.o:	%.c
	$(CC) -MMD -MP -MF $(@:%.o=%.d) $(CFLAGS) $(INCFLAGS) -c $< -o $@

%.o:	%.cc
	$(CXX) -MMD -MP -MF $(@:%.o=%.d) $(CFLAGS) $(INCFLAGS) $(CXXFLAGS) -c $< -o $@

%.o:	%.cpp
	$(CXX) -MMD -MP -MF $(@:%.o=%.d) $(CFLAGS) $(INCFLAGS) $(CXXFLAGS) -c $< -o $@

$(LIB_OUTPUT):
	$(AR) -rc $(LIB_OUTPUT) $(OFILES)

clean:
	rm -rf $(OFILES) $(DEPENDS) $(LIB_OUTPUT)

-include $(DEPENDS)
