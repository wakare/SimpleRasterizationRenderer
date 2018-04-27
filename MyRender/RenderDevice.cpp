#include"RenderDevice.h"
#include"WindowsMonitor.h"
#include<iostream>
#include<tchar.h>
#include<string>

#define SCANACCURACY 1e-8

using namespace std;
extern float fRenderFps;
VOID RenderDevice::SetRenderType(RenderType renderType)
{
	m_eRenderType = renderType;
}

RenderType RenderDevice::GetRenderType()
{
	return m_eRenderType;
}

BOOL RenderDevice::InitFrameBuffer(int width, int height)
{
	if (WindowsMonitor::m_pFrame) {
		m_frameBuffer = (INT32 *)WindowsMonitor::m_pFrame;
		return TRUE;
	}
	return FALSE;
}

BOOL RenderDevice::ClearFrameBuffer() 
{
	if (m_frameBuffer)
		memset(m_frameBuffer, backgroundColor, sizeof(INT32)*WIDTH*HEIGHT);

	if (m_zBuffer)
	{
		for (int i = 0; i < HEIGHT; i++)
		{
			for (int j = 0; j < WIDTH; j++)
				(m_zBuffer + i*WIDTH)[j] = 1.0f;
		}
	}

	return true;
}

BOOL RenderDevice::InitTextureBuffer(int width, int height)
{
	m_textureWidth = width;
	m_textureHeight = height;
	m_textureBuffer = (INT32*)malloc(sizeof(INT32) * width * height);
	if (!m_textureBuffer)
		return FALSE;
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			(m_textureBuffer + i * width)[j] = (((i / 32)+(j / 32)) & 0x1) ? 0x00fbcdef : 0x00ffffff;
		}
	}
	return TRUE;
}

BOOL RenderDevice::ClearTextureBuffer()
{
	if (m_textureBuffer)
		free(m_textureBuffer);
	return TRUE;
}

VOID RenderDevice::UpdateGraphics(HWND hwnd) 
{
	string strFPS = "FPS:" + to_string(fRenderFps);
	string strSpecularOpen;
	string strPixelShading;
	string strLightingOpen;
	
	// Show help tips
	strSpecularOpen = (m_bSpecularOpen)?"SpecularOpen(Z): true":"SpecularOpen(Z): false";
	strPixelShading = (m_bPixelBasedShadingOpen)?"PixelShadingOpen(X): true": "PixelShadingOpen(X): false";
	strLightingOpen = (m_bLightingOpen)?"LightingOpen(C): true": "LightingOpen(C): false";
	string strControlHelps[2] = { "Translate: up/down/left/right" , "Rotate: w/a/s/d/q/e"};

	HDC hDC = GetDC(hwnd);
	BitBlt(hDC, InitPositionX, InitPositionY, m_frameWidth, m_frameHeight, WindowsMonitor::m_compatibleDC, 0, 0, SRCCOPY);
	TextOut(hDC, WIDTH - 200, 50, _T(strFPS.c_str()), strFPS.length());
	TextOut(hDC, WIDTH - 200, 100, _T(strLightingOpen.c_str()), strLightingOpen.length());
	TextOut(hDC, WIDTH - 200, 150, _T(strSpecularOpen.c_str()), strSpecularOpen.length());
	TextOut(hDC, WIDTH - 200, 200, _T(strPixelShading.c_str()), strPixelShading.length());

	for (int i = 0; i < 2; i++)
	{
		TextOut(hDC, WIDTH - 200, 250 + (i * 30), _T(strControlHelps[i].c_str()), strControlHelps[i].length());
	}
	
	ReleaseDC(hwnd,hDC);
}

RenderDevice::RenderDevice(int width, int height, RenderType _renderType)
{
	Init();
	m_eRenderType = _renderType;
	m_frameWidth = width;
	m_frameHeight = height;
	if (!InitFrameBuffer(m_frameWidth, m_frameHeight)) 
	{
		MessageBox(WindowsMonitor::m_hwnd, _T("RenderDevice Init Fail"), _T("Init Fail"), MB_OK);
	}
	if (!InitTextureBuffer(256,256))
	{
		MessageBox(WindowsMonitor::m_hwnd, _T("RenderDevice Init Fail"), _T("Init Fail"), MB_OK);
	}
	m_zBuffer = (float*)malloc(sizeof(float)*WIDTH*HEIGHT);
	for (int i = 0; i < HEIGHT; i++)
	{
		for (int j = 0; j < WIDTH; j++)
			(m_zBuffer + i*WIDTH)[j] = 1.0f;
	}
	SetPointColor(0x00ffffff);
	SetBackGroundColor(0x00000000);
}

