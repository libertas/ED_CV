#include <opencv2/opencv.hpp>
#include <iostream>
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

char key_ack = 0;


VideoCapture inputVideo(1);

bool tasksRunning = true;

bool sendingPos = true;

void keyControl(char c)
{
	switch(c) {
	case 27:
		tasksRunning = false;
		break;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
	case '0':
		do {
			key_ack = 0;
			sl_send(5, 5, &c, 1);
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		} while(key_ack != c && key_ack != ~c);
		
		key_ack = 0;
	
		break;
	default:
		sl_send(2, 5, &c, 1);
		break;
	}
}

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
		
		uint8_t highs[3] = {160, 160, 255};
		uint8_t lows[3] = {0, 0, 200};
		
		rectangle(frame, Point(0, 0), Point(80, 480), Scalar(0, 0, 0), -1, 8, 0);
		rectangle(frame, Point(560, 0), Point(640, 480), Scalar(0, 0, 0), -1, 8, 0);
		
		pyrDown(frame, frame1, Size(frame.cols >> 1, frame.rows >> 1));
		triThreshold(frame1, frame1, highs, lows);
		GaussianBlur(frame1, frame1, Size(3, 3), 2.0);
		
		Point center;
		centerOfMass(frame1, center);
		
		center.x <<= 1;
		center.y <<= 1;
		
		//cout << "Center:" << center.x << " " << center.y << endl;
		
		if(sendingPos) {
			char msg[4];
			((uint16_t*)msg)[0] = center.x;
			((uint16_t*)msg)[1] = center.y;
			sl_send(2, 5, msg, 4);
		}
		
		
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
		keyControl(c);
	}
}

void callback0(char from, char to, const char* data, SIMCOM_LENGTH_TYPE length)
{
	uint16_t data16;
	float dataf;
	switch(length) {
	case 2:
		data16 = *((uint16_t*)data);
		cout << data16 << endl;
		break;
	case 4:
		dataf = *((float*)data);
		cout << dataf << endl;
		break;
	case 12:
		dataf = *((float*)data);
		cout << dataf << '\t';
		
		dataf = *((float*)data + 1);
		cout << dataf << '\t';
		
		dataf = *((float*)data + 2);
		cout << dataf << endl;
		
		break;
	default:
		cout << "Received a message of length " << length << endl;
		break;
	}
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
		switch(data[0]){
		case 'A':
			motion_ack = true;
			cout<< "motion_ack received" << endl;
			break;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
		case '0':
			key_ack = data[0];
			cout << "Ack Received:" << data[0] << endl;
			break;
		case ~'1':
		case ~'2':
		case ~'3':
		case ~'4':
		case ~'5':
		case ~'6':
		case ~'7':
		case ~'8':
		case ~'9':
		case ~'0':
			key_ack = data[0];
			cout << "NAck Received:" << (char)~(data[0]) << endl;
			break;
		default:
			break;
		}
	} else {
		cout << "callback5 received an unknown msg" << endl;
	}
}


uint16_t holePos[9][2];

void detectHoles()
{
	holePos[0][0] = 206; holePos[0][1] = 106;
	holePos[1][0] = 342; holePos[1][1] = 106;
	holePos[2][0] = 506; holePos[2][1] = 106;
	
	holePos[3][0] = 206; holePos[3][1] = 354;
	holePos[4][0] = 342; holePos[4][1] = 354;
	holePos[5][0] = 506; holePos[5][1] = 354;

	holePos[6][0] = 206; holePos[6][1] = 446;
	holePos[7][0] = 342; holePos[7][1] = 446;
	holePos[8][0] = 506; holePos[8][1] = 446;
	
	do {
		sl_send(5, 5, (char*)holePos, 36);
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	} while(motion_ack != true);
}

int main()
{
  simcom_init("/dev/ttyUSB0");
  sl_config(0, callback0);
  sl_config(1, callback1);
  sl_config(2, callback2);
  sl_config(5, callback5);
  
  detectHoles();
  
  thread producer(frameProducerTask);
  thread processor(frameProcessorTask);
  thread displayer(displayTask);

  producer.join();
  processor.join();
  displayer.join();
  
  simcom_close();

  return 0;
}
