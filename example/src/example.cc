#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include "d3dut.h"

struct Vertex {
	float pos[3];
	float color[4];
};

struct RenderState {
	float modelview[16];
	float projection[16];
};

static bool init();
static void cleanup();
static void display();
static void reshape(int x, int y);
static void keyb(unsigned char key, int x, int y);

static int width, height;

static ID3D11InputLayout *vertex_layout;
static ID3D11VertexShader *vsdr;
static ID3D11PixelShader *psdr;
static ID3D11Buffer *vbuf;
static ID3D11Buffer *rstate_buf;

static RenderState rstate;

int main(int argc, char **argv)
{
	d3dut_init(&argc, argv);
	d3dut_init_window_size(800, 600);
	d3dut_init_display_mode(D3DUT_RGB | D3DUT_DEPTH | D3DUT_DOUBLE);
	d3dut_create_window("d3dut example");

	d3dut_display_func(display);
	d3dut_idle_func(d3dut_post_redisplay);
	d3dut_reshape_func(reshape);
	d3dut_keyboard_func(keyb);

	if(!init()) {
		return 1;
	}
	atexit(cleanup);

	d3dut_main_loop();
	return 0;
}

static bool init()
{
	unsigned int sdrflags = 0;//D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG;

	ID3DBlob *vsbuf, *psbuf, *msgblob = 0;
	if(D3DX11CompileFromFile("shader.hlsl", 0, 0, "vertex_main", "vs_4_0", sdrflags, 0, 0, &vsbuf, &msgblob, 0) != 0) {
		fprintf(stderr, "failed to load vertex shader\n");
		if(msgblob && msgblob->GetBufferSize() > 0) {
			fprintf(stderr, "Vertex Shader:\n%s\n", (char*)msgblob->GetBufferPointer());
		}
		return false;
	}
	if(D3DX11CompileFromFile("shader.hlsl", 0, 0, "pixel_main", "ps_4_0", sdrflags, 0, 0, &psbuf, &msgblob, 0) != 0) {
		fprintf(stderr, "failed to load pixel shader\n");
		if(msgblob && msgblob->GetBufferSize() > 0) {
			fprintf(stderr, "Pixel Shader:\n%s\n", (char*)msgblob->GetBufferPointer());
		}
		return false;
	}

	if(d3dut_dev->CreateVertexShader(vsbuf->GetBufferPointer(), vsbuf->GetBufferSize(), 0, &vsdr) != 0) {
		fprintf(stderr, "failed to create vertex shader\n");
		return false;
	}
	if(d3dut_dev->CreatePixelShader(psbuf->GetBufferPointer(), psbuf->GetBufferSize(), 0, &psdr) != 0) {
		fprintf(stderr, "failed to create pixel shader\n");
		return false;
	}

	D3D11_INPUT_ELEMENT_DESC elem_desc[2];
	elem_desc[0].SemanticName = "position";
	elem_desc[0].SemanticIndex = 0;
	elem_desc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	elem_desc[0].InputSlot = 0;
	elem_desc[0].AlignedByteOffset = 0;
	elem_desc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	elem_desc[0].InstanceDataStepRate = 0;

	elem_desc[1].SemanticName = "color";
	elem_desc[1].SemanticIndex = 0;
	elem_desc[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	elem_desc[1].InputSlot = 0;
	elem_desc[1].AlignedByteOffset = offsetof(Vertex, color);
	elem_desc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	elem_desc[1].InstanceDataStepRate = 0;

	if(d3dut_dev->CreateInputLayout(elem_desc, 2, vsbuf->GetBufferPointer(), vsbuf->GetBufferSize(), &vertex_layout) != 0) {
		fprintf(stderr, "failed to create vertex layout\n");
		return 0;
	}
	vsbuf->Release();
	psbuf->Release();

	// --- create vertex buffer ---
	Vertex varr[] = {
		{{-0.6, -0.4, 0}, {1, 0, 0, 1}},
		{{0.0, 0.6, 0}, {0, 1, 0, 1}},
		{{0.6, -0.4, 0}, {0, 0, 1, 1}}
	};

	D3D11_BUFFER_DESC buf_desc;
	memset(&buf_desc, 0, sizeof buf_desc);
	buf_desc.Usage = D3D11_USAGE_DEFAULT;
	buf_desc.ByteWidth = sizeof varr;
	buf_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA subdata;
	memset(&subdata, 0, sizeof subdata);
	subdata.pSysMem = varr;
	if(d3dut_dev->CreateBuffer(&buf_desc, &subdata, &vbuf) != 0) {
		fprintf(stderr, "failed to create vertex buffer\n");
		return false;
	}

	// render state buffer
	memset(&buf_desc, 0, sizeof buf_desc);
	buf_desc.Usage = D3D11_USAGE_DEFAULT;
	buf_desc.ByteWidth = sizeof(RenderState);
	buf_desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	memset(&subdata, 0, sizeof subdata);
	subdata.pSysMem = &rstate;
	if(d3dut_dev->CreateBuffer(&buf_desc, &subdata, &rstate_buf) != 0) {
		fprintf(stderr, "failed to create render state buffer\n");
		return false;
	}

	return true;
}

static void cleanup()
{
	vbuf->Release();
	rstate_buf->Release();
	vsdr->Release();
	psdr->Release();
	vertex_layout->Release();
}

static void set_identity(float *mat)
{
	mat[0] = mat[5] = mat[10] = mat[15] = 1.0;
	mat[1] = mat[2] = mat[3] = mat[4] = mat[6] = mat[7] = mat[8] = mat[9] = mat[11] = mat[12] = mat[13] = mat[14] = 0.0;
}

static void set_rotation_z(float *mat, float angle)
{
	set_identity(mat);

	mat[0] = cos(angle);
	mat[1] = -sin(angle);
	mat[4] = sin(angle);
	mat[5] = cos(angle);
}

static void set_ortho(float *mat, float aspect)
{
	set_identity(mat);
	mat[0] = 1.0 / aspect;
}

static void display()
{
	unsigned int msec = timeGetTime();

	float fbcolor[] = {0.2f, 0.2f, 0.2f, 1.0f};
	d3dut_ctx->ClearRenderTargetView(d3dut_rtview, fbcolor);

	// set render state constant buffer data
	set_ortho(rstate.projection, (float)width / (float)height);
	set_rotation_z(rstate.modelview, msec / 1000.0);

	d3dut_ctx->UpdateSubresource(rstate_buf, 0, 0, &rstate, 0, 0);
	d3dut_ctx->VSSetConstantBuffers(0, 1, &rstate_buf);


	unsigned int stride = sizeof(Vertex);
	unsigned int offset = 0;
	d3dut_ctx->IASetVertexBuffers(0, 1, &vbuf, &stride, &offset);
	d3dut_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	d3dut_ctx->IASetInputLayout(vertex_layout);

	d3dut_ctx->VSSetShader(vsdr, 0, 0);
	d3dut_ctx->PSSetShader(psdr, 0, 0);

	d3dut_ctx->Draw(3, 0);

	d3dut_swap_buffers();
}

static void reshape(int x, int y)
{
	width = x;
	height = y;

	D3D11_VIEWPORT vp;
	vp.Width = (float)x;
	vp.Height = (float)y;
	vp.MinDepth = 0;
	vp.MaxDepth = 1;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;
	d3dut_ctx->RSSetViewports(1, &vp);

	// TODO probably we also need to resize render targets or whatever...
}

static void keyb(unsigned char key, int x, int y)
{
	switch(key) {
	case 27:
		exit(0);

	case ' ':
		{
			static bool anim = true;
			anim = !anim;
			d3dut_idle_func(anim ? d3dut_post_redisplay : 0);
		}
		break;
	}
}