Notes on Using the Pre-compiled libsndfile DLL.
===============================================

In order to use this pre-compiled DLL with Visual Studio, you will need to
generate a .LIB file from the DLL.

This can be achieved as follows:

  1) In a CMD window, change to the directory containing this file and
     run the command:
	 
	      dumpbin /exports libsndfile-1.dll > libsndfile.def

  2) Now run the command:
  
          lib /machine:i386 /def:libsndfile.def

You now have a .LIB and a .DLL file to be used with VisualStudio.

If for some reason these instructions don't work for you or you are still
not able to use the libsndfile DLL with you project, please do not contact
the main author of libsndfile. Instead, join the libsndfile-users mailing
list :

        http://www.mega-nerd.com/libsndfile/lists.html

and ask a question there.
