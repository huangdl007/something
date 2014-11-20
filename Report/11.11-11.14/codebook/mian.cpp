#include <cv.h>           
#include <highgui.h>
#include <cxcore.h>
#include <stdlib.h>

using namespace cv;

#define CHANNELS 3
int maxMod[CHANNELS];
int minMod[CHANNELS];
unsigned bounds[CHANNELS];

typedef struct CodeWord
{
	uchar IHigh[CHANNELS];
	uchar ILow[CHANNELS];
	uchar max[CHANNELS];
	uchar min[CHANNELS];
	int t_last;
	int stale;
}CW;

typedef struct CodeBook
{
	CW **cw;
	int numEntries;
	int t;
}CB;

int updateCodeBook(uchar *pixel, CB &cb)
{
	cb.t++;

	//set high and low bound
	unsigned int high[CHANNELS], low[CHANNELS];
	for(int i=0; i<CHANNELS; i++)
	{
		high[i] = pixel[i] + bounds[i];
		if(high[i] > 255)
			high[i] = 255;

		low[i] = pixel[i] - bounds[i];
		if(low[i] < 0)
			low[i] = 0;
	}

	int i, matchedChannels;
	for(i=0; i<cb.numEntries; i++)
	{
		matchedChannels = 0;
		for(int j=0; j<CHANNELS; j++)
		{
			if(pixel[j] >= cb.cw[i]->ILow[j] && pixel[j] <= cb.cw[i]->IHigh[j])
				matchedChannels++;
		}

		//all channels matched then update CW(t_last, max, min, IHigh, ILow)
		if(matchedChannels == CHANNELS)
		{
			cb.cw[i]->t_last = cb.t;
			for(int j=0; j<CHANNELS; j++)
			{
				if(pixel[j] > cb.cw[i]->max[j])
					cb.cw[i]->max[j] = pixel[j];

				if(pixel[j] < cb.cw[i]->min[j])
					cb.cw[i]->min[j] = pixel[j];

				if(cb.cw[i]->IHigh[j] < high[j])
					cb.cw[i]->IHigh[j] += 1;

				if(cb.cw[i]->ILow[j] > low[j])
					cb.cw[i]->ILow[j] -= 1;
			}

			break;
		}
	}

	//no CW matched
	if(i == cb.numEntries)
	{
		CW **newCWs = new CW*[cb.numEntries+1];
		for(int j=0; j<cb.numEntries; j++)
		{
			newCWs[j] = cb.cw[j];
		}

		if(cb.numEntries)
			delete []cb.cw;
		newCWs[cb.numEntries] = new CW;
		cb.cw = newCWs;

		//assigh new CW
		for(int j=0; j<CHANNELS; j++)
		{
			cb.cw[cb.numEntries]->max[j] = pixel[j];
			cb.cw[cb.numEntries]->min[j] = pixel[j];
			cb.cw[cb.numEntries]->IHigh[j] = pixel[j] + bounds[j];
			cb.cw[cb.numEntries]->ILow[j] = pixel[j] - bounds[j];
		}
		cb.cw[cb.numEntries]->t_last = cb.t;
		cb.cw[cb.numEntries]->stale = 0;
		cb.numEntries++;
	}

	//update stale
	for(int j=0; j<cb.numEntries; j++)
	{
		cb.cw[j]->stale = cb.t - cb.cw[j]->t_last;
	}

	return i;
}

int clearCodeWord(CB &cb)
{
	//keep the CW whose stale > sb.t/2
	int threshold = cb.t / 2;
	int *flag = new int[cb.numEntries];
	int keepCount = 0;
	for(int i=0; i<cb.numEntries; i++)
	{
		if(cb.cw[i]->stale > threshold)
		{
			flag[i] = 0;
		}
		else
		{
			flag[i] = 1;
			keepCount++;
		}
	}

	CW **newCWs = new CW*[keepCount];
	cb.t = 0;
	for(int i=0,  k=0; i<cb.numEntries; i++)
	{
		if(flag[i])
		{
			newCWs[k] = cb.cw[i];
			newCWs[k]->stale = 0;
			newCWs[k]->t_last = 0;
			k++;
		}
	}

	delete []flag;
	delete []cb.cw;
	cb.cw = newCWs;
	int numCleared = cb.numEntries - keepCount;
	cb.numEntries = keepCount;

	return numCleared;
}

