/**      1         2         3         4         5         6         7         8
 * 45678901234567890123456789012345678901234567890123456789012345678901234567890
 * 
 * apache2-vhost vhost management utility, v.0.0.2
 * 	Johnathan McKnight <akoimeexx@gmail.com>
 * 
 * Exit codes are used as defined by http://tldp.org/LDP/abs/html/exitcodes.html 
 * and C/C++ (/usr/include/sysexits.h)
 * 
 * compile: gcc -O3 -std=c99 -Wall main.posix.c -o apache2-vhost
 * 
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#define _POSIX_SOURCE 1
#define _POSIX_C_SOURCE 200112L
#define AUTHOR "Johnathan McKnight <akoimeexx@gmail.com>"
#define VERSION "0.0.2"

/* Standard system header includes */
#include <stdio.h>
#include <stdlib.h>
/* Unix-like/POSIX-compliant functions, getcwd, symlink, among others. */
#include <unistd.h>
/* Error reporting */ //INFO: <asm-generic/errno.h>: good human-readable strings
#include <errno.h>
/* String manipulation: strncmp, strncpy */
#include <string.h>
/* Command line option parsing made easy */
#include <getopt.h>
/* Extended exit codes for more verbose exit conditions */
#include <sysexits.h>
/* Linux OS defines describing FS limitations, etc */
#include <linux/limits.h>


/* Static string constants that generally won't be changed. */
static const char *extended_help = 
"Options:\n"
"  -a, --add <vhostdomain>     Creates an apache2 vhost configuration file using\n"
"                              the current working directory as document_root for\n"
"                              <vhostdomain> in HTTPD_ROOT/sites-available/; then\n"
"                              symlinks the file to HTTPD_ROOT/sites-enabled/ and\n"
"                              adds an entry to to /etc/hosts as if\n"
"                              apache2-vhost --link <vhostdomain> was called\n"
"  -h, --help                  Outputs this help text\n"
"  -l, --list                  Lists all files with the file extension\n"
"                              *%s in HTTPD_ROOT/sites-available/\n"
"  -p, --purge <vhostdomain>   Removes the associated <vhostdomain> file from\n"
"                              HTTPD_ROOT/sites-available/; then removes the\n"
"                              associated link from HTTPD_ROOT/sites-enabled/ and\n"
"                              entry from /etc/hosts as if\n"
"                              apache2-vhost --remove <vhostdomain> was called\n"
"  -r, --remove <vhostdomain>  Removes the associated <vhostdomain> file from\n"
"                              HTTPD_ROOT/sites-enabled/ and entry from\n"
"                              /etc/hosts\n"
"  -s, --link <vhostdomain>    Symlinks the associated <vhostdomain> file from\n"
"                              HTTPD_ROOT/sites-available/ to\n"
"                              HTTPD_ROOT/sites-enabled/ and adds an entry to\n"
"                              /etc/hosts\n"
"  -v, --version               Print the version number and exit\n";
static const char *usage = "Usage: apache2-vhost -[aprs] <vhostdomain>, apache2-vhost -[hlv]\n";
static const char *v_info = "apache2-vhost: Apache2 vhost configuration manager v%s by %s\n";
static const char *vhost_template = 
"<VirtualHost *:80>\n"
"	DocumentRoot \"%s\"\n"
"	ServerName %s\n"
"	# This should be omitted in the production environment\n"
"	SetEnv APPLICATION_ENV development\n"
"	<Directory \"%s\">\n"
"		Options Indexes MultiViews FollowSymLinks\n"
"		AllowOverride All\n"
"		Order allow,deny\n"
"		Allow from all\n"
"	</Directory>\n"
"</VirtualHost>\n";

/* Static strings that can be overridden */
static char file_extension[NAME_MAX] = ".vhost.conf"; // 255
static char httpd_root[PATH_MAX] = "/etc/apache2"; // 4096

/* Command line long option list for use with getopt_long */
static struct option long_opts[] = {
	{"add", required_argument, 0, 'a'}, 
	{"help", no_argument, 0, 'h'}, 
	{"link", required_argument, 0, 's'}, 
	{"list", no_argument, 0, 'l'}, 
	{"purge", required_argument, 0, 'p'}, 
	{"remove", required_argument, 0, 'r'}, 
	{"version", no_argument, 0, 'v'}, 
	/**
	 * Magic numbers to denote array termination. Reference: 
	 * http://www.gnu.org/software/libc/manual/html_node/Getopt.html
	 */
	{0, 0, 0, 0}
};


