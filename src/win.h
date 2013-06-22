#ifndef D3DUT_WIN_H_
#define D3DUT_WIN_H_

#include <vector>
#include <d3d11.h>

#define WINCLASSNAME	"d3dutwindow"

struct Window {
	HWND win;
	int width, height;

	IDXGISwapChain *swap;
	ID3D11RenderTargetView *rtarg_view;

	bool must_redisplay, changed_size;
	int mousex, mousey;

	D3DUT_DisplayFunc display_func;
	D3DUT_ReshapeFunc reshape_func;
	D3DUT_KeyboardFunc keyboard_func;
	D3DUT_KeyboardUpFunc keyboard_up_func;
	D3DUT_SpecialFunc special_func;
	D3DUT_SpecialUpFunc special_up_func;
	D3DUT_MouseFunc mouse_func;
	D3DUT_MotionFunc motion_func;
	D3DUT_PassiveMotionFunc passive_motion_func;
};

extern std::vector<Window*> windows;

int create_window(const char *title, int xsz, int ysz, unsigned int dmflags);
void destroy_window(int idx);

void set_active_win(int idx);
int get_active_win();

Window *get_window(int idx = -1);

long CALLBACK win_handle_event(HWND syswin, unsigned int msg, unsigned int wparam, long lparam);

#endif	// D3DUT_WIN_H_