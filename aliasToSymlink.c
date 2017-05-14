// aliasToSymlink.c
// 
// DESCRIPTION
//   Resolve HFS and HFS+ aliased files (and soft links), and return the
//   name of the "Original" or actual file. Directories have a "/"
//   appended. The error number returned is 255 on error, 0 if the file
//   was an alias, or 1 if the argument given was not an alias
// 
// BUILD INSTRUCTIONS
//   gcc-3.3 -o getTrueName -framework Carbon getTrueName.c 
//
//     Note: gcc version 4 reports the following warning
//     warning: pointer targets in passing argument 1 of 'FSPathMakeRef'
//       differ in signedness
//
// COPYRIGHT AND LICENSE
//   Copyright 2005 by Thos Davis. All rights reserved.
//   This program is free software; you can redistribute it and/or
//   modify it under the terms of the GNU General Public License as
//   published by the Free Software Foundation; either version 2 of the
//   License, or (at your option) any later version.
//
//   This program is distributed in the hope that it will be useful, but
//   WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
//   General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free
//   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston,
//   MA 02111-1307 USA


#include <Carbon/Carbon.h> 
#include <dirent.h>
#define MAX_PATH_SIZE 1024
#define CHECK(rc,check_value) if ((check_value) != noErr) return 0


void listdir(char *name, int level, char * rootDirName, char * dirPath);
int getTrueName(char * fileName, UInt8 * targetPath);
int createSymlink(char * aliasName, UInt8 * targetPath, char * rootDirName, int level);

int main ( int argc, char * argv[] ) 
  {
        int wasAliased;
        char *              name1 = "test-folder";
        char *              name2 = "test-folder.symlink";
        char                rootDirName[MAX_PATH_SIZE];
	UInt8               targetPath[MAX_PATH_SIZE+1];
        char *              dirPath[MAX_PATH_SIZE];
    // if there are no arguments, go away
    if (argc < 2 ) exit(255);

    realpath(argv[1], rootDirName);
    printf("rootDirName=%s\n", rootDirName);

    listdir(".", 0, rootDirName, dirPath);
    wasAliased = getTrueName(argv[1], targetPath);

    if (wasAliased) {
        symlink(name1, name2);
    }

    exit(wasAliased);
  }

int createSymlink(char * aliasName, UInt8 * targetPath, char * rootDirName, int level)
 {
    char * targetRelativePath[MAX_PATH_SIZE];
    char * targetRelativePathUp[MAX_PATH_SIZE];
    char * symlinkName[MAX_PATH_SIZE];
    char * buf[MAX_PATH_SIZE];
    int rootDirNameLen;
    int targetPathLen;
    
    char * commonDirPath[MAX_PATH_SIZE];
    int commonPathLen;

    sprintf(symlinkName, "%s.symlink", aliasName);

    rootDirNameLen = strlen(rootDirName);
    targetPathLen = strlen(targetPath);

    //printf(" > rootDirName length=%d\n", rootDirNameLen);
    //printf(" > targetPath length=%d\n", targetPathLen);
    //targetPath + rootDirNameLen + 1 - to skip trailing slash "/"

    targetRelativePathUp[0] = 0;
 
    if (targetPathLen > rootDirNameLen) {
        strncpy(targetRelativePath, targetPath + rootDirNameLen + 1, targetPathLen - rootDirNameLen);
    } else {
        for (int i = 0;i < strlen(targetPath);i++) {
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
        strncpy(targetRelativePath, targetPath + commonPathLen + 1, targetPathLen - commonPathLen);

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
    } else {
            symlink(targetRelativePathUp, symlinkName);
            printf(" > SYMLINK CREATED\n");
    }

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
	    wasAliased = getTrueName(fileName, targetPath);
	    if (!wasAliased) {
		targetPath[0] = 0;
	    } else {
		printf("%*s- %s alias=%s | %s\n", level*2, "", entry->d_name, wasAliased==1 ? "true" : "false", targetPath);
                createSymlink(fileName, targetPath, rootDirName, level);
	    }
	}
    } while (entry = readdir(dir));
    closedir(dir);
}

int getTrueName(char * fileName, UInt8 * targetPath)
  {
    FSRef               fsRef; 
    Boolean             targetIsFolder; 
    Boolean             wasAliased; 
    char *              marker;
    OSErr               returnCode;


    //printf("[%s\n", fileName);
    //returnCode =
    CHECK(255, FSPathMakeRef( fileName, &fsRef, NULL ));
    //printf("]%d\n", returnCode);


    //returnCode =
    CHECK(1, FSResolveAliasFile( &fsRef, TRUE, &targetIsFolder, &wasAliased));
    //printf("]%d\n", returnCode);

    CHECK( 255, FSRefMakePath( &fsRef, targetPath, MAX_PATH_SIZE));

    marker = targetIsFolder ? "/" : "" ;
    //printf( "%s%s\n", targetPath, marker ); 


    return wasAliased;
  }
