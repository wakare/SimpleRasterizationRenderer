#include"graphics.h"

#define GETRANGEVALUE(a,x,b) {((x)<(a))?(a):(((x)>(b))?(b):(x))}

Light::Light()
{
	this->Ambient = 0x00000000;
	this->Diffuse = 0x00000000;
	this->Specular = 0x00000000;
	this->type = LightType::UNDEFINED;
	this->direction = Vector4();
	this->position = Vector4();
}

Light::Light(LightType type, Vector4 position, Vector4 direction, INT32 Diffuse, INT32 Ambient, INT32 Specular)
	:type(type),position(position),direction(direction),Diffuse(Diffuse),Ambient(Ambient),Specular(Specular)
{
}

INT32 ColorAdd(INT32 src, int r, int g, int b)
{
	int R = (src & 0x00ff0000) >> 16;
	int B = (src & 0x0000ff00) >> 8;
	int G = (src & 0x000000ff);
	R = min(R + r, 255);
	G = min(G + g, 255);
	B = min(B + b, 255);
	src = (R << 16) | (G << 8) | B;
	return src;
}

INT32 ColorCross(INT32 color1, INT32 color2)
{
	int R1 = (color1 & 0x00ff0000) >> 16;
	int G1 = (color1 & 0x0000ff00) >> 8;
	int B1 = (color1 & 0x000000ff);
	int R2 = (color2 & 0x00ff0000) >> 16;
	int G2 = (color2 & 0x0000ff00) >> 8;
	int B2 = (color2 & 0x000000ff);
	INT32 result = 0x0;
	result = ((R1 * R2) / 255) << 16 | ((G1 * G2) / 255) << 8 | ((B1 * B2) / 255);
	return result;
}

//Help:http://www.cnblogs.com/qingsunny/archive/2013/03/02/2939765.html
INT32 calculateLighting(Material material, Vector4 position,Vector4 normalVector,Vector4 cameraPosition,INT32 baseColor, Light* lights, int lightCount,bool SpecularOpen)
{
	INT32 resultColor = 0x0;
	int R = 0, G = 0, B = 0;
	const int baseR = (baseColor & 0x00ff0000) >> 16;
	const int baseG = (baseColor & 0x0000ff00) >> 8;
	const int baseB = (baseColor & 0x000000ff);
	for (int i = 0; i < lightCount; i++)
	{
		if ((lights + i)->type == LightType::POINTLIGHT)
		{
			//calculate ambient
			R = GETRANGEVALUE(0,(baseR *(((lights + i)->Ambient & 0x00ff0000) >> 16) * ((material.Ambient & 0x00ff0000) >> 16) / (255 * 255)),255);
			G = GETRANGEVALUE(0,(baseG *(((lights + i)->Ambient & 0x0000ff00) >> 8)  * ((material.Ambient & 0x0000ff00) >> 8) / (255 * 255)),255);
			B = GETRANGEVALUE(0,(baseB *((lights + i)->Ambient & 0x000000ff) * (material.Ambient & 0x000000ff) / (255 * 255)),255);
			resultColor = ColorAdd(resultColor, R, G, B);

			//calculate specular.first is to get reflect vector of the vector begin with eye and end with position around normalVector.
			Vector4 lightVector = (lights + i)->position - position;
			lightVector.ResetUnitVector();
			if (SpecularOpen)
			{
				Vector4 eyeVector = position - cameraPosition;
				eyeVector.ResetUnitVector();
				Vector4 reflectVector = eyeVector - normalVector * (2 * (eyeVector * normalVector));
				reflectVector.ResetUnitVector();
				float k = reflectVector*lightVector;
				if (k > 0.0f)
				{
					k = pow(k, material.Power);
					R = GETRANGEVALUE(0, (int)(k * (((lights + i)->Specular & 0x00ff0000) >> 16)*((material.Specular & 0x00ff0000) >> 16) / 255), 255);
					G = GETRANGEVALUE(0, (int)(k * (((lights + i)->Specular & 0x0000ff00) >> 8)*((material.Specular & 0x0000ff00) >> 8) / 255), 255);
					B = GETRANGEVALUE(0, (int)(k * ((lights + i)->Specular & 0x000000ff)*(material.Specular & 0x000000ff) / 255), 255);
					resultColor = ColorAdd(resultColor, R, G, B);
				}
			}
			//calculate diffuse.
			float diffuse = lightVector*normalVector;
			if (diffuse > 0.0f)
			{
				R = GETRANGEVALUE(0, (int)(baseR * diffuse * (((lights + i)->Diffuse & 0x00ff0000) >> 16) * ((material.Diffuse & 0x00ff0000) >> 16) / (255 * 255)), 255);
				G = GETRANGEVALUE(0, (int)(baseG * diffuse * (((lights + i)->Diffuse & 0x0000ff00) >> 8) * ((material.Diffuse & 0x0000ff00) >> 8) / (255 * 255)), 255);
				B = GETRANGEVALUE(0, (int)(baseB * diffuse * ((lights + i)->Diffuse & 0x000000ff) * (material.Diffuse & 0x000000ff) / (255 * 255)), 255);
				resultColor = ColorAdd(resultColor, R, G, B);
			}

			//calculate Emissive
			R = GETRANGEVALUE(0, (int)(baseR * ((material.Emissive & 0x00ff0000) >> 16) / 255), 255);
			G = GETRANGEVALUE(0, (int)(baseG * ((material.Emissive & 0x0000ff00) >> 8) / 255), 255);
			B = GETRANGEVALUE(0, (int)(baseB * (material.Emissive & 0x000000ff) / 255), 255);
			resultColor = ColorAdd(resultColor, R, G, B);
		}
	}
	return resultColor;
}

