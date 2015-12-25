# The versions of gmp and mpfr, for matching on the archive names.
GMP_VERSION=6.1.0
MPFR_VERSION=3.1.3
# The repo to clone valgrind from.
VALGRIND_REPO_LOCATION=svn://svn.valgrind.org/valgrind/trunk
# The architecture thhat we're buiding herbgrind for, in the syntax of
# valgrind filename conventions for this sort of thing.
TARGET_PLAT=amd64-linux
ARCH_PRI=amd64
ARCH_SEC=i386

all: compile

# We use the README file of a repo as a proxy for whether or not that
# repo currently exists.
valgrind/README:
# Check out valgrind from source.
	svn co $(VALGRIND_REPO_LOCATION) valgrind
# Run a script to modify the setup files to include the herbgrind
# directory.
	cd setup && ./modify_makefiles.sh
# Make a directory for the herbgrind tool
	mkdir valgrind/herbgrind
# ...and copy the files from the top level herbgrind folder into it.
	cp -r herbgrind/* valgrind/herbgrind/

# The herbgrind makefile needs to be recreated, if it's source .am
# file changes or we've just cloned the valgrind repo
valgrind/herbgrind/Makefile: valgrind/README herbgrind/Makefile.am
# Copy over the latest version of all the herbgrind stuff, including
# the .am file that we need for this step.
	cp -r herbgrind/* valgrind/herbgrind/
# Run the autogen and configure scripts to turn the .am file into a
# real makefile.
	cd valgrind && ./autogen.sh
	cd valgrind && ./configure --prefix=$(shell pwd)/valgrind/inst \
                                   --enable-only64bit

# This is the target we call to bring in the dependencies, like gmp,
# mpfr, and valgrind, and to make sure the herbgrind files have been
# initially copied over.
setup: valgrind/herbgrind/Makefile gmp/README mpfr/README

# This is the target we call to actually get the executable built so
# we can run herbgrind. 
valgrind/inst/lib/valgrind/herbgrind-$(TARGET_PLAT): herbgrind/hg_main.c setup
# First, we've got to make sure all the dependencies are extracted and set up.
	$(MAKE) setup
# Copy over the herbgrind sources again, because why the hell not.
	cp -r herbgrind/* valgrind/herbgrind
# Run make install to build the binaries and put them in the right
# place.
	$(MAKE) -C valgrind/ install

# Alias the compile target to just "compile" for ease of use
compile: valgrind/inst/lib/valgrind/herbgrind-$(TARGET_PLAT)

# Use the gmp README to tell if gmp has been extracted yet.
gmp/README: setup/gmp-$(GMP_VERSION).tar.xz
# Extract gmp, and rename its folder so we don't have to use the
# version number all over the place.
	tar xf setup/gmp-$(GMP_VERSION).tar.xz
	mv gmp-$(GMP_VERSION) gmp
# Touch the README to update its timestamp so that we don't build it
# again next time unless the archive changes.
	touch gmp/README
# Patch the gmp files to remove instances of memory functions which
# will fail to link with the valgrind partial c library.
	cd setup && ./patch_gmp.sh
# Configure and make it, putting its output in the install folder
# locally instead of in a global location, so it doesn't conflict with
# other versions of gmp.
	cd gmp/ && ./configure --prefix=$(shell pwd)/gmp/install-$(ARCH_PRI)
	$(MAKE) -C gmp
	$(MAKE) -C gmp install
# # Make gmp for 32-bit too, so that you can run herbgrind with maximum
# # portability.
# 	cd gmp/ && ABI=32 ./configure --prefix=$(shell pwd)/gmp/install-$(ARCH_SEC)
# 	$(MAKE) -C gmp
# 	$(MAKE) -C gmp install

# Use the mpfr readme to tell if mpfr has been extracted yet.
mpfr/README: setup/mpfr-$(MPFR_VERSION).tar.xz
# Extract mpfr, and rename its folder so we don't have to use the
# version number all over the place.
	tar xf setup/mpfr-$(MPFR_VERSION).tar.xz
	mv mpfr-$(MPFR_VERSION) mpfr
# Touch the README to update its timestamp so that we don't build it
# again next time unless the archive changes.
	touch mpfr/README
# Patch the mpfr files to allow us to use alternative memory functions
# which will not fail at link time.
	cd setup && ./patch_mpfr.sh
# Configure and make mpfr. We want to use the gmp we built locally for
# this, and we'll install it locally too for the same reasons as
# above.
	cd mpfr/ && ./configure --prefix=$(shell pwd)/mpfr/install-$(ARCH_PRI) \
                                --with-gmp=$(shell pwd)/gmp/install-$(ARCH_PRI)
	$(MAKE) -C mpfr
	$(MAKE) -C mpfr install
# # Make mpfr for 32-bit too.
# 	cd mpfr/ && ./configure --prefix=$(shell pwd)/mpfr/install-$(ARCH_SEC) \
# 				--with-gmp=$(shell pwd)/gmp/install-$(ARCH_SEC) \
# 				--build=i386
#	$(MAKE) -C mpfr
#	$(MAKE) -C mpfr install
