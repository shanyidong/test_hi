/******************************************************************************
  A simple program of Hisilicon mpp implementation.
  Copyright (C), 2012-2020, Hisilicon Tech. Co., Ltd.
 ******************************************************************************
    Modification:  2013-7 Created
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "cv.h"
#include "highgui.h"
#include "cxcore.h"
#include "sample_comm.h"
#include "mpi_vgs.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "obj_detect.h"
#include "mpi_ive.h"
#include "hi_comm_ive.h"
#include "parm.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
//VIDEO_FRAME_INFO_S g_stFrameInfo;
HI_BOOL g_bUserVdecBuf = HI_FALSE;
HI_BOOL g_bUserCommVb = HI_FALSE;
//int Channel_Nums=18;
//int batch=15;
HI_U32  u32BlkSize;
HI_U32 u32PicLStride            = 0;
HI_U32 u32PicCStride            = 0;
HI_U32 u32LumaSize              = 0;
HI_U32 u32ChrmSize              = 0;
HI_U32 u32OutWidth              = 480;//480 240
HI_U32 u32OutHeight             = 270;//270 136
VB_POOL hPool  = VB_INVALID_POOLID;
VdecThreadParam stVdecSend[VDEC_MAX_CHN_NUM];
VdecThreadParam* getVdecThreadParam(int chnn)
{
    return &stVdecSend[chnn];
}

/******************************************************************************
* function : to process abnormal case                                         
******************************************************************************/
void SAMPLE_VGS_HandleSig(HI_S32 signo)
{
    if (SIGINT == signo || SIGTSTP == signo)
    {
        if (g_bUserVdecBuf)
        {
            //HI_MPI_VDEC_ReleaseImage(0, &g_stFrameInfo);
            g_bUserVdecBuf = HI_FALSE;
        }
        if (g_bUserCommVb)
        {
            //HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
            g_bUserCommVb = HI_FALSE;
        }
        SAMPLE_COMM_SYS_Exit();
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
    }
    exit(-1);
}

