#pragma once
#include "graphics.h"

#define InitPositionX 0
#define InitPositionY 0
#define WIDTH		800
#define HEIGHT		600
#define MAX_LIGHTCOUNTS 8		// 最大光源个数

enum RenderType {
	WIREFRAME,	//线框模式
	TEXTURE,	//纹理模式
	COLOR,		//颜色模式
};

enum LightRenderMode
{
	GROUND
};

static class RenderDevice {
private:
	static RenderDevice * m_instance;

	int m_frameWidth;
	int m_frameHeight;
	float* m_zBuffer;
	INT32 pointColor;
	INT32 backgroundColor;
	INT32* m_frameBuffer;
	INT32* m_textureBuffer;

	RenderType m_renderType;
	RenderDevice(int width, int height, RenderType renderType = RenderType::WIREFRAME);
public:
	int m_lightCount;
	int m_textureWidth;
	int m_textureHeight;

	Camera* m_mainCamera;
	Matrix4 m_projMatrix;
	Matrix4 m_viewportMatrix;
	Matrix4 m_viewMatrix;
    Light*	m_lights;
	
	bool m_backFaceTestOpen;
	bool m_lightingOpen;
	bool m_SpecularOpen;
	bool m_PixelBasedShadingOpen;
	bool m_fpsLock;
	bool m_cameraMatrixChange;

	~RenderDevice();
	VOID Init();
	VOID SetPointColor(INT32 Color);
	VOID SetBackGroundColor(INT32 backgroundColor);
	VOID SetRenderType(RenderType renderType);
	RenderType GetRenderType();
	INT32 GetPointColor();
	INT32 GetTextureColor(float u, float v);
	BOOL InitFrameBuffer(int width, int length);
	BOOL ClearFrameBuffer();
	BOOL InitTextureBuffer(int width,int height);
	BOOL ClearTextureBuffer();
	VOID UpdateGraphics(HWND hwnd);
	static RenderDevice* GetRenderDevice(int width = WIDTH, int height = HEIGHT, RenderType _renderType = WIREFRAME);
	VOID DrawTest();
	VOID DrawLine(Point2D start,Point2D end);
	BOOL IsPointOutRange(Point2D p);
	VOID DrawTriangle(triangle tri);
	VOID DrawStandardFilledTriangle(triangle tri,RenderType type,Vector4 originPosition[3] = NULL);
	VOID DrawCube(Cube cube);
};