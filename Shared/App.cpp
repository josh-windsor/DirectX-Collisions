#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>
#include <d3d11.h>
#include <d3dx11.h>
#include <D3Dcompiler.h>
#include <DxErr.h>

// this is for D3DX maths, which D3D11 has replaced with the more
// efficient but rather inconvenient-sounding XNA Maths.
#include <D3DX10math.h>

#include "App.h"

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "D3DHelpers.h"

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

App::App():
m_pD3DDevice(NULL),
m_pD3DDebug(NULL),
m_pD3DDeviceContext(NULL),
m_pD3DRenderTargetView(NULL),
m_pD3DDepthStencilView(NULL),
m_softwareD3D(false),
m_canRender(false),
m_hWnd(NULL),
m_pDXGISwapChain(NULL),
m_pD3DDepthStencilBuffer(NULL),
m_renderTargetWidth(0),
m_renderTargetHeight(0),
m_isInFocus(false),
m_pStartErrorMessage(NULL)
{
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

App::~App()
{
	this->StopD3D();

	free(m_pStartErrorMessage);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void App::SetSoftwareD3D(bool softwareD3D)
{
	m_softwareD3D = softwareD3D;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool App::StartD3D(HWND hWnd)
{
	m_hWnd = hWnd;

	UINT flags = 0;
#ifdef _DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif//_DEBUG

	DXGI_SWAP_CHAIN_DESC swapChainDesc;

	// 32-bit RGBA frame buffer, initially the same size as the window's
	// client area.
	swapChainDesc.BufferDesc.Width = 0;
	swapChainDesc.BufferDesc.Height = 0;
	swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// No antialiasing.
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;

	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 1;

	swapChainDesc.OutputWindow = m_hWnd;
	swapChainDesc.Windowed = TRUE;

	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	swapChainDesc.Flags = 0;

	D3D_FEATURE_LEVEL d3dFeatureLevel;

	D3D_DRIVER_TYPE driverType;
	if (m_softwareD3D)
		driverType = D3D_DRIVER_TYPE_REFERENCE;
	else
		driverType = D3D_DRIVER_TYPE_HARDWARE;

	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, driverType, NULL, flags, NULL, 0, D3D11_SDK_VERSION, &swapChainDesc,
		&m_pDXGISwapChain, &m_pD3DDevice, &d3dFeatureLevel, &m_pD3DDeviceContext)))
	{
		this->StopD3D();
		return false;
	}

	m_pD3DDevice->QueryInterface(IID_ID3D11Debug, (void **)&m_pD3DDebug);

	printf("Feature level: %s\n", GetNameD3D_FEATURE_LEVEL(d3dFeatureLevel));
	printf("Got D3D11Debug: %s\n", m_pD3DDebug ? "YES" : "NO");

	RecreateRenderTargetsAndViews();

	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void App::StopD3D()
{
	this->ReleaseRenderTargetsAndViews();

	Release(m_pDXGISwapChain);
	Release(m_pD3DDebug);
	Release(m_pD3DDevice);
	Release(m_pD3DDeviceContext);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool App::Start()
{
	if (!this->HandleStart())
	{
		this->Stop();

		return false;
	}

	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void App::Stop()
{
	this->ClearStateAndFlushDeviceContext();

	this->HandleStop();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

const char *App::GetStartErrorMessage() const
{
	return m_pStartErrorMessage;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void App::Render()
{
	if (!m_canRender)
		return;

	RECT client;
	GetClientRect(m_hWnd, &client);

	if (client.right == 0 || client.bottom == 0)
		return;//probably minimized, but it could be just very, very small.

	// If the render target is the wrong size, fix it.
	if (client.right != m_renderTargetWidth || client.bottom != m_renderTargetHeight)
	{
		if (client.right > 0 && client.bottom > 0)
			this->RecreateRenderTargetsAndViews();
	}

	this->SetDefaultRenderTarget();

	// Do the actual rendering.
	this->HandleRender();

	// Present whatever.
	m_pDXGISwapChain->Present(0, 0);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void App::Update()
{
	this->HandleUpdate();

	// ...anything else?
	//
	// This mainly exists for symmetry with the other three, where
	// it makes more sense.
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool App::CanRender() const
{
	return m_canRender;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void App::ClearStateAndFlushDeviceContext()
{
	if (!m_pD3DDeviceContext)
		return;

	m_pD3DDeviceContext->ClearState();
	m_pD3DDeviceContext->Flush();
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void App::SetWindowTitle(const char *pFmt, ...)
{
	char aBuf[500];

	va_list v;
	va_start(v, pFmt);
	_vsnprintf_s(aBuf, sizeof aBuf, _TRUNCATE, pFmt, v);
	va_end(v);

	SetWindowText(m_hWnd, aBuf);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

bool App::HandleStart()
{
	return true;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void App::HandleStop()
{
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void App::HandleRender()
{
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void App::HandleUpdate()
{
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void App::SetStartErrorMessage(const char *pFmt, ...)
{
	char buf[1000];

	va_list v;

	va_start(v,pFmt);

	_vsnprintf(buf, sizeof buf, pFmt, v);
	buf[sizeof buf - 1] = 0;

	va_end(v);

	free(m_pStartErrorMessage);
	m_pStartErrorMessage = _strdup(buf);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void App::SetDefaultRenderTarget()
{
	RECT client;
	GetClientRect(m_hWnd, &client);

	// Set render target.
	m_pD3DDeviceContext->OMSetRenderTargets(1, &m_pD3DRenderTargetView, m_pD3DDepthStencilView);

	// Set viewport for window.
	D3D11_VIEWPORT vp;

	vp.TopLeftX = FLOAT(client.left);
	vp.TopLeftY = FLOAT(client.top);
	vp.Width = FLOAT(client.right);
	vp.Height = FLOAT(client.bottom);

	vp.MinDepth = 0.f;
	vp.MaxDepth = 1.f;

	m_pD3DDeviceContext->RSSetViewports(1, &vp);
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

bool App::IsInFocus() const
{
	return m_isInFocus;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void App::ReleaseRenderTargetsAndViews()
{
	this->ClearStateAndFlushDeviceContext();

	Release(m_pD3DRenderTargetView);
	Release(m_pD3DDepthStencilView);
	Release(m_pD3DDepthStencilBuffer);

	m_renderTargetWidth = 0;
	m_renderTargetHeight = 0;

	m_canRender = false;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void App::RecreateRenderTargetsAndViews()
{
	this->ReleaseRenderTargetsAndViews();

	// Get swap chain desc.
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	if (FAILED(m_pDXGISwapChain->GetDesc(&swapChainDesc)))
		return;

	// Get size of swap chain output window.
	RECT client;
	GetClientRect(swapChainDesc.OutputWindow, &client);

	// Resize swap chain buffers.
	if (FAILED(m_pDXGISwapChain->ResizeBuffers(swapChainDesc.BufferCount, client.right, client.bottom, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags)))
		return;

	// Recreate render target view.
	ID3D11Texture2D *pD3DBackBuffer;

	if (FAILED(m_pDXGISwapChain->GetBuffer(0, IID_ID3D11Texture2D, (void **)&pD3DBackBuffer)))
		return;

	bool goodRenderTargetView = SUCCEEDED(m_pD3DDevice->CreateRenderTargetView(pD3DBackBuffer, NULL, &m_pD3DRenderTargetView));

	// Don't need the back buffer any more. If CreateRenderTargetView
	// succeeded, it took a reference to it.
	Release(pD3DBackBuffer);

	if (!goodRenderTargetView)
		return;

	SetD3DObjectDebugName(m_pD3DRenderTargetView, "RenderTargetView");

	// Recreate depth/stencil view.
	D3D11_TEXTURE2D_DESC depthStencilDesc;

	depthStencilDesc.Width = client.right;
	depthStencilDesc.Height = client.bottom;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.ArraySize = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = 1;
	depthStencilDesc.SampleDesc.Quality = 0;
	depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthStencilDesc.CPUAccessFlags = 0;
	depthStencilDesc.MiscFlags = 0;

	if (FAILED(m_pD3DDevice->CreateTexture2D(&depthStencilDesc, NULL, &m_pD3DDepthStencilBuffer)))
		return;

	if (FAILED(m_pD3DDevice->CreateDepthStencilView(m_pD3DDepthStencilBuffer, NULL, &m_pD3DDepthStencilView)))
		return;

	// That seemed to work.
	SetD3DObjectDebugName(m_pD3DDepthStencilBuffer, "DepthStencilBuffer %ldx%ld", client.right, client.bottom);
	SetD3DObjectDebugName(m_pD3DDepthStencilView, "DepthStencilView");

	m_renderTargetWidth = client.right;
	m_renderTargetHeight = client.bottom;

	//this->SetRenderTargetAndViewportForWindow();

	m_canRender = true;

	return;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

static const char WINDOW_CLASS_NAME[] = "WindowClass";

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

LRESULT CALLBACK App::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_CREATE)
	{
		CREATESTRUCT *cs = (CREATESTRUCT *)lParam;

		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)cs->lpCreateParams);
	}

	App *pApp = (App *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

	if (uMsg == WM_DESTROY)
		SetWindowLongPtr(hWnd, GWLP_USERDATA, 0);

	switch(uMsg)
	{
	case WM_ACTIVATEAPP:
		{
			pApp->m_isInFocus = !!wParam;
			dprintf("WM_ACTIVATE: in focus = %d\n", pApp->m_isInFocus);
		}
		return 0;

// 	case WM_ACTIVATE:
// 		{
// 			pApp->m_isInFocus = LOWORD(wParam) != WA_INACTIVE;
// 			dprintf("WM_ACTIVATE: in focus = %d\n", pApp->m_isInFocus);
// 		}
// 		return 0;
		
	case WM_CLOSE:
		{
			// The user closed the window, so we might as well quit.
			PostQuitMessage(0);
		}
		return 0;
	}

	return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

void App::RegisterWindowClass()
{
	WNDCLASSEX w;

	w.cbSize = sizeof w;
	w.style = CS_VREDRAW | CS_HREDRAW;
	w.lpfnWndProc = &WndProc;
	w.cbClsExtra = 0;
	w.cbWndExtra = 0;
	w.hInstance = GetModuleHandle(NULL);//you can always get the HINSTANCE this way.
	w.hIcon = NULL;//"if this member is NULL, the system provides a default icon."
	w.hCursor = LoadCursor(NULL, IDC_ARROW);
	w.hbrBackground = NULL;
	w.lpszMenuName = NULL;
	w.lpszClassName = WINDOW_CLASS_NAME;
	w.hIconSm = NULL;

	RegisterClassEx(&w);
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

// Eat up and dispatch as many messages as are available.
//
// Returns true if the program should continue.
//
// Returns false if the program should finish - either due to a WM_QUIT
// message, or some kind of problem with GetMessage.
static bool DoMessages()
{
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		BOOL good = GetMessage(&msg, NULL, 0, 0);
		if (good == 0 || good == -1)
			return false;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return true;
}

int Run(App *pApp)
{
	App::RegisterWindowClass();

	HWND hWnd = CreateWindow(WINDOW_CLASS_NAME, "DirectX 11 Test", WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, GetModuleHandle(NULL), pApp);

	if (!pApp->StartD3D(hWnd))
	{
		MessageBox(NULL, "Failed to start D3D.", "D3D Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	if (!pApp->Start())
	{
		pApp->StopD3D();

		const char *pMessage = pApp->GetStartErrorMessage();
		if (!pMessage)
			pMessage = "Failed to start app.";

		MessageBox(NULL, pMessage, "App Error", MB_OK | MB_ICONERROR);
		return 1;
	}

	ShowWindow(hWnd, SW_SHOW);

	LARGE_INTEGER oneFrame;
	QueryPerformanceFrequency(&oneFrame);
	oneFrame.QuadPart /= 60;

	// If there is more than sleepGap time left, sleep for 1ms.
	// (sleepGap itself is rather more than 1ms, because sleeping
	// isn't always very accurate.)
	//
	// This cuts down on CPU usage, so that other apps runs better,
	// laptop stays cool, etc.
	LARGE_INTEGER sleepGap;
	QueryPerformanceFrequency(&sleepGap);
	sleepGap.QuadPart /= 250;

	// This makes Sleep more accurate.
	timeBeginPeriod(1);

	// Time for next update.
	LARGE_INTEGER nextUpdate;
	QueryPerformanceCounter(&nextUpdate);
	nextUpdate.QuadPart += oneFrame.QuadPart;

	while (DoMessages())
	{
		// Wait until the next 60th-of-a-second boundary has
		// arrived (or been and gone).
		LARGE_INTEGER now;

		for(;;)
		{
			QueryPerformanceCounter(&now);

			LONGLONG delayTimeLeft = nextUpdate.QuadPart - now.QuadPart;

			if (delayTimeLeft <= 0)
				break;

			if (delayTimeLeft >= sleepGap.QuadPart)
				Sleep(1);
		}

		nextUpdate.QuadPart = now.QuadPart + oneFrame.QuadPart;

		pApp->Update();

		pApp->Render();
	}

	pApp->Stop();

	pApp->StopD3D();

	DestroyWindow(hWnd);
	hWnd = NULL;

	return 0;
}

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
