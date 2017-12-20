//
// Created by Yunzhe on 2017/12/4.
//

#ifndef GPUBASEDRENDERING_A3_SYSTEM_H
#define GPUBASEDRENDERING_A3_SYSTEM_H

// Include related head files
#include "global.h"
#include "utility.h"
#include "draw.h"

// Using namespace std for cout
using namespace std;

void Idle();

void Redraw();

void Reshape(int width, int height);

void ProcessMouseClick(int button, int state, int x, int y);

void ProcessMouseMove(int x, int y);

void ProcessFocus(int state);

void ProcessNormalKey(unsigned char k, int x, int y);

void PrintStatus();

void initVBO();

void setShader();

void updateMVP();

void updateShader();

void initShader();

#endif //GPUBASEDRENDERING_A3_SYSTEM_H
