FTP-Server
==========
An FTP server created from scratch. Allowing global access to this server is
not recommended at this time. The server can be used on a local network where
all users can be trusted.


How To Install
--------------
	1) cd into FTP-Server/source
	2) make
	3) cd into FTP-Server/config
	4) Run the guinetsetup.pl (requires GTK2) or netsetup.sh script to
	   select the interface and port for the server to use.


Directory Contents
------------------
FTP-Server/config - Configuration files for the server, and scripts to aid in
		    the configuration settings.

FTP-Server/rootdir - The default root directory of the server. This is the top
		     level directory that all users connected to the server can
		     access. This directory can be changed in the file
		     "FTP-Server/config/ftp.conf".

FTP-Server/source - The sourcecode used to create the server executable.


Authors
-------
The server is now being improved/maintained by Evan Myers, previous 
contributors may be found in the corresponding source files.
