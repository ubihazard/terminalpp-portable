#pragma once
#if (defined ARCH_UNIX && defined RENDERER_NATIVE)

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xos.h>
#include <X11/Xatom.h>
#include <X11/Xft/Xft.h>
#include <fontconfig/fontconfig.h>

#undef None
#undef RootWindow
#undef index

namespace x11 {
	static constexpr long None = 0;

	typedef ::Window Window;
} // namespace X11

#endif
