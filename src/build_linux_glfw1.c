// for some reason posix_thread.c doesn't include this which leads to an implicit delcaration warning
#define _GNU_SOURCE
#include <poll.h>

#define _GLFW_X11
#include "dependencies/glfw-3.4/src/context.c"
#include "dependencies/glfw-3.4/src/egl_context.c"
#include "dependencies/glfw-3.4/src/glx_context.c"
#include "dependencies/glfw-3.4/src/init.c"
#include "dependencies/glfw-3.4/src/input.c"
#include "dependencies/glfw-3.4/src/linux_joystick.c"
#include "dependencies/glfw-3.4/src/monitor.c"
#include "dependencies/glfw-3.4/src/osmesa_context.c"
#include "dependencies/glfw-3.4/src/platform.c"
#include "dependencies/glfw-3.4/src/posix_module.c"
#include "dependencies/glfw-3.4/src/posix_poll.c"
#include "dependencies/glfw-3.4/src/posix_thread.c"
#include "dependencies/glfw-3.4/src/posix_time.c"
#include "dependencies/glfw-3.4/src/vulkan.c"
#include "dependencies/glfw-3.4/src/window.c"
#include "dependencies/glfw-3.4/src/wl_init.c"
#include "dependencies/glfw-3.4/src/wl_monitor.c"
#include "dependencies/glfw-3.4/src/wl_window.c"
#include "dependencies/glfw-3.4/src/x11_init.c"
#include "dependencies/glfw-3.4/src/x11_monitor.c"
#include "dependencies/glfw-3.4/src/x11_window.c"
#include "dependencies/glfw-3.4/src/xkb_unicode.c"