RenderDevice* RenderDevice::GetRenderDevice(int width, int height,RenderType _renderType)
{
	if (!m_instance) {
		m_instance = new RenderDevice(width, height, _renderType);
	}
	return m_instance;
}

VOID RenderDevice::Init() 
{
	m_instance = NULL;
	m_frameBuffer = NULL;
	m_frameHeight = 0;
	m_frameWidth = 0;
	m_projMatrix = NULL;
	m_viewMatrix = NULL;
	m_viewportMatrix = NULL;
	m_bBackFaceTestOpen = true;
	m_bLightingOpen = true;
	m_lightCount = 0;
	m_bSpecularOpen = false;
	m_bPixelBasedShadingOpen = false;
	m_bFpsLock = true;
	m_bCameraMatrixChange = false;
	m_lights = (Light*)malloc(sizeof(Light)*MAX_LIGHTCOUNTS);
}

RenderDevice::~RenderDevice() 
{
	if (m_zBuffer) 
	{
		free(m_zBuffer);
		m_zBuffer = NULL;
	}
	if (m_lights)
	{
		free(m_lights);
		m_lights = NULL;
	}
	if (m_textureBuffer)
	{
		free(m_textureBuffer);
		m_textureBuffer = NULL;
	}
}

VOID RenderDevice::SetPointColor(INT32 Color)
{
	pointColor = Color;
}

VOID RenderDevice::SetBackGroundColor(INT32 _backgroundColor)
{
	backgroundColor = _backgroundColor;
	if (m_frameBuffer)
	{
		for (int i = 0; i < m_frameHeight; i++)
		{
			for (int j = 0; j < m_frameWidth; j++)
			{
				(m_frameBuffer + i * WIDTH)[j] = _backgroundColor;
			}
		}
	}
}

INT32 RenderDevice::GetPointColor()
{
	return pointColor;
}

INT32 RenderDevice::GetTextureColor(float u, float v)
{
	if (m_textureBuffer)
	{
		int X = (int)(u * (m_textureWidth - 1) + 0.5f);
		int Y = (int)(v * (m_textureHeight - 1) + 0.5f);
		if (X < 0 || X >= m_frameWidth || Y < 0 || Y >= m_frameHeight)
			return 0x00FF0000;
		return	(m_textureBuffer + m_textureWidth * Y)[X];
	}
	return 0x0000FF00;
}

BOOL RenderDevice::IsPointOutRange(Point2D p) 
{
	if (!m_frameBuffer)
		return TRUE;
	else 
	{
		if (p.x < 0 || p.x >= WIDTH || p.y < 0 || p.y >= WIDTH)
			return TRUE;
		return FALSE;
	}
}

VOID RenderDevice::DrawTest() 
{
	if (m_frameBuffer)
	{
		// TODO: Add scene manager module
		Point3D* vertices[8];
		vertices[0] = new Point3D(-10, -10, 20, 0.0f, 1.0f, 0x00FF0000);
		vertices[1] = new Point3D(10, -10, 20, 1.0f, 1.0f, 0x0000FF00);
		vertices[2] = new Point3D(10, 10, 20, 1.0f, 0.0f, 0x000000FF);
		vertices[3] = new Point3D(-10, 10, 20, 0.0f, 0.0f, 0x00FFFFFF);
		vertices[4] = new Point3D(-10, -10, 40, 1.0f, 1.0f, 0x00FF0000);
		vertices[5] = new Point3D(10, -10, 40, 0.0f, 1.0f, 0x0000FF00);
		vertices[6] = new Point3D(10, 10, 40, 0.0f, 0.0f, 0x000000FF);
		vertices[7] = new Point3D(-10, 10, 40, 1.0f, 0.0f, 0x00FFFFFF);

		Material mtrl;
		mtrl.Ambient = 0x00FFFFFF;
		mtrl.Diffuse = 0x00FFFFFF;
		mtrl.Specular = 0x00FFFFFF;
		mtrl.Emissive = 0x0;
		mtrl.Power = 8.0f;
		Cube cube(vertices);
		cube.SetMaterial(mtrl);
		DrawCube(cube);
		for each (Point3D* p in vertices)
		{
			delete(p);
		}
	}
}

