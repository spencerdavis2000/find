
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

/*
* Perform an action designated in 'action' on the file
* given in 'fullpath'.
* If no action is given, a default action of printing out 'fullpath'
* is taken.
*/
void do_action(char * fullpath, char * action, char ** remaining) {
    // available actions
    char * delete = "delete";
    char * cat = "cat";
    char * rm = "rm";
    char * mv = "mv";

    if(action == NULL ) {
        // no action, print full path
        printf("%s\n", fullpath);
    } else if(strcmp(action, delete) == 0 || strcmp(action, rm) == 0) {
        // delete file at full path
        remove(fullpath);
    } else if(strcmp(action, cat) == 0) {
        // perform cat on full path
        char * argv[3] = {cat, fullpath, NULL};
        execvp(cat, argv);
    } else if(strcmp(action, mv) == 0) {
        // perform cat on full path
        // TODO: need to prepend rest of path to the remaining[0] bit
        char * argv[4] = {mv, fullpath, remaining[0], NULL};
        execvp(mv, argv);
    }
}

/*
* Check if the file at 'fulllpath' meets any given criteria.
* return 1 (true) if it does, 0 (false) otherwise.
*/
int meets_criteria(char * fullpath, char * entry_name, char * name, char * mmin, char * inum) {

    int criteria_met = 0;
    struct stat filestats;

    if(name != NULL) {
        // if the entry name matches the critera name
        if(strcmp(entry_name, name) == 0) {
            criteria_met = 1;
        }
    } else if(mmin != NULL) {
        stat(fullpath, &filestats);
        char modifier;
        int seconds;
        int lastmodified;
        // check for '+' or '-' and convert
        // time given in minutes to seconds
        if(!isdigit(mmin[0])) {
            modifier = mmin[0];
            seconds = atoi(++mmin) * 60;
        } else {
            seconds = atoi(mmin) * 60;
        }
        lastmodified = (int)(time(0) - filestats.st_mtime);
        // greater than 
        if(modifier == '+' && lastmodified > seconds) {
            criteria_met = 1;
        // less than
        } else if( modifier == '-' && lastmodified < seconds) {
            criteria_met = 1;
        // equal (exactly)
        } else if(lastmodified == seconds) {
            criteria_met = 1;            
        }
    } else if(inum != NULL) {
        stat(fullpath, &filestats);
        // file's inode number equals input inode number
        if((long)filestats.st_ino == atol(inum)) {
            criteria_met = 1;            
        }
    } else {
        criteria_met = 1;
    }

    return criteria_met;
}


/*
* find files that meet critera if any is given, and perform an action on
* those files if any action is given. 
* Criteria available:
    * default none
    * match file name 
    * file modified greater than, less than, or exactly at specified time
    * file inode matches given inode

* Actions available:
    * default print full path of file/directory
    * delete file
*/
void find(char * where, char * name, char * mmin, char * inum, char * action, char ** remaining)  {
    DIR * dir;// directory stream
    struct dirent * entry;     
    char * e_name; 
    char fullpath[1024];
    char curr_dir[] = ".";
    char pre_dir[] = "..";
    int e_type;    
    int is_dots;  
    int criteria = name != NULL || mmin != NULL || inum != NULL;

    // no default specified, set cwd
    if(where == NULL){where = ".";}

    if (dir = opendir(where)) {
  
        while ((entry = readdir(dir)) != NULL) {
            e_type = entry -> d_type;
            e_name = entry -> d_name;
            is_dots = strcmp(e_name, curr_dir) == 0 || strcmp(e_name, pre_dir) == 0;
            
            // concat for full path
            snprintf(fullpath, sizeof(fullpath),
            "%s/%s", where, e_name);
    
            // if the entry is a sub directory and is not "." or ".."
            if (e_type == DT_DIR && !is_dots) {
                // no criteria given, print path
                if (!criteria) {
                    printf("%s\n", fullpath);                  
                }
                // recursive list sub directory
                find(fullpath, name, mmin, inum, action, remaining);
    
            } else if (!is_dots) {
                // look for criteria regarding the file
                if(meets_criteria(fullpath, e_name, name, mmin, inum)) {
                    // perform some action on file
                    do_action(fullpath, action, remaining);
                }    
            }    
        }
    
        // close directory stream
        closedir(dir);  
    } else {
        // error opening diretory stream
        printf("Could not open directory %s\n", where);
    }
}


int main (int argc, char ** argv)
{
    // valid options given for find
    // set to 1 (true) initially because the user can
    // run ./find without any args.
    int valid = 1;

    // criteria variables
    char *where = NULL;
    char *name = NULL;
    char *mmin = NULL;
    char *inum = NULL;
    // action variables
    char *action = NULL;
    char *data = NULL;
    // sepcified option variable
    char c;

    while ((c = getopt(argc, argv, "w:n:m:i:a:")) != -1) {

        switch (c) {
            case 'w':
                where = optarg;
                printf ("where: %s\n",optarg);
                break;
            case 'n':
                name = optarg;
                printf ("name: %s\n", optarg);
                break;
            case 'm':
                mmin = optarg;
                printf("mmin: %s\n", optarg);
                break;
            case 'i':
                inum = optarg;
                printf("inum: %s\n", optarg);
                break;
            case 'a':
                action = optarg;
                printf("action: %s\n", optarg);
                break;
            case '?':
                // invalid arg
                valid = 0;
                break;
            default:
                // invalid
                valid = 0;
                break;
        }
    }

    // adjust after processing of arguments
    argc -= optind;
    argv += optind;

    if(valid) {
        find(where, name, mmin, inum, action, argv);        
    }

}