#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include "d3dut.h"
#include "win.h"
#include "logmsg.h"

static void d3dut_cleanup();

D3DUTAPI ID3D11Device *d3dut_dev;
D3DUTAPI ID3D11DeviceContext *d3dut_ctx;
D3DUTAPI ID3D11RenderTargetView *d3dut_rtview;

static int init_xsz = 640;
static int init_ysz = 480;
static int init_dmflags = 0;

static long init_time = -1;

static D3DUT_IdleFunc idle_func;

void D3DUTAPI d3dut_init(int *argc, char **argv)
{
	if(init_time >= 0) {
		warning("already initialized!\n");
		return;
	}

	WNDCLASS wclass;
	memset(&wclass, 0, sizeof wclass);
	wclass.hInstance = GetModuleHandle(0);
	wclass.lpfnWndProc = win_handle_event;
	wclass.lpszClassName = WINCLASSNAME;
	wclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wclass.hIcon = LoadIcon(0, IDI_APPLICATION);
	wclass.hCursor = LoadCursor(0, IDC_ARROW);
	wclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	RegisterClass(&wclass);

	// create D3D device
	D3D_FEATURE_LEVEL feature_level[] = {
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	if(D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, 0, 0, feature_level, 3, D3D11_SDK_VERSION,
			&d3dut_dev, 0, &d3dut_ctx) != 0) {
		fatal_error("failed to create D3D11 device\n");
	}
	atexit(d3dut_cleanup);

	init_time = timeGetTime();
}

static void d3dut_cleanup()
{
	for(size_t i=0; i<windows.size(); i++) {
		if(windows[i]) {
			destroy_window(i);
		}
	}
	windows.clear();

	if(d3dut_dev) {
		d3dut_dev->Release();
		d3dut_dev = 0;
	}
	if(d3dut_ctx) {
		d3dut_ctx->Release();
		d3dut_ctx = 0;
	}
	d3dut_rtview = 0;

	UnregisterClass(WINCLASSNAME, GetModuleHandle(0));
	init_time = -1;
}

void D3DUTAPI d3dut_init_display_mode(unsigned int dmflags)
{
	init_dmflags = dmflags;
}

void D3DUTAPI d3dut_init_window_size(int xsz, int ysz)
{
	init_xsz = xsz;
	init_ysz = ysz;
}


int D3DUTAPI d3dut_create_window(const char *title)
{
	return create_window(title, init_xsz, init_ysz, init_dmflags);
}

void D3DUTAPI d3dut_destroy_window(int win)
{
	destroy_window(win);
}

void D3DUTAPI d3dut_set_window(int idx)
{
	set_active_win(idx);
}

int D3DUTAPI d3dut_get_window()
{
	return get_active_win();
}

void D3DUTAPI d3dut_display_func(D3DUT_DisplayFunc func)
{
	Window *win = get_window();
	win->display_func = func;
}

void D3DUTAPI d3dut_idle_func(D3DUT_IdleFunc func)
{
	idle_func = func;
}

void D3DUTAPI d3dut_reshape_func(D3DUT_ReshapeFunc func)
{
	Window *win = get_window();
	win->reshape_func = func;
}

void D3DUTAPI d3dut_keyboard_func(D3DUT_KeyboardFunc func)
{
	Window *win = get_window();
	win->keyboard_func = func;
}

void D3DUTAPI d3dut_keyboard_up_func(D3DUT_KeyboardUpFunc func)
{
	Window *win = get_window();
	win->keyboard_up_func = func;
}

void D3DUTAPI d3dut_special_func(D3DUT_SpecialFunc func)
{
	Window *win = get_window();
	win->special_func = func;
}

void D3DUTAPI d3dut_special_up_func(D3DUT_SpecialUpFunc func)
{
	Window *win = get_window();
	win->special_up_func = func;
}

void D3DUTAPI d3dut_mouse_func(D3DUT_MouseFunc func)
{
	Window *win = get_window();
	win->mouse_func = func;
}

void D3DUTAPI d3dut_motion_func(D3DUT_MotionFunc func)
{
	Window *win = get_window();
	win->motion_func = func;
}

void D3DUTAPI d3dut_passive_motion_func(D3DUT_PassiveMotionFunc func)
{
	Window *win = get_window();
	win->passive_motion_func = func;
}


void D3DUTAPI d3dut_post_redisplay()
{
	Window *win = get_window();
	win->must_redisplay = true;
}

void D3DUTAPI d3dut_swap_buffers()
{
	Window *win = get_window();
	win->swap->Present(0, 0);
}

void D3DUTAPI d3dut_main_loop()
{
	MSG msg;

	for(;;) {
		bool must_redisplay = false;
		for(size_t i=0; i<windows.size(); i++) {
			Window *win = windows[i];
			if(win->changed_size && win->reshape_func) {
				win->changed_size = false;
				set_active_win(i);
				win->reshape_func(win->width, win->height);
			}
			if(win->must_redisplay) {
				must_redisplay = true;
			}
		}

		if(idle_func || must_redisplay) {
			while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				if(msg.message == WM_QUIT) {
					return;
				}
			}

			if(idle_func) { // checking again because a handler might have set this to 0
				idle_func();
			}
		} else {
			if(!GetMessage(&msg, 0, 0, 0)) {
				return;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		for(size_t i=0; i<windows.size(); i++) {
			Window *win = windows[i];
			if(win->must_redisplay && win->display_func) {
				win->must_redisplay = false;
				set_active_win(i);
				win->display_func();
				ValidateRect(win->win, 0);
			}
		}
	}
}


int D3DUTAPI d3dut_get(unsigned int what)
{
	Window *win = get_window();

	switch(what) {
	case D3DUT_WINDOW_WIDTH:
		return win->width;
	case D3DUT_WINDOW_HEIGHT:
		return win->height;

	case D3DUT_ELAPSED_TIME:
		return (long)timeGetTime() - init_time;

	default:
		break;
	}
	return 0;
}


void D3DUTAPI d3dut_solid_sphere(double radius, int slices, int stacks)
{
}
