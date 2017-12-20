#include "main.h"

int main(int argc, char *argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE | GLUT_MULTISAMPLE);
    glutInitWindowSize(1280, 720);
    glutCreateWindow("GPU Based Rendering: A3");

    // Set the background color - dark grey
    glClearColor(0.1, 0.1, 0.1, 0.0);

    glutDisplayFunc(Redraw);
    glutReshapeFunc(Reshape);
    glutMouseFunc(ProcessMouseClick);
    glutPassiveMotionFunc(ProcessMouseMove);
    glutEntryFunc(ProcessFocus);
    glutKeyboardFunc(ProcessNormalKey);
    glutIdleFunc(Idle);

    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK) {
        cerr << "Error occurred when initializing GLEW: " << glewGetErrorString(glewErr) << endl;
        exit(1);
    }
    if (!glewIsSupported("GL_VERSION_4_0")) {
        cerr << "OpenGL 4.0 is not supported" << endl;
        exit(1);
    }

    initShader();
    initVBO();
    setShader();

    glutMainLoop();

    return 0;
}