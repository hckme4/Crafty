#include <X11/Xlib.h>
#include <GL/glx.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <iostream>
#include <cmath>

// Define mouse sensitivity and move speed
#define MOUSE_SENSITIVITY 0.1f
#define MOVE_SPEED 0.1f

// X11 variables
Display *dpy;
Window root, win;
GLXContext glc;
XVisualInfo *vi;
Colormap cmap;
XSetWindowAttributes swa;
XWindowAttributes gwa;
XEvent xev;

// Camera variables
GLfloat cameraPos[] = {0.0f, 0.0f, 5.0f}; // Initial camera position (x, y, z)
GLfloat cameraDir[] = {0.2f, 0.2f, -1.0f}; // Initial camera direction (x, y, z)
GLfloat cameraRight[] = {1.0f, 0.0f, 0.0f}; // Initial camera right vector (x, y, z)
GLfloat cameraUp[] = {0.0f, 1.0f, 0.0f}; // Initial camera up vector (x, y, z)

// Cursor variables
bool cursorCaptured = false;
int lastMouseX, lastMouseY;

// OpenGL functions
void initGL() {
    glEnable(GL_DEPTH_TEST); // Enable depth testing
    glClearColor(1.0, 1.0, 1.0, 1.0); // Set clear color to white
}

void resizeGL(int width, int height) {
    glViewport(0, 0, width, height); // Set viewport
    glMatrixMode(GL_PROJECTION); // Switch to projection matrix
    glLoadIdentity(); // Reset projection matrix
    gluPerspective(45.0, (float)width / (float)height, 0.1, 100.0); // Set perspective projection
    glMatrixMode(GL_MODELVIEW); // Switch back to modelview matrix
}

void drawTetrahedron() {
    // Tetrahedron vertices
    GLfloat vertices[4][3] = {
        {0.0, 1.0, 0.0}, // Top
        {-1.0, -1.0, 1.0}, // Front bottom-left
        {1.0, -1.0, 1.0}, // Front bottom-right
        {0.0, -1.0, -1.0} // Back bottom
    };

    // Tetrahedron faces (vertex indices)
    GLint faces[4][3] = {
        {0, 1, 2}, // Front face
        {0, 2, 3}, // Right face
        {0, 3, 1}, // Left face
        {1, 3, 2} // Bottom face
    };

    // Tetrahedron face colors
    GLfloat colors[4][3] = {
        {1.0, 0.0, 0.0}, // Red
        {0.0, 1.0, 0.0}, // Green
        {0.0, 0.0, 1.0}, // Blue
        {1.0, 1.0, 0.0} // Yellow
    };

    // Draw tetrahedron
    glBegin(GL_TRIANGLES);
    for (int i = 0; i < 4; ++i) {
        glColor3fv(colors[i]);
        for (int j = 0; j < 3; ++j) {
            glVertex3fv(vertices[faces[i][j]]);
        }
    }
    glEnd();
}

void drawScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers
    glLoadIdentity(); // Reset the modelview matrix

    // Set camera view
    gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2],
              cameraPos[0] + cameraDir[0], cameraPos[1] + cameraDir[1], cameraPos[2] + cameraDir[2],
              cameraUp[0], cameraUp[1], cameraUp[2]);

    // Draw tetrahedron
    drawTetrahedron();

    glXSwapBuffers(dpy, win); // Swap the front and back buffers to display the rendered image
}

void updateCameraPosition(int direction) {
    // Move camera along the direction vector
    cameraPos[0] += direction * MOVE_SPEED * cameraDir[0];
    cameraPos[1] += direction * MOVE_SPEED * cameraDir[1];
    cameraPos[2] += direction * MOVE_SPEED * cameraDir[2];
}

void updateCameraDirection(float deltaX, float deltaY) {
    // Update camera direction based on mouse movement
    GLfloat yaw = deltaX * MOUSE_SENSITIVITY;
    GLfloat pitch = deltaY * MOUSE_SENSITIVITY;

    // Rotate camera around y-axis (yaw)
    GLfloat newX = cameraDir[0] * cos(yaw) + cameraDir[2] * sin(yaw);
    GLfloat newZ = -cameraDir[0] * sin(yaw) + cameraDir[2] * cos(yaw);
    cameraDir[0] = newX;
    cameraDir[2] = newZ;

    // Rotate camera around x-axis (pitch)
    GLfloat newY = cameraDir[1] * cos(pitch) - cameraDir[2] * sin(pitch);
    newZ = cameraDir[1] * sin(pitch) + cameraDir[2] * cos(pitch);
    cameraDir[1] = newY;
    cameraDir[2] = newZ;

    // Update camera right vector
    cameraRight[0] = sin(yaw - M_PI / 2);
    cameraRight[1] = 0;
    cameraRight[2] = cos(yaw - M_PI / 2);
}

