// aliasToSymlink.c
/*

Add opts:

-c, --check-only    Perform check if file is an alias. 
                    Print alias target and return 1 if true. 
                    If false return 0 do not print anything.

 -r, --recursive    Recursively check for aliases in subfolders.
                    Not active by default.

 -d, --delete-alias Delete alias after symlink was created
 
 -

 -n, --name         Symlink name. %s stands for alias old name.
 "%s.symlink"       Syntax of `sprintf` is used.
                    If symlink name is equal "%s" and `-d` flag is not set, 
                    error will be shown.
                    

Workflow:
 - get alias target
 - convert absolute path to relative path
 - create symlink
 - delete symlink (-d)
 - rename symlink (-n)

*/

#include <Carbon/Carbon.h> 
#include <dirent.h>
#define MAX_PATH_SIZE 1024


void listdir(char *name, int level, char * rootDirName, char * dirPath);
int getTrueName(UInt8 *fileName, UInt8 * targetPath);
int createSymlink(char * aliasName, UInt8 * targetPath, char * rootDirName, int level);

int main ( int argc, char * argv[] ) 
  {
        char                rootDirName[MAX_PATH_SIZE];
	UInt8               targetPath[MAX_PATH_SIZE+1];
        char                dirPath[MAX_PATH_SIZE];
    // if there are no arguments, go away
    if (argc < 2 ) exit(255);

    realpath(argv[1], rootDirName);
    printf("rootDirName=%s\n\n", rootDirName);

    listdir(".", 0, rootDirName, dirPath);

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

    //printf(" > rootDirName length=%d\n", rootDirNameLen);
    //printf(" > targetPath length=%d\n", targetPathLen);
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

        //printf(" > commotPathLen=%d\n", commonPathLen);
        strncpy(targetRelativePath, (const char *)targetPath + commonPathLen + 1, targetPathLen - commonPathLen);

    }
    for (int i = 0;i < level; i++) {
        strcat(targetRelativePathUp, "../");
    }
    strcat(targetRelativePathUp, targetRelativePath);
    //printf(" > targetRelativePath=%s\n", targetRelativePath);
    //printf(" > targetRelativePathUp=%s\n", targetRelativePathUp);
    //printf("\n");
    printf(" > Creating symlink %s to %s\n", symlinkName, targetRelativePathUp);
    if( access( symlinkName, F_OK ) != -1 ) {
            printf(" > symlink `%s` already exists, skipping\n", symlinkName);
            return 0;
    } else {
            symlink(targetRelativePathUp, symlinkName);
            printf(" > SYMLINK CREATED\n");
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
        if (entry->d_type == DT_DIR) {
            char path[1024];
            int len = snprintf(path, sizeof(path)-1, "%s/%s", name, entry->d_name);
            path[len] = 0;
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || entry->d_name[0] == '.')
                continue;
            printf("%*s[%s]\n", level*2, "", entry->d_name);

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
		printf("%*s- %s alias=%s | %s\n", level*2, "", entry->d_name, wasAliased==1 ? "true" : "false", targetPath);
                createSymlink(fileName, targetPath, rootDirName, level);
	    }
	}
    } while ((entry = readdir(dir)) != NULL);
    closedir(dir);
}

