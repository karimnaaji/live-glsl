#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include "GLFW/glfw3.h"

#include "filewatcher.h"

const GLfloat vertices[] = {
	-1.0f,  1.0f,
	-1.0f, -1.0f,
 	 1.0f, -1.0f,
    -1.0f,  1.0f,
     1.0f, -1.0f,
     1.0f,  1.0f
};

const GLchar* vertexShaderSrc =
    "attribute vec4 position;\n"
    "void main() {\n"
    "  gl_Position = position;\n"
    "}\n";

GLuint vbo;
GLint shaderProgram;
GLint posAttrib;
GLuint fragmentId;
GLuint vertexId;
pid_t pid;
pid_t father;

bool fragHasChanged = false;

void handleError(const string& message, int exitStatus) {
    cerr << "ABORT: "<< message << endl;
    exit(exitStatus);
}

void printShaderInfoLog(GLuint shader)
{
    GLint length = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
    if(length > 1)
    {
        char* log = (char*)malloc(sizeof(char) * length);
        glGetShaderInfoLog(shader, length, NULL, log);
        printf("Log: %s\n", log);
        free(log);
    }
}

GLuint compileShader(const GLchar* src, GLenum type)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    if(!status)
    {
        printShaderInfoLog(shader);
        glDeleteShader(shader);
        return -1;
    }
    return shader;
}

void loadShaderSource(const string& path, string* into) {
    ifstream file;
    string buffer;

    file.open(path.c_str());

    if(!file.is_open()) {
        handleError("Opening file failure" + path, -1);
    }

    while(!file.eof()) {
        getline(file, buffer);
        (*into) += buffer + "\n";
    }

    file.close();
}

unsigned int linkShaderToProgram(GLuint program, const GLchar* source, GLenum type) {
    switch(type) {
        case GL_VERTEX_SHADER: {
            vertexId = compileShader(source, GL_VERTEX_SHADER);
            glAttachShader(program, vertexId);
        }
        break;
        case GL_FRAGMENT_SHADER: {
            fragmentId = compileShader(source, GL_FRAGMENT_SHADER);
            if(fragmentId == -1) {
                return -1;
            }
            glAttachShader(program, fragmentId);
        }
        break;
    }

    glLinkProgram(program);

    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);

    if(!linkStatus)
    {
        printShaderInfoLog(program);
        glDeleteProgram(program);
        handleError("Linking failed", -1);
    }

    return 0;
}

void initShader(const string& fragShaderPath) {
    glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    shaderProgram = glCreateProgram();

    string fragSource;
    loadShaderSource(fragShaderPath, &fragSource);

    linkShaderToProgram(shaderProgram, vertexShaderSrc, GL_VERTEX_SHADER);
    linkShaderToProgram(shaderProgram, fragSource.c_str(), GL_FRAGMENT_SHADER);

    posAttrib = glGetAttribLocation(shaderProgram, "position");
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posAttrib);
}

void callback() {
    cout << "file has changed" << endl;
    kill(father, SIGALRM);
}

void fileHasChanged(int sig) {
    if(sig == SIGALRM) {
        fragHasChanged = true;
    }
}

void handleKeypress(unsigned char key, int x, int y) {
	switch (key) {
		case 27: //Escape key
			exit(0);
	}
}

void handleResize(int w, int h) {
	glViewport(0, 0, w, h);
}

void redefineSignal(int sig, void (*handler)(int)) {
    struct sigaction action;
    sigset_t set;

    sigemptyset(&set);
    action.sa_handler = handler;
    action.sa_flags = 0;
    action.sa_mask = set;

    if (sigaction(sig, &action, NULL) != 0) {
        cerr << "not able to redefine signal number" << sig << endl;
    }
}

void watchingThread(const char* argv) {
    char* s = new char[1024];
    realpath(argv, s);
    string absolutePath(s);
    FileWatcher watcher(absolutePath, &callback);
    cout << pid << " start watching " << absolutePath << endl;
    watcher.startWatching();
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderProgram);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(posAttrib);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glUseProgram(0);
    glEnableVertexAttribArray(0);
}

void renderingThread(const string& fragShaderPath) {
    GLFWwindow* window;
    int width = 800;
    int height = 600;

    if(!glfwInit())
        handleError("GLFW init failed", -1);

    window = glfwCreateWindow(width, height, "GLFW Window", NULL, NULL);

    if(!window) {
        glfwTerminate();
        handleError("GLFW create window failed", -1);
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    GLenum err = glewInit();

    if(err != GLEW_OK) {
        cerr << glewGetErrorString(err) << endl;
        handleError("GlEW init failed", -1);
    }

    initShader(fragShaderPath);

    while(!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        if(fragHasChanged) {
            glDetachShader(shaderProgram, fragmentId);
            string fragSource;
            loadShaderSource(fragShaderPath, &fragSource);
            linkShaderToProgram(shaderProgram, fragSource.c_str(), GL_FRAGMENT_SHADER);
            fragHasChanged = false;
        }
        render();
        glfwPollEvents();
    }

    glfwTerminate();
}

void clean() {
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(shaderProgram);
}

int main(int argc, char **argv) {
    if(argc < 2) {
        handleError("Provide fragment shader argument", -1);
    }

    string fragShaderPath = string(argv[1]);

    father = getpid();
    pid = fork();
    redefineSignal(SIGALRM, fileHasChanged);

    if(pid < 0)
        handleError("Fork failure", -1);

    switch(pid) {
        case 0: { // child
            watchingThread(argv[1]);
        }
        break;
        default: {
            renderingThread(fragShaderPath);
            kill(pid, SIGKILL);
            clean();
        }
    }

    return 0;
}
