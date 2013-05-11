/**      1         2         3         4         5         6         7         8
 * 45678901234567890123456789012345678901234567890123456789012345678901234567890
 * 
 * vhost: Apache2 virtualhost management utility, v.0.1.3
 *    Johnathan Graham McKnight <akoimeexx@gmail.com>
 * 
 * Exit codes are used as defined by http://tldp.org/LDP/abs/html/exitcodes.html 
 * and C/C++ (/usr/include/sysexits.h)
 * 
 * compile: gcc -O3 -std=c99 -Wall main.c -o vhost
 * 
 *
 * This program was written to do one thing (manage virtual host setup), and do
 * it well. It is written to be small, portable, and easily modifiable through
 * editing its source code. Please read any comment blocks, as they are written
 * for your benefit.
 *
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
#define AUTHOR "Johnathan Graham McKnight <akoimeexx@gmail.com>"
#define VERSION "0.1.3"


/* Standard system header includes */
#include <stdio.h>
#include <stdlib.h>
/* Argument header magic for variadic functions */
#include <stdarg.h>
/* Unix-like/POSIX-compliant functions, getcwd, symlink, among others. */
#include <unistd.h>
/* Directory access */
#include <dirent.h>
/* Error reporting */ //INFO: <asm-generic/errno.h>: good human-readable strings
#include <errno.h>
/* String manipulation: strncmp, strncpy */
#include <string.h>

/* Command line option parsing made easy */
// #include <getopt.h>

/* Extended exit codes for more verbose exit conditions */
#include <sysexits.h>
/* Linux OS defines describing FS limitations, etc */
#include <linux/limits.h>


/**
 * Command line argument parsing structs
 *    Commands are named program actions that can only be called one at a time.
 *    They do not get shortened, and generally do not require a file input after
 *    the command. They may however accept a string, either from argument
 *    assignment or STDIN.
 *    
 *    Options are behaviour-modifying variables that change how a command
 *    performs. They may take any form of input from assignment only, if the
 *    option requires an argument.
 */
#define no_argument 0
#define required_argument 1
#define optional_argument 2

struct command {
    const char *name;
    int (*fn)(int, const char **, const char *);
    int optcount;
};

struct option {
    const char *name;
    int has_arg;
    int *flag;
    int val;
};

// <Command> functions
int cmd_add() {
    return EXIT_SUCCESS;
}

int cmd_list() {
    return EXIT_SUCCESS;
}

int cmd_remove() {
    return EXIT_SUCCESS;
}




static struct command commands[] = {
    {"add", cmd_add, 'a'}, 
    {"list", cmd_list, '1'}, 
    {"remove", cmd_remove, 'r'},
};

static struct option long_opts[] = {
    {"directory", required_argument, 0, 'd'}, 
    {"help", no_argument, 0, 'h'}, 
    {"output", required_argument, 0, 'o'}, 
    {"purge", no_argument, 0, 'p'}, 
    {"verbose", no_argument, 0, 'V'}, 
    {"version", no_argument, 0, 'v'}, 
    /**
     * Magic numbers to denote array termination. Reference: 
     * http://www.gnu.org/software/libc/manual/html_node/Getopt.html
     */
    {0, 0, 0, 0}
};


/* Static constants we use as option flags/arguments */
static int be_verbose = 0;
static char httpd_root[PATH_MAX] = "/etc/apache2"; // 4096

/* Static string constants that generally won't be changed. */
static const char *error[9] = {
    "vhost: file name `%s'too long: %s\n", 
    "vhost: file path `%s' too long: %s\n", 
    "vhost: cannot create regular file `%s': %s\n", 
    "vhost: failed to remove regular file `%s': %s\n", 
    "vhost: cannot create symbolic link `%s': %s\n", 
    "vhost: failed to remove symbolic link `%s': %s\n", 
    "vhost: failed to access `%s': %s\n", 
    "vhost: cannot open regular file `%s' for reading and writing: %s\n", 
    "vhost: vhost `%s' already assigned in /etc/hosts\n", 
};
static const char *extended_help = 
"Program options:\n"
"  -h, --help              Outputs this help text\n"
"  -v, --version           Print version details and exit\n"
"\n"
"Commands:\n"
"  add <DOMAIN>            \n"
"  list                    \n"
"  remove <DOMAIN>         Remove symlinked <DOMAIN> from /etc/hosts as well as \n"
"                          HTTPD_ROOT/sites-enabled/\n"
"\n"
"Command options:\n"
"  -d, --directory <PATH>  Use <PATH> for document_root instead of current \n"
"                          working directory\n"
"  -o, --output <PATH>     Send output to <PATH> instead of \n"
"                          HTTPD_ROOT/sites-available/<DOMAIN>.vhost.conf\n"
"  -p, --purge             Remove regular file from HTTPD_ROOT/sites-available/\n"
"                          in addition to removing symlink from\n"
"                          HTTPD_ROOT/sites-enabled/\n"
"  -V, --verbose           Print messages as each step is performed\n";
static const char file_extension[11] = ".vhost.conf";
static const char *usage = "Usage: vhost <command> [<arg>] [-dopV], vhost [--help] [--version]\n";
static const char *v_info = "vhost: Apache2 virtualhost management utility, v%s\n    %s\n";
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





/**
 * e_log - output error messages to stderr with fprintf
 */