void triangle::setPlaneNormalVector()
{
	normalVector = (vertices[1].position - vertices[0].position).Cross(vertices[2].position - vertices[1].position);
	normalVector = -normalVector;
	normalVector.ResetUnitVector();
}

triangle::triangle(Point3D _vertices[3]) 
{
	vertices[0] = _vertices[0];
	vertices[1] = _vertices[1];
	vertices[2] = _vertices[2];

	normalVector = (vertices[1].position - vertices[0].position).Cross(vertices[2].position - vertices[1].position);
	normalVector = -normalVector;
	normalVector.ResetUnitVector();
	
	vertices[0].VertexNormal = normalVector;
	vertices[1].VertexNormal = normalVector;
	vertices[2].VertexNormal = normalVector;
	VSplitPoint = Point3D();
}

triangle::triangle(Point3D p1, Point3D p2, Point3D p3)
{
	vertices[0] = p1;
	vertices[1] = p2;
	vertices[2] = p3;

	normalVector = (vertices[1].position - vertices[0].position).Cross(vertices[2].position - vertices[1].position);
	normalVector = -normalVector;
	normalVector.ResetUnitVector();
	
	vertices[0].VertexNormal = p1.VertexNormal;
	vertices[1].VertexNormal = p2.VertexNormal;
	vertices[2].VertexNormal = p3.VertexNormal;

	VSplitPoint = Point3D();
}

INT32 colorInterpolate(INT32 startColor, INT32 endColor, float k)
{
	INT32 resultColor = 0;

	int startR = (startColor & 0x00ff0000)>>16;
	int startG = (startColor & 0x0000ff00)>>8;
	int startB = (startColor & 0x000000ff);
	
	int endR = (endColor & 0x00ff0000)>>16;
	int endG = (endColor & 0x0000ff00)>>8;
	int endB = (endColor & 0x000000ff);
	
	int R = GETRANGEVALUE(0, lineInterpolate(startR, endR, k), 255);
	int G = GETRANGEVALUE(0, lineInterpolate(startG, endG, k), 255);
	int B = GETRANGEVALUE(0, lineInterpolate(startB, endB, k), 255);
	
	resultColor = (R << 16) | (G << 8) | B;
	return resultColor;
}

