LAUNCHERDIR = launcher
BUILDDIR = build
FILESDIR = files
SRCDIR = src

all: flagUpdater/flag_updater $(LAUNCHERDIR)/$(FILESDIR) $(LAUNCHERDIR)/$(BUILDDIR) $(LAUNCHERDIR)/$(SRCDIR) $(LAUNCHERDIR)/$(BUILDDIR)/launcher
	sudo mkdir -p /var/ctf/
	sudo chmod 777 /var/ctf/

$(LAUNCHERDIR)/$(BUILDDIR):
	mkdir -p $@

$(LAUNCHERDIR)/$(FILESDIR):
	mkdir -p $@

$(LAUNCHERDIR)/$(BUILDDIR)/launcher: $(LAUNCHERDIR)/$(SRCDIR)/launcher.c $(LAUNCHERDIR)/$(BUILDDIR)
	$(CC) $(LAUNCHERDIR)/$(SRCDIR)/base64.c $(LAUNCHERDIR)/$(SRCDIR)/jsmn.c -o$@ $<

flagUpdater/flag_updater :  
	gcc flagUpdater/flag_updater.c -o flagUpdater/flag_updater 

clean :
	rm -rf flagUpdater/flag_updater
	rm -rf $(LAUNCHERDIR)/$(BUILDDIR)
	rm -rf $(LAUNCHERDIR)/$(FILESDIR)

.PHONY: all clean
