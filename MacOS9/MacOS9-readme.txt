
***************************************************************************
     Mac OS X is UNIX so use the INSTALL instructions in the dirctory 
	 above. These instructions are for OS9.
***************************************************************************


BUILDING LIBSNDFILE FOR MACINTOSH (Pre Mac OSX)
-----------------------------------------------

These instructions are current as of libsndfile 0.0.16, and assume the
following development environment:

- MacOS 8.6
- Metrowerks CodeWarrior Pro4 (IDE3.3, and all publicly available
  compiler updates as of June, 1999)
- Apple Universal Libraries 3.2

The following procedure is recommended for building a libsndfile library
suitable for inclusion in other MacOS projects:

1. using CodeWarrior, create a new "Empty Project"

2. obtain the libsndfile source distribution (see homepage URL below);
   add all ".c" files found in the top level of the "src", "src/GSM610" and 
   "src/G72x" folders to the project

3. starting from the factory defaults, adjust the following project
   settings:
   - Target Settings panel:
        linker = "MacOS PPC Linker"
   - PPC Target panel:
        project type = "Library"
        file name = "libsndfile"

4. grab the "config.h" file from the MacOS directory and replace the default
   config.h file in the "src" directory

5. Make the project


CROSSPLATFORM (x86) BUILDS?
--------------------------

For situations in which CodeWarrior is being used to develop a project
for dual-platform operation, it is possible to build an x86 version of
libsndfile on the mac as well.  Use the procedure above to set up the
PPC target, then perform the following steps:

6. create a new target, by cloning the existing ppc target (created
   above)

7. adjust the following project settings:
   - Target Settings panel:
        linker = "Win32 x86 Linker"
   - x86 Settings panel:
        project type = "Library (LIB)"
        file name = "libsndfile.x32.lib"
   - Access Paths panel:
        add this compiler-relative path at the TOP of "System Paths"
        (note: this must be at the TOP of the path list):
          "{Compiler}:Win32-x86 Support:Headers:Win32 SDK:sys:"

8. Make the x86 target


SEE ALSO
--------

The file "README" in the libsndfile distribution, for general
information about libsndfile.


