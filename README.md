ftp-server
==========

An FTP server created from scratch. Allowing global access to this server is
not recommended at this time. The server can be used on a local network where
all users can be trusted.


Contents: [DIR] - contents
   [conf] - Configuration files for the server, as well as a script to auto
            configure the network settings.

   [rootdir] - The default root directory of the server. The server's root
            directory is the highest level directory that can be accessed by
            a user on the server. The server's root directory may be changed
            in the file './conf/ftp.conf'.

   [source] - The sourcecode used to create the server executable.



Authors: The server is now being improved/maintained by Evan Myers, previous
         contributors may be found in the corresponding source files.