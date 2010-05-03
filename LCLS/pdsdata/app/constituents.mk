libnames := appdata
libsrcs_appdata := XtcMonitorServer.cc XtcMonitorClient.cc XtcMonitorMsg.cc

tgtnames = xtcmonserver

tgtsrcs_xtcmonserver := xtcmonserver.cc
tgtlibs_xtcmonserver := pdsdata/xtcdata pdsdata/acqdata pdsdata/appdata
tgtslib_xtcmonserver := $(USRLIBDIR)/usr/lib/rt
