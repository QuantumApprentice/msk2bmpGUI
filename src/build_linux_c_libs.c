// Enable extensions, mostly because GLFW uses ppoll
// and doesn't do this before they include poll.h
#define _GNU_SOURCE

/***** GLAD *****/

#include "dependencies/GLAD/src/glad.c"

/***** Tiny File Dialogs *****/

#include "dependencies/tinyfiledialogs/tinyfiledialogs.c"

/***** GLFW *****/

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

// we need to rename these functions as the "null" platform is
// always linked by platform.c and it defines some static
// functions that conflict with the ones

#define acquireMonitor acquireMonitorNull
#define releaseMonitor releaseMonitorNull
#define createNativeWindow createNativeWindowNull

#include "dependencies/glfw-3.4/src/null_init.c"
#include "dependencies/glfw-3.4/src/null_joystick.c"
#include "dependencies/glfw-3.4/src/null_monitor.c"
#include "dependencies/glfw-3.4/src/null_window.c"

#undef acquireMonitor
#undef releaseMonitor
#undef createNativeWindow
