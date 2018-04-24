#include "Math.h"
#include <math.h>

int lineInterpolate(int start, int end, float k)
{
	return (int)(start + k*(end - start) + 0.5f);
}

void Vector4::ResetUnitVector()
{
	if (fW > 1e-6 || fW < -1e-6)
		return;
	float length = fX*fX + fY*fY + fZ*fZ;
	if (length > 1e-6)
	{
		length = sqrtf(length);
		fX = fX / length;
		fY = fY / length;
		fZ = fZ / length;
	}
}

void Vector4::Normalize() 
{
	fX = fX / fW;
	fY = fY / fW;
	fZ = fZ / fW;
	fW = 1.0;
}

Vector4 Vector4::Cross(Vector4 _V)
{
	// only use for direction vector
	Vector4 result;
	result.fW = 0;
	result.fX = fY*_V.fZ - fZ*_V.fY;
	result.fY = fZ*_V.fX - fX*_V.fZ;
	result.fZ = fX*_V.fY - fY*_V.fX;
	return result;
}

Vector4 & Vector4::operator*=(float f)
{
	fX *= f;
	fY *= f;
	fZ *= f;
	return *this;
}

Vector4 & Vector4::operator=(Vector4& v)
{
	fX = v.fX;
	fY = v.fY;
	fZ = v.fZ;
	fW = v.fW;
	return *this;
}

Vector4 Vector4::operator*(Matrix4 _M)
{
	Vector4 result;
	result.fX = fX*_M.M[0][0] + fY*_M.M[1][0] + fZ*_M.M[2][0] + fW*_M.M[3][0];
	result.fY = fX*_M.M[0][1] + fY*_M.M[1][1] + fZ*_M.M[2][1] + fW*_M.M[3][1];
	result.fZ = fX*_M.M[0][2] + fY*_M.M[1][2] + fZ*_M.M[2][2] + fW*_M.M[3][2];
	result.fW = fX*_M.M[0][3] + fY*_M.M[1][3] + fZ*_M.M[2][3] + fW*_M.M[3][3];
	return result;
}

Vector4 & Vector4::operator*=(Matrix4 _M)
{
	float _x = fX, _y = fY, _z = fZ, _w = fW;
	fX = _x*_M.M[0][0] + _y*_M.M[1][0] + _z*_M.M[2][0] + _w*_M.M[3][0];
	fY = _x*_M.M[0][1] + _y*_M.M[1][1] + _z*_M.M[2][1] + _w*_M.M[3][1];
	fZ = _x*_M.M[0][2] + _y*_M.M[1][2] + _z*_M.M[2][2] + _w*_M.M[3][2];
	fW = _x*_M.M[0][3] + _y*_M.M[1][3] + _z*_M.M[2][3] + _w*_M.M[3][3];
	return *(this);
}

Vector4 & Vector4::operator+=(Vector4 v)
{
	fX += v.fX;
	fY += v.fY;
	fZ += v.fZ;
	fW += v.fW;
	return *this;
}

Vector4 & Vector4::operator/=(float f)
{
	float _f = 1.0f / f;
	fX *= _f;
	fY *= _f; 
	fZ *= _f;
	fW *= _f;
	return *this;

}

Vector4 Vector4::operator*(float f)
{
	Vector4 result;
	result.fX = fX * f;
	result.fY = fY * f;
	result.fZ = fZ * f;
	result.fW = fW;
	return result;
}

Vector4 Vector4::operator+(Vector4 v)
{
	Vector4 result;
	result.fX = fX + v.fX;
	result.fY = fY + v.fY;
	result.fZ = fZ + v.fZ;
	if (v.fW < 1e-6 && v.fW > -1e-6)
		result.fW = 0.0f;
	else
		result.fW = 1.0f;
	return result;
}

float Vector4::operator*(Vector4 _V)
{
	if(fW>0.00000001||fW<-0.00000001)
		Normalize();
	if(_V.fW>0.00000001 || _V.fW<-0.00000001)
		_V.Normalize();
	return (fX*_V.fX + fY*_V.fY + fZ*_V.fZ);
}

Vector4 Vector4::operator- ( Vector4 v)
{
	Vector4 result;
	//ResetUnitVector();
	//v.ResetUnitVector();
	result.fX = fX - v.fX;
	result.fY = fY - v.fY;
	result.fZ = fZ - v.fZ;
	result.fW = 0;
	return result;
}

Vector4 Vector4::operator-()
{
	Vector4 result = Vector4();
	result.fX = -fX;
	result.fY = -fY;
	result.fZ = -fZ;
	return result;
}

Matrix4 Matrix4::operator*(Matrix4 _M) 
{
	float resultArray[4][4] = { 0 };
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++) 
		{
			resultArray[i][j] = M[i][0] * _M.M[0][j] + M[i][1] * _M.M[1][j] + M[i][2] * _M.M[2][j] + M[i][3] * _M.M[3][j];
		}
	}
	Matrix4 result = Matrix4(resultArray);
	return result;
}