HI_VOID SAMPLE_VGS_Usage(HI_VOID)
{
    printf("\n\n/************************************/\n");
    printf("please choose the case which you want to run:\n");
    printf("\t0:  Decompress tile picture from vdec\n");
    printf("\tq:  quit the whole sample\n");
    printf("sample command:");
}
int num_fps=0;
void Get2Process(int chn)
{
    int n_channel=0;
    n_channel=chn;
    int p=0;
    while(1)
    {

        IVE_SRC_IMAGE_S stSrc, stDst;
        IVE_CSC_CTRL_S stCscCtrl;
        HI_S32 s32Ret;
        IVE_HANDLE hIveHandle;
        HI_BOOL bFinish,bBlock;
        VIDEO_FRAME_INFO_S stFrmInfo;
        VGS_HANDLE hHandle;
        SAMPLE_MEMBUF_S g_stMem;
        g_stMem.hPool = hPool;
        //VIDEO_FRAME_INFO_S stFrameInfo;
        VGS_TASK_ATTR_S stTask;
        VdecThreadParam *Param =&stVdecSend[n_channel];
        Param->switchround=1;
        if(Param->s32ChnId+1>Channel_Nums)
        {
            printf("s32ChnId fault............\n");
        }
        VIDEO_FRAME_INFO_S g_stFrameInfo;
        int i;
        int chn_bind=Param->s32ChnId%batch;
        //    for(i =0;i<1;i++)
        //    {
        Param->Sending=HI_TRUE;
        //usleep(50000);

        //        while(Param->switchround!=4)
        //        {
        //            usleep(500);
        //        }

        //usleep(2000);

        VDEC_CHN_STAT_S pstStat;
        //printf("123 channel %d \n",Param->s32ChnId+1);
        //printf("starting get image NO : %d round: %d\n",Param->s32ChnId+1,i+1);

        while(1)
        {
            p++;
            HI_BOOL s32Ret = HI_MPI_VDEC_GetImage(chn_bind, &g_stFrameInfo, 2000);
            if(s32Ret != HI_SUCCESS)
            {
                SAMPLE_PRT("get vdec image failed NO : %d round: %d error : %d\n",Param->s32ChnId+1,i+1,s32Ret);
                //Param->Sending=HI_FALSE;
                usleep(1000);
                continue;
                //return;
            }
            if(p%1==0)//4
            {
                break;
            }
            else
            {
                HI_MPI_VDEC_ReleaseImage(chn_bind, &g_stFrameInfo);
            }
        }
//                while(1)
//                {
//                    HI_MPI_VDEC_Query(chn_bind,&pstStat);
//                    if(pstStat.u32LeftStreamFrames==0)
//                    {
//                        HI_BOOL s32Ret = HI_MPI_VDEC_GetImage(chn_bind, &g_stFrameInfo, 2000);
//                        if(s32Ret != HI_SUCCESS)
//                        {
//                            SAMPLE_PRT("156 get vdec image failed NO : %d round: %d error : %d\n",Param->s32ChnId+1,i+1,s32Ret);

//                        }
//                    }
//                    else if(pstStat.u32LeftStreamFrames=1)
//                    {
//                    HI_BOOL s32Ret = HI_MPI_VDEC_GetImage(chn_bind, &g_stFrameInfo, 2000);
//                    if(s32Ret != HI_SUCCESS)
//                    {
//                        SAMPLE_PRT("165 get vdec image failed NO : %d round: %d error : %d\n",Param->s32ChnId+1,i+1,s32Ret);
//                        //Param->Sending=HI_FALSE;
//                        //usleep(1000);
//                        //continue;
//                        //return;
//                    }
//                    else
//                    {
//                        break;
//                    }

//                        //HI_MPI_VDEC_ReleaseImage(chn_bind, &g_stFrameInfo);
//                    }
//                    else
//                    {
//                        HI_MPI_VDEC_GetImage(chn_bind, &g_stFrameInfo, 2000);
//                        HI_MPI_VDEC_ReleaseImage(chn_bind, &g_stFrameInfo);
//                    }
//                }
        //printf("getimage done NO : %d round: %d\n",Param->s32ChnId+1,i+1);
        //printf("146 channel %d \n",Param->s32ChnId+1);
        Param->Sending=HI_FALSE;
        Param->switchround=2;
        //HI_MPI_VDEC_Query(chn_bind,&pstStat);
//        while(pstStat.u32LeftStreamFrames>0)
//        {
//            VIDEO_FRAME_INFO_S stFrm;
//            printf("Query chnn %d u32LeftStreamFrames is %d\n",chn_bind,pstStat.u32LeftStreamFrames);
//            HI_MPI_VDEC_GetImage(chn_bind,&stFrm,-1);
//            HI_MPI_VDEC_ReleaseImage(chn_bind, &stFrm);
//        }
        //printf("Query chnn %d u32LeftStreamFrames is %d\n",chn_bind,pstStat.u32LeftStreamFrames);
        //printf("chnn %d\n",Param->s32ChnId+1);
        //test
        if((Param->s32ChnId+1)==2)
        {
            num_fps++;
        }


        g_bUserVdecBuf = HI_TRUE;
        //printf("173\n");
        while((g_stMem.hBlock = HI_MPI_VB_GetBlock(g_stMem.hPool, u32BlkSize,NULL)) == VB_INVALID_HANDLE)
        {
            ;
        }
        //printf("178\n");
        g_bUserCommVb = HI_TRUE;

        g_stMem.u32PhyAddr = HI_MPI_VB_Handle2PhysAddr(g_stMem.hBlock);

        g_stMem.pVirAddr = (HI_U8 *) HI_MPI_SYS_Mmap( g_stMem.u32PhyAddr, u32BlkSize );
        if(g_stMem.pVirAddr == NULL)
        {
            SAMPLE_PRT("Mem dev may not open\n");
            HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
            g_bUserCommVb = HI_FALSE;
            HI_MPI_VDEC_ReleaseImage(0, &g_stFrameInfo);
            g_bUserVdecBuf = HI_FALSE;
            //goto END3;
        }

        memset(&stFrmInfo.stVFrame, 0, sizeof(VIDEO_FRAME_S));
        stFrmInfo.stVFrame.u32PhyAddr[0] = g_stMem.u32PhyAddr;
        stFrmInfo.stVFrame.u32PhyAddr[1] = stFrmInfo.stVFrame.u32PhyAddr[0] + u32LumaSize;
        stFrmInfo.stVFrame.u32PhyAddr[2] = stFrmInfo.stVFrame.u32PhyAddr[1] + u32ChrmSize;


        stFrmInfo.stVFrame.pVirAddr[0] = g_stMem.pVirAddr;
        stFrmInfo.stVFrame.pVirAddr[1] = (HI_U8 *) stFrmInfo.stVFrame.pVirAddr[0] + u32LumaSize;
        stFrmInfo.stVFrame.pVirAddr[2] = (HI_U8 *) stFrmInfo.stVFrame.pVirAddr[1] + u32ChrmSize;

        stFrmInfo.stVFrame.u32Width     = u32OutWidth;
        stFrmInfo.stVFrame.u32Height    = u32OutHeight;
        stFrmInfo.stVFrame.u32Stride[0] = u32PicLStride;
        stFrmInfo.stVFrame.u32Stride[1] = u32PicLStride;
        stFrmInfo.stVFrame.u32Stride[2] = u32PicLStride;

        stFrmInfo.stVFrame.enCompressMode = COMPRESS_MODE_NONE;
        stFrmInfo.stVFrame.enPixelFormat  = PIXEL_FORMAT_YUV_SEMIPLANAR_420;
        stFrmInfo.stVFrame.enVideoFormat  = VIDEO_FORMAT_LINEAR;

        stFrmInfo.stVFrame.u64pts     = (i * 40);
        stFrmInfo.stVFrame.u32TimeRef = (i * 2);

        stFrmInfo.u32PoolId = hPool;

        s32Ret = HI_MPI_VGS_BeginJob(&hHandle);
        if(s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_VGS_BeginJob failed %x\n",s32Ret);
            //SAMPLE_PRT("HI_MPI_VGS_BeginJob failed %#x\n", s32Ret);
            HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
            g_bUserCommVb = HI_FALSE;
            HI_MPI_VDEC_ReleaseImage(0, &g_stFrameInfo);
            g_bUserVdecBuf = HI_FALSE;
            //goto END3;
        }

        memcpy(&stTask.stImgIn,&g_stFrameInfo,sizeof(VIDEO_FRAME_INFO_S));

        memcpy(&stTask.stImgOut ,&stFrmInfo,sizeof(VIDEO_FRAME_INFO_S));
        s32Ret = HI_MPI_VGS_AddScaleTask(hHandle, &stTask);
        if(s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_VGS_AddScaleTask failed %x\n",s32Ret);
            //SAMPLE_PRT("HI_MPI_VGS_AddScaleTask failed\n");
            HI_MPI_VGS_CancelJob(hHandle);
            HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
            g_bUserCommVb = HI_FALSE;
            HI_MPI_VDEC_ReleaseImage(0, &g_stFrameInfo);
            g_bUserVdecBuf = HI_FALSE;
            //goto END3;
        }



        //printf("247\n");
        s32Ret = HI_MPI_VGS_EndJob(hHandle);
        if(s32Ret != HI_SUCCESS)
        {
            printf("HI_MPI_VGS_EndJob failed %x\n",s32Ret);
            //SAMPLE_PRT("HI_MPI_VGS_EndJob failed\n");
            HI_MPI_VGS_CancelJob(hHandle);
            HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
            g_bUserCommVb = HI_FALSE;
            HI_MPI_VDEC_ReleaseImage(0, &g_stFrameInfo);
            g_bUserVdecBuf = HI_FALSE;
            //goto END3;
        }




        /* --------------------convert image-------------------------------*/
        stSrc.u16Width = stFrmInfo.stVFrame.u32Width;
        stSrc.u16Height = stFrmInfo.stVFrame.u32Height;
        stSrc.u32PhyAddr[0] = stFrmInfo.stVFrame.u32PhyAddr[0];
        stSrc.u32PhyAddr[1] = stFrmInfo.stVFrame.u32PhyAddr[1];
        stSrc.u16Stride[0] = stFrmInfo.stVFrame.u32Stride[0];
        stSrc.u16Stride[1] = stFrmInfo.stVFrame.u32Stride[1];
        //       stSrc.pu8VirAddr[0]=g_stFrameInfo.stVFrame.pVirAddr[0];
        //       stSrc.pu8VirAddr[1]=g_stFrameInfo.stVFrame.pVirAddr[1];
        //       stSrc.pu8VirAddr[2]=g_stFrameInfo.stVFrame.pVirAddr[2];
        stSrc.enType = IVE_IMAGE_TYPE_YUV420SP;
        stCscCtrl.enMode = IVE_CSC_MODE_PIC_BT709_YUV2RGB;
        stDst.u32PhyAddr[0]=0;
        stDst.pu8VirAddr[0]=HI_NULL;
        stDst.u16Height=stSrc.u16Height = stFrmInfo.stVFrame.u32Height;
        stDst.u16Width=stFrmInfo.stVFrame.u32Width;
        stDst.u16Stride[0] = stFrmInfo.stVFrame.u32Width;
        stDst.enType = IVE_IMAGE_TYPE_U8C3_PACKAGE;
        HI_U16 s=stDst.u16Height;
        HI_U16 d = stDst.u16Stride[0];
        HI_U32 len = s*d*8;
        s32Ret = HI_MPI_SYS_MmzAlloc_Cached(&stDst.u32PhyAddr, &stDst.pu8VirAddr,"User", HI_NULL, len);
        if(s32Ret!=HI_SUCCESS)
        {
            printf("can't HI_MPI_SYS_MmzAlloc_Cached %x\n",s32Ret);
            return HI_NULL;
        }
        s32Ret = HI_MPI_IVE_CSC(&hIveHandle, &stSrc, &stDst,&stCscCtrl, HI_TRUE);
        if(s32Ret!=HI_SUCCESS)
        {
            printf("can't HI_MPI_IVE_CSC %x\n",s32Ret);
            return HI_NULL;
        }
        bBlock = HI_FALSE;
        s32Ret = HI_MPI_IVE_Query(hIveHandle, &bFinish, bBlock);
        while(HI_ERR_IVE_QUERY_TIMEOUT == s32Ret)
        {
            usleep(100);
            s32Ret = HI_MPI_IVE_Query(hIveHandle,&bFinish,bBlock);
        }
        //printf("csc finish:\n");
        //printf("293 channel %d \n",Param->s32ChnId+1);
        IplImage *yframe = cvCreateImageHeader(cvSize(stFrmInfo.stVFrame.u32Width,stFrmInfo.stVFrame.u32Height),IPL_DEPTH_8U,3);
        //void* pirr=HI_MPI_SYS_Mmap(stDst.u32PhyAddr[0], g_stFrameInfo.stVFrame.u32Width*g_stFrameInfo.stVFrame.u32Height*3);
        cvSetData(yframe,stDst.pu8VirAddr[0],yframe->widthStep);
        //printf("168\n");
        char path[100];
        //cvCvtColor(yframe,yframe,CV_BGR2RGB);
        sprintf(path,"/mnt/mydir/yangwenquan/lll/3536pic/3536Channel%d.png",Param->s32ChnId+1);
        /*^^^^^^^^^^^^^^^^^^^^^^^^^^^ time test ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
        if((Param->s32ChnId+1)==2)
        {
            sprintf(path,"/mnt/mydir/yangwenquan/lll/3536pic/frame%d.png",num_fps);
            //cvSaveImage(path,yframe,0);
        }
        //printf("307 channel %d \n",Param->s32ChnId+1);
        /*^^^^^^^^^^^^^^^^^^^^^^^^^^^ time test ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/
        //cvSaveImage(path,yframe,0);
        //printf("171\n");
        int detectNum=0;
        //        //******************image detect*********************//
        CvMat *mat = cvCreateMat(yframe->height, yframe->width, CV_8UC3);    //注意height和width的顺序
        cvConvert(yframe, mat);
        if(mat->rows==0)
        {
            printf("307\n");
        }
        //printf("319 channel %d \n",Param->s32ChnId+1);
        detectRun(mat,&detectNum);
        //        if(Param->s32ChnId+1==18)
        //        {
        //            printf("channel %d num is %d\n",Param->s32ChnId+1,detectNum);
        //        }
        cvReleaseMat(&mat);
        //        //********************image detect*******************//

        //        printf("328 channel %d \n",Param->s32ChnId+1);
        cvReleaseImageHeader(&yframe);


        //HI_MPI_SYS_MmzFree(stSrc.u32PhyAddr, stSrc.pu8VirAddr);
        // HI_MPI_SYS_MmzFree(stSrc.u32PhyAddr[0], stSrc.pu8VirAddr[0]);

        //printf("333 channel %d \n",Param->s32ChnId+1);
        //HI_S32 S=HI_MPI_SYS_Munmap(pirr, g_stFrameInfo.stVFrame.u32Width*g_stFrameInfo.stVFrame.u32Height*3);
        //HI_MPI_SYS_MmzFlushCache(stDst.u32PhyAddr[0] , stDst.pu8VirAddr[0] , stDst.u16Height * stDst.u16Width*8);
        HI_MPI_SYS_MmzFree(stDst.u32PhyAddr[0],stDst.pu8VirAddr[0]);
        HI_MPI_SYS_Munmap(g_stMem.pVirAddr,u32BlkSize);
        g_bUserCommVb = HI_FALSE;
        g_bUserVdecBuf = HI_FALSE;
        HI_MPI_VB_ReleaseBlock(g_stMem.hBlock);
        HI_MPI_VDEC_ReleaseImage(chn_bind, &g_stFrameInfo);

        /* --------------------convert image-------------------------------*/
        fflush(stdout);
        //}


        //usleep(50000);
        //printf("349 channel %d \n",Param->s32ChnId+1);
        if((Param->s32ChnId+batch)>(Channel_Nums-1))
        {
            n_channel=Param->s32ChnId%batch;
        }
        else
        {
            n_channel=Param->s32ChnId+batch;
        }

        if((n_channel+1)>Channel_Nums)
        {
            printf("n_channel error %d\n",n_channel);
        }
        //printf("VGS my channel is %d  next dst channel is %d\n",Param->s32ChnId+1,n_channel+1);

        //Get2Process(n_channel);
    }
}

