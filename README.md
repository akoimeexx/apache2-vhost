apache2-vhost
=============

NAME
----
apache2-vhost - Apache2 vhost configuration manager

SYNOPSIS
--------
```bash
apache2-vhost -[aprs] <vhostdomain>, apache2-vhost -[hlv]
```

DESCRIPTION
-----------
apache2-vhost is a command line program to assist in adding and removing apache2 virtual hosts to and from the system. It completes this task by utilizing the creation and removal of regular and symlinked files in the HTTPD_ROOT/sites-available/ and HTTPD_ROOT/sites-enabled/ directories respectively, and managing _&lt;vhostdomain&gt;_ entries in the system's hosts file. It uses the current working directory for the virtual host's document_root.

This program was written primarily for web developers that need to host several web projects on one system when developing and testing. It should allow a web developer to navigate to http://&lt;vhostdomain&gt;_/ without too much mucking around with apache2 configs themselves.
