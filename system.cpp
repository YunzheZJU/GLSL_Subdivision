//
// System.cpp
// Processing system display and control
// Created by Yunzhe on 2017/12/4.
//

#include "system.h"

mat4 model;
mat4 view;
mat4 projection;
mat4 viewport;
GLuint vboHandle[2];
GLfloat angle = 0.0f;                               // Use this to control the rotation
GLfloat camera[3] = {0, 0, 5};                      // Position of camera
GLfloat target[3] = {0, 0, 0};                      // Position of target of camera
GLfloat camera_polar[3] = {5, -1.57f, 0};           // Polar coordinates of camera
GLfloat camera_locator[3] = {0, -5, 10};            // Position of shadow of camera
int maxTessLevel = 5;                               // Maximun level of the tessellation
int fpsMode = 2;                                    // 0:off, 1:on, 2:waiting
int window[2] = {1280, 720};                        // Window size
int windowCenter[2];                                // Center of this window, to be updated
char message[70] = "Welcome!";                      // Message string to be shown
bool bMsaa = false;                                 // Switch of multisampling anti-alias
bool bCamera = true;                                // Switch of camera/target control
bool bFocus = true;                                 // Status of window focus
bool bMouse = false;                                // Whether mouse postion should be moved
bool bDrawWireFrame = true;                         // Whether draw the wire frame or not

void Idle() {
    glutPostRedisplay();
}

void Reshape(int width, int height) {
    if (height == 0) {                        // Prevent A Divide By Zero By
        height = 1;                            // Making Height Equal One
    }
    glViewport(static_cast<GLint>(width / 2.0 - 640), static_cast<GLint>(height / 2.0 - 360), 1280, 720);
    window[W] = width;
    window[H] = height;
    float w2 = width / 2.0f;
    float h2 = height / 2.0f;
    viewport = mat4(vec4(w2, 0.0f, 0.0f, 0.0f),
                    vec4(0.0f, h2, 0.0f, 0.0f),
                    vec4(0.0f, 0.0f, 1.0f, 0.0f),
                    vec4(w2, h2, 0.0f, 1.0f));
    updateWindowcenter(window, windowCenter);

    glMatrixMode(GL_PROJECTION);            // Select The Projection Matrix
    glLoadIdentity();                        // Reset The Projection Matrix
    gluPerspective(45.0f, 1.7778f, 0.1f, 30000.0f);    // 1.7778 = 1280 / 720
    glMatrixMode(GL_MODELVIEW);                // Select The Modelview Matrix
}

void Redraw() {
    shader.use();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();                        // Reset The Current Modelview Matrix
    gluLookAt(camera[X], camera[Y], camera[Z],
              target[X], target[Y], target[Z],
              0, 1, 0);                            // Define the view matrix
    if (bMsaa) {
        glEnable(GL_MULTISAMPLE_ARB);
    } else {
        glDisable(GL_MULTISAMPLE_ARB);
    }
    angle += 0.5f;
    glEnable(GL_DEPTH_TEST);
    // Draw something here
    updateMVP();
    updateShader();
    DrawScene();
    // Disable shader
    shader.disable();
    // Draw crosshair and locator in fps mode, or target when in observing mode(fpsMode == 0).
    if (fpsMode == 0) {
        glDisable(GL_DEPTH_TEST);
        drawLocator(target, LOCATOR_SIZE);
        glEnable(GL_DEPTH_TEST);
    } else {
        drawCrosshair();
        camera_locator[X] = camera[X];
        camera_locator[Z] = camera[Z];
        glDisable(GL_DEPTH_TEST);
        drawLocator(camera_locator, LOCATOR_SIZE);
        glEnable(GL_DEPTH_TEST);
    }
    // Show fps, message and other information
    PrintStatus();

    glutSwapBuffers();
}

void ProcessMouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        bMsaa = !bMsaa;
        cout << "LMB pressed. Switch on/off multisampling anti-alias." << endl;
        strcpy(message, "LMB pressed. Switch on/off multisampling anti-alias.");
        glutPostRedisplay();
    }
}

