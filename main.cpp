#include <iostream>
#include "GLWindow.h"

using namespace std;

int main()
{
    GLWindow* renderer = new GLWindow;

    renderer->run(renderer);

    std::cout<<"OK"<<std::endl;

    return 0;
}
