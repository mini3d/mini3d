
// Copyright (c) <2011-2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#ifdef __APPLE__

#import <mach-o/dyld.h>
#import <stdlib.h>
#import <string.h>

namespace mini3d {
namespace graphics {

void * NSglGetProcAddress (const char *name)
{
    NSSymbol symbol;
    char *symbolName;
    symbolName = (char*)malloc (strlen (name) + 2); // 1
    strcpy(symbolName + 1, name); // 2
    symbolName[0] = '_'; // 3
    symbol = NULL;
    if (NSIsSymbolNameDefined (symbolName)) // 4
        symbol = NSLookupAndBindSymbol (symbolName);
    free (symbolName); // 5
    return symbol ? NSAddressOfSymbol (symbol) : NULL; // 6
}

}
}

#endif
