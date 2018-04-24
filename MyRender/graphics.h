#pragma once
#include "Math.h"

enum LightType
{
	UNDEFINED = -1,
	POINTLIGHT = 0,
	DIRECTIONLIGHT = 1
};

class Light
{
public:
	Light();
	Light(LightType type, Vector4 position, Vector4 direction, INT32 Diffuse, INT32 Ambient, INT32 Specular);
	LightType type;
	Vector4 position;
	Vector4 direction;
	INT32 Diffuse;
	INT32 Ambient;
	INT32 Specular;
};

class Material
{
public:
	INT32 Ambient;		//环境光反射率
	INT32 Diffuse;		//漫反射光反射率
	INT32 Specular;		//镜面光反射率
	INT32 Emissive;		//自发光
	float Power;		//镜面反射系数
};

//All triangle in MyRender should be AntiClockWise order.
//It'll be used for backface test.
enum TriangleType 
{
	UP = 0,			//up triangle is like this:
					//			/\
					//		   /  \
					//		  /    \
					//		 /		\
					//		----------
	DOWN = 1,		//down triangle is opposite of up triangle
	LEFT = 2,		//left triangle is like this
					//			/\
					//		   /  \
					//		  /	   \
					//		 /
					//		/
	RIGHT = 3		//right triangle is opposite of left triangle
};

class triangle {
public:
	Vector4 normalVector;	//面法向量
	Point3D vertices[3];	//三个顶点
	Point3D VSplitPoint;	//分割点
	TriangleType type;		//三角形类型
	Material material;		//三角形材质

	triangle(Point3D vertices[3]);
	triangle(Point3D p1,Point3D p2,Point3D p3);
	triangle() {};
	void operator=(triangle&);
	void getTriangleType(Vector4* originVector = NULL);
	void setPlaneNormalVector();
	bool faceBackTest(Vector4 cameraPosition);
};

class IndexTriangle {
public:
	Vector4 normalVector;	//面法向量
	Point3D* vertices;		//顶点数组指针
	int index[3];			//三个顶点索引
	Point3D VSplitPoint;	//分割点
	TriangleType type;		//三角形类型
	Material material;		//三角形材质

	IndexTriangle(Point3D* vertices, int p1, int p2, int p3);
	IndexTriangle() { vertices = NULL; };
	triangle toTriangle();
	void setPlaneNormalVector();
};

class Plane
{
public:
	IndexTriangle t1;
	IndexTriangle t2;
	Vector4 normalVector;
	int indexs[4];
	Plane(IndexTriangle t1, IndexTriangle t2, int p1, int p2, int p3, int p4) : t1(t1), t2(t2) { indexs[0] = p1, indexs[1] = p2, indexs[2] = p3, indexs[3] = p4; normalVector = Vector4(); }
	Plane() {}
	bool IsIndexExist(int index);
};

class Cube 
{
public:
	Cube(Point3D** vertices);
	void SetMaterial(Material mtrl);
	Point3D vertices[8];
	Plane planes[6];
	Material material;
};

INT32 colorInterpolate(INT32 startColor, INT32 endColor, float k);
INT32 calculateLighting(Material material, Vector4 position, Vector4 normalVector, Vector4 cameraPosition, INT32 baseColor, Light* lights, int lightCount,bool SpecularOpen);
INT32 ColorAdd(INT32 src, int r, int g, int b);
INT32 ColorCross(INT32 color1, INT32 color2);
