# Architecture flags
# ------------------
arch_tgts := ppc-linux ppc-rtems-rce405 ppc-rtems-ml405 i386-linux x86_64-linux sparc-solaris x86_64-Darwin
arch_opts := opt dbg

define arch_opt_template
arch_tgts += $$(addsuffix -$(1),$$(arch_tgts))
endef
$(foreach opt,$(arch_opts),$(eval $(call arch_opt_template,$(opt))))

# Separate the components of tgt_arch using the dash as a separator.
archwords      := $(subst -, ,$(strip $(tgt_arch)))
tgt_opt        := $(filter $(arch_opts),$(archwords))
archwords      := $(filter-out $(arch_opts),$(archwords))
tgt_cpu_family := $(word 1,$(archwords))
tgt_os         := $(word 2,$(archwords))
tgt_board      := $(word 3,$(archwords))

# Define a compile-time macro for each architecture component.
DEFINES := -Dtgt_opt_$(tgt_opt)
DEFINES += -Dtgt_cpu_family_$(tgt_cpu_family)
DEFINES += -Dtgt_os_$(tgt_os)
DEFINES += -Dtgt_board_$(tgt_board)

# i386 Linux specific flags
ifeq ($(tgt_cpu_family)-$(tgt_os),i386-linux)
AS  := as
CPP := gcc -E
CC  := gcc
CXX := g++
LD  := g++
LX  := g++

LIBEXTNS := so
DEPFLAGS := -MM
DEFINES  += -fPIC -D_REENTRANT -D__pentium__ -Wall
CPPFLAGS :=
CFLAGS   := -m32
CXXFLAGS := $(CFLAGS)
CASFLAGS := -x assembler-with-cpp -P $(CFLAGS)
LDFLAGS  := -m32 -shared
LXFLAGS  := -m32
else
ifeq ($(tgt_cpu_family)-$(tgt_os),x86_64-Darwin)
AS  := as
CPP := gcc -E
CC  := gcc
CXX := g++
LD  := g++
LX  := g++

LIBEXTNS := so
DEPFLAGS := -MM
DEFINES  += -fPIC -D_REENTRANT -D__pentium__ -Wall
CPPFLAGS :=
CFLAGS   := -m64
CXXFLAGS := $(CFLAGS)
CASFLAGS := -x assembler-with-cpp -P $(CFLAGS)
LDFLAGS  := -Wl,-rpath,@loader_path/../lib -dynamiclib -single_module -undefined dynamic_lookup -install_name @rpath -m64 
LXFLAGS  := -m64
else
ifeq ($(tgt_cpu_family),x86_64)
AS  := as
CPP := gcc -E
CC  := gcc
CXX := g++
LD  := g++
LX  := g++

LIBEXTNS := so
DEPFLAGS := -MM
DEFINES  += -fPIC -D_REENTRANT -D__pentium__ -Wall
CPPFLAGS :=
CFLAGS   := -m64
CXXFLAGS := $(CFLAGS)
CASFLAGS := -x assembler-with-cpp -P $(CFLAGS)
LDFLAGS  := -m64 -shared
LXFLAGS  := -m64
else
# Sparc Solaris specific flags
ifeq ($(tgt_cpu_family)-$(tgt_os),sparc-solaris)
AS  := as
CPP := gcc -E
CC  := gcc
CXX := g++
LD  := g++
LX  := g++

LIBEXTNS := so
DEPFLAGS := -MM
DEFINES  += -fPIC -D_REENTRANT -Wall
CPPFLAGS :=
CFLAGS   :=
CXXFLAGS := $(CFLAGS)
CASFLAGS := -x assembler-with-cpp -P $(CFLAGS)
LDFLAGS  := -shared
LXFLAGS  :=
else
# PowerPC RTEMS specific flags
ifeq ($(tgt_cpu_family)-$(tgt_os),ppc-rtems)
AS  := powerpc-rtems-as
CPP := powerpc-rtems-cpp
CC  := powerpc-rtems-gcc
CXX := powerpc-rtems-g++
LD  := powerpc-rtems-ld
LX  := powerpc-rtems-g++

ifeq ($(tgt_board),rce405)
RTEMSDIR := $(RELEASE_DIR)/build/rtems/target/powerpc-rtems/rce405/lib
endif

ifeq ($(tgt_board),ml405)
RTEMSDIR := $(RELEASE_DIR)/build/rtems/target/powerpc-rtems/ml405/lib
endif

LIBEXTNS := a
DEPFLAGS := -B$(RTEMSDIR) -MM
DEFINES  += -Dppc405 -mcpu=403 -Wall
CPPFLAGS :=
CFLAGS   := -B$(RTEMSDIR) -specs bsp_specs -qrtems
CXXFLAGS := $(CFLAGS)
CASFLAGS := -x assembler-with-cpp -P $(CFLAGS)
LDFLAGS  := -r
LXFLAGS  := -B$(RTEMSDIR) -specs $(RELEASE_DIR)/rce/ldtools/dynamic_specs -qrtems -qnolinkcmds -Wl,-T$(RELEASE_DIR)/rce/ldtools/dynamic.ld
MANAGERS := timer sem msg event signal part region dpmem io rtmon ext mp
else
ifeq ($(tgt_cpu_family)-$(tgt_os),ppc-linux)
AS  := as
CPP := gcc -E
CC  := gcc
CXX := g++
LD  := g++
LX  := g++

LIBEXTNS := so
DEPFLAGS := -MM
DEFINES  += -D_REENTRANT -Wall
CPPFLAGS :=
CFLAGS   :=
CXXFLAGS := $(CFLAGS)
CASFLAGS := -x assembler-with-cpp -P $(CFLAGS)
LDFLAGS  := -shared
LXFLAGS  :=

endif
endif
endif
endif
endif
endif

ifeq ($(tgt_opt),opt)
DEFINES += -O4
endif

ifeq ($(tgt_opt),dbg)
DEFINES += -g
endif