VOID RenderDevice::DrawLine(Point2D start,Point2D end)
{
	if (m_frameBuffer)
	{
		if (IsPointOutRange(start) || IsPointOutRange(end))
		{
			cout << "[ERROR] Point out of range.";
			return;
		}

		int step;
		// check horizontal
		if (start.x == end.x)
		{
			step = (start.y > end.y) ? -1 : 1;
			for (int i = start.y; i != end.y; i += step)
			{
				(m_frameBuffer + i * WIDTH)[start.x] = pointColor;
			}
			return;
		}
		// normal line
		else
		{
			step = (start.x > end.x) ? -1 : 1;
			float k = (end.y - start.y) / (float)(end.x - start.x);
			int y = start.y;
			float tempY = 0.0f;
			for (int i = start.x; i != end.x; i += step)
			{
				(m_frameBuffer + y * WIDTH)[i] = pointColor;
				tempY += k;
				if (tempY > 1.0f || tempY < -1.0f)
				{
					int _step = (tempY > 0) ? 1 : -1;
					for (int _y = 0; _y != (int)tempY; _y += _step)
					{
						(m_frameBuffer + (y + (step)*_y) * WIDTH)[i] = pointColor;
					}
					y += (int)tempY*step;
					tempY -= (int)tempY;
				}
			}
		}
	}
}

