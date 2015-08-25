# Configuration section

AR := ar
CC := gcc
CFLAGS := -g -O2 -I../../api/include
MKDIR := mkdir -p

APPS := console
FRAMEWORKS := framework_dyn

BUILDDIR := build

# Don't apply default rules

MAKEFLAGS += -r

# Configuration of the individual products

framework_dyn_SRC_FILES := 2d/2d.c 2d/2d_data.c 2d/2d_fw_version.c 2d/2d_rtc.c 2d/2d_update.c \
                           3d/3d.c 3d/3d_data.c 3d/3d_fw_version.c 3d/3d_rtc.c 3d/3d_update.c \
                           io/cdcserial_linux.c io/hid_3dtouchpad.c io/serial.c \
                           io/hidapi/linux/hid.c \
                           dynamic/dynamic.c core.c
framework_dyn_SRC_PATH  := ../../api/src
framework_dyn_BUILDDIR  := $(BUILDDIR)/framework/dynamic
framework_dyn_FILENAME  := libmchp_hmi.so
framework_dyn_CFLAGS    := -fpic -DHMI_API_EXPORT -DHMI_API_DYNAMIC -I../../api/src/io/hidapi
framework_dyn_LDFLAGS   := -shared -ludev

console_SRC_FILES := hmi.c main.c menu.c ui.c
console_SRC_PATH  := console
console_BUILDDIR  := $(BUILDDIR)/console
console_FILENAME  := console
console_CFLAGS    := -DHMI_API_DYNAMIC
console_LDFLAGS   := -L$(BUILDDIR)/bin -lmchp_hmi -Wl,-rpath,\$$ORIGIN -lcurses -lglut -lGL -lGLEW -lGLU -lm

.PHONY: all framework apps clean
.SUFFIXES:

all: framework apps

framework: $(FRAMEWORKS)

apps: $(APPS)

# Macro for creating object files and their dependency information
define make-object
$3: $2
	$(MKDIR) $$(dir $$@)
	$(CC) $(CFLAGS) $($1_CFLAGS) -MM -MF $$(patsubst %.o,%.d,$$@) $$<
	$(CC) $(CFLAGS) $($1_CFLAGS) -c -o $$@ $$<
endef

# Macro for creating whole products
define make-product
# Create substituted list of sources, objects and depend-files
$1_SOURCES := $$(patsubst %,$$($1_SRC_PATH)/%,$$($1_SRC_FILES))
$1_OBJECTS := $$(patsubst %.c,$$($1_BUILDDIR)/%.o,$$($1_SRC_FILES))
$1_DEPENDS := $$(patsubst %.c,$$($1_BUILDDIR)/%.d,$$($1_SRC_FILES))
# Add to overall list of targets that have to be removed on a clean
CLEAN += $$($1_DEPENDS) $$($1_OBJECTS) $$($1_BUILDDIR)/$$($1_FILENAME)
# Add depend files to overall list of depend-files that will be include
DEPENDS += $$($1_DEPENDS)
# Rule for creating object files from source files
$$(foreach src,$$($1_SRC_FILES),$$(eval $$(call make-object,$1,$$(patsubst %,$$($1_SRC_PATH)/%,$$(src)),$$(patsubst %.c,$$($1_BUILDDIR)/%.o,$$(src)))))
# Rule for identifying the target with its full path
$1: $(BUILDDIR)/bin/$$($1_FILENAME)
# Depending on target suffix select the right rule
ifeq ($$(suffix $$($1_FILENAME)),.a)
# Rule for creating static libraries
$(BUILDDIR)/bin/$$($1_FILENAME): $$($1_OBJECTS)
	$(MKDIR) $(BUILDDIR)/bin
	$(AR) rcs $$@ $$^
else
# Rule for creating dynamic libraries
$(BUILDDIR)/bin/$$($1_FILENAME): $$($1_OBJECTS)
	$(MKDIR) $(BUILDDIR)/bin
	$(CC) -o $$@ $$^ $$(LDFLAGS) $$($1_LDFLAGS)
endif
endef

$(foreach framework,$(FRAMEWORKS),$(eval $(call make-product,$(framework))))
$(foreach app,$(APPS),$(eval $(call make-product,$(app))))

-include $(DEPENDS)

clean:
	rm -f $(CLEAN)
