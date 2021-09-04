#define NOBUILD_IMPLEMENTATION
#include "nobuild.h"

#include <stdlib.h>
#include <string.h>

#define CFLAGS "-Wall", "-Wextra", "-pedantic", "-std=c11", "-O0", "-g0"
#define MSVC_CFLAGS "/nologo", "/W3", "/std:c11"

#ifdef _WIN32
#define DEFAULT_CC "cl"
#define DEFAULT_CXX "cl"
#define RUN(executable) CMD(".\\" executable ".exe")
#else
#define DEFAULT_CC "gcc"
#define DEFAULT_CXX "g++"
#define RUN(executable) CMD("./" executable)
#endif // _WIN32

#define SET_COMPILER_EXECUTABLE(env_var, runtime_var, default_executable)      \
    do                                                                         \
    {                                                                          \
        char *c = getenv(env_var);                                             \
        if (c == NULL)                                                         \
            memcpy(runtime_var, default_executable,                            \
                   sizeof(default_executable));                                \
        else                                                                   \
            strcpy(runtime_var, c);                                            \
    } while (0)

char cc[32];

void build()
{
    SET_COMPILER_EXECUTABLE("cc", cc, DEFAULT_CC);
    if (strcmp(cc, "cl") == 0)
    {
        CMD(cc, MSVC_CFLAGS, "gol.c", "/Fe:", "gol");
    }
    else
    {
#ifdef _WIN32
        CMD(cc, CFLAGS, "gol.c", "-o", "gol", "-lGdi32");
#else
        CMD(cc, CFLAGS, "gol.c", "-o", "gol", "-lX11");
#endif // _WIN32
    }
}

int main(int argc, char **argv)
{
    GO_REBUILD_URSELF(argc, argv);

    if (argc > 1)
    {
        if (strcmp(argv[1], "run") == 0)
        {
            build();
            RUN("gol");
            return 0;
        }
    }

    build();
    return 0;
}