NOTE: This is not the way the author builds libsndfile on Win32 
so this desciption may be out of date. For the authors method
of building libsndfile on Win32 have a look at the file named
win32.html in the doc\ directory of the source code distribution.

*****************************************************************

This is the readme-Win32.txt file associated with the LibSndFile 
library. It describes how the included workspace and project 
was created for Microsoft Visual C++ developer's studio (MSVC), 
version 5.0.  Skip to point 7 if you wish to create a new 
project for building an executable.

1. Extracted libsndfile.zip to d:\files\msvc\

2. It created (replace X.Y.Z with the libsndfile version number)
     d:\files\msvc\libsndfile-X.Y.Z\Win32      *
     d:\files\msvc\libsndfile-X.Y.Z\src        *
     d:\files\msvc\libsndfile-X.Y.Z\tests      *
     d:\files\msvc\libsndfile-X.Y.Z\examples   
     d:\files\msvc\libsndfile-X.Y.Z\doc
     d:\files\msvc\libsndfile-X.Y.Z\m4
     d:\files\msvc\libsndfile-X.Y.Z\MacOS

     * are needed for this example

3. From MSVC:New->Workspace, I created LibSndFileWorkspace at:
    d:\files\msvc\libsndfile-X.Y.Z\Win32\
    (workspace files have the extension .dsw)

3. In MSVC, rt-click on "Workspace LibSndFileWorkspace" and add project:
    Project type:      Win32 Static Library  
    Project Name:      LibSndFile 
    Project Location:  D:\files\msvc\libsndfile-X.Y.Z\Win32
    Select button:     'Add to current workspace' 
    Platforms:         Win32

4.  Rt-click newly formed "LibSndFile files" and add files:
     d:\files\msvc\libsndfile-X.Y.Z\src\*.*
     d:\files\msvc\libsndfile-X.Y.Z\src\Gsm610\*.*
     d:\files\msvc\libsndfile-X.Y.Z\src\G72x\*.*

5.  Rt-click 'LibSndFile files' and  go to Settings
     a. Select all configurations on the left hand side
     b. Then select C/C++/Preprocessor and add
     "..\" (no quotes) to 'Additional include directories'
     (This allows ..Win32\config.h and unistd.h to be found.)

6.  At this point you should be able to build the library. The output
    will be found in ..\Win32\LibSndFile\Debug\LibSndFile.lib. You can
    change the LibSndFile project to Release and a similar release 
    path will be created.

The following describes how to add an application project to the 
workspace. You may add as many as you wish. In general, you will
need one project for each executable you want to create.

7. Rt-click LibSndFileWorkspace and select 'Add project'
    Project type:      Win32 Console Application   
    Project Name:      sfversion 
    Location: d:\files\msvc\libsndfile-X.Y.Z\Win32\sfversion 
    Select button:     'Add to current workspace' 
    Platforms:         Win32
    
    Notes: 
     - MSVC will create a directory ..\Win32\sfversion\
     - MSVC will create the file sfversion.dsp in this directory

8. Rt-click 'sfversion files' and add file:
     d:\files\msvc\libsndfile-X.Y.Z\tests\sfversion.c

9. Rt-click 'sfversion files' and go to Settings:
     a. Select 'All configurations' on the left hand side
     b. Then select C/C++/Preprocessor and add
     "..\..\src,..\" (no quotes) to 'Additional include directories'

9. Rt-click 'sfversion files' and go to Settings:
     a. Select 'Debug Configuration' on left hand side
     b. Then select Link tab and add
     "..\LibSndFile\Debug\LibSndFile.lib " (no quotes) to 
     the list of 'Object/library modules'. Leave a space between new
     addition existing lib files.

10. Repeat above for Release build adding Release path info.

11. Build your application, it should link and create an .exe

Final notes:

Files created during build by msvc but are not needed for archive:
*ncb *.plg *.opt *.obj *.idb *.pch *.lib *.exe

Files associated with LibSndFile but not used by msvc:
Makefile.in
Makefile.am
                          - End -  