int main(int argc, char *argv[]) {
	/* Attempt to find HTTPD_ROOT from apache2 -V */
	FILE *apache_pipe;
	apache_pipe = popen("apache2 -V", "r");
	if(apache_pipe == NULL) {
		fprintf(stderr, "apache2-vhost: unable to locate apache2: %s\n", strerror(errno));
		exit(EX_OSFILE); // Exit 72
	} else {
		char buf[255];
		while(fgets(buf, 255, apache_pipe) != NULL) {
			// apache2 was available, check for HTTPD_ROOT flag
			if(strncmp(buf, " -D HTTPD_ROOT=\"", 16) == 0) {
				// Found the flag, extract the location
				strncpy(httpd_root, &buf[16], strlen(buf)-18);
				break;
			}
		}
	}
	pclose(apache_pipe);
	
	
	/* Process our options and act accordingly */
	int c = 0;
	int option_index = 0;
	while(c != -1) {
		c = getopt_long(argc, argv, "a:hlp:r:s:v", long_opts, &option_index);
		switch(c) {
			case 'a':
				if(optarg) {
					// Get the current working directory
					char *cwd = getcwd(0, 0);
					if(!cwd) {
						fprintf(stderr, "apache2-vhost: unable to get current working directory: %s\n", strerror(errno));
						exit(EX_SOFTWARE); // Exit 70
					}
					
					// Put together the vhost filename
					char vhost_name[NAME_MAX]; // 255
					int vhost_fnlen = snprintf(vhost_name, sizeof vhost_name, "%s%s", optarg, file_extension);
					// Check to make sure the filename is not too long
					if(vhost_fnlen == -1 || vhost_fnlen >= NAME_MAX) {
						fprintf(stderr, "apache2-vhost: file name `%s'too long: %s\n", vhost_name, strerror(errno));
						exit(EX_SOFTWARE); // Exit 70
					}
					// Put together the vhost absolute path
					char vhost_absolutepath[PATH_MAX]; // 4096
					int vhost_abslen = snprintf(vhost_absolutepath, sizeof vhost_absolutepath, "%s/sites-available/%s", httpd_root, vhost_name);
					// Check to make sure the absolute path is not too long
					if(vhost_abslen == -1 || vhost_abslen >= PATH_MAX) {
						fprintf(stderr, "apache2-vhost: file path `%s' too long: %s\n", vhost_absolutepath, strerror(errno));
						exit(EX_SOFTWARE); // Exit 70
					}
					
					// Open up a new file to save the vhost configuration to.
					FILE *vhost_file = fopen(vhost_absolutepath, "w");
					// Handle not being able to write out to the filepath
					if(vhost_file == NULL) {
						fprintf(stderr, "apache2-vhost: cannot create regular file `%s': %s\n", vhost_absolutepath, strerror(errno));
						exit(EX_SOFTWARE); // Exit 70
					}
					fprintf(vhost_file, vhost_template, cwd, optarg, cwd);
					fclose(vhost_file);
				} else {
					fprintf(stderr, usage);
					exit(EX_USAGE); // Exit 64
				}
				/* Fall through */
			case 's':
				if(optarg) {
					// Put together the vhost filename
					char vhost_name[NAME_MAX]; // 255
					int vhost_fnlen = snprintf(vhost_name, sizeof vhost_name, "%s%s", optarg, file_extension);
					// Check to make sure the filename is not too long
					if(vhost_fnlen == -1 || vhost_fnlen >= NAME_MAX) {
						fprintf(stderr, "apache2-vhost: file name `%s'too long: %s\n", vhost_name, strerror(errno));
						exit(EX_SOFTWARE); // Exit 70
					}
					
					// Put together the vhost absolute path
					char vhost_absolutepath[PATH_MAX]; // 4096
					int vhost_abslen = snprintf(vhost_absolutepath, sizeof vhost_absolutepath, "%s/sites-available/%s", httpd_root, vhost_name);
					// Check to make sure the absolute path is not too long
					if(vhost_abslen == -1 || vhost_abslen >= PATH_MAX) {
						fprintf(stderr, "apache2-vhost: file path `%s' too long: %s\n", vhost_absolutepath, strerror(errno));
						exit(EX_SOFTWARE); // Exit 70
					}
					// Put together the vhost symlink path
					char vhost_symlinkpath[PATH_MAX]; // 4096
					int vhost_symlen = snprintf(vhost_symlinkpath, sizeof vhost_symlinkpath, "%s/sites-enabled/%s", httpd_root, vhost_name);
					// Check to make sure the symlink path is not too long
					if(vhost_symlen == -1 || vhost_symlen >= PATH_MAX) {
						fprintf(stderr, "apache2-vhost: file path `%s' too long: %s\n", vhost_symlinkpath, strerror(errno));
						exit(EX_SOFTWARE); // Exit 70
					}
					
					// Create the symbolic link
					int vhost_symlink = symlink(vhost_absolutepath, vhost_symlinkpath);
					if(vhost_symlink == -1) {
						fprintf(stderr, "apache2-vhost: failed to create symbolic link `%s': %s\n", vhost_symlinkpath, strerror(errno));
						exit(EX_SOFTWARE); // Exit 70
					}
					//TODO: edit /etc/hosts
					exit(EXIT_SUCCESS); // Exit 0
				} else {
					fprintf(stderr, usage);
					exit(EX_USAGE); // Exit 64
				}
				break;
			case 'h':
				printf(usage);
				printf("\n");
				printf(extended_help, file_extension);
				exit(EXIT_SUCCESS); // Exit 0
				break;
			case 'l':
				//TODO: Display a list of *.vhost.conf files from httpd_root/sites-available/
				printf("TODO: Display a list of *.vhost.conf files from httpd_root/sites-available/");
				exit(EXIT_SUCCESS); // Exit 0
				break;
			case 'p':
				if(optarg) {
					// Put together the vhost filename
					char vhost_name[NAME_MAX]; // 255
					int vhost_fnlen = snprintf(vhost_name, sizeof vhost_name, "%s%s", optarg, file_extension);
					// Check to make sure the filename is not too long
					if(vhost_fnlen == -1 || vhost_fnlen >= NAME_MAX) {
						fprintf(stderr, "apache2-vhost: file name `%s'too long: %s\n", vhost_name, strerror(errno));
						exit(EX_SOFTWARE); // Exit 70
					}
					// Put together the vhost absolute path & symlink path
					char vhost_absolutepath[PATH_MAX]; // 4096
					int vhost_abslen = snprintf(vhost_absolutepath, sizeof vhost_absolutepath, "%s/sites-available/%s", httpd_root, vhost_name);
					// Check to make sure the absolute path is not too long
					if(vhost_abslen == -1 || vhost_abslen >= PATH_MAX) {
						fprintf(stderr, "apache2-vhost: file path `%s' too long: %s\n", vhost_absolutepath, strerror(errno));
						exit(EX_SOFTWARE); // Exit 70
					}
					
					int vhost_unabslink = unlink(vhost_absolutepath);
					if(vhost_unabslink != 0) {
						fprintf(stderr, "apache2-vhost: failed to remove regular file `%s': %s\n", vhost_absolutepath, strerror(errno));
						exit(EX_SOFTWARE); // Exit 70
					}
				} else {
					fprintf(stderr, usage);
					exit(EX_USAGE); // Exit 64
				}
				/* Fall through */
			case 'r':
				if(optarg) {
					// Put together the vhost filename
					char vhost_name[NAME_MAX]; // 255
					int vhost_fnlen = snprintf(vhost_name, sizeof vhost_name, "%s%s", optarg, file_extension);
					// Check to make sure the filename is not too long
					if(vhost_fnlen == -1 || vhost_fnlen >= NAME_MAX) {
						fprintf(stderr, "apache2-vhost: file name `%s'too long: %s\n", vhost_name, strerror(errno));
						exit(EX_SOFTWARE); // Exit 70
					}
					// Put together the vhost absolute path & symlink path
					char vhost_symlinkpath[PATH_MAX]; // 4096
					int vhost_symlen = snprintf(vhost_symlinkpath, sizeof vhost_symlinkpath, "%s/sites-enabled/%s", httpd_root, vhost_name);
					// Check to make sure the symlink path is not too long
					if(vhost_symlen == -1 || vhost_symlen >= PATH_MAX) {
						fprintf(stderr, "apache2-vhost: file path `%s' too long: %s\n", vhost_symlinkpath, strerror(errno));
						exit(EX_SOFTWARE); // Exit 70
					}
					
					int vhost_unsymlink = unlink(vhost_symlinkpath);
					if(vhost_unsymlink != 0) {
						fprintf(stderr, "apache2-vhost: failed to remove symbolic link `%s': %s\n", vhost_symlinkpath, strerror(errno));
						exit(EX_SOFTWARE); // Exit 70
					}
					// TODO: Remove from /etc/hosts
					exit(EXIT_SUCCESS); // Exit 0
				} else {
					fprintf(stderr, usage);
					exit(EX_USAGE); // Exit 64
				}
				break;
			case 'v':
				printf(v_info, VERSION, AUTHOR);
				exit(EXIT_SUCCESS); // Exit 0
				break;
			default:
				fprintf(stderr, usage);
				exit(EX_USAGE); // Exit 64
				break;
		}
	}
	/* If we reach here, we should fail. */
	exit(EXIT_FAILURE);
}