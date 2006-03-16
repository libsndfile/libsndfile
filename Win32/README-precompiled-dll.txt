Notes on Using the Pre-compiled libsndfile DLL.
===============================================

The following describes how to use the pre-compiled DLL with 

1) run dumpbin /exports libsndfile.dll > libsndfile.def (libsndfile-1.dll)

(pexports, in mingw, does not seem to do more than generate a help header)

2) edit libsndfile.def so that it looks like the text below

3) lib /machine:i386 /def:libsndfile.def
