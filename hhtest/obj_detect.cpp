#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include "obj_detect.h"
using namespace cv;

//    int detectInit()
//    {

//    }

    int detectRun(CvMat* frame,int* detectNum,char* path)
    {
#ifndef Cascade
#define Cascade
        if(strlen(path)>0)
        {
            //imwrite(path,frame);
            //cvSaveImage(path,frame);
        }
        CascadeClassifier cascade;
        string cascadeName;
        cascadeName = "model//cascade-ped-HOG_Classfier_DAB-2.xml";
        //printf("load cascade ..\n");
        if (!cascade.load(cascadeName))
        {
            //cerr << "ERROR: Could not load classifier cascade" << endl;
            printf("ERROR: Could not load classifier cascade");
            return -1;
        }
#endif

        //detectNum=0;
        Mat frame1;
        frame1= Mat(frame,true);
        if (frame1.empty())
        {
            printf("empty\n");
            return -1;
        }
        //Mat smallImg;
        //double scale=0.5;
        //resize(frame1, smallImg, Size(), scale, scale, INTER_LINEAR);
        vector<Rect> objs;
        cascade.detectMultiScale(frame1, objs,
                                1.2, 10, 0
                                //|CASCADE_FIND_BIGGEST_OBJECT
                                //|CASCADE_DO_ROUGH_SEARCH
                                |CASCADE_SCALE_IMAGE,
                                Size(30,50));
    //*********************************
    //nms begin
    //*********************************
//    vector<Rect> objs_out;
//    objs_out.clear();
//    const size_t size=objs.size();
//    if(!size)
//    {

//    }
//    std::multimap<int,size_t> idxs;
//    for (size_t i = 0; i < size; ++i)
//    {
//        idxs.insert(std::pair<int, size_t>(objs[i].br().y, i));
//    }
//    while (idxs.size() > 0)
//    {
//        // grab the last rectangle
//        auto lastElem = --std::end(idxs);
//        const cv::Rect& rect1 = objs[lastElem->second];

//        objs_out.push_back(rect1);

//        idxs.erase(lastElem);

//        for (auto pos = std::begin(idxs); pos != std::end(idxs); )
//        {
//            // grab the current rectangle
//            const cv::Rect& rect2 = objs[pos->second];

//            float intArea = (rect1 & rect2).area();
//            float unionArea = rect1.area() + rect2.area() - intArea;
//            float overlap = intArea / unionArea;

//            // if there is sufficient overlap, suppress the current bounding box
//            if (overlap > 0.2f)
//            {
//                pos = idxs.erase(pos);
//            }
//            else
//            {
//                ++pos;
//            }
//        }
//    }
    //*********************************
    //nms end

    //*********************************
    vector<Rect> objs_out;
        objs_out=objs;
        *detectNum=objs_out.size();
    if (*detectNum>0)
    {
        for (size_t i = 0; i < objs_out.size(); i++)
        {
            Rect r = objs_out[i];
            rectangle(frame1, r, Scalar(255, 0, 0));
        }
        if(strlen(path)>0)
        {
        //imwrite(path,frame1);
        }

    }
        return *detectNum;
    }

