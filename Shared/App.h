#ifndef HEADER_8DBC5033A6CE4ECA9F90C96F3AF6C56F
#define HEADER_8DBC5033A6CE4ECA9F90C96F3AF6C56F

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//
// The App class provides bare-bones D3D setup, and does some of the
// standard stuff for you, including creating a window and initialising/
// shutting down D3D. There are hooks for your own initialisation, 
// shut down, rendering and logic update.
// 
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

struct IDXGISwapChain;
struct ID3D11Device;
struct ID3D11Debug;
struct ID3D11DeviceContext;
struct ID3D11RenderTargetView;
struct ID3D11DepthStencilView;
struct ID3D11Texture2D;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

class App
{
public:
	static void RegisterWindowClass();

	App();
	virtual ~App();

	void SetSoftwareD3D(bool softwareD3D);

	bool StartD3D(HWND hWnd);
	void StopD3D();

	bool Start();
	void Stop();

	// Returns NULL if no description available.
	const char *GetStartErrorMessage() const;

	//
	void Update();

	//
	void Render();
protected:
	bool CanRender() const;

	// Call this before deleting anything. As its name suggests, it
	// calls ClearState and then Flush on the device context, ensuring
	// that no objects are selected into it.
	void ClearStateAndFlushDeviceContext();

	// Set window title, like printf. This can be called at any time, though
	// ideally in the HandleStart function.
	void SetWindowTitle(const char *pFmt, ...);

	// Default implementation does nothing and returns true.
	//
	// If this function returns false, HandleStop will be called. This
	// means that HandleStart probably won't need any cleanup code,
	// but things must be in a cleanup-able state.
	//
	// If returning false, may use SetStartErrorMessage to set an error
	// message.
	virtual bool HandleStart();

	// Default implementation does nothing.
	virtual void HandleStop();

	// Called with window's render target and viewport set, 
	//
	// Default implementation does nothing.
	virtual void HandleRender();

	// Gets called at roughly 60Hz.
	//
	// Default implementation does nothing.
	virtual void HandleUpdate();

	// Set the error message displayed, if HandleStart returns false.
	void SetStartErrorMessage(const char *pFmt, ...);

	// If the render target was changed, call this function to set the
	// default one back. It also sets an appropriate viewport rect.
	void SetDefaultRenderTarget();

	//
	bool IsInFocus() const;

	// Don't change these; the app looks after them itself.
	//
	// However you generally need them all the time, so they're
	// easily-accessible, for your convenience.
	ID3D11Device *m_pD3DDevice;
	ID3D11Debug *m_pD3DDebug;
	ID3D11DeviceContext *m_pD3DDeviceContext;
	ID3D11RenderTargetView *m_pD3DRenderTargetView;
	ID3D11DepthStencilView *m_pD3DDepthStencilView;
	HWND m_hWnd;
private:
	char *m_pStartErrorMessage;

	bool m_softwareD3D;

	bool m_canRender;
	IDXGISwapChain *m_pDXGISwapChain;

	ID3D11Texture2D *m_pD3DDepthStencilBuffer;

	LONG m_renderTargetWidth;
	LONG m_renderTargetHeight;

	bool m_isInFocus;

	void ReleaseRenderTargetsAndViews();
	void RecreateRenderTargetsAndViews();

	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	App(const App &);
	App &operator=(const App &);
};

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int Run(App *pApp);

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

#endif//HEADER_8DBC5033A6CE4ECA9F90C96F3AF6C56F
