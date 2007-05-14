@echo off

if "%1"=="check" GOTO CHECK
if "%1"=="clean" GOTO CLEAN

copy /y Win32\sndfile.h src\sndfile.h
copy /y Win32\config.h src\config.h

nmake -f Win32\Makefile.msvc
goto END


:CHECK
nmake -f Win32\Makefile.msvc check
goto END

:CLEAN
nmake -f Win32\Makefile.msvc clean
goto END


:END


goto skipArchTag

 Do not edit or modify anything in this comment block.
 The arch-tag line is a file identity tag for the GNU Arch 
 revision control system.

 arch-tag: 8700080b-8d9a-4852-ad8a-8ecd027f1f61

:skipArchTag
