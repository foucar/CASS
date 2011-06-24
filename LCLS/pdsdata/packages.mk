# List of packages (low level first)
ifneq ($(findstring ppc-rtems-rce,$(tgt_arch)),)
packages := xtc
else
packages := ipimb pnCCD acqiris camera evr opal1k pulnix control xtc \
      epics bld princeton lusi app
endif
