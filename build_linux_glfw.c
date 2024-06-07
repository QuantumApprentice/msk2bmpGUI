//this #define is only shown in the man page for poll
#define _GNU_SOURCE
// #include "poll.h"
//define _GLFW_X11 as per instructions on
//https://github.com/glfw/glfw/blob/master/docs/compile.md#compiling-glfw-manually-compile_manual
#define _GLFW_X11

#include "src/dependencies/GLFW/glfw-3.4/src/cocoa_time.c"
#include "src/dependencies/GLFW/glfw-3.4/src/context.c"
#include "src/dependencies/GLFW/glfw-3.4/src/egl_context.c"
#include "src/dependencies/GLFW/glfw-3.4/src/glx_context.c"
#include "src/dependencies/GLFW/glfw-3.4/src/init.c"
#include "src/dependencies/GLFW/glfw-3.4/src/input.c"
#include "src/dependencies/GLFW/glfw-3.4/src/linux_joystick.c"
#include "src/dependencies/GLFW/glfw-3.4/src/monitor.c"
//
// #include "src/dependencies/GLFW/glfw-3.4/src/null_init.c"
// #include "src/dependencies/GLFW/glfw-3.4/src/null_joystick.c"
// #include "src/dependencies/GLFW/glfw-3.4/src/null_monitor.c"
// #include "src/dependencies/GLFW/glfw-3.4/src/null_window.c"
//^^^^^^^
#include "src/dependencies/GLFW/glfw-3.4/src/osmesa_context.c"
#include "src/dependencies/GLFW/glfw-3.4/src/platform.c"
#include "src/dependencies/GLFW/glfw-3.4/src/posix_module.c"
#include "src/dependencies/GLFW/glfw-3.4/src/posix_poll.c"
#include "src/dependencies/GLFW/glfw-3.4/src/posix_thread.c"
#include "src/dependencies/GLFW/glfw-3.4/src/posix_time.c"
#include "src/dependencies/GLFW/glfw-3.4/src/vulkan.c"
//
#include "src/dependencies/GLFW/glfw-3.4/src/wgl_context.c"
#include "src/dependencies/GLFW/glfw-3.4/src/win32_init.c"
#include "src/dependencies/GLFW/glfw-3.4/src/win32_joystick.c"
#include "src/dependencies/GLFW/glfw-3.4/src/win32_module.c"
#include "src/dependencies/GLFW/glfw-3.4/src/win32_monitor.c"
#include "src/dependencies/GLFW/glfw-3.4/src/win32_thread.c"
#include "src/dependencies/GLFW/glfw-3.4/src/win32_time.c"
#include "src/dependencies/GLFW/glfw-3.4/src/win32_window.c"
//^^^^^^^
#include "src/dependencies/GLFW/glfw-3.4/src/window.c"
#include "src/dependencies/GLFW/glfw-3.4/src/wl_init.c"
#include "src/dependencies/GLFW/glfw-3.4/src/wl_monitor.c"
#include "src/dependencies/GLFW/glfw-3.4/src/wl_window.c"
#include "src/dependencies/GLFW/glfw-3.4/src/x11_init.c"
#include "src/dependencies/GLFW/glfw-3.4/src/x11_monitor.c"
#include "src/dependencies/GLFW/glfw-3.4/src/x11_window.c"
#include "src/dependencies/GLFW/glfw-3.4/src/xkb_unicode.c"