void grabCursor() {
    cursorCaptured = true;
    XGrabPointer(dpy, win, True, ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
                 GrabModeAsync, GrabModeAsync, None, None, CurrentTime);
    XGrabKeyboard(dpy, win, True, GrabModeAsync, GrabModeAsync, CurrentTime);
    XWarpPointer(dpy, None, win, 0, 0, 0, 0, gwa.width / 2, gwa.height / 2);
    lastMouseX = gwa.width / 2;
    lastMouseY = gwa.height / 2;
    XDefineCursor(dpy, win, None);
    XFlush(dpy);
}

void releaseCursor() {
    cursorCaptured = false;
    XUngrabPointer(dpy, CurrentTime);
    XUngrabKeyboard(dpy, CurrentTime);
    XUndefineCursor(dpy, win);
    XFlush(dpy);
}

int main() {
    dpy = XOpenDisplay(NULL); // Open the default X display

    if (!dpy) {
        std::cerr << "Unable to open display\n"; // Print error message if display cannot be opened
        return -1;
    }

    root = DefaultRootWindow(dpy); // Get the root window of the default screen
    int att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None}; // Specify GLX visual attributes
    vi = glXChooseVisual(dpy, 0, att); // Choose a suitable visual with desired attributes
    cmap = XCreateColormap(dpy, root, vi->visual, AllocNone); // Create a colormap using the chosen visual

    swa.colormap = cmap; // Set the colormap attribute in window attributes
    swa.event_mask = ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask; // Set event mask for window
    win = XCreateWindow(dpy, root, 0, 0, 800, 600, 0, vi->depth, InputOutput, vi->visual, CWColormap | CWEventMask, &swa); // Create a window
    XMapWindow(dpy, win); // Map the window to the screen
    XStoreName(dpy, win, "First Person Shooter"); // Set window title

    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE); // Create an OpenGL context
    glXMakeCurrent(dpy, win, glc); // Make the OpenGL context current

    initGL(); // Initialize OpenGL settings

    while (true) {
        XNextEvent(dpy, &xev); // Get the next X event

        if (xev.type == Expose) {
            XGetWindowAttributes(dpy, win, &gwa); // Get window attributes
            resizeGL(gwa.width, gwa.height); // Resize the OpenGL viewport
            drawScene(); // Draw the scene
        }

        if (xev.type == KeyPress) {
            switch (XLookupKeysym(&xev.xkey, 0)) {
                case XK_w: // Move camera forward
                    updateCameraPosition(1);
                    break;
                case XK_s: // Move camera backward
                    updateCameraPosition(-1);
                    break;
                case XK_a: // Strafe camera left
                    // Move camera perpendicular to camera right vector
                    cameraPos[0] -= MOVE_SPEED * cameraRight[0];
                    cameraPos[1] -= MOVE_SPEED * cameraRight[1];
                    cameraPos[2] -= MOVE_SPEED * cameraRight[2];
                    break;
                case XK_d: // Strafe camera right
                    // Move camera perpendicular to camera right vector
                    cameraPos[0] += MOVE_SPEED * cameraRight[0];
                    cameraPos[1] += MOVE_SPEED * cameraRight[1];
                    cameraPos[2] += MOVE_SPEED * cameraRight[2];
                    break;
                case XK_space: // Toggle cursor grab
                    if (cursorCaptured)
                        releaseCursor();
                    else
                        grabCursor();
                    break;
            }
        }

        if (xev.type == MotionNotify) {
            if (cursorCaptured) {
                // Update camera direction based on mouse movement
                updateCameraDirection(xev.xmotion.x - lastMouseX, xev.xmotion.y - lastMouseY);
                XWarpPointer(dpy, None, win, 0, 0, 0, 0, gwa.width / 2, gwa.height / 2);
                lastMouseX = gwa.width / 2;
                lastMouseY = gwa.height / 2;
            }
        }
    }

    XDestroyWindow(dpy, win); // Destroy the window
    XCloseDisplay(dpy); // Close the X display

    return 0;
}
