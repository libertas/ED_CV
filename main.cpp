#include <opencv2/opencv.hpp>
#include <mutex>
#include <thread>

#include <SimCom.h>
#include <ServiceLayer.h>

#include "imageProcessing.h"


using namespace cv;
using namespace std;

Mat camFrame;
uint8_t camFrameCount = 0;
uint8_t camFrameDisplayCount = 0;
mutex camFrameLock;

VideoCapture inputVideo(0);

bool tasksRunning = true;

void frameProducerTask()
{
	Mat frame;
	
	while(tasksRunning)
	{
		inputVideo >> frame;
		
		camFrameLock.lock();
		
		camFrame = frame;
		
		camFrameCount = 1;
		camFrameDisplayCount = 1;
		
		camFrameLock.unlock();
	}
}

void frameProcessorTask()
{
	Mat frame;
	
	while(tasksRunning)
	{
		camFrameLock.lock();
		
		if(camFrameCount == 0) {
			camFrameLock.unlock();
			
			std::this_thread::sleep_for(std::chrono::nanoseconds(1));
			
			continue;
		}
		
		camFrameCount = 0;
		
		frame = camFrame;
		
		camFrameLock.unlock();
	}
}

void displayTask()
{
	Mat frame;
	
	while(tasksRunning)
	{
		camFrameLock.lock();
		
		if(camFrameDisplayCount == 1) {
			frame = camFrame;
			
			camFrameDisplayCount = 0;
			
			camFrameLock.unlock();
			
			imshow("frame", frame);
			
			continue;
		}
		
		
		camFrameLock.unlock();
		
		char c = waitKey(1);
		if(c == 27) {
			tasksRunning = false;

			break;
		}
	}
}

int main()
{
  Mat img;
  
  thread producer(frameProducerTask);
  thread processor(frameProcessorTask);
  thread displayer(displayTask);

  producer.join();
  processor.join();
  displayer.join();

  return 0;
}
