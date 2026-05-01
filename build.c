#define SKN_BUILD_IMPLEMENTATION
#include "build.h"

int
main()
{
    build_begin();
    {
        Build umai = create_build("umai", "main.c");

        add_include_dir(&umai, "./vendor/raylib-6.0_linux_amd64/include/");
        add_lib(&umai, "./vendor/raylib-6.0_linux_amd64/lib/libraylib.a");
        add_system_lib(&umai, "m");
        add_system_lib(&umai, "X11");

        if (compile_build(umai))
        {
            run_build(umai);
        }
    }
    build_end();
    return 0;
}
