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
#include <dxgi.h>
#include "d3dut.h"
#include "win.h"
#include "logmsg.h"

std::vector<Window*> windows;
int active_win = -1;

int create_window(const char *title, int xsz, int ysz, unsigned int dmflags)
{
	IDXGIDevice *dxgidev;
	if(d3dut_dev->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgidev) != 0) {
		fatal_error("failed to get IDXGIDevice interface\n");
		return 0;
	}
	IDXGIAdapter *adapter;
	if(dxgidev->GetParent(__uuidof(IDXGIAdapter), (void**)&adapter) != 0) {
		fatal_error("failed to get IDXGIAdapter pointer\n");
		return 0;
	}
	IDXGIFactory *dxgi_factory;
	if(adapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgi_factory) != 0) {
		fatal_error("failed to get IDXGIFactory pointer\n");
		return 0;
	}
	adapter->Release();
	dxgidev->Release();

	Window *win = new Window;
	memset(win, 0, sizeof *win);
	win->must_redisplay = true;
	win->changed_size = true;
	win->width = xsz;
	win->height = ysz;

	int xpos = (GetSystemMetrics(SM_CXSCREEN) - xsz) / 2;
	int ypos = (GetSystemMetrics(SM_CYSCREEN) - ysz) / 2;
	win->win = CreateWindow(WINCLASSNAME, title, WS_OVERLAPPEDWINDOW, xpos, ypos, xsz, ysz, 0, 0, GetModuleHandle(0), 0);
	if(!win->win) {
		fprintf(stderr, "failed to create window: %s\n", title);
		return 0;
	}
	ShowWindow(win->win, SW_SHOW);

	unsigned int nsamples = 1;
	unsigned int quality = 0;
	if(dmflags & D3DUT_MULTISAMPLE) {
		nsamples = D3D11_MAX_MULTISAMPLE_SAMPLE_COUNT;
		d3dut_dev->CheckMultisampleQualityLevels(DXGI_FORMAT_R8G8B8A8_UNORM, nsamples, &quality);
	}

	DXGI_SWAP_CHAIN_DESC sd;
	memset(&sd, 0, sizeof sd);
	sd.OutputWindow = win->win;
	sd.BufferDesc.Width = xsz;
	sd.BufferDesc.Height = ysz;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	//sd.Stereo = (dmflags & D3DUT_STEREO) ? 1 : 0;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = (dmflags & D3DUT_DOUBLE) ? 1 : 0;
	sd.SampleDesc.Count = nsamples;
	sd.SampleDesc.Quality = quality;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.Windowed = 1;

	if(dxgi_factory->CreateSwapChain(d3dut_dev, &sd, &win->swap) != 0) {
		warning("failed to create swap chain\n");
		return 0;
	}
	dxgi_factory->Release();

	ID3D11Texture2D *rtex;
	if(win->swap->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&rtex) != 0) {
		warning("failed to get default render target texture\n");
		return 0;
	}
	if(d3dut_dev->CreateRenderTargetView(rtex, 0, &win->rtarg_view) != 0) {
		warning("failed to create render target view\n");
		rtex->Release();
		return 0;
	}
	rtex->Release();
	d3dut_ctx->OMSetRenderTargets(1, &win->rtarg_view, 0);

	int idx = (int)windows.size();
	windows.push_back(win);
	set_active_win(idx);
	return idx;
}

void destroy_window(int idx)
{
	if(idx < 0 || idx >= (int)windows.size()) {
		warning("destroy_window: invalid index: %d\n", idx);
		return;
	}

	Window *win = windows[idx];
	if(win) {
		DestroyWindow(win->win);
		win->rtarg_view->Release();
		win->swap->Release();
		delete win;
		windows[idx] = 0;
	}
}

void set_active_win(int idx)
{
	if(idx < 0 || idx >= (int)windows.size()) {
		warning("set_active_win: invalid window: %d\n", idx);
		return;
	}
	d3dut_rtview = windows[idx]->rtarg_view;
	active_win = idx;
}

