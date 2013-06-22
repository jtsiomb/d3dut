/*
D3DUT - Simple window creation and event handling for Direct3D 11 applications.
Copyright (C) 2013  John Tsiombikas <nuclear@member.fsf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef D3DUT_H_
#define D3DUT_H_

#ifdef _MSC_VER
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "winmm.lib")

#ifndef D3DUT_IMPLEMENTATION
#pragma comment(lib, "d3dut.lib")
#endif
#endif

#ifdef D3DUT_IMPLEMENTATION
#define D3DUTAPI	__declspec(dllexport)
#else
#define D3DUTAPI	__declspec(dllimport)
#endif

#include <d3d11.h>
#include <d3dx11.h>

#define D3DUT_RGB			0
#define D3DUT_RGBA			0
#define D3DUT_SINGLE		0

#define D3DUT_DOUBLE		1
#define D3DUT_DEPTH			2
#define D3DUT_STENCIL		4
#define D3DUT_STEREO		8
#define D3DUT_MULTISAMPLE	16

enum {
	D3DUT_WINDOW_WIDTH,
	D3DUT_WINDOW_HEIGHT,
	D3DUT_ELAPSED_TIME
};

enum {
	D3DUT_LEFT_BUTTON,
	D3DUT_MIDDLE_BUTTON,
	D3DUT_RIGHT_BUTTON,

	D3DUT_WHEELDOWN_BUTTON,
	D3DUT_WHEELUP_BUTTON
};

enum {D3DUT_DOWN = 0, D3DUT_UP = 1};

typedef void (*D3DUT_DisplayFunc)();
typedef void (*D3DUT_IdleFunc)();
typedef void (*D3DUT_ReshapeFunc)(int, int);
typedef void (*D3DUT_KeyboardFunc)(unsigned char, int, int);
typedef void (*D3DUT_KeyboardUpFunc)(unsigned char, int, int);
typedef void (*D3DUT_SpecialFunc)(int, int, int);
typedef void (*D3DUT_SpecialUpFunc)(int, int, int);
typedef void (*D3DUT_MouseFunc)(int, int, int, int);
typedef void (*D3DUT_MotionFunc)(int, int);
typedef void (*D3DUT_PassiveMotionFunc)(int, int);

extern D3DUTAPI ID3D11Device *d3dut_dev;
extern D3DUTAPI ID3D11DeviceContext *d3dut_ctx;
extern D3DUTAPI ID3D11RenderTargetView *d3dut_rtview;

void D3DUTAPI d3dut_init(int *argc, char **argv);
void D3DUTAPI d3dut_init_display_mode(unsigned int dmflags);
void D3DUTAPI d3dut_init_window_size(int xsz, int ysz);

int D3DUTAPI d3dut_create_window(const char *title);
void D3DUTAPI d3dut_destroy_window(int win);
void D3DUTAPI d3dut_set_window(int idx);
int D3DUTAPI d3dut_get_window();

void D3DUTAPI d3dut_display_func(D3DUT_DisplayFunc func);
void D3DUTAPI d3dut_idle_func(D3DUT_IdleFunc func);
void D3DUTAPI d3dut_reshape_func(D3DUT_ReshapeFunc func);
void D3DUTAPI d3dut_keyboard_func(D3DUT_KeyboardFunc func);
void D3DUTAPI d3dut_keyboard_up_func(D3DUT_KeyboardUpFunc func);
void D3DUTAPI d3dut_special_func(D3DUT_SpecialFunc func);
void D3DUTAPI d3dut_special_up_func(D3DUT_SpecialUpFunc func);
void D3DUTAPI d3dut_mouse_func(D3DUT_MouseFunc func);
void D3DUTAPI d3dut_motion_func(D3DUT_MotionFunc func);
void D3DUTAPI d3dut_passive_motion_func(D3DUT_PassiveMotionFunc func);

void D3DUTAPI d3dut_post_redisplay();
void D3DUTAPI d3dut_swap_buffers();

void D3DUTAPI d3dut_main_loop();

int D3DUTAPI d3dut_get(unsigned int what);

void D3DUTAPI d3dut_solid_sphere(double radius, int slices, int stacks);
// TODO ... more stuff

#endif	// D3DUT_H_
