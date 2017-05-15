// aliasToSymlink.c
/*

Add opts:

 -c, --check-only   Perform check if file is an alias.                              DONE 
                    Print alias target and return 1 if true. 
                    If false return 0 do not print anything.

 -r, --recursive    Recursively check for aliases in subfolders.                    DONE
                    Not active by default.

 -d, --delete-alias Delete alias after symlink was created
 
 -n, --name         Symlink name. %s stands for alias old name.
 "%s.symlink"       Syntax of `sprintf` is used.
                    If symlink name is equal "%s" and `-d` flag is not set, 
                    error will be shown.
                    
 --verbose          Print all output                                                DONE
 --brief            Print only important output, default behaviour


Workflow:
 - if (-c) us set, getTrueName of the file and exit
 - check all files in current folder, include subfolders if (-r) set
 - get alias target
 - convert absolute path to relative path
 - create symlink
 - delete alias (-d)
 - rename symlink (-n)

*/

#include <Carbon/Carbon.h> 
#include <dirent.h>
#include <getopt.h>
#define MAX_PATH_SIZE 1024

#define VERBOSE_PRINTF(a) if (opt_verbose_flag) {printf("<verbose> "); printf a;}


void listdir(char *name, int level, char * rootDirName, char * dirPath);
int getTrueName(UInt8 *fileName, UInt8 * targetPath);
int createSymlink(char * aliasName, UInt8 * targetPath, char * rootDirName, int level);

//flags
static int          opt_verbose_flag;
static int          opt_recursive_flag;
static int          opt_delete_flag;
static char         opt_symlink_name[256] = "%s.symlink";
static char*        opt_check_only_file;
static char*        opt_work_folder;

int main ( int argc, char * argv[] ) 
  {
        char                rootDirName[MAX_PATH_SIZE];
	UInt8               targetPath[MAX_PATH_SIZE+1];
        char                dirPath[MAX_PATH_SIZE];
        int                 wasAliased;


    static struct option long_options[] =
    {
        {"verbose",     no_argument,    &opt_verbose_flag,  1},
        {"brief",       no_argument,    &opt_verbose_flag,  0},
        {"recursive",   no_argument,    0,      'r'},
        {"delete",      no_argument,    0,      'd'},
        {"check-only",  required_argument,    0,      'c'},
        {"name",        required_argument,    0,      'n'},
        {0, 0, 0, 0}

    };

    int c;

    while(1)
    {
        int option_index = 0;
        c = getopt_long(argc, argv, "rdc:n:", long_options, &option_index);

        if (c == -1)
            break;

        switch (c)
        {
            case 0:
                break;

            case 'r':
                opt_recursive_flag = 1;
                break;

            case 'd':
                opt_delete_flag = 1;
                break;

            case 'c':
                opt_check_only_file = optarg;
                break;

            case 'n':
                strcpy(opt_symlink_name, optarg);
                break;

            default:
                abort();
        }

    }

    if (opt_verbose_flag)
        VERBOSE_PRINTF(("--verbose flag set\n"));

    if (opt_recursive_flag)
        VERBOSE_PRINTF(("--recursive flag set\n"));

    if (opt_delete_flag)
       VERBOSE_PRINTF(("--delete flag set\n"));

    if (opt_check_only_file)
        VERBOSE_PRINTF(("--check-only flag set\n"));

    VERBOSE_PRINTF(("--name=%s\n", opt_symlink_name));

    if (opt_check_only_file) {
	wasAliased = getTrueName((UInt8 *)opt_check_only_file, targetPath);
        if (wasAliased) {
            printf("%s\n", targetPath);
        }
        exit(wasAliased);
    }

    if (argc <= optind) {
        printf("Please specify folder: ./aliasToSymlink --verboase -[rcd] [-n \"\"] [FOLDER]\n");
        exit(255);
    } else {
        opt_work_folder = argv[optind];
        VERBOSE_PRINTF(("opt_work_folder=%s\n\n", opt_work_folder));
    }

    realpath(opt_work_folder, rootDirName);
    VERBOSE_PRINTF(("rootDirName=%s\n\n", rootDirName));

    listdir(opt_work_folder, 0, rootDirName, dirPath);

    exit(0);
  }

