mod_realpath.la: mod_realpath.slo
	$(SH_LINK) -rpath $(libexecdir) -module -avoid-version  mod_realpath.lo
DISTCLEAN_TARGETS = modules.mk
shared =  mod_realpath.la
