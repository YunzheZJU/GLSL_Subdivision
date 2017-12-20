//
// Created by Yunzhe on 2017/12/4.
//

#ifndef GPUBASEDRENDERING_A3_DRAW_H
#define GPUBASEDRENDERING_A3_DRAW_H

// include global.h
#include "global.h"
#include "Shader.h"

extern GLuint vaoHandle[2];
extern Shader shader;
extern GLuint pass1Index;
extern GLuint pass2Index;
extern GLuint pass3Index;

void DrawScene();

void drawLocator(GLfloat *center, GLfloat radius);

void drawCrosshair();

#endif //GPUBASEDRENDERING_A3_DRAW_H
