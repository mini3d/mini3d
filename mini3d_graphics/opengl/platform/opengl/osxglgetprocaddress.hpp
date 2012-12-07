
// Copyright(c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef __APPLE__

#ifndef MINI3D_GRAPHICS_OSX_GETPROCADDRESS_H
#define MINI3D_GRAPHICS_OSX_GETPROCADDRESS_H

#import <mach-o/dyld.h>
#import <stdlib.h>
#import <string.h>



void * NSGLGetProcAddress (const char *name)
{
    static const struct mach_header* image = NULL;
    NSSymbol symbol;
    char* symbolName;
    if (NULL == image)
    {
        image = NSAddImage("/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL", NSADDIMAGE_OPTION_RETURN_ON_ERROR);
    }
    /* prepend a '_' for the Unix C symbol mangling convention */
    symbolName = (char*)malloc(strlen((const char*)name) + 2);
    strcpy(symbolName+1, (const char*)name);
    symbolName[0] = '_';
    symbol = NULL;
    /* if (NSIsSymbolNameDefined(symbolName))
	 symbol = NSLookupAndBindSymbol(symbolName); */
    symbol = image ? NSLookupSymbolInImage(image, symbolName, NSLOOKUPSYMBOLINIMAGE_OPTION_BIND | NSLOOKUPSYMBOLINIMAGE_OPTION_RETURN_ON_ERROR) : NULL;
    free(symbolName);
    return symbol ? NSAddressOfSymbol(symbol) : NULL;
}

#endif
#endif