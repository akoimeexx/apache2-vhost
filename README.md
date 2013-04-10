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
Removes the associated _&lt;vhostdomain&gt;_ file from __HTTPD_ROOT__/sites-available/; then removes the associated link from __HTTPD_ROOT__/sites-enabled/ and entry from /etc/hosts as if 
```bash
apache2-vhost --remove <vhostdomain>
```
 was called

*  __-r, --remove__ _&lt;vhostdomain&gt;_
Removes the associated _&lt;vhostdomain&gt;_ file from __HTTPD_ROOT__/sites-enabled/ and entry from /etc/hosts

*  __-s, --link__ _&lt;vhostdomain&gt;_
Symlinks the associated _&lt;vhostdomain&gt;_ file from __HTTPD_ROOT__/sites-available/ to __HTTPD_ROOT__/sites-enabled/ and adds an entry to /etc/hosts

*  __-v, --version__
Print the version number and exit


EXAMPLES
--------
```bash
apache2-vhost --add fake.localhost
```
adds a new apache2 vhost file to __HTTPD_ROOT__/sites-available/, then symlinks it to __HTTPD_ROOT__/sites-enabled/ and adds an entry to /etc/hosts

```bash
apache2-vhost --remove fake.localhost
```
removes the fake.localhost symlink from __HTTPD_ROOT__/sites-enabled/, then removes the entry from /etc/hosts. The file located at __HTTPD_ROOT__/sites-available/fake.localhost.vhost.conf is left alone


ENVIRONMENT
-----------
This section describes any environment dependencies that are required for apache2-vhost to properly operate. Generally speaking, the main requirement is that apache2 is installed on the system apache-vhost is executed on.

__HTTPD_ROOT__
Location of apache2's configuration files. apache2-vhost attempts to find the location by forking and calling 
```bash
apache2 -V
```
and comparing the output for the __HTTPD_ROOT__ configuration. If no __HTTPD_ROOT__ configuration can be found, /etc/apache2 is used as a fallback location.


FILES
-----
*  __HTTPD_ROOT/sites-available/__
Directory where generated _&lt;vhostdomain&gt;_.vhost.conf files are stored

*  __HTTPD_ROOT/sites-enabled/__
Directory where _&lt;vhostdomain&gt;_.vhost.conf symlinks are stored

*  __/etc/hosts__
System file to point _&lt;vhostdomain&gt;_ to 127.0.0.1


SEE ALSO
--------
The full documentation for apache2-vhost can be found in the source code. If apache2-vhost has been distributed as a binary only, you can find the source code online at https://github.com/akoimeexx/apache2-vhost


DIAGNOSTICS
-----------
apache2-vhost returns __EXIT_SUCCESS__ (0) on normal operation, or an exit code as defined in /usr/include/sysexits.h on failure, along with a human-readable error description on stderr.

It should be noted that this program will exit out on a file operation error if the user running it does not have correct permissions. This is by design; if you don't have sudo access you probably shouldn't be mucking about with /etc/hosts and the like.


BUGS
----
No known bugs, please report any bugs found to the author.


AUTHOR
------
Johnathan McKnight


COPYRIGHT
---------
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>. 
