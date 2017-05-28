LAUNCHERDIR = launcher
BUILDDIR = build
FILESDIR = files
SRCDIR = src

all: flagUpdater/flag_updater $(LAUNCHERDIR)/$(FILESDIR) $(LAUNCHERDIR)/$(BUILDDIR) $(LAUNCHERDIR)/$(SRCDIR) $(LAUNCHERDIR)/$(BUILDDIR)/launcher
	mkdir -p /var/ctf/
	chown flagupdater /var/ctf/

$(LAUNCHERDIR)/$(BUILDDIR):
	mkdir -p $@

$(LAUNCHERDIR)/$(FILESDIR):
	mkdir -p $@
	chown flagupdater $@

$(LAUNCHERDIR)/$(BUILDDIR)/launcher: $(LAUNCHERDIR)/$(SRCDIR)/launcher.c $(LAUNCHERDIR)/$(BUILDDIR)
	$(CC) -D_FILE_OFFSET_BITS=64 -lb64 -lgpgme -lassuan -lgpg-error $(LAUNCHERDIR)/$(SRCDIR)/jsmn.c -o$@ $<

flagUpdater/flag_updater :  
	gcc flagUpdater/flag_updater.c -o flagUpdater/flag_updater 

clean :
	rm -rf flagUpdater/flag_updater
	rm -rf $(LAUNCHERDIR)/$(BUILDDIR)
	rm -rf $(LAUNCHERDIR)/$(FILESDIR)

.PHONY: all clean