void ProcessMouseMove(int x, int y) {
    cout << "Mouse moves to (" << x << ", " << y << ")" << endl;
    if (fpsMode) {
        // Track target and reverse mouse moving to center point.
        if (fpsMode == 2) {
            // 鼠标位置居中，为确保在glutPositionWindow()之后执行
            updateWindowcenter(window, windowCenter);
            SetCursorPos(windowCenter[X], windowCenter[Y]);
            glutSetCursor(GLUT_CURSOR_NONE);
            fpsMode = 1;
            return;
        }
        if (x < window[W] * 0.25) {
            x += window[W] * 0.5;
            bMouse = !bMouse;
        } else if (x > window[W] * 0.75) {
            x -= window[W] * 0.5;
            bMouse = !bMouse;
        }
        if (y < window[H] * 0.25) {
            y = static_cast<int>(window[H] * 0.25);
            bMouse = !bMouse;
        } else if (y > window[H] * 0.75) {
            y = static_cast<int>(window[H] * 0.75);
            bMouse = !bMouse;
        }
        // 将新坐标与屏幕中心的差值换算为polar的变化
        camera_polar[A] = static_cast<GLfloat>((window[W] / 2 - x) * (180 / 180.0 * PI) / (window[W] / 4.0) *
                                               PANNING_PACE);            // Delta pixels * 180 degrees / (1/4 width) * PANNING_PACE
        camera_polar[T] = static_cast<GLfloat>((window[H] / 2 - y) * (90 / 180.0 * PI) / (window[H] / 4.0) *
                                               PANNING_PACE);            // Delta pixels * 90 degrees / (1/4 height) * PANNING_PACE
        // 移动光标
        if (bMouse) {
            SetCursorPos(glutGet(GLUT_WINDOW_X) + x, glutGet(GLUT_WINDOW_Y) + y);
            bMouse = !bMouse;
        }
        // 更新摄像机目标
        updateTarget(camera, target, camera_polar);
    }
}

void ProcessFocus(int state) {
    if (state == GLUT_LEFT) {
        bFocus = !bFocus;
        cout << "Focus is on other window." << endl;
    } else if (state == GLUT_ENTERED) {
        bFocus = !bFocus;
        cout << "Focus is on this window." << endl;
    }
}

