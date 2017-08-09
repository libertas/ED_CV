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
mutex camFrameLock;

Mat camFrameDisplay;
uint8_t camFrameDisplayCount = 0;
mutex camFrameDisplayLock;


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
		
		camFrameLock.unlock();
	}
}

void frameProcessorTask()
{
	Mat frame;
	
	uint32_t fps_count = 0;
	uint32_t fps = 0;
	
	auto lasttime = std::chrono::system_clock::now();
	
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
		
		/* Process the frame */
		fps_count++;
		auto thistime = std::chrono::system_clock::now();
		auto difftime = thistime - lasttime;
		if(difftime > std::chrono::duration<int>(1)) {
			fps = fps_count;
			fps_count = 0;
			lasttime = thistime;
		}
		
		
		/* Use the processed data */
		cout << "fps:" << fps << endl;
		
		camFrameDisplayLock.lock();
		
		camFrameDisplayCount = 1;
		camFrameDisplay = frame;
		
		camFrameDisplayLock.unlock();
	}
}

void displayTask()
{
	Mat frame;
	
	while(tasksRunning)
	{
		camFrameDisplayLock.lock();
		
		if(camFrameDisplayCount == 1) {
			frame = camFrame;
			
			camFrameDisplayCount = 0;
			
			camFrameDisplayLock.unlock();
			
			imshow("frame", frame);
			
			continue;
		}
		
		
		camFrameDisplayLock.unlock();
		
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
