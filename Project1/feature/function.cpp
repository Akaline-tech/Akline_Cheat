#include "function.h"
#include <cstring> // for memcpy
bool WorldToScreen(Vector3 pWorldPos, Vector3& pScreenPos, float* pMatrixPtr, const FLOAT pWinWidth, const FLOAT pWinHeight)
{
    // 函数体不变（从function.h中移过来的）
    float matrix2[4][4];//[0][1][2][3][4][5]
    memcpy(matrix2, pMatrixPtr, 16 * sizeof(float));
    const float mX{ pWinWidth / 2 };
    const float mY{ pWinHeight / 2 };
    const float w{
        matrix2[3][0] * pWorldPos.x +
        matrix2[3][1] * pWorldPos.y +
        matrix2[3][2] * pWorldPos.z +
        matrix2[3][3] };
    if (w < 0.65f)return false;
    const float x{
        matrix2[0][0] * pWorldPos.x +
        matrix2[0][1] * pWorldPos.y +
        matrix2[0][2] * pWorldPos.z +
        matrix2[0][3]
    };
    const float y{
        matrix2[1][0] * pWorldPos.x +
        matrix2[1][1] * pWorldPos.y +
        matrix2[1][2] * pWorldPos.z +
        matrix2[1][3]
    };
    pScreenPos.x = (mX + mX * x / w);
    pScreenPos.y = (mY - mY * y / w);
    pScreenPos.z = 0;
    return true;
}