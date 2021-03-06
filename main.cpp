#include "opencv2/core/core.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "opencv2/contrib/contrib.hpp"
#include <iostream>
#include <iomanip>

#define W 0.1
#define OFFSET 3
#define FF 300
#define CX 160
#define CY 120

// Purpose1: check the arguments of opencv functions.
// Purpose2: see if a 3D point can be successfully calculated by triangulation.

// Result: the arguments can be well understood and triangulation has been successful. 

using namespace cv;
using namespace std;

int main(int argc, char* argv[])
{
   int i,j,k,m;
  
   vector<Point3f> obj1;

   for(j=0; j<=OFFSET*2; j++) {
     for(i=0; i<=OFFSET*2; i++) {
       
       obj1.push_back(Point3f(W * (float)( i-OFFSET  ), W * (float)( j-OFFSET ), 0.0));
       
     }
   }

  float thetaB = 0.1;
  float thetaC = 0.1;

// Change Extrinsics slightly. This means complete settings are impossible.   
   
   Mat Ext_L = (Mat_<float>(3,4) <<
               cos(thetaB),0,-sin(thetaB), -1.5*sin(thetaB),
               0,1,0,0,
               sin(thetaB),0,cos(thetaB),1.5*cos(thetaB));
/*               
   Mat Ext_R = (Mat_<float>(3,4) <<
               cos(thetaC),-sin(thetaC),0, -0.2,
               sin(thetaC),cos(thetaC),0,0,
               0,0,1,1.5);
*/
   Mat Ext_R = (Mat_<float>(3,4) <<
               cos(thetaC),-sin(thetaC),0,-0.2*cos(thetaC),
               sin(thetaC),cos(thetaC),0,-0.2*sin(thetaC),
               0,0,1,1.5);

   Mat Intr = (Mat_<float>(3, 3) <<
              FF, 0, CX,
              0,  FF, CY,
              0,  0,  1);



   vector<vector<Point3f> > object_points;
   vector<vector<Point2f> > imagePoints_L, imagePoints_R;

   float theta = 3.14159265 / 6.0;

   vector<Mat> Rot;

   Mat RotA = (Mat_<float>(3, 3) <<
              1, 0, 0,
              0, 1, 0,
              0, 0, 1);
 
   Rot.push_back(RotA);

   RotA = (Mat_<float>(3, 3) <<
              cos(theta), -sin(theta), 0,
              sin(theta),  cos(theta), 0,
              0,          0,           1);
 
   Rot.push_back(RotA);

   RotA = (Mat_<float>(3, 3) <<
              cos(-theta), -sin(-theta), 0,
              sin(-theta),  cos(-theta), 0,
              0,          0,           1);
 
   Rot.push_back(RotA);
 
   RotA = (Mat_<float>(3, 3) <<
              cos(theta), 0, -sin(theta),
              0, 1, 0,
              sin(theta), 0, cos(theta));

   Rot.push_back(RotA);

   RotA = (Mat_<float>(3, 3) <<
              cos(-theta), 0, -sin(-theta),
              0, 1, 0,
              sin(-theta), 0, cos(-theta));

   Rot.push_back(RotA);

   RotA = (Mat_<float>(3, 3) <<
              1, 0, 0,
              0, cos(theta), -sin(theta),
              0, sin(theta), cos(theta));

   Rot.push_back(RotA);

   RotA = (Mat_<float>(3, 3) <<
              1, 0, 0,
              0, cos(-theta), -sin(-theta),
              0, sin(-theta), cos(-theta));

   Rot.push_back(RotA);
   
   Mat_<float> p1(3,1), p2(3,1), pL(3,1), pR(3,1), p4(4,1);

   for(k=0; k<Rot.size(); k++) {
 
     vector<Point2f> imageL,imageR;

     Mat imgL(Size(320,240),CV_8UC3, Scalar(0,0,0));
     Mat imgR(Size(320,240),CV_8UC3, Scalar(0,0,0));

     int s = (OFFSET*2+1)*(OFFSET*2+1);
    
     for(j=0; j<s ;j++) {
       p1(0,0) = (obj1.at(j)).x;
       p1(1,0) = (obj1.at(j)).y;
       p1(2,0) = (obj1.at(j)).z;
    
       p2 = Rot[k] * p1;

       p4 = (Mat_<float>(4,1) << p2(0,0), p2(1,0), p2(2,0), 1);

       pL = Intr * Ext_L * p4;  

       pR = Intr * Ext_R * p4;

       imageL.push_back(Point2f( pL(0,0) / pL(2,0) , pL(1,0) / pL(2,0) ));
       imageR.push_back(Point2f( pR(0,0) / pR(2,0) , pR(1,0) / pR(2,0) ));
       
       circle(imgL, Point( pL(0,0) / pL(2,0) , pL(1,0) / pL(2,0) ), 2, Scalar(255,255,255));
       circle(imgR, Point( pR(0,0) / pR(2,0) , pR(1,0) / pR(2,0) ), 2, Scalar(255,255,255));
     }
    
     object_points.push_back(obj1);
     imagePoints_L.push_back(imageL);
     imagePoints_R.push_back(imageR);

     namedWindow("R", CV_WINDOW_AUTOSIZE);
     namedWindow("L", CV_WINDOW_AUTOSIZE);
     while(cvWaitKey(1) != 'q') {
   
       imshow("L", imgL);
       imshow("R", imgR);
     }
   }
    
   Mat DL, DR, CML,CMR;
   vector<Mat> rvecsL, tvecsL, rvecsR, tvecsR;
   Mat R, T, E, F; 
  
   calibrateCamera(object_points,imagePoints_L,Size(320,240),CML,DL,rvecsL,tvecsL);
   calibrateCamera(object_points,imagePoints_R,Size(320,240),CMR,DR,rvecsR,tvecsR);

   stereoCalibrate(object_points,imagePoints_L,imagePoints_R,CML,DL,CMR,DR,Size(320,240),R,T,E,F, cvTermCriteria(CV_TERMCRIT_ITER+CV_TERMCRIT_EPS, 100, 1e-5),CV_CALIB_FIX_INTRINSIC);
  
   Mat RL,RR,PL,PR,Q;
   
   stereoRectify(CML, DL, CMR, DR, Size(320,240), R, T, RL, RR, PL, PR, Q);
// x' = R.x + T  
// x : L coordinates
// x': R coordinates

//   cout << fixed << CML << endl;
//   cout << fixed << CMR << endl;
  
// We want to reproduce this point (p4) by triangulatePoints(), and it is appearing to be successful.
 
   p4 = (Mat_<float>(4,1) << -0.203, 0.02, 0.3, 1);
 
   pL = Intr * Ext_L * p4;  

   pR = Intr * Ext_R * p4;


   Mat ptL(Size(1,1), CV_32FC2);

   ptL.at<Vec2f>(0,0)[0] = pL.at<float>(0,0)/pL.at<float>(2,0);
   ptL.at<Vec2f>(0,0)[1] = pL.at<float>(1,0)/pL.at<float>(2,0);


   Mat ptR(Size(1,1), CV_32FC2);

   ptR.at<Vec2f>(0,0)[0] = pR.at<float>(0,0)/pR.at<float>(2,0);
   ptR.at<Vec2f>(0,0)[1] = pR.at<float>(1,0)/pR.at<float>(2,0);

   Mat dst1_L,dst1_R;

   undistortPoints(ptL,dst1_L,CML,DL,RL,PL);
   undistortPoints(ptR,dst1_R,CMR,DR,RR,PR);


   Mat uptL = ( Mat_<float>(2,1) <<
             dst1_L.at<float>(0,0),
             dst1_L.at<float>(0,1));
  
   Mat uptR = ( Mat_<float>(2,1) <<
             dst1_R.at<float>(0,0),
             dst1_R.at<float>(0,1));
  
   Mat pt4;

   triangulatePoints(PL,PR,uptL,uptR,pt4);

   cout << pt4 / pt4.at<float>(3,0)   << endl;

   return 0;
}
