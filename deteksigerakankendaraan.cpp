/* All rights are reserved (c) soca-research, soca.research@gmail.com */
/* SOCA-research (c) NinJa project  2014-2016 */
#define OPENCV21 0
#define OPENCV242 1

/* INCLUDE FILE */

#if OPENCV21 
#include "cv.h"
#include "highgui.h"
#include "cxcore.h"
#endif

#if OPENCV242
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#endif

/* VARIABEL YANG DIPERLUKAN*/
CvCapture* capture;
CvCapture* capture2;
IplImage* frame;
IplImage* frame_p;
IplImage* frame_c;
IplImage* frame_p_gray;
IplImage* frame_c_gray;
IplImage* silh;
IplImage* BG_Image;
cv::Mat frame_p_mat;
int frame_idx=0;
int big_car=0;
int small_car=0;
int dt=4;
int th_min=40;
int th_max=255;
int close_size=4;
int open_size=4;
int n_objects;
int n_objects_cum;
int init;
float mov_avg;
int i;

CvRect Rect;
double fourcc;
FILE *fp;

int state_cross=0;
char jam_status[100];
char file_name[200];

char *camera_vid;

int num_car=0;
char *dir_vid;
double fps=10.0f;

int main(int argc, char **argv)
{

    camera_vid=argv[1];
    dir_vid=argv[2];
  
 
    if (argc<2)
    {
        printf("Usage: deteksigerakankendaraan.exe [input_video.avi] [video_dir].\n");
        return -1;
    }
	
    capture = cvCaptureFromFile(camera_vid);
    capture2 = cvCaptureFromFile(camera_vid);


    cvSetCaptureProperty(capture, CV_CAP_PROP_POS_FRAMES, 0);
    cvSetCaptureProperty(capture2, CV_CAP_PROP_POS_FRAMES, 0);

    init=0;
    for(;;)
    {
        frame_idx++;

        if (frame_idx<dt)
        {
            init=1;
            frame_p = cvQueryFrame(capture);    
            if( !frame_p )
            break;
            continue;
        }

        if (init==1)
        {
            frame_p = cvQueryFrame(capture);    
            frame_c = cvQueryFrame(capture2);
            if( !frame_p )
            break;
            if( !frame_c )
            break;
        }

        
        /*VARIABEL UNTUK FRAME SEBELUM DAN SAAT INI*/
        frame_p_gray=cvCreateImage(cvSize(frame_c->width,frame_c->height), IPL_DEPTH_8U, 1 );
        frame_c_gray=cvCreateImage(cvSize(frame_c->width,frame_c->height), IPL_DEPTH_8U, 1 );
        frame=cvCreateImage(cvSize(frame_c->width,frame_c->height), IPL_DEPTH_8U, 1 );
        
        cvCvtColor(frame_p, frame_p_gray, CV_BGR2GRAY ); 
        cvCvtColor(frame_c, frame_c_gray, CV_BGR2GRAY );
        
        /*EKSTRAKSI FOREGROUND/OBJEK*/   
        cvAbsDiff( frame_p_gray, frame_c_gray, frame ); 
        cvThreshold( frame, frame,  th_min, th_max, CV_THRESH_BINARY ); 
        cvDilate( frame, frame, NULL, close_size);
        cvErode( frame, frame, NULL, open_size+close_size);
        cvDilate( frame, frame, NULL, open_size);

        CvMemStorage *mem;
        mem = cvCreateMemStorage(0);
        CvSeq *contours = 0;

	/*ESTIMASI JUMLAH KENDARAAN/OBJEK YANG BERGERAK*/
        n_objects=cvFindContours(frame, mem, &contours, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, cvPoint(0,0));
        cvDrawContours(frame, contours, cvScalarAll(255),cvScalarAll(255),100);
        
        for(CvSeq* c=contours; c!=NULL; c=c->h_next ) 
        { 
            Rect=cvBoundingRect(c);
            cvRectangle(frame_p, cvPoint(Rect.x,Rect.y),cvPoint(Rect.x+Rect.width,Rect.y+Rect.height),CV_RGB(0,255,0),1,8,0);
            num_car++;
        }

        if (num_car <3)
        {
	    /*PERSIAPKAN CITRA BACKGROUND*/
            BG_Image=cvCreateImage(cvSize(frame_c->width,frame_c->height), IPL_DEPTH_8U, 1 );
            cvCvtColor(frame_p, BG_Image, CV_BGR2GRAY ); 
            cvAddWeighted(frame_p_gray, 0.5, BG_Image, 0.5, 0.0, BG_Image);
            sprintf(file_name,"./%s/BG.png",dir_vid);
            cvSaveImage(file_name, BG_Image);
            BG_Image=cvCreateImage(cvSize(frame_c->width,frame_c->height), IPL_DEPTH_8U, 1 );
            BG_Image=cvLoadImage(file_name,CV_LOAD_IMAGE_GRAYSCALE);
       
            /*EKSTRAKSI FOREGROUND/OBJEK*/   
            cvAbsDiff( frame_p_gray, BG_Image, frame );
            cvThreshold( frame, frame,  th_min, th_max, CV_THRESH_BINARY ); 
            cvDilate( frame, frame, NULL, close_size);
            cvErode( frame, frame, NULL, open_size+close_size);
            cvDilate( frame, frame, NULL, open_size);
            
            CvMemStorage *mem;
            mem = cvCreateMemStorage(0);
            CvSeq *contours = 0;
            
            n_objects=cvFindContours(frame, mem, &contours, sizeof(CvContour), CV_RETR_CCOMP, CV_CHAIN_APPROX_NONE, cvPoint(0,0));
            cvDrawContours(frame, contours, cvScalarAll(255),cvScalarAll(255),100);

            for(CvSeq* c=contours; c!=NULL; c=c->h_next ) 
            { 
                 Rect=cvBoundingRect(c);
                cvRectangle(frame_p, cvPoint(Rect.x,Rect.y),cvPoint(Rect.x+Rect.width,Rect.y+Rect.height),CV_RGB(0,255,0),-1,8,0);
            }

        }

		/*MENGHITUNG TOTAL JUMLAH KENDARAAN YANG MELEWATI TITIK TERTENTU, PERLU DIPERBAIKI*/
		if (Rect.x>(frame->width/2)-5 && Rect.x<(frame->width/2)+5)
        {
           state_cross++;
        }
        
        n_objects_cum+=n_objects;
	/*MENGHITUNG RERATA JUMLAH KENDARAAN YANG MUNCUL PER TOTAL FRAME*/
        mov_avg=(float)n_objects_cum/frame_idx;

        sprintf(file_name,"./%s/%08d.png",dir_vid,frame_idx);
        cvSaveImage(file_name, frame_p);

        cvWaitKey(0);

    }
 
 }

}
    
 if (mov_avg>10) 
        printf("\"totalRerata\": %d, \"status\":\"%s\" ", (int) ceil(mov_avg), "Padat");
    else
        printf("\"totalRerata\": %d, \"status\": \"%s\" ", (int) ceil(mov_avg), "Tidak Padat");

    cvReleaseCapture(&capture);
    cvReleaseCapture(&capture2);
    return 0;

}