VOID RenderDevice::DrawTriangle(triangle tri) 
{
	// Save original vertex info for calculating light.
	Vector4 originVertices[3] = 
	{
		tri.vertices[0].position,
		tri.vertices[1].position,
		tri.vertices[2].position
	};

	// World Coordination --> View Coordination.
	triangle _tri = tri;
	_tri.vertices[0].position *= m_viewMatrix;
	_tri.vertices[1].position *= m_viewMatrix;
	_tri.vertices[2].position *= m_viewMatrix;

	// BackFace Test
	if (m_bBackFaceTestOpen)
	{
		if (m_eRenderType != RenderType::WIREFRAME)
		{
			_tri.setPlaneNormalVector();
			if (!_tri.faceBackTest(m_mainCamera->position * m_viewMatrix))
				return;
		}
	}

	// View Matrix --> Projection Coordination.
	_tri.vertices[0].position *= m_projMatrix;
	_tri.vertices[1].position *= m_projMatrix;
	_tri.vertices[2].position *= m_projMatrix;

	// Use rhw (1.0f / w) to avoid division. 
	_tri.vertices[0].rhw = 1 / _tri.vertices[0].position.fW;
	_tri.vertices[1].rhw = 1 / _tri.vertices[1].position.fW;
	_tri.vertices[2].rhw = 1 / _tri.vertices[2].position.fW;

	_tri.vertices[0].position.Normalize();
	_tri.vertices[1].position.Normalize();
	_tri.vertices[2].position.Normalize();

	// Clip these outside triangle.
	// TODO: Generate subtriangles when a triangle is clipped.
	int index = 0;
	while (index < 3)
	{
		if (_tri.vertices[index].position.fX < -1 ||
			_tri.vertices[index].position.fX > 1  ||
			_tri.vertices[index].position.fY < -1 ||
			_tri.vertices[index].position.fY > 1  ||
			_tri.vertices[index].position.fZ < 0  ||
			_tri.vertices[index].position.fZ > 1)
			return; //this triangle is outside.
		index++;
	}

	// Calculate light
	if (m_eRenderType != RenderType::WIREFRAME && !m_bPixelBasedShadingOpen)
	{
		if (m_bLightingOpen)
		{
			if (m_eRenderType == RenderType::COLOR)
			{
				// Calculate vertex light color.(vertex color as baseColor)
				_tri.vertices[0].color = CalculateLighting(_tri.material, originVertices[0], _tri.normalVector, m_mainCamera->position, _tri.vertices[0].color,
					m_lights, m_lightCount,m_bSpecularOpen);
				_tri.vertices[1].color = CalculateLighting(_tri.material, originVertices[1], _tri.normalVector, m_mainCamera->position, _tri.vertices[1].color,
					m_lights, m_lightCount, m_bSpecularOpen);
				_tri.vertices[2].color = CalculateLighting(_tri.material, originVertices[2], _tri.normalVector, m_mainCamera->position, _tri.vertices[2].color,
					m_lights, m_lightCount, m_bSpecularOpen);
			}
			else if (m_eRenderType == RenderType::TEXTURE)
			{
				// Calculate vertex light color.(default white as baseColor)
				_tri.vertices[0].color = CalculateLighting(_tri.material, originVertices[0], _tri.normalVector, m_mainCamera->position, 0x00FFFFFF,
					m_lights, m_lightCount, m_bSpecularOpen);
				_tri.vertices[1].color = CalculateLighting(_tri.material, originVertices[1], _tri.normalVector, m_mainCamera->position, 0x00FFFFFF,
					m_lights, m_lightCount, m_bSpecularOpen);
				_tri.vertices[2].color = CalculateLighting(_tri.material, originVertices[2], _tri.normalVector, m_mainCamera->position, 0x00FFFFFF,
					m_lights, m_lightCount, m_bSpecularOpen);
			}
		}
	}

	// Projection Coordination --> Viewport Coordination.
	_tri.vertices[0].position *= m_viewportMatrix;
	_tri.vertices[1].position *= m_viewportMatrix;
	_tri.vertices[2].position *= m_viewportMatrix;

	switch (m_eRenderType)
	{
	case RenderType::WIREFRAME:
		DrawLine(Point2D(_tri.vertices[0].position.fX, _tri.vertices[0].position.fY), Point2D(_tri.vertices[1].position.fX, _tri.vertices[1].position.fY));
		DrawLine(Point2D(_tri.vertices[1].position.fX, _tri.vertices[1].position.fY), Point2D(_tri.vertices[2].position.fX, _tri.vertices[2].position.fY));
		DrawLine(Point2D(_tri.vertices[2].position.fX, _tri.vertices[2].position.fY), Point2D(_tri.vertices[0].position.fX, _tri.vertices[0].position.fY));
		break;

	case RenderType::TEXTURE:
	case RenderType::COLOR:
	{
		_tri.GetTriangleType(originVertices);
		Vector4 splitOriginPoint;
		if (_tri.type == TriangleType::UP || _tri.type == TriangleType::DOWN)
		{
			if (m_bPixelBasedShadingOpen)
				DrawStandardFilledTriangle(_tri, m_eRenderType, originVertices);
			else
				DrawStandardFilledTriangle(_tri, m_eRenderType);
			break;
		}
		else
		{
			float k = (_tri.vertices[1].position.fY - _tri.vertices[0].position.fY) / (_tri.vertices[2].position.fY - _tri.vertices[0].position.fY);
			splitOriginPoint = originVertices[0] + (originVertices[2] - originVertices[0])* k;
			if (_tri.type == TriangleType::LEFT)
			{
				triangle upSubTriangle = triangle(_tri.vertices[0], _tri.vertices[1], _tri.VSplitPoint);
				triangle downSubTriangle = triangle(_tri.VSplitPoint, _tri.vertices[1], _tri.vertices[2]);
				upSubTriangle.GetTriangleType();
				downSubTriangle.GetTriangleType();
				upSubTriangle.material = tri.material;
				downSubTriangle.material = tri.material;
				Vector4 OriginUpPOS[3] = { originVertices[0],splitOriginPoint,originVertices[1] };
				Vector4 OriginDownPOS[3] = { splitOriginPoint,originVertices[1],originVertices[2] };
				if (m_bPixelBasedShadingOpen)
				{
					DrawStandardFilledTriangle(upSubTriangle, m_eRenderType, OriginUpPOS);
					DrawStandardFilledTriangle(downSubTriangle, m_eRenderType, OriginDownPOS);
				}
				else
				{
					DrawStandardFilledTriangle(upSubTriangle, m_eRenderType);
					DrawStandardFilledTriangle(downSubTriangle, m_eRenderType);
				}
			}
			else if (_tri.type == TriangleType::RIGHT)
			{
				triangle upSubTriangle = triangle(_tri.vertices[0], _tri.VSplitPoint, _tri.vertices[1]);
				triangle downSubTriangle = triangle(_tri.vertices[1], _tri.VSplitPoint, _tri.vertices[2]);
				upSubTriangle.GetTriangleType();
				downSubTriangle.GetTriangleType();
				upSubTriangle.material = tri.material;
				downSubTriangle.material = tri.material;
				Vector4 OriginUpPOS[3] = { originVertices[0],originVertices[1],splitOriginPoint };
				Vector4 OriginDownPOS[3] = { originVertices[1],splitOriginPoint,originVertices[2] };
				if (m_bPixelBasedShadingOpen)
				{
					DrawStandardFilledTriangle(upSubTriangle, m_eRenderType, OriginUpPOS);
					DrawStandardFilledTriangle(downSubTriangle, m_eRenderType, OriginDownPOS);
				}
				else
				{
					DrawStandardFilledTriangle(upSubTriangle, m_eRenderType);
					DrawStandardFilledTriangle(downSubTriangle, m_eRenderType);
				}
			}
		}
	}
	break;
	}
}

