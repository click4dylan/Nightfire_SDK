#include "filelib.h"
#include "log.h"

FILE* SafeOpenWrite(const char* const filename)
{
    FILE* f;

    f = fopen(filename, "wb");

    if (!f)
        Error("Error opening %s: %s", filename, strerror(errno));

    return f;
}

FILE* SafeOpenRead(const char* a1)
{
    FILE* v1; // esi
    char* v2; // eax
    CHAR Buffer[260]; // [esp+4h] [ebp-104h] BYREF

    v1 = fopen(a1, "rb");
    if (!v1)
    {
        GetCurrentDirectoryA(0x104u, Buffer);
        Log("Current directory is '%s'\n", Buffer);
        v2 = strerror(errno);
        Error("Error opening %s (errno %d : %s)", a1, errno, v2);
    }
    return v1;
}

void SafeRead(FILE* f, void* buffer, int count)
{
    if (fread(buffer, 1, count, f) != (size_t)count)
        Error("File read failure");
}

void SafeWrite(FILE* f, const void* const buffer, int count)
{
    if (fwrite(buffer, 1, count, f) != (size_t)count)
        Error("File read failure");
}

int q_filelength(FILE* f)
{
    int             pos;
    int             end;

    pos = ftell(f);
    fseek(f, 0, SEEK_END);
    end = ftell(f);
    fseek(f, pos, SEEK_SET);

    return end;
}

int LoadFile(const char* name, void** bufferptr)
{
    FILE* v2; // edi
    int length; // esi
    void* v4; // ebx

    v2 = SafeOpenRead(name);
    length = q_filelength(v2);
    v4 = calloc(1, length + 1);
    SafeRead(v2, v4, length);
    fclose(v2);
    *bufferptr = v4;
    return length;
}