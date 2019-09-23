#
# Rules for building library
#

# ----------------------------------------------------------------------------
# common rules
# ----------------------------------------------------------------------------
ROOT_PATH := ../../../..

include $(ROOT_PATH)/gcc.mk

# ----------------------------------------------------------------------------
# library and objects
# ----------------------------------------------------------------------------
LIBS := libaudio.a
DIRS_IGNORE := ./effect%
DIRS_IGNORE += ./codec%

ifeq ($(findstring y,$(CONFIG_AUDIO_DECODER_AMR) \
	$(CONFIG_AUDIO_ENCODER_AMR) \
	$(CONFIG_AUDIO_DECODER_MP3) \
	$(CONFIG_AUDIO_ENCODER_SPEEX)), y)
include ./codec/codec.mk
endif

DIRS_ALL := $(shell find . -type d)
DIRS := $(filter-out $(DIRS_IGNORE),$(DIRS_ALL))

SRCS := $(basename $(foreach dir,$(DIRS),$(wildcard $(dir)/*.[cS])))

OBJS := $(addsuffix .o,$(SRCS))


# library make rules
INSTALL_PATH := $(ROOT_PATH)/lib

.PHONY: all install size clean install_clean

$(LIBS): $(LIBS_CODEC_OBJS) $(OBJS)
	$(Q)$(AR) -crs $@ $^

$(INSTALL_PATH):
	$(Q)test -d $(INSTALL_PATH) || mkdir -p $(INSTALL_PATH)

install: $(LIBS) $(INSTALL_PATH)
	$(Q)$(CP) -t $(INSTALL_PATH) $<

size:
	$(Q)$(SIZE) -t $(LIBS)

clean:
	$(Q)-rm -f $(LIBS) $(OBJS) $(DEPS) $(LIBS_CLEAN_OBJS) $(LIBS_ALL_NAME)

install_clean:
	$(Q)-rm -f $(INSTALL_PATH)/$(LIBS)

# ----------------------------------------------------------------------------
# dependent rules
# ----------------------------------------------------------------------------
DEPS = $(OBJS:.o=.d)
-include $(DEPS)