uchar getBackground(uchar *pixel, CB &cb)
{
	int i, matchedChannels;
	for(i=0; i<cb.numEntries; i++)
	{
		matchedChannels = 0;
		for(int j=0; j<CHANNELS; j++)
		{
			if( (pixel[j] <= cb.cw[i]->max[j] + maxMod[j]) && (pixel[j] >= cb.cw[i]->min[j] - maxMod[j]) )
				matchedChannels++;
			else
				break;
		}

		if(matchedChannels == CHANNELS)
			break;
	}

	if(i == cb.numEntries)
		return 255;

	return 0;
}
int main()
{
	//declare variables
	CvCapture*		capture;
	IplImage*		rawFrame;
	IplImage*		CBFrame;
	CB*				cB;
	int				imageLength;
	uchar*			rawPixel;

	//initialize variables
	capture = cvCreateFileCapture("codebook/tree.avi");
	if(!capture)
	{
		std::cout << "Could not open capture" << std::endl;
		return -1;
	}

	
	rawFrame = cvQueryFrame(capture);
	imageLength = rawFrame->height * rawFrame->width;
	CBFrame = cvCreateImage(cvGetSize(rawFrame), IPL_DEPTH_8U, 1);

	cvNamedWindow("Raw Frame");
	cvNamedWindow("CB");

	for(int i=0; i<CHANNELS; i++)
	{
		bounds[i] = 10;
		maxMod[i] = 20;
		minMod[i] = 20;
	}

	cB = new CB[imageLength];
	for(int i=0; i<imageLength; i++)
	{
		cB[i].numEntries = 0;
		cB[i].t = 0;
	}


	//process every frame of the video
	for(int i=0; ;i++)
	{
		//learn the background with the first 30 frames
		CvFont font;
		cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX | CV_FONT_ITALIC,0.8,0.8,0,2);
		cvSet(CBFrame, Scalar(255));

		if(i <= 30)
		{
			rawPixel = (uchar*)rawFrame->imageData;
			//update codebook
			for(int j=0; j<imageLength; j++)
			{
				updateCodeBook(rawPixel, cB[j]);
				rawPixel += 3;	//3 channels image
			}

			char str[10];
			itoa(i, str, 10);
			string s = "Learning Background " + string(str);
			cvPutText(CBFrame, s.c_str(), cvPoint(0, 100), &font, Scalar(0));

			//clear code word
			if(i == 30)
			{
				for(int k=0; k<imageLength; k++)
				{
					clearCodeWord(cB[k]);
				}
			}
		}
		else
		{
			//build the codebook for the rest frames
			rawPixel = (uchar*)rawFrame->imageData;
			uchar *CBPixel = (uchar*)CBFrame->imageData;
			for(int j=0; j<imageLength; j++)
			{
				*CBPixel = getBackground(rawPixel, cB[j]);
				CBPixel++;
				rawPixel += 3;	//3 channels image
			}
		}

		//get the next raw frame
		rawFrame = cvQueryFrame(capture);
		if(!rawFrame)
			break;
		//show the image
		cvShowImage("Raw Frame", rawFrame);
		cvShowImage("CB", CBFrame);

		cvWaitKey(100);

	}

	cvWaitKey(0);
	//release resources
	cvReleaseCapture(&capture);
	if(rawFrame)
		cvReleaseImage(&rawFrame);
	if(CBFrame)
		cvReleaseImage(&CBFrame);
	destroyAllWindows();
	delete []cB;

	return 0;
}