#include"WindowsMonitor.h"
#include"RenderDevice.h"

// Const variable define
#define PI 3.1415926

// Static member declare
RenderDevice* RenderDevice::m_instance = NULL;
HWND WindowsMonitor::m_hwnd = NULL;
LPVOID WindowsMonitor::WindowsMonitor::m_pFrame;
HDC WindowsMonitor::m_compatibleDC;
HDC WindowsMonitor::m_screenDc;
HBITMAP WindowsMonitor::m_screenOb;
HBITMAP WindowsMonitor::m_screenHb;
HBITMAP WindowsMonitor::m_compatibleBitmap;
WNDCLASS WindowsMonitor::m_wc;
BITMAPINFO WindowsMonitor::m_bi;

float fRenderFps = 0;

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	int			nFrameCount = 0;
	DWORD		dwPreTime = 0;
	DWORD		dwNowTime = 0;
	DWORD		dwTimeDelta = 0;
	MSG			msg;
	
	if (!WindowsMonitor::InitWindow(hInstance, nCmdShow))
		return FALSE;

	RenderDevice* renderDevice = NULL;
	renderDevice = RenderDevice::GetRenderDevice(WIDTH,HEIGHT,RenderType::TEXTURE);
	if (!renderDevice)
		return FALSE;

	// Set camera config to generate camera matrix.
	Vector4 look;
	Vector4 up;
	Vector4 right;
	Vector4 position;
	look.fX = 0; look.fY = 0; look.fZ = 1; look.fW = 0;
	up.fX = 0; up.fY = 1; up.fZ = 0; up.fW = 0;
	right.fX = 1; right.fY = 0; right.fZ = 0; right.fW = 0;
	position.fX = 0; position.fY = 0; position.fZ = 0; position.fW = 1;
	Camera* mainCamera = new Camera(position, up, look, right);

	renderDevice->m_mainCamera = mainCamera;
	renderDevice->m_viewMatrix = mainCamera->getViewTransformMatrix();
	renderDevice->m_projMatrix = getPerspectiveProjMatrix(PI/2, 10, 1000, WIDTH / (float)HEIGHT);
	renderDevice->m_viewportMatrix = getViewPortMatrix(0, 0, WIDTH, HEIGHT, 0, 1);
	//renderDevice->m_lightingOpen = false;
	//renderDevice->m_backFaceTestOpen = false;

	//Create a point light.
	Vector4 lightPosition;
	lightPosition.fX = -20; lightPosition.fY = 0; lightPosition.fZ = 0; lightPosition.fW = 1.0f;
	Light pointLight = Light(LightType::POINTLIGHT, lightPosition,Vector4(),0x00808080,0x00181818,0x00FFFFFF);
	renderDevice->m_lights[0] = pointLight;
	renderDevice->m_lightCount++;
	
	while (TRUE) {
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			dwNowTime = GetTickCount();
			//Sleep(5);
			if (renderDevice->m_bCameraMatrixChange)
			{
				renderDevice->m_viewMatrix = renderDevice->m_mainCamera->getViewTransformMatrix();
				renderDevice->m_bCameraMatrixChange = false;
			}
			renderDevice->ClearFrameBuffer();
			renderDevice->DrawTest();
			renderDevice->UpdateGraphics(WindowsMonitor::m_hwnd);
			nFrameCount++;
			dwTimeDelta += (dwNowTime - dwPreTime);
			if (dwTimeDelta > 1000)
			{
				fRenderFps = 1000.0f * ((float)nFrameCount / dwTimeDelta);
				dwTimeDelta = 0;
				nFrameCount = 0;
			}
			dwPreTime = dwNowTime;
		}
	}
	
	return msg.wParam;
}