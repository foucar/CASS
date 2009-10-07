# Call Qt meta object compiler (moc)
MOC := $(RELEASE_DIR)/build/qt/bin/moc

$(objdir)/%_moc.o: %_moc.cc
	@echo "[CX] Compiling $<"
	$(quiet)$(CXX) $(incdirs_$<) $(DEFINES) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

%_moc.cc: %.hh
	@echo "[MOC] Qt moc $<"
	$(quiet)$(MOC) $< -o $@
