#include<windows.h>
#include<windowsx.h>
#include<tchar.h>
#include"WindowsMonitor.h"
#include"RenderDevice.h"

#define PI 3.1415926
BOOL WindowsMonitor::InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	//��䴰����ṹ
	m_wc.style = CS_VREDRAW | CS_HREDRAW;
	m_wc.lpfnWndProc = (WNDPROC)WinProc;
	m_wc.cbClsExtra = 0;
	m_wc.cbWndExtra = 0;
	m_wc.hInstance = hInstance;
	m_wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	m_wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	m_wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	m_wc.lpszMenuName = NULL;
	m_wc.lpszClassName = _T("MyWindow");
	ZeroMemory(&m_bi, sizeof(m_bi));
	m_bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_bi.bmiHeader.biWidth = WIDTH;
	m_bi.bmiHeader.biHeight = -HEIGHT;
	m_bi.bmiHeader.biPlanes = 1;
	m_bi.bmiHeader.biCompression = BI_RGB;
	m_bi.bmiHeader.biBitCount = 32;
	m_bi.bmiHeader.biSizeImage = WIDTH*HEIGHT*4 ;
	
	//ע�ᴰ����
	RegisterClass(&m_wc);

	//����������
	m_hwnd = CreateWindow(
		_T("MyWindow"),									//����������
		_T("MyRender(author:wj@xidianUniversity)"),		//���ڱ���
		WS_OVERLAPPEDWINDOW,							//���ڷ�񣬶���Ϊ��ͨ��
		InitPositionX,									//����λ�õ�x����
		InitPositionY,									//����λ�õ�y����
		WIDTH,											//���ڵĿ��
		HEIGHT,											//���ڵĸ߶�
		NULL,											//�����ھ��
		NULL,											//�˵����
		hInstance,										//Ӧ�ó���ʵ�����
		NULL);											//���ڴ�������ָ��
	if (!m_hwnd) return FALSE;
	ShowWindow(m_hwnd, nCmdShow);
	m_screenDc = GetDC(m_hwnd);
	m_compatibleDC = CreateCompatibleDC(m_screenDc);
	m_compatibleBitmap = CreateCompatibleBitmap(m_screenDc, WIDTH, HEIGHT);
	m_screenHb = CreateDIBSection(m_compatibleDC, &m_bi, DIB_RGB_COLORS, &m_pFrame, 0, 0);
	m_screenOb = (HBITMAP)SelectObject(m_compatibleDC, m_screenHb);
	memset(m_pFrame, 0, 4*WIDTH*HEIGHT);
	return TRUE;
}

