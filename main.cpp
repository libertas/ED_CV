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

bool motion_ack = false;


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
			cout << "fps:" << fps << endl;
		}
		
		Mat frame1;
		
		uint8_t highs[3] = {80, 80, 255};
		uint8_t lows[3] = {0, 0, 100};
		
		GaussianBlur(frame, frame1, Size(5, 5), 1.5);
		triThreshold(frame1, frame1, highs, lows);
		
		Point center;
		centerOfMass(frame1, center);
		
		char msg[4];
		((uint16_t*)msg)[0] = center.x;
		((uint16_t*)msg)[1] = center.y;
		sl_send(5, 5, msg, 4);
		
		
		
		/* Use the processed data */
		
		camFrameDisplayLock.lock();
		
		camFrameDisplayCount = 1;
		camFrameDisplay = frame;
		
		circle(camFrameDisplay, center, 10, CV_RGB(0,255,255), 8, 8, 0);
		
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
			frame = camFrameDisplay;
			
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

void callback0(char from, char to, const char* data, SIMCOM_LENGTH_TYPE length)
{
}

void callback1(char from, char to, const char* data, SIMCOM_LENGTH_TYPE length)
{
	cout << "data:\n" << data << endl;
}

void callback2(char from, char to, const char* data, SIMCOM_LENGTH_TYPE length)
{
	cout << from << " " << to << " " << data << " " << length << endl;
}

void callback5(char from, char to, const char* data, SIMCOM_LENGTH_TYPE length)
{
	if(length == 1) {
		if(data[0] == 'A') {
			motion_ack = true;
		}
	}
}

int main()
{
  simcom_init("/dev/ttyUSB0");
  sl_config(0, callback0);
  sl_config(1, callback1);
  sl_config(2, callback2);
  sl_config(5, callback5);
  
  thread producer(frameProducerTask);
  thread processor(frameProcessorTask);
  thread displayer(displayTask);

  producer.join();
  processor.join();
  displayer.join();
  
  simcom_close();

  return 0;
}
