#!/usr/bin/env bash

##
# The original bash function I was writing that inspired apache2-vhost.
# Do not actually run this script, it's unfinished and should be buried in soft 
# peat for three months then recycled as firelighters. I've intentionally added 
# an exit to the beginning of the function to disable it.
# 
# Exit codes are used as defined by http://tldp.org/LDP/abs/html/exitcodes.html 
# and C/C++ (/usr/include/sysexits.h)
# 
#        1         2         3         4         5         6         7         8
# 345678901234567890123456789012345678901234567890123456789012345678901234567890
##


# Custom localhost manipulation function. Adds, lists, and removes custom 
# apache localhost entries
function localhost () {
	echo "FUNCTION DISABLED\n"; exit 1;
	
	
	
	# Get apache2 location, and exit with code EX_OSFILE if it's not available.
	httpdroot=$(apache2 -V | grep "HTTPD_ROOT" | cut -d'=' -f2 | tr -d '"')
	if [[ "$httpdroot" = "" ]]
		then exit 72
	fi
	case "$1" in
		add) (
			if [ ! -f "$httpdroot/sites-available/$2.autogen.conf" -a ! -h "$httpdroot/sites-enabled/$2.autogen.conf" ]
				then {
					# Check to see if a valid port number is passed in, otherwise use port 80
					case $3 in
						''|*[!0-9]*) portnum=80;;
						*) echo portnum=$3;;
					esac
					read -d '' CONFDOC <<_NOWDOC_
<VirtualHost *:$portnum>
	DocumentRoot "$(pwd)"
	ServerName $2
	# This should be omitted in the production environment
	SetEnv APPLICATION_ENV development
	<Directory "$(pwd)">
		Options Indexes MultiViews FollowSymLinks
		AllowOverride All
		Order allow,deny
		Allow from all
	</Directory>
</VirtualHost>
_NOWDOC_
					echo "$CONFDOC" | sudo tee "$httpdroot/sites-available/$2.autogen.conf" > /dev/null || exit 73
					sudo ln -s "$httpdroot/sites-available/$2.autogen.conf" "$httpdroot/sites-enabled/$2.autogen.conf" || exit 73
					hostscheck=$(grep -iox "127.0.0.1	$2" /etc/hosts)
					if [[ "$hostscheck" = "" ]]
						#sed -i -e 's/abc/XYZ/g' /tmp/file.txt
						then echo "No entry in /etc/hosts"
					fi
				}
			elif [ -f "$httpdroot/sites-available/$2.autogen.conf" -a ! -h "$httpdroot/sites-enabled/$2.autogen.conf" ]
				then sudo ln -s "$httpdroot/sites-available/$2.autogen.conf" "$httpdroot/sites-enabled/$2.autogen.conf" || exit 73
			else
				echo "Unable to add $2.autogen.conf to $httpdroot/sites-* directories. Do they already exist?"
				exit 73
			fi
		);;
		help) (
			echo "Usage: localhost (add|help|list|purge|remove) <domainname> [port:80]"
			echo ""
			echo "Commands:"
			echo "   add <domainname> [port:80] Creates an autogenerated apache configuration"
			echo "                              using the current working directory as" 
			echo "                              document_root for <domainname> in "
			echo "                              $httpdroot/sites-available/ with optional port"
			echo "                              if it doesn't exist; then symlinks the file to"
			echo "                              $httpdroot/sites-enabled/"
			echo "   help                       This help text"
			echo "   list                       Displays existing auto-gen'ed confs"
			echo "   purge <domainname>         Completely removes autogenerated apache"
			echo "                              configuration from $httpdroot/sites-available/"
			echo "                              as well as $httpdroot/sites-enabled/"
			echo "   remove <domainname>        deletes conf from enabled but leaves in available"
			exit 0;
		);;
		list) find "$httpdroot/sites-available/" -name *.autogen.conf;;
		purge) (
			if [[ -f "$httpdroot/sites-available/$2.autogen.conf" ]]
				then {
					localhost remove $2
					sudo rm -i "$httpdroot/sites-available/$2.autogen.conf" || exit 73;
				}
				else
					echo "file not found"
					exit 74;
			fi
		);;
		remove) (
			# Check to make sure it's a symbolic link.
			if [[ -h "$httpdroot/sites-enabled/$2.autogen.conf" ]]
				then {
					sudo rm -i "$httpdroot/sites-enabled/$2.autogen.conf"
					hostscheck=$(grep -iox "127.0.0.1	$2" /etc/hosts)
					if [[ "$hostscheck" != "" ]]
						sudo sed -i -e "s/127.0.0.1	$2//g" /etc/hosts
						then echo "We found an entry in /etc/hosts"
					fi
				}
				else
					echo "file not found"
					exit 74
			fi
		);;
		*) echo 'usage: localhost (add|help|list|purge|remove) [domainname] [port:80]';;
	esac
}
## Localhost Subdomains
#sudo vim /etc/hosts
#127.0.0.1	<SUBDOMAIN>.localhost