int get_active_win()
{
	return active_win;
}

Window *get_window(int idx)
{
	if(idx < 0) {
		idx = active_win;
	}
	if(idx < 0 || idx >= (int)windows.size()) {
		return 0;
	}
	return windows[idx];
}

static int find_window(HWND syswin)
{
	for(size_t i=0; i<windows.size(); i++) {
		if(windows[i]->win == syswin) {
			return i;
		}
	}
	return -1;
}

static int count_windows()
{
	int count = 0;
	for(size_t i=0; i<windows.size(); i++) {
		if(windows[i]) {
			count++;
		}
	}
	return count;
}

static void mouse_handler(Window *win, int bn, bool pressed);

long CALLBACK win_handle_event(HWND syswin, unsigned int msg, unsigned int wparam, long lparam)
{
	Window *win = 0;
	int winid = find_window(syswin);
	if(winid != -1) {
		set_active_win(winid);
		win = get_window();
	}

	switch(msg) {
	case WM_PAINT:
		win->must_redisplay = true;
		ValidateRect(win->win, 0);
		break;

	case WM_CLOSE:
		destroy_window(winid);
		if(count_windows() == 0) {
			PostQuitMessage(0);
		}
		break;

	case WM_SIZE:
		win->width = LOWORD(lparam);
		win->height = HIWORD(lparam);
		win->changed_size = true;
		break;

	case WM_KEYDOWN:
		if(wparam < 256) {
			if(win->keyboard_func) {
				win->keyboard_func(wparam, win->mousex, win->mousey);
			}
		} else {
			if(win->special_func) {
				win->special_func(wparam, win->mousex, win->mousey);
			}
		}
		break;

	case WM_KEYUP:
		if(wparam < 256) {
			if(win->keyboard_up_func) {
				win->keyboard_up_func(wparam, win->mousex, win->mousey);
			}
		} else {
			if(win->special_up_func) {
				win->special_up_func(wparam, win->mousex, win->mousey);
			}
		}
		break;

	case WM_MOUSEMOVE:
		win->mousex = LOWORD(lparam);
		win->mousey = HIWORD(lparam);

		if(wparam & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON)) {
			if(win->motion_func) {
				win->motion_func(win->mousex, win->mousey);
			}
		} else {
			if(win->passive_motion_func) {
				win->passive_motion_func(win->mousex, win->mousey);
			}
		}
		break;

	case WM_LBUTTONDOWN:
		mouse_handler(win, D3DUT_LEFT_BUTTON, true);
		break;
	case WM_RBUTTONDOWN:
		mouse_handler(win, D3DUT_RIGHT_BUTTON, true);
		break;
	case WM_MBUTTONDOWN:
		mouse_handler(win, D3DUT_MIDDLE_BUTTON, true);
		break;
	case WM_LBUTTONUP:
		mouse_handler(win, D3DUT_LEFT_BUTTON, false);
		break;
	case WM_RBUTTONUP:
		mouse_handler(win, D3DUT_RIGHT_BUTTON, false);
		break;
	case WM_MBUTTONUP:
		mouse_handler(win, D3DUT_MIDDLE_BUTTON, false);
		break;

	case WM_MOUSEWHEEL:
		{
			int delta = GET_WHEEL_DELTA_WPARAM(wparam);
			mouse_handler(win, delta < 0 ? D3DUT_WHEELDOWN_BUTTON : D3DUT_WHEELUP_BUTTON, true);
		}
		break;

	default:
		return DefWindowProc(syswin, msg, wparam, lparam);
	}

	return 0;
}

static void mouse_handler(Window *win, int bn, bool pressed)
{
	if(win->mouse_func) {
		win->mouse_func(bn, pressed ? D3DUT_DOWN : D3DUT_UP, win->mousex, win->mousey);
	}
}
