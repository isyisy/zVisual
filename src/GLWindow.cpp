#include "GLWindow.h"
#include "DataLoader.h"
#include <iostream>

#include <GL/glm/glm.hpp>
#include <GL/glm/gtc/matrix_transform.hpp>
#include <GL/glm/gtc/type_ptr.hpp>

using namespace std;

GLWindow* GLWindow::m_app = NULL;

/**************************************************/
GLWindow::GLWindow()
    : firstCatch(true)
    , rayActor(NULL)
    , mcActor(NULL)
{
}

GLWindow::~GLWindow()
{
}

void GLWindow::init()
{
    //glfw window settings
	if (!glfwInit())
	{
		std::cout<<"Failed to initialize GLFW\n";
		return;
	}

 	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
 	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
  	glfwWindowHint(GLFW_SAMPLES, 0);

  	//create windows
	window = glfwCreateWindow(800, 800, "zVisual", NULL, NULL);
	if (!window)
	{
		std::cout<<"Failed to open window.\n";
		glfwTerminate();
		return;
	}

	glfwMakeContextCurrent(window);
	glfwSetWindowSizeCallback(window, glfw_onResize);
	glfwSetKeyCallback(window, glfw_onKey);
	glfwSetMouseButtonCallback(window, glfw_onMouseButton);
	glfwSetCursorPosCallback(window, glfw_onMouseMove);
	glfwSetScrollCallback(window, glfw_onMouseWheel);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	glfwSwapInterval(1);

	//glew init
	glewExperimental = GL_TRUE;//important, do not forget it!
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		std::cout<<"Failed to initialize glew.\n";
		return;
	}else{
        if(!GLEW_VERSION_3_3){
    		std::cout<<"OpengGL 3.3 is not suppported!\n";
    		return;
    	}
    	std::cout<<"VENDOR: "<<glGetString(GL_VENDOR)<<std::endl;
        std::cout<<"RENDERER: "<<glGetString(GL_RENDERER)<<std::endl;
        std::cout<<"GLSL_VERSION: "<<glGetString(GL_SHADING_LANGUAGE_VERSION)<<std::endl;
	}

    return;
}

void GLWindow::startup()
{
    //init camera
    myCam.SetCameraPosition(glm::vec3(0,0,3));

    //load data
	DataLoader::instance()->createCubeData();
	if(!DataLoader::instance()->isEnable())
        return;

    //init ray casting actor
    rayActor = new RayCastingActor(&myCam);
	rayActor->initShader();
    rayActor->updateData();

    //init marching cubes actor
    //mcActor = new MarchingCubesActor(&myCam);
    //mcActor->initShader();
    //mcActor->updateData();
    //mcActor->setIsoValue(1500);
}

void GLWindow::shutdown()
{
}

void GLWindow::render(double currenttime)
{
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glClearColor(0,0,0,1.0);

	rayActor->render();
	//mcActor->render();
}

/************************************************************/
void GLWindow::run(GLWindow* the_app)
{
    init();

    m_app = the_app;
    bool running = true;

	startup();//initialize

	do{
		render(glfwGetTime());

		glfwSwapBuffers(window);

		glfwWaitEvents();

		if (glfwWindowShouldClose(window))
            running = false;

	} while (running);

	shutdown();//finalize

	glfwDestroyWindow(window);
	glfwTerminate();
}

void GLWindow::onResize(int w, int h)
{
    glViewport(0, 0, GLsizei(w), GLsizei(h));
    myCam.SetPerspective(45.0f, (GLfloat)w / h);
}

void GLWindow::onMouseButton(int button, int action)
{
    if(action==GLFW_RELEASE)
    {
        state = MOUSE_BUTTON_NONE;
        firstCatch = true;
        return;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        state = LEFT_BUTTON_DOWN;
    else if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
        state = MIDDLE_BUTTON_DOWN;
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
        state = RIGHT_BUTTON_DOWN;
}

void GLWindow::onMouseMove(int pos_x, int pos_y)
{
    if(state==MOUSE_BUTTON_NONE)
        return;

    if(firstCatch)
    {
        oldX = pos_x;
        oldY = pos_y;
        firstCatch = false;
        return;
    }

    int window_width, window_height;
    glfwGetWindowSize(window,&window_width,&window_height);

    if (state == LEFT_BUTTON_DOWN)
    {
        myCam.Rotate(glm::vec2(oldX, oldY), glm::vec2(pos_x, pos_y),window_width,window_height);
    }
    else if (state == MIDDLE_BUTTON_DOWN)
    {

        myCam.Scale(glm::vec2(oldX, oldY), glm::vec2(pos_x, pos_y), window_width, window_height);
    }
    else if (state == RIGHT_BUTTON_DOWN)
    {
        myCam.Pan(glm::vec2(oldX, oldY), glm::vec2(pos_x, pos_y), window_width, window_height);
    }

    oldX = pos_x;
    oldY = pos_y;
}

void GLWindow::onMouseWheel(int pos)
{

}

void GLWindow::onKey(int key, int action)
{
    if(key==GLFW_KEY_ESCAPE && action==GLFW_PRESS)
        glfwSetWindowShouldClose(window,1);
}