Matrix4 Camera::RotateAroundArbitraryAxis(Vector4 axis,float theta)
{
	
	//first,we get standard rotation matrix of X,Y,Z axis.
	//X Axis:									Y Axis:											Z Axis:
	//1			0			0			0		cos(theta)		0		-sin(theta)		0		cos(theta)		sin(theta)		0		0
	//0			cos(theta)	sin(theta)	0		0				1		0				0		-sin(theta)		cos(theta)		0		0
	//0			-sin(theta)	cos(theta)	0		sin(theta)		0		cos(theta)		0		0				0				1		0
	//0			0			0			1		0				0		0				1		0				0				0		1


	//then,we rotate the Axis to XOZ Plane(rotate around X Axis)
	//suppose the angle is a,we can calculate the sina = axis.y/sqrt(axis.y^2 + axis.z^2)  cosa = axis.c/sqrt(axis.b^2 + axis.c^2)
	float sina, cosa;
	if (axis.fY == 0 && axis.fZ == 0)
	{
		sina = 0;
		cosa = 1;
	}
	else
	{
		sina = axis.fY / sqrt(axis.fY*axis.fY + axis.fZ*axis.fZ);
		cosa = axis.fZ / sqrt(axis.fY*axis.fY + axis.fZ*axis.fZ);
	}
	float rotateArray1[4][4] =
	{
		1,		0,		0,		0,
		0,		cosa,	-sina,	0,
		0,		sina,	cosa,	0,
		0,		0,		0,		1
	};
	Matrix4 matrix1 = rotateArray1;

	//next,we rotate the Axis to YOZ Plane(rotate with Y Axis)
	//this rotation is negetive angle. b is abs(angle).
	float sinb = axis.fX / sqrt(axis.fX*axis.fX + axis.fY*axis.fY + axis.fZ*axis.fZ);
	float cosb = sqrt(axis.fY*axis.fY + axis.fZ*axis.fZ) / sqrt(axis.fX*axis.fX + axis.fY*axis.fY + axis.fZ*axis.fZ);
	float rotateArray2[4][4] =
	{
		cosb,	0,	-sinb,		0,
		0,		1,		0,		0,
		sinb,	0,	 cosb,		0,
		0,		0,		0,		1
	};
	Matrix4 matrix2 = rotateArray2;

	//rotate with Z Axis
	float rotateArray3[4][4] =
	{
		cos(theta),		sin(theta),		0,		0,
		-sin(theta),	cos(theta),		0,		0,
		0,				0,				1,		0,
		0,				0,				0,		1
	};
	Matrix4 matrix3 = rotateArray3;

	//next is calculate (Matrix1)^T and (Matrix2)^T
	float rotateArray4[4][4] =
	{
		cosb,	0,	sinb,		0,
		0,		1,		0,		0,
		-sinb,	0,	 cosb,		0,
		0,		0,		0,		1
	};
	Matrix4 matrix4 = rotateArray4;
	
	float rotateArray5[4][4] =
	{
		1,		0,		0,		0,
		0,		cosa,	sina,	0,
		0,		-sina,	cosa,	0,
		0,		0,		0,		1
	};
	Matrix4 matrix5 = rotateArray5;
	return matrix1*matrix2*matrix3*matrix4*matrix5;
	
	/*
	Matrix4 result;
	float u = axis.x;
	float v = axis.y;
	float w = axis.z;

	result.M[0][0] = cosf(theta) + (u * u) * (1 - cosf(theta));
	result.M[0][1] = u * v * (1 - cosf(theta)) + w * sinf(theta);
	result.M[0][2] = u * w * (1 - cosf(theta)) - v * sinf(theta);
	result.M[0][3] = 0;

	result.M[1][0] = u * v * (1 - cosf(theta)) - w * sinf(theta);
	result.M[1][1] = cosf(theta) + v * v * (1 - cosf(theta));
	result.M[1][2] = w * v * (1 - cosf(theta)) + u * sinf(theta);
	result.M[1][3] = 0;

	result.M[2][0] = u * w * (1 - cosf(theta)) + v * sinf(theta);
	result.M[2][1] = v * w * (1 - cosf(theta)) - u * sinf(theta);
	result.M[2][2] = cosf(theta) + w * w * (1 - cosf(theta));
	result.M[2][3] = 0;

	result.M[3][0] = 0;
	result.M[3][1] = 0;
	result.M[3][2] = 0;
	result.M[3][3] = 1;

	return result;
	*/
}

