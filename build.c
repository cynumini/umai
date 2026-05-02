#define DEBUG
#define SKN_BUILD_IMPLEMENTATION
#include "build.h"

int
main(int argc, char **argv)
{
    begin_build(argc, argv);

    Build umai = create_build("umai", "main.c");
    umai.debug = true;
    umai.warning = true;
    umai.warning_error = true;
    add_include_dir(&umai, "./vendor/raylib-6.0_linux_amd64/include/");
    add_lib(&umai, "./vendor/raylib-6.0_linux_amd64/lib/libraylib.a");
    add_system_lib(&umai, "m");
    add_system_lib(&umai, "X11");

    if (compile_build(umai))
    {
        run_build(umai);
    }

    return 0;
}