int createSymlink(char * aliasName, UInt8 * targetPath, char * rootDirName, int level)
 {
    char targetRelativePath[MAX_PATH_SIZE];
    char targetRelativePathUp[MAX_PATH_SIZE];
    char symlinkName[MAX_PATH_SIZE];
    char buf[MAX_PATH_SIZE];
    int rootDirNameLen;
    int targetPathLen;
    
    char commonDirPath[MAX_PATH_SIZE];
    int commonPathLen;

    sprintf(symlinkName, "%s.symlink", aliasName);

    rootDirNameLen = strlen(rootDirName);
    targetPathLen = strlen((const char *)targetPath);

    VERBOSE_PRINTF(("rootDirName length=%d\n", rootDirNameLen));
    VERBOSE_PRINTF(("targetPath length=%d\n", targetPathLen));
    //targetPath + rootDirNameLen + 1 - to skip trailing slash "/"

    targetRelativePathUp[0] = 0;
 
    if (targetPathLen > rootDirNameLen) {
        strncpy(targetRelativePath, (const char *)targetPath + rootDirNameLen + 1, targetPathLen - rootDirNameLen);
    } else {
        for (int i = 0;i < strlen((const char *)targetPath);i++) {
            if (targetPath[i] != rootDirName[i]) {
                level = level + 1;
                break;
            }
            if (targetPath[i] == '/') {
                level = level - 1;
                commonPathLen = i;
            }
        }
        for (int i = 0;i < strlen(rootDirName);i++) {
            if (rootDirName[i] == '/') {
                level = level + 1;
            }
        }

        VERBOSE_PRINTF(("commotPathLen=%d\n", commonPathLen));
        strncpy(targetRelativePath, (const char *)targetPath + commonPathLen + 1, targetPathLen - commonPathLen);

    }
    for (int i = 0;i < level; i++) {
        strcat(targetRelativePathUp, "../");
    }
    strcat(targetRelativePathUp, targetRelativePath);

    VERBOSE_PRINTF(("targetRelativePath=%s\n", targetRelativePath));
    VERBOSE_PRINTF(("targetRelativePathUp=%s\n", targetRelativePathUp));
    VERBOSE_PRINTF(("\n"));

    printf("%s:\tCreating symlink to %s\n", symlinkName, targetRelativePathUp);
    if( access( symlinkName, F_OK ) != -1 ) {
            printf("symlink `%s` already exists, skipping\n", symlinkName);
            printf("\n");
            return 0;
    } else {
            //symlink(targetRelativePathUp, symlinkName);
            printf("SYMLINK CREATED\n");
            printf("\n");
    }


    return 1;

 }
void listdir(char *name, int level, char * rootDirName, char * dirPath)
{
    DIR *dir;
    struct dirent *entry;
    int wasAliased;
    UInt8               targetPath[MAX_PATH_SIZE+1];
    char fileName[MAX_PATH_SIZE];

    //printf("dirPath: %s\n", dirPath);

    if (!(dir = opendir(name)))
        return;
    if (!(entry = readdir(dir)))
        return;

    do {
        if (entry->d_type == DT_DIR && opt_recursive_flag) {
            char path[1024];
            int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
            path[len] = 0;
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || entry->d_name[0] == '.')
                continue;
            //printf("%*s[%s]\n", level*2, "", entry->d_name);

            strcat(dirPath, entry->d_name);
            strcat(dirPath, "/");

            listdir(path, level + 1, rootDirName, dirPath);
        }
        else {
            strcpy(fileName, dirPath);
            strcat(fileName, entry->d_name);
	    wasAliased = getTrueName((UInt8 *)fileName, targetPath);
	    if (!wasAliased) {
		targetPath[0] = 0;
	    } else {
		VERBOSE_PRINTF(("%*s- %s alias=%s | %s\n", level*2, "", entry->d_name, wasAliased==1 ? "true" : "false", targetPath));
                createSymlink(fileName, targetPath, rootDirName, level);
	    }
	}
    } while ((entry = readdir(dir)) != NULL);
    closedir(dir);
}

