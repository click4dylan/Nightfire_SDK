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

FILE* SafeOpenRead(const char* filename)
{
    char buffer[MAX_PATH];

    FILE* fp = fopen(filename, "rb");
    if (!fp)
    {
        GetCurrentDirectoryA(MAX_PATH, buffer);
        Log("Current directory is '%s'\n", buffer);
        Error("Error opening %s (errno %d : %s)", filename, errno, strerror(errno));
    }
    return fp;
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

bool q_exists(const char* const filename)
{
    FILE* f;

    f = fopen(filename, "rb");

    if (!f)
    {
        return false;
    }
    else
    {
        fclose(f);
        return true;
    }
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