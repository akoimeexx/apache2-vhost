apache2-vhost
=============

NAME
----
apache2-vhost - Apache2 vhost configuration manager

SYNOPSIS
--------
```bash
apache2-vhost -[aprs] <vhostdomain>; apache2-vhost -[hlv]
```

DESCRIPTION
-----------
apache2-vhost is a command line program to assist in adding and removing apache2 virtual hosts to and from the system. It completes this task by utilizing the creation and removal of regular and symlinked files in the __HTTPD_ROOT__/sites-available/ and __HTTPD_ROOT__/sites-enabled/ directories respectively, and managing _&lt;vhostdomain&gt;_ entries in the system's hosts file. It uses the current working directory for the virtual host's document_root.

This program was written primarily for web developers that need to host several web projects on one system when developing and testing. It should allow a web developer to navigate to http://_&lt;vhostdomain&gt;_/ without too much mucking around with apache2 configs themselves.



OPTIONS
-------
*  __-a, --add__ _&lt;vhostdomain&gt;_
Creates an apache2 vhost configuration file using the current working directory as document_root for _&lt;vhostdomain&gt;_ in __HTTPD_ROOT__/sites-available/; then symlinks the file to __HTTPD_ROOT__/sites-enabled/ and adds an entry to to /etc/hosts as if 
```bash
apache2-vhost --link <vhostdomain>
```
 was called

*  __-h, --help__
Outputs this help text

*  __-l, --list__
Lists all files with the file extension *.vhost.conf in __HTTPD_ROOT__/sites-available/

*  __-p, --purge__ _&lt;vhostdomain&gt;_
Removes the associated _&lt;vhostdomain&gt;_ file from __HTTPD_ROOT__/sites-available/; then removes the associated link from __HTTPD_ROOT__/sites-enabled/ and entry from /etc/hosts as if apache2-vhost --remove _&lt;vhostdomain&gt;_ was called
