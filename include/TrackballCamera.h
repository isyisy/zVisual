#pragma once

#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/glm.hpp>

/************************************************************************/
//simple trackball camera
/************************************************************************/

class TrackballCamera
{
public:
	TrackballCamera();
	~TrackballCamera();

	void SetCameraPosition(glm::vec3 pos);
	void SetPerspective(const float angle, const float ratio );

	void Scale(glm::vec2 p0, glm::vec2 p1, int width, int height);//scale objects by moving camera far or near
	void Pan(glm::vec2 p0, glm::vec2 p1, int width, int height);//translate the object
	void Rotate(glm::vec2 p0, glm::vec2 p1, int width, int height);//trackball rotate

	glm::mat4 GetMVP()      const { return MVP;}
	glm::mat4 GetMV()       const { return MV;}
	glm::mat4 GetModal()    const { return M;}
	glm::mat4 GetView()     const {return V;}

	glm::vec3 GetCameraPosition() const { return _cameraPosition;}


private:
	void update_mvp();
	glm::vec3 plane_point_mapto_sphere(glm::vec2 p, int width, int height);

private:
	glm::vec3 _cameraPosition;//camera position in global coordinate

	glm::vec3 _look;
	glm::vec3 _up;
	glm::vec3 _right;

	glm::mat4 M;
	glm::mat4 V;
	glm::mat4 MV;
	glm::mat4 P;
	glm::mat4 MVP;

};