// Draw UP or DOWN triangle.
VOID RenderDevice::DrawStandardFilledTriangle(triangle tri,RenderType type,Vector4 originPosition[3]) 
{
	if (tri.type == TriangleType::LEFT || tri.type == TriangleType::RIGHT)
		return;

	int nLeftX, nLeftY;
	float fLeftZ, fLeftRHW, fLeftU, fLeftV, fLeftOriginX, fLeftOriginY;
	int nRightX, nRightY;
	float fRightZ, fRightRHW, fRightU, fRightV, fRightOriginX, fRightOriginY;
	INT32 LeftColor, RightColor;
	Vector4 LeftNormalVector, RightNormalVector,LeftOriginPOS;
	double scanlineRHWLength;

	for (int startY = (int)(tri.vertices[0].position.fY); startY <= (int)(tri.vertices[2].position.fY + 0.5f); startY++)
	{
		float k = (startY - tri.vertices[0].position.fY) / (tri.vertices[2].position.fY - tri.vertices[0].position.fY);
		if (k > 1.00000001f || k < 0.0f)
			continue;

		if (tri.type == TriangleType::UP) {
			nLeftX = (int)(tri.vertices[0].position.fX + k * (tri.vertices[1].position.fX - tri.vertices[0].position.fX) + 0.5f);
			nLeftY = startY;
			if (originPosition)
			{
				fLeftOriginX = originPosition[0].fX + k* (originPosition[1].fX - originPosition[0].fX);
				fLeftOriginY = originPosition[0].fY + k* (originPosition[1].fY - originPosition[0].fY);
			}
			nRightX = (int)(tri.vertices[0].position.fX + k * (tri.vertices[2].position.fX - tri.vertices[0].position.fX) + 0.5f);
			nRightY = startY;
			if (originPosition)
			{
				fRightOriginX = originPosition[0].fX + k* (originPosition[2].fX - originPosition[0].fX);
				fRightOriginY = originPosition[0].fY + k* (originPosition[2].fY - originPosition[0].fY);
			}
			fLeftZ = tri.vertices[0].position.fZ + k*(tri.vertices[1].position.fZ - tri.vertices[0].position.fZ);
			fLeftRHW = tri.vertices[0].rhw + k*(tri.vertices[1].rhw - tri.vertices[0].rhw);
			fRightZ = tri.vertices[0].position.fZ + k*(tri.vertices[2].position.fZ - tri.vertices[0].position.fZ);
			fRightRHW = tri.vertices[0].rhw + k*(tri.vertices[2].rhw - tri.vertices[0].rhw);
			scanlineRHWLength = fRightRHW - fLeftRHW;
			if ((tri.vertices[1].rhw > SCANACCURACY || tri.vertices[1].rhw< -SCANACCURACY || tri.vertices[0].rhw >SCANACCURACY || tri.vertices[0].rhw < -SCANACCURACY))
			{
				float deltaZ1 = (1 / tri.vertices[1].rhw) - (1 / tri.vertices[0].rhw);
				if ((scanlineRHWLength > SCANACCURACY || scanlineRHWLength < -SCANACCURACY) && (deltaZ1 > SCANACCURACY || deltaZ1 < -SCANACCURACY))
					k = ((1 / fLeftRHW) - (1 / tri.vertices[0].rhw)) / deltaZ1;
				else
					k = (startY - tri.vertices[0].position.fY) / (tri.vertices[2].position.fY - tri.vertices[0].position.fY);
			}
			else
				k = (startY - tri.vertices[0].position.fY) / (tri.vertices[2].position.fY - tri.vertices[0].position.fY);
			if (k < SCANACCURACY && k> -SCANACCURACY)
			{
				k = (startY - tri.vertices[0].position.fY) / (tri.vertices[2].position.fY - tri.vertices[0].position.fY);
			}
			if (type == RenderType::TEXTURE)
			{
				fLeftU = tri.vertices[0].tex_u + k * (tri.vertices[1].tex_u - tri.vertices[0].tex_u);
				fLeftV = tri.vertices[0].tex_v + k * (tri.vertices[1].tex_v - tri.vertices[0].tex_v);
			}
			LeftColor = colorInterpolate(tri.vertices[0].color, tri.vertices[1].color, k);
			if (originPosition)
				LeftNormalVector = tri.vertices[0].VertexNormal + (tri.vertices[1].VertexNormal - tri.vertices[0].VertexNormal) * k;

			LeftOriginPOS.fX = nLeftX;
			LeftOriginPOS.fY = nLeftY;
			LeftOriginPOS.fZ = 1.0f / fLeftRHW;
			LeftOriginPOS.fW = 1.0f;

			if ((tri.vertices[2].rhw > SCANACCURACY || tri.vertices[2].rhw< -SCANACCURACY || tri.vertices[0].rhw >SCANACCURACY || tri.vertices[0].rhw < -SCANACCURACY))
			{
				float deltaZ2 = (1 / tri.vertices[2].rhw) - (1 / tri.vertices[0].rhw);
				if ((scanlineRHWLength > SCANACCURACY || scanlineRHWLength < -SCANACCURACY) && (deltaZ2 > SCANACCURACY || deltaZ2 < -SCANACCURACY))
					k = ((1 / fRightRHW) - (1 / tri.vertices[0].rhw)) / deltaZ2;
				else
					k = (startY - tri.vertices[0].position.fY) / (tri.vertices[2].position.fY - tri.vertices[0].position.fY);
			}
			else
				k = (startY - tri.vertices[0].position.fY) / (tri.vertices[2].position.fY - tri.vertices[0].position.fY);
			if (k < SCANACCURACY && k> -SCANACCURACY)
			{
				k = (startY - tri.vertices[0].position.fY) / (tri.vertices[2].position.fY - tri.vertices[0].position.fY);
			}
			if (type == RenderType::TEXTURE)
			{
				fRightU = tri.vertices[0].tex_u + k * (tri.vertices[2].tex_u - tri.vertices[0].tex_u);
				fRightV = tri.vertices[0].tex_v + k * (tri.vertices[2].tex_v - tri.vertices[0].tex_v);
			}
			RightColor = colorInterpolate(tri.vertices[0].color, tri.vertices[2].color, k);
			if(originPosition)
				RightNormalVector = tri.vertices[0].VertexNormal + (tri.vertices[2].VertexNormal - tri.vertices[0].VertexNormal) * k;
			
		}
		else
		{
			nLeftX = (int)(tri.vertices[0].position.fX + k * (tri.vertices[2].position.fX - tri.vertices[0].position.fX) + 0.5f);
			nLeftY = startY;
			if (originPosition)
			{
				fLeftOriginX = originPosition[0].fX + k* (originPosition[2].fX - originPosition[0].fX);
				fLeftOriginY = originPosition[0].fY + k* (originPosition[2].fY - originPosition[0].fY);
			}
			fLeftZ = tri.vertices[0].position.fZ + k*(tri.vertices[2].position.fZ - tri.vertices[0].position.fZ);
			fLeftRHW = tri.vertices[0].rhw + k*(tri.vertices[2].rhw - tri.vertices[0].rhw);
			nRightX = (int)(tri.vertices[1].position.fX + k * (tri.vertices[2].position.fX - tri.vertices[1].position.fX) + 0.5f);
			nRightY = startY;
			if (originPosition)
			{
				fRightOriginX = originPosition[1].fX + k* (originPosition[2].fX - originPosition[1].fX);
				fRightOriginY = originPosition[1].fY + k* (originPosition[2].fY - originPosition[1].fY);
			}
			fRightZ = tri.vertices[1].position.fZ + k*(tri.vertices[2].position.fZ - tri.vertices[1].position.fZ);;
			fRightRHW = tri.vertices[1].rhw + k*(tri.vertices[2].rhw - tri.vertices[1].rhw);
			scanlineRHWLength = fRightRHW - fLeftRHW;
			if ((tri.vertices[2].rhw > SCANACCURACY || tri.vertices[2].rhw< -SCANACCURACY || tri.vertices[0].rhw >SCANACCURACY || tri.vertices[0].rhw < -SCANACCURACY))
			{
				float deltaZ1 = (1 / tri.vertices[2].rhw) - (1 / tri.vertices[0].rhw);
				if ((scanlineRHWLength > SCANACCURACY || scanlineRHWLength < -SCANACCURACY) && (deltaZ1 > SCANACCURACY || deltaZ1 < -SCANACCURACY))
					k = (1 / fLeftRHW - 1 / tri.vertices[0].rhw) / deltaZ1;
				else
					k = (startY - tri.vertices[0].position.fY) / (tri.vertices[2].position.fY - tri.vertices[0].position.fY);
			}
			else
				k = (startY - tri.vertices[0].position.fY) / (tri.vertices[2].position.fY - tri.vertices[0].position.fY);
			if (k < SCANACCURACY && k> -SCANACCURACY)
			{
				k = (startY - tri.vertices[0].position.fY) / (tri.vertices[2].position.fY - tri.vertices[0].position.fY);
			}
			if (type == RenderType::TEXTURE)
			{
				fLeftU = tri.vertices[0].tex_u + k * (tri.vertices[2].tex_u - tri.vertices[0].tex_u);
				fLeftV = tri.vertices[0].tex_v + k * (tri.vertices[2].tex_v - tri.vertices[0].tex_v);
			}
			LeftColor = colorInterpolate(tri.vertices[0].color, tri.vertices[2].color, k);
			if (originPosition)
				LeftNormalVector = tri.vertices[0].VertexNormal + (tri.vertices[2].VertexNormal - tri.vertices[0].VertexNormal) * k;
			
			LeftOriginPOS.fX = nLeftX;
			LeftOriginPOS.fY = nLeftY;
			LeftOriginPOS.fZ = 1.0f / fLeftRHW;
			LeftOriginPOS.fW = 1.0f;

			if ((tri.vertices[2].rhw > SCANACCURACY || tri.vertices[2].rhw< -SCANACCURACY || tri.vertices[1].rhw >SCANACCURACY || tri.vertices[1].rhw < -SCANACCURACY))
			{
				float deltaZ2 = (1 / tri.vertices[2].rhw) - (1 / tri.vertices[1].rhw);
				if ((scanlineRHWLength > SCANACCURACY || scanlineRHWLength < -SCANACCURACY) && (deltaZ2 > SCANACCURACY || deltaZ2 < -SCANACCURACY))
					k = ((1 / fRightRHW) - (1 / tri.vertices[1].rhw)) / deltaZ2;
				else
					k = (startY - tri.vertices[0].position.fY) / (tri.vertices[2].position.fY - tri.vertices[0].position.fY);
			}
			else
				k = (startY - tri.vertices[0].position.fY) / (tri.vertices[2].position.fY - tri.vertices[0].position.fY);
			if (k < SCANACCURACY && k > -SCANACCURACY)
			{
				k = (startY - tri.vertices[0].position.fY) / (tri.vertices[2].position.fY - tri.vertices[0].position.fY);
			}
			if (type == RenderType::TEXTURE)
			{
				fRightU = tri.vertices[1].tex_u + k * (tri.vertices[2].tex_u - tri.vertices[1].tex_u);
				fRightV = tri.vertices[1].tex_v + k * (tri.vertices[2].tex_v - tri.vertices[1].tex_v);
			}
			RightColor = colorInterpolate(tri.vertices[1].color, tri.vertices[2].color, k);
			if (originPosition)
				RightNormalVector = tri.vertices[1].VertexNormal + (tri.vertices[2].VertexNormal - tri.vertices[1].VertexNormal) * k;
			
		}
		if (nLeftX == nRightX)
		{
			if (nLeftX < 0 || nLeftX >= WIDTH || nLeftY < 0 || nLeftY >= HEIGHT)
				continue;
			if (fLeftZ < (m_zBuffer + nLeftY*WIDTH)[nLeftX])
			{
				//Depth Pass,we update the frameBuffer
				(m_zBuffer + nLeftY * WIDTH)[nLeftX] = fLeftZ;
				if (type == RenderType::COLOR)
				{
					if (originPosition && m_bLightingOpen)
						(m_frameBuffer + nLeftY * WIDTH)[nLeftX] = CalculateLighting(tri.material, LeftOriginPOS, LeftNormalVector, m_mainCamera->position, LeftColor, m_lights, m_lightCount, m_bSpecularOpen);
					else
						(m_frameBuffer + nLeftY * WIDTH)[nLeftX] = LeftColor;
				}
				if (type == RenderType::TEXTURE)
					if (originPosition && m_bLightingOpen)
						(m_frameBuffer + nLeftY * WIDTH)[nLeftX] = CalculateLighting(tri.material, LeftOriginPOS, LeftNormalVector, m_mainCamera->position, GetTextureColor(fLeftU, fLeftV), m_lights, m_lightCount, m_bSpecularOpen);
					else if(m_bLightingOpen)
						(m_frameBuffer + nLeftY * WIDTH)[nLeftX] = ColorCross(GetTextureColor(fLeftU, fLeftV),LeftColor);
					else
						(m_frameBuffer + nLeftY * WIDTH)[nLeftX] = GetTextureColor(fLeftU, fLeftV);
			}
			continue;
		}
		double rhdeltaX = 1.0 / (nRightX - nLeftX);
		double RHWStep = scanlineRHWLength * rhdeltaX;
		double currentRHW = fLeftRHW;
		Vector4 currentPosition;
		Vector4 currentNormal;
		float _k;
		float u, v;
		//from LeftPoint to RightPoint,we calculate every point to update the frameBuffer
		for (int startX = nLeftX; startX <= nRightX; startX++, currentRHW += RHWStep)
		{
			_k = (startX - nLeftX) / (float)(nRightX - nLeftX);
			if (originPosition)
			{
				float OriginX = fLeftOriginX + _k * (fRightOriginX - fLeftOriginX);
				float OriginY = fLeftOriginY + _k * (fRightOriginY - fLeftOriginY);
				float OriginZ = 1.0f / currentRHW;
				currentPosition.fX = OriginX;
				currentPosition.fY = OriginY;
				currentPosition.fZ = OriginZ;
				currentPosition.fW = 1.0f;
			}
			float z = _k*(fRightZ - fLeftZ) + fLeftZ;
			if ((currentRHW > SCANACCURACY || currentRHW < -SCANACCURACY) && (scanlineRHWLength > SCANACCURACY || scanlineRHWLength < -SCANACCURACY))
			{
				double currentZ = 1.0f / currentRHW;
				if ((fRightRHW > SCANACCURACY || fRightRHW < -SCANACCURACY) && (fLeftRHW > SCANACCURACY || fLeftRHW < -SCANACCURACY))
				{
					float deltaZ = (1 / fRightRHW) - (1 / fLeftRHW);
					if (deltaZ > SCANACCURACY || deltaZ < -SCANACCURACY)
						_k = (currentZ - (1 / fLeftRHW)) / deltaZ;
				}
			}
			if (originPosition)
			{
				currentNormal = LeftNormalVector + (RightNormalVector - LeftNormalVector)*_k;
				currentNormal.ResetUnitVector();
			}
			INT32 currentColor = colorInterpolate(LeftColor, RightColor, _k);
			if (type == RenderType::TEXTURE)
			{
				u = fLeftU + _k*(fRightU - fLeftU);
				v = fLeftV + _k*(fRightV - fLeftV);
			}
			//Depth Test
			int X = startX;
			int Y = startY;
			if (X < nLeftX || X > nRightX)
				continue;
			if (z < (m_zBuffer + Y * WIDTH)[X])
			{
				//Depth Pass,we update the frameBuffer
				(m_zBuffer + Y * WIDTH)[X] = z;
				if (type == RenderType::COLOR)
				{
					if(originPosition && m_bLightingOpen)
						(m_frameBuffer + Y * WIDTH)[X] = CalculateLighting(tri.material, currentPosition, currentNormal, m_mainCamera->position, currentColor, m_lights, m_lightCount, m_bSpecularOpen);
					else
						(m_frameBuffer + Y * WIDTH)[X] = currentColor;
				}
				if (type == RenderType::TEXTURE)
				{
					if (originPosition && m_bLightingOpen)
						(m_frameBuffer + Y * WIDTH)[X] = CalculateLighting(tri.material, currentPosition, currentNormal, m_mainCamera->position, GetTextureColor(u, v), m_lights, m_lightCount, m_bSpecularOpen);
					else if(m_bLightingOpen)
						(m_frameBuffer + Y * WIDTH)[X] = ColorCross(GetTextureColor(u, v),currentColor);
					else
						(m_frameBuffer + Y * WIDTH)[X] = GetTextureColor(u, v);
				}
			}
		}
	}
}


VOID RenderDevice::DrawCube(Cube cube)
{
	for each  (Plane plane in cube.planes)
	{
		DrawTriangle(plane.t1.toTriangle());
		DrawTriangle(plane.t2.toTriangle());
	}
}