#pragma once
#include <iostream>
#include <windows.h>

/*
In MyRender, we: 
	1)use line Vector 
	2)Matrix must be in right position when a vector multiply with matrix.
*/
class Point2D
{
public:
	int x;
	int y;
	Point2D(int x, int y) :x(x), y(y) {}
};

class Matrix4 //4*4 matrix
{
public:
	float M[4][4];
	Matrix4 operator*(Matrix4);
	Matrix4(float Array[4][4]);
	Matrix4();
};

class Vector4
{
public:
	float fX;
	float fY;
	float fZ;
	float fW;

	Vector4&	operator*=(float);
	Vector4&	operator=(Vector4&);
	Vector4		operator*(Matrix4);
	Vector4&	operator*=(Matrix4);
	Vector4&	operator+=(Vector4);
	Vector4&	operator/=(float);
	float		operator*(Vector4);
	Vector4		operator*(float);
	Vector4		operator+(Vector4);
	Vector4		operator-(Vector4);
	Vector4		operator-();

	Vector4 Cross(Vector4);
	void Normalize();				// reset (x,y,z,w) = (x/w,y/w,z/w,1.0f)
	void ResetUnitVector();
};

class Point3D
{
public:
	Vector4 position;
	Vector4 VertexNormal;
	INT32	color;

	float rhw;
	float tex_u;
	float tex_v;

	void operator=(Point3D&);

	Point3D(float x, float y, float z) { position.fX = x; position.fY = y; position.fZ = z; position.fW = 1; tex_u = 0.0f; tex_v = 0.0f; VertexNormal.fX = 0; VertexNormal.fY = 0; VertexNormal.fZ = 0; VertexNormal.fW = 0; }
	Point3D(float x, float y, float z, INT32 _color) { position.fX = x; position.fY = y; position.fZ = z; position.fW = 1; color = _color; tex_u = 0.0f; tex_v = 0.0f; VertexNormal.fX = 0; VertexNormal.fY = 0; VertexNormal.fZ = 0; VertexNormal.fW = 0;}
	Point3D(float x, float y, float z,float u,float v,INT32 _color) { position.fX = x; position.fY = y; position.fZ = z; position.fW = 1; color = _color; tex_u = u; tex_v = v; VertexNormal.fX = 0; VertexNormal.fY = 0; VertexNormal.fZ = 0; VertexNormal.fW = 0;}
	Point3D() { VertexNormal.fX = 0; VertexNormal.fY = 0; VertexNormal.fZ = 0; VertexNormal.fW = 0; }
};

class Camera
{
public:
	Vector4 position;
	Vector4 up;
	Vector4 look;
	Vector4 right;
	Matrix4& getViewTransformMatrix();
	Camera(Vector4 pos, Vector4 up, Vector4 look, Vector4 right) :position(pos), up(up), look(look), right(right) {}
	Matrix4 RotateAroundArbitraryAxis(Vector4 axis,float theta);
};

Matrix4& getPerspectiveProjMatrix(float theta,float zn,float zf,float widthHeightScale);

Matrix4& getViewPortMatrix(float x, float y, float width, float height,float minZ,float maxZ);

int lineInterpolate(int start, int end, float k);