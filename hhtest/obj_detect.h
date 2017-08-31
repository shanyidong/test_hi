#ifndef OBJ_DETECT_H
#define OBJ_DETECT_H
#include "cv.h"



//#include "cv.hpp"
//#include "opencv2/objdetect/objdetect.hpp"
//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
#ifdef __cplusplus
extern "C" {
//int detectInit();
int detectRun(CvMat *frame,int *detectNum,char* path);
}
#endif

#endif // OBJ_DETECT_H
