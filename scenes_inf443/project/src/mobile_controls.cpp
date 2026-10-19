#include "mobile_controls.hpp"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

bool mobile_controls::is_active() const
{
	return EM_ASM_INT({ return (window.tronMobile && window.tronMobile.active) ? 1 : 0; }) != 0;
}

player_input mobile_controls::read() const
{
	player_input input;
	input.steer = float(EM_ASM_DOUBLE({ return window.tronMobile.steer; }));
	input.throttle = float(EM_ASM_DOUBLE({ return window.tronMobile.throttle; }));
	return input;
}

bool mobile_controls::take_tap()
{
	return EM_ASM_INT({ var n = window.tronMobile.taps; window.tronMobile.taps = 0; return n; }) > 0;
}

int mobile_controls::take_start_request()
{
	return EM_ASM_INT({ var d = window.tronMobile.startRequest; window.tronMobile.startRequest = -1; return d; });
}

bool mobile_controls::take_menu_request()
{
	return EM_ASM_INT({ var m = window.tronMobile.menuRequest; window.tronMobile.menuRequest = false; return m ? 1 : 0; }) != 0;
}

#else

bool mobile_controls::is_active() const { return false; }
player_input mobile_controls::read() const { return player_input(); }
bool mobile_controls::take_tap() { return false; }
int mobile_controls::take_start_request() { return -1; }
bool mobile_controls::take_menu_request() { return false; }

#endif