void triangle::operator=(triangle &t)
{
	normalVector = t.normalVector;
	vertices[0] = t.vertices[0];
	vertices[1] = t.vertices[1];
	vertices[2] = t.vertices[2];
	VSplitPoint = t.VSplitPoint;
	type = t.type;
	material = t.material;
}

void triangle::getTriangleType(Vector4* originVector)
{
	//wo resort the index of three vertices.
	if (vertices[0].position.fY > vertices[1].position.fY)
	{
		Point3D temp = vertices[0];
		vertices[0] = vertices[1];
		vertices[1] = temp;
		if (originVector)
		{
			Vector4 temp = originVector[0];
			originVector[0] = originVector[1];
			originVector[1] = temp;
		}
	}
	if (vertices[1].position.fY > vertices[2].position.fY)
	{
		Point3D temp = vertices[1];
		vertices[1] = vertices[2];
		vertices[2] = temp;
		if (originVector)
		{
			Vector4 temp = originVector[2];
			originVector[2] = originVector[1];
			originVector[1] = temp;
		}
		if (vertices[0].position.fY > vertices[1].position.fY)
		{
			Point3D temp = vertices[0];
			vertices[0] = vertices[1];
			vertices[1] = temp;
			if (originVector)
			{
				Vector4 temp = originVector[0];
				originVector[0] = originVector[1];
				originVector[1] = temp;
			}
		}
	}
	//now the vertices feds vertices[0].y<vertices[1].y<vertices[2].y
	if ((vertices[0].position.fY - vertices[1].position.fY)<1e-8 && (vertices[0].position.fY - vertices[1].position.fY)> -1e-8)
	{
		if (vertices[0].position.fX > vertices[1].position.fX)
		{
			Point3D temp = vertices[0];
			vertices[0] = vertices[1];
			vertices[1] = temp;
			if (originVector)
			{
				Vector4 temp = originVector[0];
				originVector[0] = originVector[1];
				originVector[1] = temp;
			}
		}
		type = TriangleType::DOWN;
		return;
	}

	if ((vertices[1].position.fY - vertices[2].position.fY)<1e-8 && (vertices[1].position.fY - vertices[2].position.fY)> -1e-8)
	{
		if (vertices[1].position.fX > vertices[2].position.fX)
		{
			Point3D temp = vertices[1];
			vertices[1] = vertices[2];
			vertices[2] = temp;
			if (originVector)
			{
				Vector4 temp = originVector[2];
				originVector[2] = originVector[1];
				originVector[1] = temp;
			}
		}
		type = TriangleType::UP;
		return;
	}

	// we try to find the special point S of the line which top point is vertices[0] and end point is vertices[2]
	float k = (vertices[1].position.fY - vertices[0].position.fY) / (vertices[2].position.fY - vertices[0].position.fY);
	float x = vertices[0].position.fX + k*(vertices[2].position.fX - vertices[0].position.fX);
	float z = vertices[0].position.fZ + k*(vertices[2].position.fZ - vertices[0].position.fZ);
	float rhw = vertices[0].rhw + k*(vertices[2].rhw - vertices[0].rhw);
	
	if (((vertices[2].rhw - vertices[0].rhw) > 1e-10 || (vertices[2].rhw - vertices[0].rhw) < -1e-10))
	{
		k = ((1 / rhw) - (1 / vertices[0].rhw)) / ((1 / vertices[2].rhw) - (1 / vertices[0].rhw));
	}

	if ( k > 1.0f || k < 0.0f)
		k = (vertices[1].position.fY - vertices[0].position.fY) / (vertices[2].position.fY - vertices[0].position.fY);
	Vector4 normalVector = vertices[0].VertexNormal + (vertices[2].VertexNormal - vertices[0].VertexNormal) * k;
	float u = (vertices[0].tex_u + k * (vertices[2].tex_u - vertices[0].tex_u));
	float v = (vertices[0].tex_v + k * (vertices[2].tex_v - vertices[0].tex_v));
	INT32 color = colorInterpolate(vertices[0].color, vertices[2].color, k);
	VSplitPoint.position.fX = x;
	VSplitPoint.position.fY = vertices[1].position.fY;
	VSplitPoint.position.fZ = z;
	VSplitPoint.rhw = rhw;
	VSplitPoint.position.fW = 1.0f;
	VSplitPoint.color = color;
	VSplitPoint.tex_u = u;
	VSplitPoint.tex_v = v;
	VSplitPoint.VertexNormal = normalVector;

	if (vertices[1].position.fX > x)
	{
		type = TriangleType::LEFT;
		return;
	}
	else if (vertices[1].position.fX < x)
	{
		type = TriangleType::RIGHT;
		return;
	}
}