void ProcessNormalKey(unsigned char k, int x, int y) {
    switch (k) {
        // 退出程序
        case 27: {
            cout << "Bye." << endl;
            exit(0);
        }
            // 切换摄像机本体/焦点控制
        case 'Z':
        case 'z': {
            strcpy(message, "Z pressed. Switch camera control!");
            bCamera = !bCamera;
            break;
        }
            // 切换第一人称控制
        case 'C':
        case 'c': {
            strcpy(message, "C pressed. Switch fps control!");
            // 摄像机归零
            cameraMakeZero(camera, target, camera_polar);
            if (!fpsMode) {
                // 调整窗口位置
                int windowmaxx = glutGet(GLUT_WINDOW_X) + window[W];
                int windowmaxy = glutGet(GLUT_WINDOW_Y) + window[H];
                if (windowmaxx >= glutGet(GLUT_SCREEN_WIDTH) || windowmaxy >= glutGet(GLUT_SCREEN_HEIGHT)) {
                    // glutPositionWindow()并不会立即执行！
                    glutPositionWindow(glutGet(GLUT_SCREEN_WIDTH) - window[W], glutGet(GLUT_SCREEN_HEIGHT) - window[H]);
                    fpsMode = 2;
                    break;
                }
                // 鼠标位置居中
                updateWindowcenter(window, windowCenter);
                // windowCenter[X] - window[W] * 0.25 为什么要减？
                SetCursorPos(windowCenter[X], windowCenter[Y]);
                glutSetCursor(GLUT_CURSOR_NONE);
                fpsMode = 1;
            } else {
                glutSetCursor(GLUT_CURSOR_RIGHT_ARROW);
                fpsMode = 0;
            }
            break;
        }
            // 第一人称移动/摄像机本体移动/焦点移动
        case 'A':
        case 'a': {
            strcpy(message, "A pressed. Watch carefully!");
            if (fpsMode) {
                saveCamera(camera, target, camera_polar);
                camera[X] -= cos(camera_polar[A]) * MOVING_PACE;
                camera[Z] += sin(camera_polar[A]) * MOVING_PACE;
                target[X] -= cos(camera_polar[A]) * MOVING_PACE;
                target[Z] += sin(camera_polar[A]) * MOVING_PACE;
            } else {
                if (bCamera) {
                    camera_polar[A] -= OBSERVING_PACE * 0.1;
                    updateCamera(camera, target, camera_polar);
                    cout << fixed << setprecision(1) << "A pressed.\n\tPosition of camera is set to (" <<
                         camera[X] << ", " << camera[Y] << ", " << camera[Z] << ")." << endl;
                } else {
                    target[X] -= OBSERVING_PACE;
                    updatePolar(camera, target, camera_polar);
                    cout << fixed << setprecision(1) << "A pressed.\n\tPosition of camera target is set to (" <<
                         target[X] << ", " << target[Y] << ", " << target[Z] << ")." << endl;
                }
            }
            break;
        }
        case 'D':
        case 'd': {
            strcpy(message, "D pressed. Watch carefully!");
            if (fpsMode) {
                saveCamera(camera, target, camera_polar);
                camera[X] += cos(camera_polar[A]) * MOVING_PACE;
                camera[Z] -= sin(camera_polar[A]) * MOVING_PACE;
                target[X] += cos(camera_polar[A]) * MOVING_PACE;
                target[Z] -= sin(camera_polar[A]) * MOVING_PACE;
            } else {
                if (bCamera) {
                    camera_polar[A] += OBSERVING_PACE * 0.1;
                    updateCamera(camera, target, camera_polar);
                    cout << fixed << setprecision(1) << "D pressed.\n\tPosition of camera is set to (" <<
                         camera[X] << ", " << camera[Y] << ", " << camera[Z] << ")." << endl;
                } else {
                    target[X] += OBSERVING_PACE;
                    updatePolar(camera, target, camera_polar);
                    cout << fixed << setprecision(1) << "D pressed.\n\tPosition of camera target is set to (" <<
                         target[X] << ", " << target[Y] << ", " << target[Z] << ")." << endl;
                }
            }
            break;
        }
        case 'W':
        case 'w': {
            strcpy(message, "W pressed. Watch carefully!");
            if (fpsMode) {
                saveCamera(camera, target, camera_polar);
                camera[X] -= sin(camera_polar[A]) * MOVING_PACE;
                camera[Z] -= cos(camera_polar[A]) * MOVING_PACE;
                target[X] -= sin(camera_polar[A]) * MOVING_PACE;
                target[Z] -= cos(camera_polar[A]) * MOVING_PACE;
            } else {
                if (bCamera) {
                    camera[Y] += OBSERVING_PACE;
                    cout << fixed << setprecision(1) << "W pressed.\n\tPosition of camera is set to (" <<
                         camera[X] << ", " << camera[Y] << ", " << camera[Z] << ")." << endl;
                } else {
                    target[Y] += OBSERVING_PACE;
                    updatePolar(camera, target, camera_polar);
                    cout << fixed << setprecision(1) << "W pressed.\n\tPosition of camera target is set to (" <<
                         target[X] << ", " << target[Y] << ", " << target[Z] << ")." << endl;
                }
            }
            break;
        }
        case 'S':
        case 's': {
            strcpy(message, "S pressed. Watch carefully!");
            if (fpsMode) {
                saveCamera(camera, target, camera_polar);
                camera[X] += sin(camera_polar[A]) * MOVING_PACE;
                camera[Z] += cos(camera_polar[A]) * MOVING_PACE;
                target[X] += sin(camera_polar[A]) * MOVING_PACE;
                target[Z] += cos(camera_polar[A]) * MOVING_PACE;
            } else {
                if (bCamera) {
                    camera[Y] -= OBSERVING_PACE;
                    cout << fixed << setprecision(1) << "S pressed.\n\tPosition of camera is set to (" <<
                         camera[X] << ", " << camera[Y] << ", " << camera[Z] << ")." << endl;
                    strcpy(message, "S pressed. Watch carefully!");
                } else {
                    target[Y] -= OBSERVING_PACE;
                    updatePolar(camera, target, camera_polar);
                    cout << fixed << setprecision(1) << "S pressed.\n\tPosition of camera target is set to (" <<
                         target[X] << ", " << target[Y] << ", " << target[Z] << ")." << endl;
                }
            }
            break;
        }
        case 'Q':
        case 'q': {
            if (bCamera) {
                strcpy(message, "Q pressed. Camera is moved...nearer!");
                camera_polar[R] *= 0.95;
                updateCamera(camera, target, camera_polar);
                cout << fixed << setprecision(1) << "Q pressed.\n\tPosition of camera is set to (" <<
                     camera[X] << ", " << camera[Y] << ", " << camera[Z] << ")." << endl;
            } else {
                strcpy(message, "Q pressed. Camera target is moving towards +Z!");
                target[Z] += OBSERVING_PACE;
                updatePolar(camera, target, camera_polar);
                cout << fixed << setprecision(1) << "Q pressed.\n\tPosition of camera target is set to (" <<
                     target[X] << ", " << target[Y] << ", " << target[Z] << ")." << endl;
            }
            break;
        }
        case 'E':
        case 'e': {
            if (bCamera) {
                strcpy(message, "E pressed. Camera is moved...farther!");
                camera_polar[R] *= 1.05;
                updateCamera(camera, target, camera_polar);
                cout << fixed << setprecision(1) << "E pressed.\n\tPosition of camera is set to (" <<
                     camera[X] << ", " << camera[Y] << ", " << camera[Z] << ")." << endl;
            } else {
                strcpy(message, "E pressed. Camera target is moving towards -Z!");
                target[Z] -= OBSERVING_PACE;
                updatePolar(camera, target, camera_polar);
                cout << fixed << setprecision(1) << "E pressed.\n\tPosition of camera target is set to (" <<
                     target[X] << ", " << target[Y] << ", " << target[Z] << ")." << endl;
            }
            break;
        }
        case 'O':
        case 'o': {
            bDrawWireFrame = !bDrawWireFrame;
            cout << "O pressed. Switch on/off wire frame." << endl;
            strcpy(message, "O pressed. Switch on/off wire frame.");
        }
            // 最大细分层次
        case '+': {
            cout << "+ pressed." << endl;
            if (maxTessLevel < 50) {
                maxTessLevel += 1;
                cout << "Maximum level of tesselation is set to " << maxTessLevel << "." << endl;
            }
            break;
        }
        case '-': {
            cout << "- pressed." << endl;
            if (maxTessLevel > 2) {
                maxTessLevel -= 1;
                cout << "Maximum level of tesselation is set to " << maxTessLevel << "." << endl;
            }
            break;
        }
            // 屏幕截图
        case 'X':
        case 'x': {
            cout << "X pressed." << endl;
            if (screenshot(window[W], window[H])) {
                cout << "Screenshot is saved." << endl;
                strcpy(message, "X pressed. Screenshot is saved.");
            } else {
                cout << "Screenshot failed." << endl;
                strcpy(message, "X pressed. Screenshot failed.");
            }
            break;
        }
        default:
            break;
    }
}

