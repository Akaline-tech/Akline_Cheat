#pragma once
#include "windows.h"
#include "../Vec/Vector.h"

// ֻ�����������Ƴ�ʵ��
bool WorldToScreen(Vector3 pWorldPos, Vector3& pScreenPos, float* pMatrixPtr, const FLOAT pWinWidth, const FLOAT pWinHeight);