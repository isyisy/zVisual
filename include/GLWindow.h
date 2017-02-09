#ifndef GLWINDOW_H
#define GLWINDOW_H

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include "TrackballCamera.h"
#include "RayCastingActor.h"
#include "MarchingCubesActor.h"

enum MouseState
{
    MOUSE_BUTTON_NONE = 0,
    LEFT_BUTTON_DOWN,
    MIDDLE_BUTTON_DOWN,
    RIGHT_BUTTON_DOWN
};

class GLWindow
{
public:
    GLWindow();

    virtual ~GLWindow();

    //启动主循环
    virtual void run(GLWindow* app=NULL);

    //渲染准备
	virtual void startup();

	//渲染
	virtual void render(double currentTime);

	//资源回收清除
	virtual void shutdown();

	//
    virtual void onResize(int w, int h);

	virtual void onMouseButton(int button, int action);

	virtual void onMouseMove(int x, int y);

	virtual void onMouseWheel(int pos);

	virtual void onKey(int key, int action);

protected:

    static void glfw_onResize(GLFWwindow* window, int w, int h)
	{
		m_app->onResize(w, h);
	}

	static void glfw_onMouseButton(GLFWwindow* window, int button, int action, int mods)
	{
		m_app->onMouseButton(button, action);
	}

	static void glfw_onMouseMove(GLFWwindow* window, double x, double y)
	{
		m_app->onMouseMove(static_cast<int>(x), static_cast<int>(y));
	}

	static void glfw_onMouseWheel(GLFWwindow* window, double xoffset, double yoffset)
	{
		m_app->onMouseWheel(static_cast<int>(yoffset));
	}

	static void glfw_onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		m_app->onKey(key, action);
	}

private:
    void init();

protected:

    static GLWindow*    m_app;

	GLFWwindow*         window;

	int                 oldX, oldY;

	bool                firstCatch;

    MouseState          state;

    RayCastingActor*    rayActor;

    MarchingCubesActor* mcActor;

    TrackballCamera     myCam;

};

#endif // GLWINDOW_H