void PrintStatus() {
    static int frame = 0;
    static int currenttime;
    static int timebase = 0;
    static char fpstext[50];
    char *c;
    char cameraPositionMessage[50];
    char targetPositionMessage[50];
    char cameraPolarPositonMessage[50];

    frame++;
    currenttime = glutGet(GLUT_ELAPSED_TIME);
    if (currenttime - timebase > 1000) {
        sprintf(fpstext, "FPS:%4.2f",
                frame * 1000.0 / (currenttime - timebase));
        timebase = currenttime;
        frame = 0;
    }

    sprintf(cameraPositionMessage, "Camera Position  %2.1f   %2.1f   %2.1f",
            camera[X], camera[Y], camera[Z]);
    sprintf(targetPositionMessage, "Target Position     %2.1f   %2.1f   %2.1f",
            target[X], target[Y], target[Z]);
    sprintf(cameraPolarPositonMessage, "Camera Polar      %2.1f   %2.3f   %2.3f",
            camera_polar[R], camera_polar[A], camera_polar[T]);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);                     // 不受灯光影响
    glMatrixMode(GL_PROJECTION);                // 选择投影矩阵
    glPushMatrix();                             // 保存原矩阵
    glLoadIdentity();                           // 装入单位矩阵
    glOrtho(-window[W] / 2, window[W] / 2, -window[H] / 2, window[H] / 2, -1, 1);       // 设置裁减区域
    glMatrixMode(GL_MODELVIEW);                 // 选择Modelview矩阵
    glPushMatrix();                             // 保存原矩阵
    glLoadIdentity();                           // 装入单位矩阵
    glPushAttrib(GL_LIGHTING_BIT);
    glRasterPos2f(20 - window[W] / 2, window[H] / 2 - 20);
    for (c = fpstext; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    glRasterPos2f(window[W] / 2 - 240, window[H] / 2 - 20);
    for (c = cameraPositionMessage; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
    glRasterPos2f(window[W] / 2 - 240, window[H] / 2 - 55);
    for (c = targetPositionMessage; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
    glRasterPos2f(window[W] / 2 - 240, window[H] / 2 - 90);
    for (c = cameraPolarPositonMessage; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
    }
    glRasterPos2f(20 - window[W] / 2, 20 - window[H] / 2);
    for (c = message; *c != '\0'; c++) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);                // 选择投影矩阵
    glPopMatrix();                              // 重置为原保存矩阵
    glMatrixMode(GL_MODELVIEW);                 // 选择Modelview矩阵
    glPopMatrix();                              // 重置为原保存矩阵
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

void initVBO() {
    float v0[] = {0.0f, 1.0f, 0.0f,
                  0.0f, 0.0f, 1.0f,
                  1.0f, 0.0f, 0.0f};
    float v1[] = {0.0f, 1.0f, 0.0f,
                  0.0f, 0.0f, 1.0f,
                  -1.0f, 0.0f, 0.0f};

    glGenVertexArrays(2, vaoHandle);
    glGenBuffers(2, vboHandle);

    glBindVertexArray(vaoHandle[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vboHandle[0]);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), v0, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(vaoHandle[1]);
    glBindBuffer(GL_ARRAY_BUFFER, vboHandle[1]);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), v1, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void setShader() {
    GLuint shaderProgram = shader.getProgram();
    pass1Index = glGetSubroutineIndex(shaderProgram, GL_TESS_EVALUATION_SHADER, "pass1");
    pass2Index = glGetSubroutineIndex(shaderProgram, GL_TESS_EVALUATION_SHADER, "pass2");
    pass3Index = glGetSubroutineIndex(shaderProgram, GL_TESS_EVALUATION_SHADER, "pass3");
    // Set the number of vertices per patch.  IMPORTANT!!
    glPatchParameteri(GL_PATCH_VERTICES, 3);
    ///////////// Uniforms ////////////////////
    shader.setUniform("MinTessLevel", 2);
    shader.setUniform("MaxDepth", 20.0f);
    shader.setUniform("MinDepth", 5.0f);
    shader.setUniform("LineWidth", 0.1f);
    shader.setUniform("LineColor", vec4(0.05f, 0.0f, 0.05f, 1.0f));
    shader.setUniform("Material.Ks", 0.95f, 0.95f, 0.95f);
    shader.setUniform("Material.Ka", 0.1f, 0.1f, 0.1f);
    shader.setUniform("Material.Shininess", 100.0f);
    shader.setUniform("Light.Intensity", vec3(1.0f, 1.0f, 1.0f));
    /////////////////////////////////////////////
    updateShader();
}

void updateMVP() {
    model = mat4(1.0f);
    model = glm::translate(model, vec3(0.0f, -1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(angle), vec3(0.0f, 1.0f, 0.0f));
    view = glm::lookAt(vec3(camera[X], camera[Y], camera[Z]), vec3(target[X], target[Y], target[Z]),
                       vec3(0.0f, 1.0f, 0.0f));
    projection = glm::perspective(45.0f, 1.7778f, 0.1f, 30000.0f);
}

void updateShader() {
    mat4 mv = view * model;
    shader.setUniform("ModelViewMatrix", mv);
    shader.setUniform("NormalMatrix", mat3(vec3(mv[0]), vec3(mv[1]), vec3(mv[2])));
    shader.setUniform("MVP", projection * mv);
    shader.setUniform("ViewportMatrix", viewport);
    shader.setUniform("MaxTessLevel", maxTessLevel);
    shader.setUniform("DrawWireFrame", bDrawWireFrame);
    shader.setUniform("Light.Position", view * vec4(0.0f, 0.0f, 10.0f, 1.0f));
}

void initShader() {
    try {
        shader.compileShader("basic.vert");
        shader.compileShader("basic.tcs");
        shader.compileShader("basic.tes");
        shader.compileShader("basic.geom");
        shader.compileShader("basic.frag");
        shader.link();
        shader.use();
    } catch (GLSLProgramException &e) {
        cerr << e.what() << endl;
        exit(EXIT_FAILURE);
    }
}