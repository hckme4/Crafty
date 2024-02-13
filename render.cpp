#include <iostream>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include <X11/Xlib.h>
#include <cmath>

// Global variables
Display *display;
Window window;
GLXContext glContext;

// Function to create an OpenGL window
void createWindow(int width, int height) {
    // Open the X display
    display = XOpenDisplay(nullptr);
    if (!display) {
        std::cerr << "Failed to open X display" << std::endl;
        exit(1);
    }

    // Get the default screen and root window
    int screen = DefaultScreen(display);
    Window root = RootWindow(display, screen);

    // Create an RGB visual
    int attribs[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
    XVisualInfo *visualInfo = glXChooseVisual(display, screen, attribs);
    if (!visualInfo) {
        std::cerr << "Failed to find suitable visual" << std::endl;
        exit(1);
    }

    // Create a colormap
    Colormap colormap = XCreateColormap(display, root, visualInfo->visual, AllocNone);

    // Set window attributes
    XSetWindowAttributes windowAttributes;
    windowAttributes.colormap = colormap;
    windowAttributes.event_mask = ExposureMask | KeyPressMask;

    // Create the window
    window = XCreateWindow(display, root, 0, 0, width, height, 0, visualInfo->depth,
                           InputOutput, visualInfo->visual, CWColormap | CWEventMask, &windowAttributes);

    // Create GLX context
    glContext = glXCreateContext(display, visualInfo, nullptr, GL_TRUE);
    if (!glContext) {
        std::cerr << "Failed to create OpenGL context" << std::endl;
        exit(1);
    }

    // Make GLX context current
    if (!glXMakeCurrent(display, window, glContext)) {
        std::cerr << "Failed to make OpenGL context current" << std::endl;
        exit(1);
    }

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // Map the window to the screen
    XMapWindow(display, window);
}

// Function to render a rotating 3D cube
/*void render() {
    static float angle = 0.0f;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    glTranslatef(0.0f, 0.0f, -5.0f);
    glRotatef(angle, 1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);

    // Front face
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);

    // Back face
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);

    // Top face
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);

    // Bottom face
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);

    // Right face
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(1.0f, -1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, -1.0f);
    glVertex3f(1.0f, 1.0f, 1.0f);
    glVertex3f(1.0f, -1.0f, 1.0f);

    // Left face
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-1.0f, -1.0f, -1.0f);
    glVertex3f(-1.0f, -1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, 1.0f);
    glVertex3f(-1.0f, 1.0f, -1.0f);

    glEnd();

    glXSwapBuffers(display, window);

    angle += 0.5f;
    if (angle >= 360.0f)
        angle -= 360.0f;
}*/

void render() {
    // Get window size
    XWindowAttributes windowAttributes;
    XGetWindowAttributes(display, window, &windowAttributes);
    int width = windowAttributes.width;
    int height = windowAttributes.height;

    // Create graphics context
    GC gc = XCreateGC(display, window, 0, NULL);

    // Set background color to white
    XSetForeground(display, gc, WhitePixel(display, 0));
    XFillRectangle(display, window, gc, 0, 0, width, height);

    // Set foreground color to red
    XSetForeground(display, gc, BlackPixel(display, 0));

    // Draw the cube
    int cubeSize = 100; // Adjust size as needed
    int cubeX = (width - cubeSize) / 2;
    int cubeY = (height - cubeSize) / 2;
    XFillRectangle(display, window, gc, cubeX, cubeY, cubeSize, cubeSize);

    // Free graphics context
    XFreeGC(display, gc);

    // Flush X server
    XFlush(display);
}




// Function to clean up resources
void destroyWindow() {
    glXMakeCurrent(display, None, nullptr);
    glXDestroyContext(display, glContext);
    XDestroyWindow(display, window);
    XCloseDisplay(display);
}

int main() {
    createWindow(800, 600);

    XEvent event;
    while (true) {
        render();
        XNextEvent(display, &event);
        if (event.type == Expose) {
            render();
        }
        if (event.type == KeyPress) {
            break;
        }
    }

    destroyWindow();
    return 0;
}
