#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void MDS_Activate();

void MDS_SetWindowSize(int w, int h);

void MDS_SetWindowScale(float scale);

void MDS_SetTopScreen(float x, float y, float w, float h);
void MDS_SetBottomScreen(float x, float y, float w, float h);

#ifdef __cplusplus
}
#endif