//
// Created by Yunzhe on 2017/12/4.
//

#ifndef GPUBASEDRENDERING_A1_SYSTEM_H
#define GPUBASEDRENDERING_A1_SYSTEM_H

// Include related head files
#include "global.h"
#include "utility.h"
#include "draw.h"
#include "light.h"
#include "textfile.h"
#include "Shader.h"

// Using namespace std for cout
using namespace std;

void Idle();
void Redraw();
void Reshape(int width, int height);
void ProcessMouseClick(int button, int state, int x, int y);
void ProcessMouseMove(int x, int y);
void ProcessFocus(int state);
void ProcessNormalKey(unsigned char k, int x, int y);
void ProcessSpecialKey(int k, int x, int y);
void PrintStatus();
void initVBO();
void setShader();
void updateMVPZero();
void updateMVPOne();
void updateMVPTwo();
void updateMVPThree();
void updateShaderMVP();
void setupFBO();
void setupVAO();
void initShader();

#endif //GPUBASEDRENDERING_A1_SYSTEM_H