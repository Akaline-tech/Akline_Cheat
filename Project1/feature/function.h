#pragma once
#include "windows.h"
#include "../Vec/Vector.h"

// 只保留声明，移除实现
bool WorldToScreen(Vector3 pWorldPos, Vector3& pScreenPos, float* pMatrixPtr, const FLOAT pWinWidth, const FLOAT pWinHeight);