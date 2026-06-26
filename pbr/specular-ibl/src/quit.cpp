#include <cstdlib>

#include "glad/glad.h"
#include <GLFW/glfw3.h>

void quit(int status_code) {
    glfwTerminate();
    std::exit(status_code);
}