bool triangle::faceBackTest(Vector4 cameraPosition)
{
	Vector4 testVec =  cameraPosition - vertices[0].position;
	return ((testVec * normalVector) > 0 ? true : false);
}

bool Plane::IsIndexExist(int index)
{
	if (index == indexs[0] || index == indexs[3])
		return true;
	if (index < indexs[0] || index > indexs[3])
		return false;
	if (index == indexs[1] || index == indexs[2])
		return true;
	return false;
}

Cube::Cube(Point3D** _vertices)//to simply calculation,we suppose the index of vertices is:Plane1.p1 Plane1.p2 Plane1.p3 Plane1.p4 and Plane2(with the same index). 
{
	for (int i = 0; i < 8; i++)
	{
		vertices[i] = *(_vertices[i]);
	}

	planes[0] = Plane(IndexTriangle(vertices, 0, 1, 2), IndexTriangle(vertices, 0, 2, 3), 0, 1, 2, 3);
	planes[1] = Plane(IndexTriangle(vertices, 0, 4, 1), IndexTriangle(vertices, 1, 4, 5), 0, 1, 4, 5);
	planes[2] = Plane(IndexTriangle(vertices, 0, 3, 4), IndexTriangle(vertices, 3, 7, 4), 0, 3, 4, 7);
	planes[3] = Plane(IndexTriangle(vertices, 4, 6, 5), IndexTriangle(vertices, 4, 7, 6), 4, 5, 6, 7);
	planes[4] = Plane(IndexTriangle(vertices, 1, 5, 6), IndexTriangle(vertices, 1, 6, 2), 1, 2, 5, 6);
	planes[5] = Plane(IndexTriangle(vertices, 3, 2, 6), IndexTriangle(vertices, 3, 6, 7), 2, 3, 6, 7);
	
	for (int i = 0; i < 6; i++)
	{
		planes[i].t1.setPlaneNormalVector();
		planes[i].t2.setPlaneNormalVector();
		planes[i].normalVector = planes[i].t1.normalVector;
	}

	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			if (planes[j].IsIndexExist(i))
			{
				vertices[i].VertexNormal += planes[j].normalVector;
			}
		}
		vertices[i].VertexNormal /= 3.0f;
	}
}

void Cube::SetMaterial(Material mtrl)
{
	material = mtrl;
	for (int i = 0; i < 6; i++)
	{
		planes[i].t1.material = material;
		planes[i].t2.material = material;
	}
}

IndexTriangle::IndexTriangle(Point3D* _vertices, int p1, int p2, int p3)
{
	vertices = _vertices;
	index[0] = p1;
	index[1] = p2;
	index[2] = p3;
	normalVector = (vertices[index[1]].position - vertices[index[0]].position).Cross(vertices[index[2]].position - vertices[index[1]].position);
	normalVector = -normalVector;
	normalVector.ResetUnitVector();
	VSplitPoint = Point3D();
}

triangle IndexTriangle::toTriangle()
{
	triangle res;
	res.material = material;
	res.normalVector = normalVector;
	res.type = type;
	res.vertices[0] = vertices[index[0]];
	res.vertices[1] = vertices[index[1]];
	res.vertices[2] = vertices[index[2]];
	res.VSplitPoint = VSplitPoint;
	return res;
}

void IndexTriangle::setPlaneNormalVector()
{
	normalVector = (vertices[index[1]].position - vertices[index[0]].position).Cross(vertices[index[2]].position - vertices[index[1]].position);
	normalVector = -normalVector;
	normalVector.ResetUnitVector();
}