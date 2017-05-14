// getTrueName.c 
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

int getTrueName(UInt8 * fileName, UInt8 * targetPath);

int getTrueName(UInt8 * fileName, UInt8 * targetPath)
  {
    FSRef               fsRef; 
    Boolean             targetIsFolder; 
    Boolean             wasAliased; 
    char *              marker;
    OSErr               returnCode;


    CHECK(255, FSPathMakeRef( fileName, &fsRef, NULL ));

    CHECK(1, FSResolveAliasFile( &fsRef, TRUE, &targetIsFolder, &wasAliased));

    CHECK( 255, FSRefMakePath( &fsRef, targetPath, MAX_PATH_SIZE));

    marker = targetIsFolder ? "/" : "" ;

    return wasAliased;
  }
