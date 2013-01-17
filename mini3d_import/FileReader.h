
// Copyright (c) <2012> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

#if 0

#ifndef MINI3D_IMPORT_FILE_READER_H
#define MINI3D_IMPORT_FILE_READER_H

void mini3d_assert(bool expression, const char* text, ...);

namespace mini3d {
namespace import {

class File
{
public:

    enum AccessMode { READ = 0, WRITE = 1 };

    File(const char* filename, AccessMode mode);
    ~File();

    long Read(void* target, long count);
    long Write(const void* source, long count);
    long GetLength();
    long GetOffset();
    void SetOffset(long offset);

private:
    struct Internal;
    Internal* m_pI;
};

}
}


#ifdef __ANDROID__

////////// ANDROID IMPLEMENTATION //////////////////////////////////////////////////////////////

#include <cstdio>
using namespace mini3d::import;

struct File::Internal { FILE* file; AccessMode mode; };
const char* modeMatrix[] = {"rb", "wb"};

File::File(const char* filename, AccessMode mode)   { m_pI = new Internal(); m_pI->mode = mode; mini3d_assert(m_pI->file = fopen(filename, modeMatrix[mode]), "Unable to open file %s!", filename); } 
File::~File()                                       { fclose(m_pI->file); }
long File::Read(void* target, long count)           { return (long)fread(target, sizeof(char), count, m_pI->file); }
long File::Write(const void* source, long count)    { return (long)fwrite(source, sizeof(char), count, m_pI->file); }
long File::GetLength()                              { long pos = GetOffset(); fseek (m_pI->file, 0, SEEK_END); SetOffset(pos); }
long File::GetOffset()                              { return ftell(m_pI->file); }
void File::SetOffset(long offset)                   { fseek(m_pI->file, offset, SEEK_SET); }


#else

////////// DEFAULT IMPLEMENTATION //////////////////////////////////////////////////////////////

#include <cstdio>
using namespace mini3d::import;

struct File::Internal { FILE* file; AccessMode mode; };
const char* modeMatrix[] = {"rb", "wb"};

File::File(const char* filename, AccessMode mode)   { m_pI = new Internal(); m_pI->mode = mode; mini3d_assert(m_pI->file = fopen(filename, modeMatrix[mode]), "Unable to open file %s!", filename); } 
File::~File()                                       { fclose(m_pI->file); }
long File::Read(void* target, long count)           { return (long)fread(target, sizeof(char), count, m_pI->file); }
long File::Write(const void* source, long count)    { return (long)fwrite(source, sizeof(char), count, m_pI->file); }
long File::GetLength()                              { long pos = GetOffset(); fseek (m_pI->file, 0, SEEK_END); SetOffset(pos); }
long File::GetOffset()                              { return ftell(m_pI->file); }
void File::SetOffset(long offset)                   { fseek(m_pI->file, offset, SEEK_SET); }

#endif


#endif
#endif