Matrix4& Camera::getViewTransformMatrix() 
{
	//solve the problem how to move camera to origin while camera's rightVector equals the world x axis 
	//and camera's upVector equals world y axis

	//1.move to origin
	//we know the camera position vector,then we try to calculate a matrix to implement the condition(position*M = (0,0,0,1))
	float moveArray[4][4] = {
		1			,0			,0			,0,
		0			,1			,0			,0,
		0			,0			,1			,0,
		-position.fX	,-position.fY,-position.fZ,1
	};
	Matrix4 moveMatrix = moveArray;

	//2.Rotate with x,y,z axis
	//find the matrix feds the equation
	//right.x	right.y		right.z		0		*		M11		M12		M13		M14		 =		1	0	0	0
	//up.x		up.y		up.z		0				M21		M22		M23		M24				0	1	0	0
	//look.x	look.u		look.z		0				M31		M32		M33		M34				0	0	1	0
	//0			0			0			1				M41		M42		M43		M44				0	0	0	1

	//we can easily find the result
	float RotateArray[4][4] = 
	{
		right.fX	,up.fX	,look.fX	,	0,
		right.fY	,up.fY	,look.fY	,	0,
		right.fZ	,up.fZ	,look.fZ	,	0,
		0		,0		,0		,	1
	};
	Matrix4 rotateMatrix = RotateArray;
	return moveMatrix * rotateMatrix;
}

Matrix4::Matrix4(float Array[4][4])
{
	if(Array != NULL)
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				M[i][j] = Array[i][j];
	else
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 4; j++)
				M[i][j] = 0.0f;
}

Matrix4::Matrix4() 
{
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			M[i][j] = 0.0f;
}


Matrix4& getPerspectiveProjMatrix(float theta, float zn, float zf, float widthHeightScale)
{
	//theta -- the view frustum angle(up to down)
	//zn -- near Z clipPlane
	//zf -- far Z clipPlane
	//widthHeightScale -- the value of Width / Height

	//step1. proj the point to znPlane.
	//suppose the point(x,y) ,we try to find correct value of x,y when the point are projed into projPlane.
	//p'.x/p.x = n/p.z		p'.y/p.y = n/p.z	=> p'.x = n * p.x/p.z		p'.y = n * p.y/p.z
	
	//step2. proj znPlane point to a standard plane(x'',y'' is [-1,1])
	//p'.x/p''.x = W/2		p'.y/p''.y = H/2	=> p''.x = 2 * p'.x/W		p''.y = 2 * p'.y/H
	//p''.x = 2*n*p.x/W*p.z		p''.y = 2*n*p.y/H*p.z
	//cot(theta) = 2n/H ==> p''.y = cot(theta)*p.y/p.z		p''.x / p''.y = (p.x/p.y)*(H/W) ==> p''.x = p.x*cot(theta)/p.z*widthHeightScale

	//last step is to find z-value after projection(z'' is [0,1])
	//find a linear transform equation feds z'' value is small when the object is nearer than which z'' value is larger.
	//a * 1/n + b = 0		a * 1/f + b = 1	=>	-n * b/f + b = 1	=>	b = 1/(1 - n/f)	a = n/(n/f - 1)
	//the finally z value = n/(n/f - 1)/z + 1/(1 - n/f)
	//we get the proj position(p.x,p.y,p.z) => ((p.x/p.z)*cot(theta)/widthHeightScale,cot(theta)*p.y/p.z,nf/(n - f)*p.z + f/(f - n))
	//ProjMatrix feds equation (p.x,p.y,p.z,1) * ProjMatrix = (p''.x,p''.y,p''.z,1)
	//we can calculate the matrix is :
	float ProjArray[4][4] =
	{
		1/(tan(theta/2)*widthHeightScale),0,0,0,
		0,1/tan(theta/2),0,0,
		0,0,zf/(zf-zn),1,
		0,0,zf*zn/(zn-zf),0
	};
	Matrix4 *ProjMatrix = new Matrix4(ProjArray);
	return *ProjMatrix;
}

Matrix4& getViewPortMatrix(float x, float y, float width, float height,float minZ,float maxZ) 
{
	// CVV vertex -> Viewport position
	// use several special position to calculate the viewport matrix.
	// (-1,1,0,1)	->	(x,y,minZ,1)
	// (1,1,0,1)	->	(x + width,y,minZ,1)
	// (-1,-1,1,1)	->	(x,y+height,maxZ,1)
	// (1,-1,1,1)	->	(x+width,y+height,maxZ,1)
	float viewportArray[4][4] =
	{
		width/2,0,0,0,
		0,-height/2,0,0,
		0,0,maxZ - minZ,0,
		x+width/2,y+height/2,minZ,1
	};

	Matrix4* viewportMatrix = new Matrix4(viewportArray);
	return *viewportMatrix;
}

void Point3D::operator=(Point3D& p)
{
	position = p.position;
	color = p.color;
	tex_u = p.tex_u;
	tex_v = p.tex_v;
	rhw = p.rhw;
	VertexNormal = p.VertexNormal;
}
