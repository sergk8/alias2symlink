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
#define CHECK(rc,check_value) if ((check_value) != noErr) exit((rc))


void listdir(char *name, int level, char * rootDirName);
int getTrueName(char * fileName, UInt8 * targetPath);
int createSymlink(char * aliasName, UInt8 * targetPath, char * rootDirName);

int main ( int argc, char * argv[] ) 
  {
        int wasAliased;
        char *              name1 = "test-folder";
        char *              name2 = "test-folder.symlink";
        char                rootDirName[MAX_PATH_SIZE];
	UInt8               targetPath[MAX_PATH_SIZE+1];
    // if there are no arguments, go away
    if (argc < 2 ) exit(255);

    realpath(argv[1], rootDirName);
    printf("resolved_name=%s\n", rootDirName);

    listdir(".", 0, rootDirName);
    wasAliased = getTrueName(argv[1], targetPath);

    if (wasAliased) {
        symlink(name1, name2);
    }

    exit(wasAliased);
  }

int createSymlink(char * aliasName, UInt8 * targetPath, char * rootDirName)
 {
    char * targetRelativePath[MAX_PATH_SIZE];
    char * symlinkName[MAX_PATH_SIZE];
    char * buf[MAX_PATH_SIZE];
    int rootDirNameLen;
    int targetPathLen;
    
    sprintf(symlinkName, "%s.symlink", aliasName);

    rootDirNameLen = strlen(rootDirName);
    targetPathLen = strlen(targetPath);

    printf(" > rootDirName length=%d\n", rootDirNameLen);
    printf(" > targetPath length=%d\n", targetPathLen);
    //targetPath + rootDirNameLen + 1 - to skip trailing slash "/"
    strncpy(targetRelativePath, targetPath + rootDirNameLen + 1, targetPathLen - rootDirNameLen);
    printf(" > targetRelativePath=%s\n", targetRelativePath);
    
    printf(" > Creating symlink %s to %s\n", symlinkName, targetRelativePath);

 }
void listdir(char *name, int level, char * rootDirName)
{
    DIR *dir;
    struct dirent *entry;
    int wasAliased;
    UInt8               targetPath[MAX_PATH_SIZE+1];

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
            listdir(path, level + 1, rootDirName);
        }
        else {
	    wasAliased = getTrueName(entry->d_name, targetPath);
            printf("--%s\n", entry->d_name);
	    if (!wasAliased) {
		targetPath[0] = 0;
	    } else {
		printf("%*s- %s alias=%s | %s\n", level*2, "", entry->d_name, wasAliased==1 ? "true" : "false", targetPath);
                createSymlink(entry->d_name, targetPath, rootDirName);
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


    CHECK( 255,
      FSPathMakeRef( fileName, &fsRef, NULL ));

    CHECK( 1,
      FSResolveAliasFile( &fsRef, TRUE, &targetIsFolder, &wasAliased));

    CHECK( 255,
      FSRefMakePath( &fsRef, targetPath, MAX_PATH_SIZE)); 

    marker = targetIsFolder ? "/" : "" ;
    //printf( "%s%s\n", targetPath, marker ); 


    return wasAliased;
  }
