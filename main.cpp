#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

Mat camFrame;
VideoCapture inputVideo(0);

int main()
{
  Mat img;


  while(1) {
    inputVideo >> img;

    imshow("img", img);

    char c = waitKey(1);
    if(c == 27) {
      break;
    }
  }

  return 0;
}
