#include "TrackballCamera.h"
#include <math.h>
#include <iostream>

glm::vec3 UP = glm::vec3(0, 1, 0);

TrackballCamera::TrackballCamera()
	: _cameraPosition(0,0,0)
{
	P = glm::perspective(45.0f, 1.0f, 0.01f, 1000.0f);
}


TrackballCamera::~TrackballCamera()
{
}

void TrackballCamera::SetCameraPosition(glm::vec3 pos)
{
	_cameraPosition = pos;
	update_mvp();
}

void TrackballCamera::SetPerspective(const float angle, const float ratio)
{
	P = glm::perspective(angle, ratio, 0.01f, 1000.0f);
	update_mvp();
}

void TrackballCamera::Scale(glm::vec2 p0, glm::vec2 p1, int width, int height)
{
	float deltaY = (p0.y - p1.y) / (float)height*10.0;//inverse
	float dist = std::sqrt(_cameraPosition.x*_cameraPosition.x + _cameraPosition.y*_cameraPosition.y
							+ _cameraPosition.z*_cameraPosition.z);
	_cameraPosition += deltaY*dist*_look;
	update_mvp();
}

void TrackballCamera::Pan(glm::vec2 p0, glm::vec2 p1, int width, int height)
{
	float deltaX = (p1.x - p0.x) / (float)width*10.0;
	float deltaY = (p0.y - p1.y) / (float)height*10.0;//inverse

	//convert pan vector in world-space to vector in local-space
	glm::vec3 delta_world = glm::vec3(deltaX, deltaY, 0);//
	glm::vec3 delta_local = glm::vec3(glm::inverse(M)*glm::vec4(delta_world, 0));

	//update mvp matrix
	glm::mat4 T = glm::translate(glm::mat4(1), delta_local);
	M = M * T;
	update_mvp();
}

void TrackballCamera::Rotate(glm::vec2 point0, glm::vec2 point1, int width, int height)
{
	//points in viewport window convert to points on sphere
	glm::vec3 p0 = plane_point_mapto_sphere(point0, width, height);
	glm::vec3 p1 = plane_point_mapto_sphere(point1, width, height);

	//covert the rotation axis in world-space to local-space
	glm::vec3 axis_world = glm::cross(p0, p1);
	glm::vec3 axis_local = glm::vec3(glm::inverse(M)*glm::vec4(axis_world, 0));

	//calculate the rotation angle
	glm::vec3 diff = p1 - p0;
	float angle = std::sqrt(diff.x*diff.x + diff.y*diff.y + diff.z*diff.z);

	//update mvp matrix in world-space
	glm::mat4 R = glm::rotate(glm::mat4(1), angle, axis_local);
	M = M * R;
	update_mvp();
}


/************************************************************************/
/* private methods                                              */
/************************************************************************/
void TrackballCamera::update_mvp()
{
	_look = glm::normalize(-_cameraPosition);
	_right = glm::normalize(glm::cross(_look, UP));
	_up = glm::normalize(glm::cross(_right, _look));

	V = glm::lookAt(_cameraPosition, glm::vec3(0, 0, 0), glm::normalize(_up));
	MV = V * M;
	MVP = P * MV;
}

glm::vec3 TrackballCamera::plane_point_mapto_sphere(glm::vec2 p, int width, int height)
{
	glm::vec3 result;
	result.x = (2.0 * p.x - (float)width) / (float)width;
	result.y = ((float)height - 2.0 * p.y) / (float)height;

	float len2 = result.x*result.x + result.y*result.y;
	len2 = (len2 > 1.0) ? 1.0 : len2;
	//result.z = sqrt(1.0 - len2);

	float len = std::sqrt(len2);
	result.z = std::cos((M_PI / 2)*((len > 1.0) ? 1.0 : len));
	float norm = std::sqrt(len2 + result.z*result.z);
	result.x /= norm;
	result.y /= norm;
	result.z /= norm;

	return result;
}