LRESULT CALLBACK WindowsMonitor::WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_KEYDOWN:
	{
		Camera* camera = RenderDevice::GetRenderDevice()->m_mainCamera;
		if (!camera)	return DefWindowProc(hWnd, message, wParam, lParam);
		switch (wParam)
		{
		case VK_SPACE: //SPCAE change RenderType
		{
			if (RenderDevice::GetRenderDevice()->GetRenderType() == RenderType::COLOR)
				RenderDevice::GetRenderDevice()->SetRenderType(RenderType::TEXTURE);
			else if (RenderDevice::GetRenderDevice()->GetRenderType() == RenderType::TEXTURE)
				RenderDevice::GetRenderDevice()->SetRenderType(RenderType::WIREFRAME);
			else
				RenderDevice::GetRenderDevice()->SetRenderType(RenderType::COLOR);
			break;
		}
		case 0x5A://Z Press,Change SpecularOpen state.
		{
			if (RenderDevice::GetRenderDevice()->m_bSpecularOpen == false)
				RenderDevice::GetRenderDevice()->m_bSpecularOpen = true;
			else
				RenderDevice::GetRenderDevice()->m_bSpecularOpen = false;
			break;
		}
		case 0x58://X Press,Change PixelBasedShadingOpen state.
		{
			if (RenderDevice::GetRenderDevice()->m_bPixelBasedShadingOpen == false)
				RenderDevice::GetRenderDevice()->m_bPixelBasedShadingOpen = true;
			else
				RenderDevice::GetRenderDevice()->m_bPixelBasedShadingOpen = false;
			break;
		}
		case 0x43://C Press,Change lightingOpen state
		{
			if (RenderDevice::GetRenderDevice()->m_bLightingOpen == false)
				RenderDevice::GetRenderDevice()->m_bLightingOpen = true;
			else
				RenderDevice::GetRenderDevice()->m_bLightingOpen = false;
			break;
		}
		case VK_UP:
			camera->position.fY += 0.2f;
			break;
		case VK_DOWN:
			camera->position.fY -= 0.2f;
			break;
		case VK_LEFT:
			camera->position.fX -= 0.2f;
			break;
		case VK_RIGHT:
			camera->position.fX += 0.2f;
			break;
		case 0x57://W Press
		{
			Matrix4 rotateMatrix = camera->RotateAroundArbitraryAxis(camera->right, PI / 180.0f);
			camera->look = camera->look*rotateMatrix;
			camera->up = camera->up*rotateMatrix;
			camera->right = -camera->look.Cross(camera->up);
			camera->right.ResetUnitVector();
		}
		break;
		case 0x53://S Press
		{
			Matrix4 rotateMatrix = camera->RotateAroundArbitraryAxis(camera->right, -PI / 180.0f);
			camera->look = camera->look*rotateMatrix;
			camera->up = camera->up*rotateMatrix;
			camera->right = -camera->look.Cross(camera->up);
			camera->right.ResetUnitVector();
		}
		break;

		case 0x41://A Press
		{
			Matrix4 rotateMatrix = camera->RotateAroundArbitraryAxis(camera->up, PI / 180.0f);
			camera->look = camera->look*rotateMatrix;
			camera->right = camera->right*rotateMatrix;
			camera->up = -camera->right.Cross(camera->look);
			camera->up.ResetUnitVector();
		}
		break;

		case 0x44://D Press
		{
			Matrix4 rotateMatrix = camera->RotateAroundArbitraryAxis(camera->up, -PI / 180.0f);
			camera->look = camera->look*rotateMatrix;
			camera->right = camera->right*rotateMatrix;
			camera->up = -camera->right.Cross(camera->look);
			camera->up.ResetUnitVector();
		}
		break;

		case 0x45://E Press
		{
			Matrix4 rotateMatrix = camera->RotateAroundArbitraryAxis(camera->look, PI / 180.0f);
			camera->up = camera->up*rotateMatrix;
			camera->right = camera->right*rotateMatrix;
			camera->look = -camera->up.Cross(camera->right);
			camera->look.ResetUnitVector();
		}
		break;

		case 0x51://Q Press
		{
			Matrix4 rotateMatrix = camera->RotateAroundArbitraryAxis(camera->look, -PI / 180.0f);
			camera->up = camera->up*rotateMatrix;
			camera->right = camera->right*rotateMatrix;
			camera->look = -camera->up.Cross(camera->right);
			camera->look.ResetUnitVector();
		}
		break;
		}
			//RenderDevice::GetRenderDevice()->m_viewMatrix = camera->getViewTransformMatrix();
			//RenderDevice::GetRenderDevice()->ClearFrameBuffer();
			//RenderDevice::GetRenderDevice()->DrawTest();
		RenderDevice::GetRenderDevice()->m_bCameraMatrixChange = true;
			break;
	}
	case WM_MOUSEWHEEL:
	{
		Camera* camera = RenderDevice::GetRenderDevice()->m_mainCamera;
		if (!camera)	return DefWindowProc(hWnd, message, wParam, lParam);
		if ((INT)wParam > 0)
		{
			camera->position.fZ += 0.2;
		}
		else
		{
			camera->position.fZ -= 0.2;
		}
			RenderDevice::GetRenderDevice()->m_viewMatrix = camera->getViewTransformMatrix();
			RenderDevice::GetRenderDevice()->ClearFrameBuffer();
			RenderDevice::GetRenderDevice()->DrawTest();
			break;
	}

	case WM_RBUTTONDOWN:					//�����Ϣ
	{
		RenderDevice::GetRenderDevice()->ClearFrameBuffer();
		RenderDevice::GetRenderDevice()->DrawTest();
		break;
	}
	case WM_PAINT:							//�����ػ���Ϣ
	{
		RenderDevice::GetRenderDevice()->ClearFrameBuffer();
		RenderDevice::GetRenderDevice()->UpdateGraphics(m_hwnd);
		break;
	}
	case WM_DESTROY:						//�˳���Ϣ
		PostQuitMessage(0);					//�����˳�����
		break;
	}
	//����ȱʡ��Ϣ�������
	return DefWindowProc(hWnd, message, wParam, lParam);
}



