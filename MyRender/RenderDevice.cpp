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
	m_renderType = renderType;
}

RenderType RenderDevice::GetRenderType()
{
	return m_renderType;
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
		for (int i = 0; i < HEIGHT; i++)
		{
			for (int j = 0; j < WIDTH; j++)
				(m_zBuffer + i*WIDTH)[j] = 1.0f;
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
	strSpecularOpen = (m_SpecularOpen)?"SpecularOpen(Z): true":"SpecularOpen(Z): false";
	strPixelShading = (m_PixelBasedShadingOpen)?"PixelShadingOpen(X): true": "PixelShadingOpen(X): false";
	strLightingOpen = (m_lightingOpen)?"LightingOpen(C): true": "LightingOpen(C): false";
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
	m_renderType = _renderType;
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
	m_backFaceTestOpen = true;
	m_lightingOpen = true;
	m_lightCount = 0;
	m_SpecularOpen = false;
	m_PixelBasedShadingOpen = false;
	m_fpsLock = true;
	m_cameraMatrixChange = false;
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
		{
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
}

VOID RenderDevice::DrawLine(Point2D start,Point2D end)
{
	if (m_frameBuffer)
	{
		if (IsPointOutRange(start) || IsPointOutRange(end))
		{
			cout << "Point OutOfRange!!";
			return;
		}
		int step;
		if (start.x == end.x)
		{
			step = (start.y > end.y) ? -1 : 1;
			for (int i = start.y; i != end.y; i += step)
			{
				(m_frameBuffer + i * WIDTH)[start.x] = pointColor;
			}
			return;
		}
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
	Vector4 originVertices[3] = 
	{
		tri.vertices[0].position,
		tri.vertices[1].position,
		tri.vertices[2].position
	};
	triangle _tri = tri;
	_tri.vertices[0].position *= m_viewMatrix;
	_tri.vertices[1].position *= m_viewMatrix;
	_tri.vertices[2].position *= m_viewMatrix;

	//BackFace Test
	if (m_backFaceTestOpen)
	{
		if (m_renderType != RenderType::WIREFRAME)
		{
			_tri.setPlaneNormalVector();
			if (!_tri.faceBackTest(m_mainCamera->position * m_viewMatrix))
				return;
		}
	}

	_tri.vertices[0].position *= m_projMatrix;
	_tri.vertices[1].position *= m_projMatrix;
	_tri.vertices[2].position *= m_projMatrix;
	_tri.vertices[0].rhw = 1 / _tri.vertices[0].position.fW;
	_tri.vertices[1].rhw = 1 / _tri.vertices[1].position.fW;
	_tri.vertices[2].rhw = 1 / _tri.vertices[2].position.fW;
	_tri.vertices[0].position.Normalize();
	_tri.vertices[1].position.Normalize();
	_tri.vertices[2].position.Normalize();
	//clip these outside triangle
	int index = 0;
	while (index < 3)
	{
		if (_tri.vertices[index].position.fX < -1 ||
			_tri.vertices[index].position.fX > 1  ||
			_tri.vertices[index].position.fY < -1 ||
			_tri.vertices[index].position.fY > 1  ||
			_tri.vertices[index].position.fZ < 0  ||
			_tri.vertices[index].position.fZ > 1)
			return;//this triangle is outside
		index++;
	}
	//LightCalculate
	if (m_renderType != RenderType::WIREFRAME && !m_PixelBasedShadingOpen)
	{
		if (m_lightingOpen)
		{
			if (m_renderType == RenderType::COLOR)
			{
				_tri.vertices[0].color = calculateLighting(_tri.material, originVertices[0], _tri.normalVector, m_mainCamera->position, _tri.vertices[0].color,
					m_lights, m_lightCount,m_SpecularOpen);
				_tri.vertices[1].color = calculateLighting(_tri.material, originVertices[1], _tri.normalVector, m_mainCamera->position, _tri.vertices[1].color,
					m_lights, m_lightCount, m_SpecularOpen);
				_tri.vertices[2].color = calculateLighting(_tri.material, originVertices[2], _tri.normalVector, m_mainCamera->position, _tri.vertices[2].color,
					m_lights, m_lightCount, m_SpecularOpen);
			}
			else if (m_renderType == RenderType::TEXTURE)
			{
				_tri.vertices[0].color = calculateLighting(_tri.material, originVertices[0], _tri.normalVector, m_mainCamera->position, 0x00FFFFFF,
					m_lights, m_lightCount, m_SpecularOpen);
				_tri.vertices[1].color = calculateLighting(_tri.material, originVertices[1], _tri.normalVector, m_mainCamera->position, 0x00FFFFFF,
					m_lights, m_lightCount, m_SpecularOpen);
				_tri.vertices[2].color = calculateLighting(_tri.material, originVertices[2], _tri.normalVector, m_mainCamera->position, 0x00FFFFFF,
					m_lights, m_lightCount, m_SpecularOpen);
			}
		}
	}

	_tri.vertices[0].position *= m_viewportMatrix;
	_tri.vertices[1].position *= m_viewportMatrix;
	_tri.vertices[2].position *= m_viewportMatrix;
	switch (m_renderType)
	{
	case RenderType::WIREFRAME:
		DrawLine(Point2D(_tri.vertices[0].position.fX, _tri.vertices[0].position.fY), Point2D(_tri.vertices[1].position.fX, _tri.vertices[1].position.fY));
		DrawLine(Point2D(_tri.vertices[1].position.fX, _tri.vertices[1].position.fY), Point2D(_tri.vertices[2].position.fX, _tri.vertices[2].position.fY));
		DrawLine(Point2D(_tri.vertices[2].position.fX, _tri.vertices[2].position.fY), Point2D(_tri.vertices[0].position.fX, _tri.vertices[0].position.fY));
		break;
	case RenderType::TEXTURE:
	case RenderType::COLOR:
	{
		_tri.getTriangleType(originVertices);
		Vector4 splitOriginPOS;
		if (_tri.type == TriangleType::UP || _tri.type == TriangleType::DOWN)
		{
			if(m_PixelBasedShadingOpen)
				DrawStandardFilledTriangle(_tri, m_renderType, originVertices);
			else
				DrawStandardFilledTriangle(_tri, m_renderType);
			break;
		}
		else
		{
			float k = (_tri.vertices[1].position.fY - _tri.vertices[0].position.fY) / (_tri.vertices[2].position.fY - _tri.vertices[0].position.fY);
			splitOriginPOS = originVertices[0] + (originVertices[2] - originVertices[0])* k;
			if (_tri.type == TriangleType::LEFT)
			{
				triangle upSubTriangle = triangle(_tri.vertices[0], _tri.vertices[1], _tri.VSplitPoint);
				triangle downSubTriangle = triangle(_tri.VSplitPoint, _tri.vertices[1], _tri.vertices[2]);
				upSubTriangle.getTriangleType();
				downSubTriangle.getTriangleType();
				upSubTriangle.material = tri.material;
				downSubTriangle.material = tri.material;
				Vector4 OriginUpPOS[3] = { originVertices[0],splitOriginPOS,originVertices[1] };
				Vector4 OriginDownPOS[3] = { splitOriginPOS,originVertices[1],originVertices[2] };
				if (m_PixelBasedShadingOpen)
				{
					DrawStandardFilledTriangle(upSubTriangle, m_renderType, OriginUpPOS);
					DrawStandardFilledTriangle(downSubTriangle, m_renderType, OriginDownPOS);
				}
				else
				{
					DrawStandardFilledTriangle(upSubTriangle, m_renderType);
					DrawStandardFilledTriangle(downSubTriangle, m_renderType);
				}
			}
			else if (_tri.type == TriangleType::RIGHT)
			{
				triangle upSubTriangle = triangle(_tri.vertices[0], _tri.VSplitPoint, _tri.vertices[1]);
				triangle downSubTriangle = triangle(_tri.vertices[1], _tri.VSplitPoint, _tri.vertices[2]);
				upSubTriangle.getTriangleType();
				downSubTriangle.getTriangleType();
				upSubTriangle.material = tri.material;
				downSubTriangle.material = tri.material;
				Vector4 OriginUpPOS[3] = { originVertices[0],originVertices[1],splitOriginPOS };
				Vector4 OriginDownPOS[3] = { originVertices[1],splitOriginPOS,originVertices[2] };
				if (m_PixelBasedShadingOpen)
				{
					DrawStandardFilledTriangle(upSubTriangle, m_renderType, OriginUpPOS);
					DrawStandardFilledTriangle(downSubTriangle, m_renderType, OriginDownPOS);
				}
				else
				{
					DrawStandardFilledTriangle(upSubTriangle, m_renderType);
					DrawStandardFilledTriangle(downSubTriangle, m_renderType);
				}
			}
		}
	}
	break;
	}
}


VOID RenderDevice::DrawStandardFilledTriangle(triangle tri,RenderType type,Vector4 originPosition[3]) 
{
	if (tri.type == TriangleType::LEFT || tri.type == TriangleType::RIGHT)
		return;
	int LeftX, LeftY;
	float LeftZ, LeftRHW, LeftU, LeftV, LeftOriginX, LeftOriginY;
	int RightX, RightY;
	float RightZ, RightRHW, RightU, RightV, RightOriginX, RightOriginY;
	INT32 LeftColor, RightColor;
	Vector4 LeftNormalVector, RightNormalVector,LeftOriginPOS;
	double scanlineRHWLength;

	for (int startY = (int)(tri.vertices[0].position.fY); startY <= (int)(tri.vertices[2].position.fY + 0.5f); startY++)
	{
		float k = (startY - tri.vertices[0].position.fY) / (tri.vertices[2].position.fY - tri.vertices[0].position.fY);
		if (k > 1.00000001f || k < 0.0f)
			continue;
		if (tri.type == TriangleType::UP) {
			LeftX = (int)(tri.vertices[0].position.fX + k * (tri.vertices[1].position.fX - tri.vertices[0].position.fX) + 0.5f);
			LeftY = startY;
			if (originPosition)
			{
				LeftOriginX = originPosition[0].fX + k* (originPosition[1].fX - originPosition[0].fX);
				LeftOriginY = originPosition[0].fY + k* (originPosition[1].fY - originPosition[0].fY);
			}
			RightX = (int)(tri.vertices[0].position.fX + k * (tri.vertices[2].position.fX - tri.vertices[0].position.fX) + 0.5f);
			RightY = startY;
			if (originPosition)
			{
				RightOriginX = originPosition[0].fX + k* (originPosition[2].fX - originPosition[0].fX);
				RightOriginY = originPosition[0].fY + k* (originPosition[2].fY - originPosition[0].fY);
			}
			LeftZ = tri.vertices[0].position.fZ + k*(tri.vertices[1].position.fZ - tri.vertices[0].position.fZ);
			LeftRHW = tri.vertices[0].rhw + k*(tri.vertices[1].rhw - tri.vertices[0].rhw);
			RightZ = tri.vertices[0].position.fZ + k*(tri.vertices[2].position.fZ - tri.vertices[0].position.fZ);
			RightRHW = tri.vertices[0].rhw + k*(tri.vertices[2].rhw - tri.vertices[0].rhw);
			scanlineRHWLength = RightRHW - LeftRHW;
			if ((tri.vertices[1].rhw > SCANACCURACY || tri.vertices[1].rhw< -SCANACCURACY || tri.vertices[0].rhw >SCANACCURACY || tri.vertices[0].rhw < -SCANACCURACY))
			{
				float deltaZ1 = (1 / tri.vertices[1].rhw) - (1 / tri.vertices[0].rhw);
				if ((scanlineRHWLength > SCANACCURACY || scanlineRHWLength < -SCANACCURACY) && (deltaZ1 > SCANACCURACY || deltaZ1 < -SCANACCURACY))
					k = ((1 / LeftRHW) - (1 / tri.vertices[0].rhw)) / deltaZ1;
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
				LeftU = tri.vertices[0].tex_u + k * (tri.vertices[1].tex_u - tri.vertices[0].tex_u);
				LeftV = tri.vertices[0].tex_v + k * (tri.vertices[1].tex_v - tri.vertices[0].tex_v);
			}
			LeftColor = colorInterpolate(tri.vertices[0].color, tri.vertices[1].color, k);
			if (originPosition)
				LeftNormalVector = tri.vertices[0].VertexNormal + (tri.vertices[1].VertexNormal - tri.vertices[0].VertexNormal) * k;

			LeftOriginPOS.fX = LeftX;
			LeftOriginPOS.fY = LeftY;
			LeftOriginPOS.fZ = 1.0f / LeftRHW;
			LeftOriginPOS.fW = 1.0f;

			if ((tri.vertices[2].rhw > SCANACCURACY || tri.vertices[2].rhw< -SCANACCURACY || tri.vertices[0].rhw >SCANACCURACY || tri.vertices[0].rhw < -SCANACCURACY))
			{
				float deltaZ2 = (1 / tri.vertices[2].rhw) - (1 / tri.vertices[0].rhw);
				if ((scanlineRHWLength > SCANACCURACY || scanlineRHWLength < -SCANACCURACY) && (deltaZ2 > SCANACCURACY || deltaZ2 < -SCANACCURACY))
					k = ((1 / RightRHW) - (1 / tri.vertices[0].rhw)) / deltaZ2;
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
				RightU = tri.vertices[0].tex_u + k * (tri.vertices[2].tex_u - tri.vertices[0].tex_u);
				RightV = tri.vertices[0].tex_v + k * (tri.vertices[2].tex_v - tri.vertices[0].tex_v);
			}
			RightColor = colorInterpolate(tri.vertices[0].color, tri.vertices[2].color, k);
			if(originPosition)
				RightNormalVector = tri.vertices[0].VertexNormal + (tri.vertices[2].VertexNormal - tri.vertices[0].VertexNormal) * k;
			
		}
		else
		{
			LeftX = (int)(tri.vertices[0].position.fX + k * (tri.vertices[2].position.fX - tri.vertices[0].position.fX) + 0.5f);
			LeftY = startY;
			if (originPosition)
			{
				LeftOriginX = originPosition[0].fX + k* (originPosition[2].fX - originPosition[0].fX);
				LeftOriginY = originPosition[0].fY + k* (originPosition[2].fY - originPosition[0].fY);
			}
			LeftZ = tri.vertices[0].position.fZ + k*(tri.vertices[2].position.fZ - tri.vertices[0].position.fZ);
			LeftRHW = tri.vertices[0].rhw + k*(tri.vertices[2].rhw - tri.vertices[0].rhw);
			RightX = (int)(tri.vertices[1].position.fX + k * (tri.vertices[2].position.fX - tri.vertices[1].position.fX) + 0.5f);
			RightY = startY;
			if (originPosition)
			{
				RightOriginX = originPosition[1].fX + k* (originPosition[2].fX - originPosition[1].fX);
				RightOriginY = originPosition[1].fY + k* (originPosition[2].fY - originPosition[1].fY);
			}
			RightZ = tri.vertices[1].position.fZ + k*(tri.vertices[2].position.fZ - tri.vertices[1].position.fZ);;
			RightRHW = tri.vertices[1].rhw + k*(tri.vertices[2].rhw - tri.vertices[1].rhw);
			scanlineRHWLength = RightRHW - LeftRHW;
			if ((tri.vertices[2].rhw > SCANACCURACY || tri.vertices[2].rhw< -SCANACCURACY || tri.vertices[0].rhw >SCANACCURACY || tri.vertices[0].rhw < -SCANACCURACY))
			{
				float deltaZ1 = (1 / tri.vertices[2].rhw) - (1 / tri.vertices[0].rhw);
				if ((scanlineRHWLength > SCANACCURACY || scanlineRHWLength < -SCANACCURACY) && (deltaZ1 > SCANACCURACY || deltaZ1 < -SCANACCURACY))
					k = (1 / LeftRHW - 1 / tri.vertices[0].rhw) / deltaZ1;
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
				LeftU = tri.vertices[0].tex_u + k * (tri.vertices[2].tex_u - tri.vertices[0].tex_u);
				LeftV = tri.vertices[0].tex_v + k * (tri.vertices[2].tex_v - tri.vertices[0].tex_v);
			}
			LeftColor = colorInterpolate(tri.vertices[0].color, tri.vertices[2].color, k);
			if (originPosition)
				LeftNormalVector = tri.vertices[0].VertexNormal + (tri.vertices[2].VertexNormal - tri.vertices[0].VertexNormal) * k;
			
			LeftOriginPOS.fX = LeftX;
			LeftOriginPOS.fY = LeftY;
			LeftOriginPOS.fZ = 1.0f / LeftRHW;
			LeftOriginPOS.fW = 1.0f;

			if ((tri.vertices[2].rhw > SCANACCURACY || tri.vertices[2].rhw< -SCANACCURACY || tri.vertices[1].rhw >SCANACCURACY || tri.vertices[1].rhw < -SCANACCURACY))
			{
				float deltaZ2 = (1 / tri.vertices[2].rhw) - (1 / tri.vertices[1].rhw);
				if ((scanlineRHWLength > SCANACCURACY || scanlineRHWLength < -SCANACCURACY) && (deltaZ2 > SCANACCURACY || deltaZ2 < -SCANACCURACY))
					k = ((1 / RightRHW) - (1 / tri.vertices[1].rhw)) / deltaZ2;
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
				RightU = tri.vertices[1].tex_u + k * (tri.vertices[2].tex_u - tri.vertices[1].tex_u);
				RightV = tri.vertices[1].tex_v + k * (tri.vertices[2].tex_v - tri.vertices[1].tex_v);
			}
			RightColor = colorInterpolate(tri.vertices[1].color, tri.vertices[2].color, k);
			if (originPosition)
				RightNormalVector = tri.vertices[1].VertexNormal + (tri.vertices[2].VertexNormal - tri.vertices[1].VertexNormal) * k;
			
		}
		if (LeftX == RightX)
		{
			if (LeftX < 0 || LeftX >= WIDTH || LeftY < 0 || LeftY >= HEIGHT)
				continue;
			if (LeftZ < (m_zBuffer + LeftY*WIDTH)[LeftX])
			{
				//Depth Pass,we update the frameBuffer
				(m_zBuffer + LeftY * WIDTH)[LeftX] = LeftZ;
				if (type == RenderType::COLOR)
				{
					if (originPosition && m_lightingOpen)
						(m_frameBuffer + LeftY * WIDTH)[LeftX] = calculateLighting(tri.material, LeftOriginPOS, LeftNormalVector, m_mainCamera->position, LeftColor, m_lights, m_lightCount, m_SpecularOpen);
					else
						(m_frameBuffer + LeftY * WIDTH)[LeftX] = LeftColor;
				}
				if (type == RenderType::TEXTURE)
					if (originPosition && m_lightingOpen)
						(m_frameBuffer + LeftY * WIDTH)[LeftX] = calculateLighting(tri.material, LeftOriginPOS, LeftNormalVector, m_mainCamera->position, GetTextureColor(LeftU, LeftV), m_lights, m_lightCount, m_SpecularOpen);
					else if(m_lightingOpen)
						(m_frameBuffer + LeftY * WIDTH)[LeftX] = ColorCross(GetTextureColor(LeftU, LeftV),LeftColor);
					else
						(m_frameBuffer + LeftY * WIDTH)[LeftX] = GetTextureColor(LeftU, LeftV);
			}
			continue;
		}
		double rhdeltaX = 1.0 / (RightX - LeftX);
		double RHWStep = scanlineRHWLength * rhdeltaX;
		double currentRHW = LeftRHW;
		Vector4 currentPosition;
		Vector4 currentNormal;
		float _k;
		float u, v;
		//from LeftPoint to RightPoint,we calculate every point to update the frameBuffer
		for (int startX = LeftX; startX <= RightX; startX++, currentRHW += RHWStep)
		{
			_k = (startX - LeftX) / (float)(RightX - LeftX);
			if (originPosition)
			{
				float OriginX = LeftOriginX + _k * (RightOriginX - LeftOriginX);
				float OriginY = LeftOriginY + _k * (RightOriginY - LeftOriginY);
				float OriginZ = 1.0f / currentRHW;
				currentPosition.fX = OriginX;
				currentPosition.fY = OriginY;
				currentPosition.fZ = OriginZ;
				currentPosition.fW = 1.0f;
			}
			float z = _k*(RightZ - LeftZ) + LeftZ;
			if ((currentRHW > SCANACCURACY || currentRHW < -SCANACCURACY) && (scanlineRHWLength > SCANACCURACY || scanlineRHWLength < -SCANACCURACY))
			{
				double currentZ = 1.0f / currentRHW;
				if ((RightRHW > SCANACCURACY || RightRHW < -SCANACCURACY) && (LeftRHW > SCANACCURACY || LeftRHW < -SCANACCURACY))
				{
					float deltaZ = (1 / RightRHW) - (1 / LeftRHW);
					if (deltaZ > SCANACCURACY || deltaZ < -SCANACCURACY)
						_k = (currentZ - (1 / LeftRHW)) / deltaZ;
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
				u = LeftU + _k*(RightU - LeftU);
				v = LeftV + _k*(RightV - LeftV);
			}
			//Depth Test
			int X = startX;
			int Y = startY;
			if (X < LeftX || X > RightX)
				continue;
			if (z < (m_zBuffer + Y * WIDTH)[X])
			{
				//Depth Pass,we update the frameBuffer
				(m_zBuffer + Y * WIDTH)[X] = z;
				if (type == RenderType::COLOR)
				{
					if(originPosition && m_lightingOpen)
						(m_frameBuffer + Y * WIDTH)[X] = calculateLighting(tri.material, currentPosition, currentNormal, m_mainCamera->position, currentColor, m_lights, m_lightCount, m_SpecularOpen);
					else
						(m_frameBuffer + Y * WIDTH)[X] = currentColor;
				}
				if (type == RenderType::TEXTURE)
				{
					if (originPosition && m_lightingOpen)
						(m_frameBuffer + Y * WIDTH)[X] = calculateLighting(tri.material, currentPosition, currentNormal, m_mainCamera->position, GetTextureColor(u, v), m_lights, m_lightCount, m_SpecularOpen);
					else if(m_lightingOpen)
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