libnames := appdata
libsrcs_appdata := XtcMonitorClient.cc

tgtnames = xtcmonserver

tgtsrcs_xtcmonserver := xtcmonserver.cc
tgtlibs_xtcmonserver := pdsdata/xtcdata pdsdata/acqdata
tgtslib_xtcmonserver := $(USRLIBDIR)/usr/lib/rt