HI_S32 SAMPLE_VGS_Decompress_TilePicture(HI_VOID)
{
    VB_CONF_S stVbConf, stModVbConf;
    HI_S32 i = 0, s32Ret = HI_SUCCESS;
    VDEC_CHN_ATTR_S stVdecChnAttr[VDEC_MAX_CHN_NUM];

    SIZE_S stSize;


    pthread_t   VdecThread[2*VDEC_MAX_CHN_NUM];

    pthread_t Threadn[batch];
    stSize.u32Width  = HD_WIDTH;
    stSize.u32Height = HD_HEIGHT;

    u32BlkSize = u32OutWidth*u32OutHeight*3>>1;

//    s32Ret =HI_MPI_VB_Exit();
//    if(s32Ret != HI_SUCCESS)
//    {
//        SAMPLE_PRT("HI_MPI_VB_Exit fail for %#x!\n", s32Ret);
//    }
//    s32Ret =HI_MPI_SYS_Exit();
//    if(s32Ret != HI_SUCCESS)
//    {
//        SAMPLE_PRT("HI_MPI_SYS_Exit fail for %#x!\n", s32Ret);
//    }

    /************************************************
    step1:  init SYS and common VB 
    *************************************************/
    SAMPLE_COMM_VDEC_Sysconf(&stVbConf, &stSize);
    s32Ret = SAMPLE_COMM_SYS_Init(&stVbConf);
    if(s32Ret != HI_SUCCESS)
    {
        SAMPLE_PRT("init sys fail for %#x!\n", s32Ret);
        goto END1;
    }
	
    /************************************************
    step2:  init mod common VB
    *************************************************/
    SAMPLE_COMM_VDEC_ModCommPoolConf(&stModVbConf, PT_H264, &stSize, 1);	
    s32Ret = SAMPLE_COMM_VDEC_InitModCommVb(&stModVbConf);
    if(s32Ret != HI_SUCCESS)
    {	    	
        SAMPLE_PRT("init mod common vb fail for %#x!\n", s32Ret);
        goto END1;
    }


    /******************************************
    step 3: create private pool on ddr0
    ******************************************/
    hPool   = HI_MPI_VB_CreatePool( u32BlkSize, 100,NULL);
    if (hPool == VB_INVALID_POOLID)
    {
        SAMPLE_PRT("HI_MPI_VB_CreatePool failed! \n");
        goto END1;
    }

   
    /************************************************
    step4:  start VDEC
    *************************************************/
    SAMPLE_COMM_VDEC_ChnAttr(batch, &stVdecChnAttr[0], PT_H264, &stSize);
    s32Ret = SAMPLE_COMM_VDEC_Start(batch, &stVdecChnAttr[0]);
    if(s32Ret != HI_SUCCESS)
    {	
        SAMPLE_PRT("start VDEC fail for %#x!\n", s32Ret);
        goto END2;
    }

    /************************************************
    step4:  send stream to VDEC
    *************************************************/
    SAMPLE_COMM_VDEC_ThreadParam(batch, &stVdecSend[0], &stVdecChnAttr[0], SAMPLE_1080P_H264_PATH);
    int k;
            strcpy(stVdecSend[0].RtspStr,SAMPLE_1080P_H264_PATH);
            strcpy(stVdecSend[1].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/2.h264");
            strcpy(stVdecSend[2].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/3.h264");
            strcpy(stVdecSend[3].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/4.h264");
            strcpy(stVdecSend[4].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/5.h264");
            strcpy(stVdecSend[5].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/6.h264");
            strcpy(stVdecSend[6].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/7.h264");
            strcpy(stVdecSend[7].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/8.h264");
            strcpy(stVdecSend[8].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/9.h264");
            strcpy(stVdecSend[9].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/10.h264");
            strcpy(stVdecSend[10].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/11.h264");
            strcpy(stVdecSend[11].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/12.h264");
            strcpy(stVdecSend[12].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/13.h264");
            strcpy(stVdecSend[13].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/14.h264");
            strcpy(stVdecSend[14].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/15.h264");
            strcpy(stVdecSend[15].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/16.h264");
            strcpy(stVdecSend[16].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/17.h264");
            strcpy(stVdecSend[17].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/18.h264");
            strcpy(stVdecSend[18].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/19.h264");
            strcpy(stVdecSend[19].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/20.h264");
            strcpy(stVdecSend[20].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/21.h264");
            strcpy(stVdecSend[21].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/22.h264");
            strcpy(stVdecSend[22].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/23.h264");
            strcpy(stVdecSend[23].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/24.h264");
            strcpy(stVdecSend[24].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/25.h264");
            strcpy(stVdecSend[25].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/26.h264");
            strcpy(stVdecSend[26].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/27.h264");
            strcpy(stVdecSend[27].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/28.h264");
            strcpy(stVdecSend[28].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/29.h264");
            strcpy(stVdecSend[29].RtspStr,"/mnt/mydir/yangwenquan/lll/264file/30.h264");

//        strcpy(stVdecSend[0].RtspStr,"rtsp://admin:hisense.1@10.16.3.124:554/0");
//        strcpy(stVdecSend[1].RtspStr,"rtsp://admin:hisense.1@10.16.3.124:554/0");
//        strcpy(stVdecSend[2].RtspStr,"rtsp://admin:hisense.1@10.16.3.124:554/0");
//        strcpy(stVdecSend[3].RtspStr,"rtsp://admin:hisense.1@10.16.3.124:554/0");
//        strcpy(stVdecSend[4].RtspStr,"rtsp://admin:hisense.1@10.16.3.124:554/0");
//        strcpy(stVdecSend[5].RtspStr,"rtsp://admin:hisense.1@10.16.3.125:554/0");
//        strcpy(stVdecSend[6].RtspStr,"rtsp://admin:hisense.1@10.16.3.125:554/0");
//        strcpy(stVdecSend[7].RtspStr,"rtsp://admin:hisense.1@10.16.3.125:554/0");
//        strcpy(stVdecSend[8].RtspStr,"rtsp://admin:hisense.1@10.16.3.125:554/0");
//        strcpy(stVdecSend[9].RtspStr,"rtsp://admin:hisense.1@10.16.3.125:554/0");
//        strcpy(stVdecSend[10].RtspStr,"rtsp://admin:hisense.1@10.16.3.121:554/0");
//        strcpy(stVdecSend[11].RtspStr,"rtsp://admin:hisense.1@10.16.3.121:554/0");
//        strcpy(stVdecSend[12].RtspStr,"rtsp://admin:hisense.1@10.16.3.121:554/0");
//        strcpy(stVdecSend[13].RtspStr,"rtsp://admin:hisense.1@10.16.3.121:554/0");
//        strcpy(stVdecSend[14].RtspStr,"rtsp://admin:hisense.1@10.16.3.121:554/0");
//        strcpy(stVdecSend[15].RtspStr,"rtsp://admin:hisense.1@10.16.3.128:554/0");
//        strcpy(stVdecSend[16].RtspStr,"rtsp://admin:hisense.1@10.16.3.128:554/0");
//        strcpy(stVdecSend[17].RtspStr,"rtsp://admin:hisense.1@10.16.3.128:554/0");
//        strcpy(stVdecSend[18].RtspStr,"rtsp://admin:hisense.1@10.16.3.128:554/0");
//        strcpy(stVdecSend[19].RtspStr,"rtsp://admin:hisense.1@10.16.3.128:554/0");
//        strcpy(stVdecSend[20].RtspStr,"rtsp://admin:hisense.1@10.16.3.127:554/0");
//        strcpy(stVdecSend[21].RtspStr,"rtsp://admin:hisense.1@10.16.3.127:554/0");
//        strcpy(stVdecSend[22].RtspStr,"rtsp://admin:hisense.1@10.16.3.127:554/0");
//        strcpy(stVdecSend[23].RtspStr,"rtsp://admin:hisense.1@10.16.3.127:554/0");
//        strcpy(stVdecSend[24].RtspStr,"rtsp://admin:hisense.1@10.16.3.127:554/0");
//        strcpy(stVdecSend[25].RtspStr,"rtsp://admin:abcd1234@10.16.3.137:554/0");
//        strcpy(stVdecSend[26].RtspStr,"rtsp://admin:abcd1234@10.16.3.137:554/0");
//        strcpy(stVdecSend[27].RtspStr,"rtsp://admin:abcd1234@10.16.3.137:554/0");
//        strcpy(stVdecSend[28].RtspStr,"rtsp://admin:abcd1234@10.16.3.137:554/0");
//        strcpy(stVdecSend[29].RtspStr,"rtsp://admin:abcd1234@10.16.3.137:554/0");
        av_register_all();
        avformat_network_init();
    for(k=0;k<Channel_Nums;k++)
    {
//        if(stVdecSend[k].cFileName != 0)
//        {
            //stVdecSend[k].fpStrm = fopen(stVdecSend[k].cFileName, "rb");

            //strcpy(stVdecSend[k].RtspStr,"rtsp://admin:hisense.1@10.16.3.121:554/0");
//            stVdecSend[k].fpStrm = fopen(SAMPLE_1080P_H264_PATH, "rb");
//            if(stVdecSend[k].fpStrm  == NULL)
//            {
//                printf("SAMPLE_TEST:can't open file %s in send stream thread channel:%d\n",stVdecSend[k].cFileName, k+1);
//            }

            stVdecSend[k].pFormatCtx=avformat_alloc_context();
            av_dict_set(&stVdecSend[k].options, "rtsp_transport", "tcp", 0);  //以udp方式打开，如果以tcp方式打开将udp替换为tcp
            av_dict_set(&stVdecSend[k].options, "stimeout", "3000000", 0);
//            if(avformat_open_input(&stVdecSend[k].pFormatCtx,stVdecSend[k].RtspStr,NULL,&stVdecSend[k].options)!=0)////打开网络流或文件流
//            {
//               printf("460 Couldn't open input stream channel NO.%d.\n",k+1);
//            }
            stVdecSend[k].packet=(AVPacket *)av_malloc(sizeof(AVPacket));
            stVdecSend[k].s32ChnId=k;
            stVdecSend[k].s32MilliSec     = 0;
            stVdecSend[k].s32IntervalTime = 1;
            stVdecSend[k].u64PtsInit      = 0;
            stVdecSend[k].u64PtsIncrease  = 0;
            stVdecSend[k].eCtrlSinal      = VDEC_CTRL_START;
            stVdecSend[k].bLoopSend       = HI_TRUE;
            stVdecSend[k].enType          = PT_H264;
            stVdecSend[k].s32MinBufSize   = (3 * stSize.u32Width * stSize.u32Height)>>1;
            stVdecSend[k].s32StreamMode=VIDEO_MODE_STREAM;
            stVdecSend[k].Sending=HI_FALSE;
            stVdecSend[k].switchround=4;
//        }
    }

//    stVdecSend[1].RtspStr="";
    int c_Num=0;
    if(batch<=Channel_Nums)
    {
        c_Num=batch;
    }
    else
    {
        c_Num=Channel_Nums;
    }
    SAMPLE_COMM_VDEC_StartSendStream(c_Num, &stVdecSend[0], &VdecThread[0]);
    u32PicLStride = CEILING_2_POWER(u32OutWidth, SAMPLE_SYS_ALIGN_WIDTH);
    u32PicCStride = CEILING_2_POWER(u32OutWidth, SAMPLE_SYS_ALIGN_WIDTH);
    u32LumaSize = (u32PicLStride * u32OutHeight);
    u32ChrmSize = (u32PicCStride * u32OutHeight) >> 2;
    //***********************************//
    //algorithm initial//
//    cv::CascadeClassifier cascade;
//    char* cascadeName = "model//cascade.xml";
//    if (!cascade.load(cascadeName))
//    {
//        //cerr << "ERROR: Could not load classifier cascade" << endl;
//        return -1;
//    }
    //***********************************//

    int chn_i;
        for(chn_i=0;chn_i<c_Num;chn_i++)
            {
                pthread_create(&Threadn[chn_i], 0, Get2Process,chn_i);
            }
        printf("###-------------------- service start  --------------------###\n");
                int mins=0;
    while(1)
    {
        mins++;
        sleep(60);
        printf("-------------------- %d mins passed  --------------------\n",mins);
        printf("-------------------- frame %d/min  --------------------\n",num_fps);
        //num_fps=0;
    }
    printf("finish\n");
    SAMPLE_COMM_VDEC_StopSendStream(batch, &stVdecSend[0], &VdecThread[0]);

    
END3:
    SAMPLE_COMM_VDEC_Stop(1);		

END2:
    HI_MPI_VB_DestroyPool( hPool );

END1:
    SAMPLE_COMM_SYS_Exit();	

    
    return s32Ret;
}


/******************************************************************************
* function    : main()
* Description : vgs sample
******************************************************************************/
int main(int argc, char *argv[])
{
    HI_S32 s32Ret = HI_SUCCESS;
    char ch;
    HI_BOOL bExit = HI_FALSE;
    //signal(SIGINT, SAMPLE_VGS_HandleSig);
    //signal(SIGTERM, SAMPLE_VGS_HandleSig);
  
    /******************************************
    1 choose the case
    ******************************************/
    while (1)
    {
        SAMPLE_VGS_Usage();
//        ch = getchar();
//        if (10 == ch)
//        {
//            continue;
//        }
//        getchar();
//        switch (ch)
//        {
//            case '0':
//            {
//                SAMPLE_VGS_Decompress_TilePicture();
//                break;
//            }
//            case 'q':
//            case 'Q':
//            {
//                bExit = HI_TRUE;
//                break;
//            }

//            default :
//            {
//                printf("input invaild! please try again.\n");
//                break;
//            }
//        }

//        if (bExit)
//        {
//            break;
//        }
        SAMPLE_VGS_Decompress_TilePicture();
        sleep(20);
    }

    return s32Ret;
}