int e_log(const char *f, ...) {
    // Variable arguments list for printf
    va_list pargs;
    return vfprintf(stderr, f, pargs);
}

/**
 * strncmp_r - Compare strings from the rightmost side to n length
 */
int strncmp_r(const char *s1, const char *s2, int n) {
    // Set maxlength, then re-assign if second is smaller
    int s_maxlen = strlen(s1);
    if(strlen(s2) < strlen(s1)) { s_maxlen = strlen(s2); }
    // If checking length is smaller than maxlength, use that.
    if(n < s_maxlen) { s_maxlen = n; }
    // Truncate strings to maxlength, from the right.
    char sub1[s_maxlen];
    char sub2[s_maxlen];
    strncpy(sub1, &s1[strlen(s1) - s_maxlen], s_maxlen + 1);
    strncpy(sub2, &s2[strlen(s2) - s_maxlen], s_maxlen + 1);
    // Compare the strings
    return strcmp(sub1, sub2);
}




/**
 *  Individual functions for adding, listing, & removing vhost files.
 */
/**
 * Create a *.vhost.conf file for the specified vhost_name in
 * HTTPD_ROOT/sites-available/
 */
int add_vhost(const char *vhost_name, const char *document_root, const char filepath[PATH_MAX]) {
    // Put together the vhost filename
    char output_name[NAME_MAX]; // 255
    int fn_len = snprintf(output_name, sizeof output_name, "%s%s", vhost_name, file_extension);
    // Check to make sure the filename is not too long
    if(fn_len == -1 || fn_len >= NAME_MAX) {
        e_log("vhost: file name `%s'too long: %s\n", output_name, strerror(errno));
        return EX_SOFTWARE; // Exit 70
    }
    // Put together the vhost absolute path
    char output_abspath[PATH_MAX]; // 4096
    int fp_len = snprintf(output_abspath, sizeof output_abspath, "%s/sites-available/%s", filepath, output_name);
    // Check to make sure the absolute path is not too long
    if(fp_len == -1 || fp_len >= PATH_MAX) {
        e_log("vhost: file path `%s' too long: %s\n", output_abspath, strerror(errno));
        return EX_SOFTWARE; // Exit 70
    }
    
    // Open up a new file to save the vhost configuration to.
    FILE *output_file = fopen(output_abspath, "w");
    // Handle not being able to write out to the filepath
    if(output_file == NULL) {
        e_log("vhost: cannot create regular file `%s': %s\n", output_abspath, strerror(errno));
        return EX_SOFTWARE; // Exit 70
    }
    fprintf(output_file, vhost_template, document_root, vhost_name, document_root);
    fclose(output_file);
    
    return EXIT_SUCCESS;
}

/**
 * Symlink the *.vhost.conf file for the specified vhost_name to
 * HTTPD_ROOT/sites-enabled/
 */
int link_vhost(const char *vhost_name, const char filepath[PATH_MAX]) {
    return EXIT_SUCCESS;
}

/**
 * List available and active *.vhost.conf files in HTTPD_ROOT/sites-*
 */
int list_vhost() {
    DIR *dir_vhost_p;
    struct dirent *dir_entry_p;
    
    char vconf_path[PATH_MAX];
    sprintf(vconf_path, "%s/sites-available/", httpd_root);
    if(access(vconf_path, F_OK) == 0) {
        dir_vhost_p = opendir(vconf_path);
        while(dir_entry_p = readdir(dir_vhost_p)) {
            if(strncmp_r(dir_entry_p->d_name, file_extension, sizeof file_extension) == 0) {
                printf("Config: %s\n", dir_entry_p->d_name);
            }
        }
        if(be_verbose) {
            char vsyml_path[PATH_MAX];
            sprintf(vsyml_path, "%s/sites-enabled/", httpd_root);
            dir_vhost_p = opendir(vsyml_path);
            while(dir_entry_p = readdir(dir_vhost_p)) {
                if(strncmp_r(dir_entry_p->d_name, file_extension, sizeof file_extension) == 0) {
                    printf("Symlink: %s\n", dir_entry_p->d_name);
                }
            }
        }
    } else {
        // vhost: failed to access `%s': %s\n"
        e_log(error[6], vconf_path, strerror(errno));
        return EX_SOFTWARE; // Exit 70
    }
    return EXIT_SUCCESS;
}

/**
 * Remove the regular *.vhost.conf file for the specified vhost_name from
 * HTTPD_ROOT/sites-available/
 */
int purge_vhost(const char *vhost_name) {
    return EXIT_SUCCESS;
}

/**
 * Remove the symlinked *.vhost.conf file for the specified vhost_name from
 * HTTPD_ROOT/sites-enabled/
 */
int remove_vhost(const char *vhost_name) {
    return EXIT_SUCCESS;
}




int main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, usage);
        exit(EX_USAGE); // Exit 64
    }
    
    // Set environment flags so we know what we can output
    /*be_verbose = 1;
    for (int i=1; i<argc; i++) {
        printf("Argument #%d: %s\n", i, argv[i]);
    }
    printf("%s", usage);
    printf("\n%s", extended_help);*/
    list_vhost();
    for (int i=0; i < ((sizeof error) - 1); i++) {
        printf(error[i], "string", "Another");
    }
    return EXIT_SUCCESS;
}
