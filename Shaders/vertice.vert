#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 textureUV;

out vec2 uv;

uniform mat4 modelView;
uniform mat4 projection;
uniform mat4 viewMatrix;
uniform mat4 translation;
uniform int Time;

mat4 M;
mat4 alterM;
vec4 q;

vec4 rotate(float angle, vec3 rhs)
{
	vec4 q;

	float sin = sin(0.05f * angle * 0.5f);
	float cos = cos(0.05f * angle * 0.5f);

	q.w = cos;
	q.x = rhs.x * sin;
	q.y = rhs.y * sin;
	q.z = rhs.z * sin;

	return q;
}

mat4 toMat4(vec4 q)
{
	mat4 res;
	
	res[0][0] = 1.0 - 2.0 * q.y * q.y - 2.0 * q.z * q.z;
	res[0][1] = 2 * q.x * q.y - 2 * q.z * q.w;
	res[0][2] = 2 * q.x * q.z + 2 * q.y * q.w;
	res[0][3] = 0;

	res[1][0] = 2 * q.x * q.y + 2 * q.z * q.w;
	res[1][1] = 1 - 2 * q.x * q.x - 2 * q.z * q.z;
	res[1][2] = 2 * q.y * q.z - 2 * q.x * q.w;
	res[1][3] = 0;

	res[2][0] = 2 * q.x * q.z - 2 * q.y * q.w;
	res[2][1] = 2 * q.y * q.z + 2 * q.x * q.w;
	res[2][2] = 1 - 2 * q.x * q.x - 2 * q.y * q.y;
	res[2][3] = 0;
	
	res[3][0] = 0;
	res[3][1] = 0;
	res[3][2] = 0;
	res[3][3] = 1;	

	return res;
}

void main()
{
	uv = textureUV;															// texture uv
	M = inverse(viewMatrix) * modelView;									// Model matrix

	q = rotate(Time, vec3(1.0, 0.0, 0.0));
	vec4 tmp = rotate(Time, vec3(0.0, 1.0, 0.0));
	
	alterM = toMat4(tmp) * translation;
	mat4 MVP = projection * modelView * alterM * toMat4(q);
	gl_Position = MVP * vec4(position.x, position.y, position.z, 1.0);
}