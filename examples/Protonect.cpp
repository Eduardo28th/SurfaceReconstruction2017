/*
 * This file is part of the OpenKinect Project. http://www.openkinect.org
 *
 * Copyright (c) 2011 individual OpenKinect contributors. See the CONTRIB file
 * for details.
 *
 * This code is licensed to you under the terms of the Apache License, version
 * 2.0, or, at your option, the terms of the GNU General Public License,
 * version 2.0. See the APACHE20 and GPL2 files for the text of the licenses,
 * or the following URLs:
 * http://www.apache.org/licenses/LICENSE-2.0
 * http://www.gnu.org/licenses/gpl-2.0.txt
 *
 * If you redistribute this file in source form, modified or unmodified, you
 * may:
 *   1) Leave this header intact and distribute it under the same terms,
 *      accompanying it with the APACHE20 and GPL20 files, or
 *   2) Delete the Apache 2.0 clause and accompany it with the GPL2 file, or
 *   3) Delete the GPL v2 clause and accompany it with the APACHE20 file
 * In all cases you must keep the copyright notice intact and include a copy
 * of the CONTRIB file.
 *
 * Binary distributions must follow the binary distribution requirements of
 * either License.
 */

/** @file Protonect.cpp Main application file. */

#include <iostream>
#include <cstdlib>
#include <signal.h>

/// [headers]
#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
#include <libfreenect2/registration.h>
#include <libfreenect2/packet_pipeline.h>
#include <libfreenect2/logger.h>
#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/video.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"



#include <vector>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

/// [headers]
#ifdef EXAMPLES_WITH_OPENGL_SUPPORT
#include "viewer.h"
#endif


bool protonect_shutdown = false; ///< Whether the running application should shut down.

void sigint_handler(int s)
{
  protonect_shutdown = true;
}

bool protonect_paused = false;
libfreenect2::Freenect2Device *devtopause;

//Doing non-trivial things in signal handler is bad. If you want to pause,
//do it in another thread.
//Though libusb operations are generally thread safe, I cannot guarantee
//everything above is thread safe when calling start()/stop() while
//waitForNewFrame().
void sigusr1_handler(int s)
{
  if (devtopause == 0)
    return;
/// [pause]
  if (protonect_paused)
    devtopause->start();
  else
    devtopause->stop();
  protonect_paused = !protonect_paused;
/// [pause]
}

//The following demostrates how to create a custom logger
/// [logger]
#include <fstream>
#include <cstdlib>
class MyFileLogger: public libfreenect2::Logger
{
private:
  std::ofstream logfile_;
public:
  MyFileLogger(const char *filename)
  {
    if (filename)
      logfile_.open(filename);
    level_ = Debug;
  }
  bool good()
  {
    return logfile_.is_open() && logfile_.good();
  }
  virtual void log(Level level, const std::string &message)
  {
    logfile_ << "[" << libfreenect2::Logger::level2str(level) << "] " << message << std::endl;
  }
};
/// [logger]



void calcNormal(std::vector<glm::vec3> &points, int w, int h, std::vector<glm::vec3> &normals);

/// [main]
/**
 * Main application entry point.
 *
 * Accepted argumemnts:
 * - cpu Perform depth processing with the CPU.
 * - gl  Perform depth processing with OpenGL.
 * - cl  Perform depth processing with OpenCL.
 * - <number> Serial number of the device to open.
 * - -noviewer Disable viewer window.
 */
int main(int argc, char *argv[])
/// [main]
{
  std::string program_path(argv[0]);
  std::cerr << "Version: " << LIBFREENECT2_VERSION << std::endl;
  std::cerr << "Environment variables: LOGFILE=<protonect.log>" << std::endl;
  std::cerr << "Usage: " << program_path << " [-gpu=<id>] [gl | cl | clkde | cuda | cudakde | cpu] [<device serial>]" << std::endl;
  std::cerr << "        [-noviewer] [-norgb | -nodepth] [-help] [-version]" << std::endl;
  std::cerr << "        [-frames <number of frames to process>]" << std::endl;
  std::cerr << "To pause and unpause: pkill -USR1 Protonect" << std::endl;
  size_t executable_name_idx = program_path.rfind("Protonect");

  std::string binpath = "/";


  if(executable_name_idx != std::string::npos)
  {
    binpath = program_path.substr(0, executable_name_idx);
  }

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__)
  // avoid flooing the very slow Windows console with debug messages
  libfreenect2::setGlobalLogger(libfreenect2::createConsoleLogger(libfreenect2::Logger::Info));
#else
  // create a console logger with debug level (default is console logger with info level)
/// [logging]
  libfreenect2::setGlobalLogger(libfreenect2::createConsoleLogger(libfreenect2::Logger::Debug));
/// [logging]
#endif
/// [file logging]
  MyFileLogger *filelogger = new MyFileLogger(getenv("LOGFILE"));
  if (filelogger->good())
    libfreenect2::setGlobalLogger(filelogger);
  else
    delete filelogger;
/// [file logging]

/// [context]
  libfreenect2::Freenect2 freenect2;
  libfreenect2::Freenect2Device *dev = 0;
  libfreenect2::PacketPipeline *pipeline = 0;
/// [context]
  
  


  std::string serial = "";

  bool viewer_enabled = true;
  bool enable_rgb = true;
  bool enable_depth = true;
  int deviceId = -1;
  size_t framemax = -1;

  for(int argI = 1; argI < argc; ++argI)
  {
    const std::string arg(argv[argI]);

    if(arg == "-help" || arg == "--help" || arg == "-h" || arg == "-v" || arg == "--version" || arg == "-version")
    {
      // Just let the initial lines display at the beginning of main
      return 0;
    }
    else if(arg.find("-gpu=") == 0)
    {
      if (pipeline)
      {
        std::cerr << "-gpu must be specified before pipeline argument" << std::endl;
        return -1;
      }
      deviceId = atoi(argv[argI] + 5);
    }
    else if(arg == "cpu")
    {
      if(!pipeline)
/// [pipeline]
        pipeline = new libfreenect2::CpuPacketPipeline();
/// [pipeline]
    }
    else if(arg == "gl")
    {
#ifdef LIBFREENECT2_WITH_OPENGL_SUPPORT
      if(!pipeline)
        pipeline = new libfreenect2::OpenGLPacketPipeline();
#else
      std::cout << "OpenGL pipeline is not supported!" << std::endl;
#endif
    }
    else if(arg == "cl")
    {
#ifdef LIBFREENECT2_WITH_OPENCL_SUPPORT
      if(!pipeline)
        pipeline = new libfreenect2::OpenCLPacketPipeline(deviceId);
#else
      std::cout << "OpenCL pipeline is not supported!" << std::endl;
#endif
    }
    else if(arg == "clkde")
    {
#ifdef LIBFREENECT2_WITH_OPENCL_SUPPORT
      if(!pipeline)
        pipeline = new libfreenect2::OpenCLKdePacketPipeline(deviceId);
#else
      std::cout << "OpenCL pipeline is not supported!" << std::endl;
#endif
    }
    else if(arg == "cuda")
    {
#ifdef LIBFREENECT2_WITH_CUDA_SUPPORT
      if(!pipeline)
        pipeline = new libfreenect2::CudaPacketPipeline(deviceId);
#else
      std::cout << "CUDA pipeline is not supported!" << std::endl;
#endif
    }
    else if(arg == "cudakde")
    {
#ifdef LIBFREENECT2_WITH_CUDA_SUPPORT
      if(!pipeline)
        pipeline = new libfreenect2::CudaKdePacketPipeline(deviceId);
#else
      std::cout << "CUDA pipeline is not supported!" << std::endl;
#endif
    }
    else if(arg.find_first_not_of("0123456789") == std::string::npos) //check if parameter could be a serial number
    {
      serial = arg;
    }
    else if(arg == "-noviewer" || arg == "--noviewer")
    {
      viewer_enabled = false;
    }
    else if(arg == "-norgb" || arg == "--norgb")
    {
      enable_rgb = false;
    }
    else if(arg == "-nodepth" || arg == "--nodepth")
    {
      enable_depth = false;
    }
    else if(arg == "-frames")
    {
      ++argI;
      framemax = strtol(argv[argI], NULL, 0);
      if (framemax == 0) {
        std::cerr << "invalid frame count '" << argv[argI] << "'" << std::endl;
        return -1;
      }
    }
    else
    {
      std::cout << "Unknown argument: " << arg << std::endl;
    }
  }

  if (!enable_rgb && !enable_depth)
  {
    std::cerr << "Disabling both streams is not allowed!" << std::endl;
    return -1;
  }

/// [discovery]
  if(freenect2.enumerateDevices() == 0)
  {
    std::cout << "no device connected!" << std::endl;
    return -1;
  }

  if (serial == "")
  {
    serial = freenect2.getDefaultDeviceSerialNumber();
  }
/// [discovery]
  pipeline = new libfreenect2::OpenCLPacketPipeline(deviceId);


  if(pipeline)
  {
/// [open]
    dev = freenect2.openDevice(serial, pipeline);
/// [open]
  }
  else
  {
    dev = freenect2.openDevice(serial);
  }

  libfreenect2::Freenect2Device::Config config1;
  config1.MaxDepth = 100.0f;
  config1.MinDepth = 0.2f;
  dev->setConfiguration(config1);
  




  if(dev == 0)
  {
    std::cout << "failure opening device!" << std::endl;
    return -1;
  }

  devtopause = dev;

  signal(SIGINT,sigint_handler);
#ifdef SIGUSR1
  signal(SIGUSR1, sigusr1_handler);
#endif
  protonect_shutdown = false;

/// [listeners]
  int types = 0;
  if (enable_rgb)
    types |= libfreenect2::Frame::Color;
  if (enable_depth)
    types |= libfreenect2::Frame::Ir | libfreenect2::Frame::Depth;
  libfreenect2::SyncMultiFrameListener listener(types);
  libfreenect2::FrameMap frames;

  dev->setColorFrameListener(&listener);
  dev->setIrAndDepthFrameListener(&listener);
/// [listeners]

/// [start]
  if (enable_rgb && enable_depth)
  {
    if (!dev->start())
      return -1;
  }
  else
  {
    if (!dev->startStreams(enable_rgb, enable_depth))
      return -1;
  }

  std::cout << "device serial: " << dev->getSerialNumber() << std::endl;
  std::cout << "device firmware: " << dev->getFirmwareVersion() << std::endl;
/// [start]

/// [registration setup]
  libfreenect2::Registration* registration = new libfreenect2::Registration(dev->getIrCameraParams(), dev->getColorCameraParams());
  libfreenect2::Frame undistorted(512, 424, 4), registered(512, 424, 4), bigdepth(1920, 1082, 4);
  
/// [registration setup]

  size_t framecount = 0;
#ifdef EXAMPLES_WITH_OPENGL_SUPPORT
  Viewer viewer;
  if (viewer_enabled)
    viewer.initialize();
#else
  viewer_enabled = false;
#endif

/// [loop start]
  int scanner = 0;
  bool once = false;
  float minvalugray = 100.0f;
  std::vector<glm::vec3> background;
  std::vector<glm::vec3> normalBg;
  std::vector<glm::vec3> backgroundSum;
  glm::vec3 bminXYZ = glm::vec3(100.0, 100.0, 100.0);
  glm::vec3 bmaxXYZ = glm::vec3(0.0, 0.0, 0.0);
  bool backReady = false;
  int backFrames = 0;
  //std::vector<glm::vec3> point981;
  //std::vector<glm::vec3> point982;
  //std::vector<float> dis;
  /*
  int gridW1 = 64;
  int indices1[6552];
  bool once = true;*/
  

  while(!protonect_shutdown && (framemax == (size_t)-1 || framecount < framemax))
  {
    if (!listener.waitForNewFrame(frames, 10*1000)) // 10 sconds
    {
      std::cout << "timeout!" << std::endl;
      return -1;
    }
    libfreenect2::Frame *rgb = frames[libfreenect2::Frame::Color];
   // libfreenect2::Frame *ir = frames[libfreenect2::Frame::Ir];
    libfreenect2::Frame *depth = frames[libfreenect2::Frame::Depth];

	//registration->apply(rgb, depth, &undistorted, &registered);
//////////////////////////POINT CLOUD
	std::vector<glm::vec3> clpoints;
	std::vector<glm::vec3> diagonals;
	std::vector<std::vector<glm::vec3>> normals;
	std::vector<glm::vec3> minXYZ;
	std::vector<glm::vec3> maxXYZ;
	glm::vec3 nullval;
	int pixelObj = 0;
	bool lookingNull = false;
	float minVal = 100.0f;
	float maxVal = -100.0f;
	int gridSize = 8;
	int gridH = depth->height / gridSize;
	int gridW = depth->width / gridSize;
	int nanValue = 0;
	int num = 0;
	float midz = 0.0f;
	float colorz = 0.0f;
	cv::Mat horizontal(gridH, gridW, CV_8UC1);
	cv::Mat vertical(gridH, gridW, CV_8UC1);
	std::vector<std::vector<glm::vec3>> regions;
	//cv::Mat segmentation(gridH, gridW, CV_8UC1);      
	//if (!once)
	{
		for (int r = 0; r < depth->height; r += gridSize)
			//for (int r = scanner; r < scanner +20; r++)
		{
			//for (int c = scanner; c < scanner + 20; c++)
			for (int c = 0; c < depth->width; c += gridSize)
			{
				glm::vec3 points;
				registration->getPointXYZ(depth, r, c, points.x, points.y, points.z);
				//Maximum and mimimum points
				//if (points.z < minVal)
					//minVal = points.z;
				//if (points.z > maxVal)
					//maxVal = points.z;
				if (!lookingNull)
				{
					if (isnan(points.x))
					{
						nullval = points;
						lookingNull = true;
					}
				}
				points.y = -points.y;
				points.x = -points.x;
				/*
				if (num == 981)
				{
					point981.push_back(points);
				}
				if (num == 982)
				{
					point982.push_back(points);
					float tempDis;
					int sec = point981.size();
					tempDis = (point981[sec - 1].x - point982[sec - 1].x)*(point981[sec - 1].x - point982[sec - 1].x)+ (point981[sec - 1].y - point982[sec - 1].y)*(point981[sec - 1].y - point982[sec - 1].y)+ (point981[sec - 1].z - point982[sec - 1].z)*(point981[sec - 1].z - point982[sec - 1].z);
					dis.push_back(tempDis);
				}
				
				*/
				clpoints.push_back(points);

				if (r == 25*8 && c == 32*8)
					midz = points.z;

				////////////////
				////SEGMENTATION USING MAT
				/////////////////
				
				/*

				int row = (num - 1) / gridW;
				int col = (num - 1) % gridW;
				//horizontal
				if (col < 63 && num > 0)
				{
						if (abs(clpoints[num - 1].z - points.z) < 0.05)
						{
							if (horizontal.at<uchar>(row, col) != 0)
							{
								horizontal.at<uchar>(row, col) = 255;
							}

							if (col == gridW - 2)
							{
								horizontal.at<uchar>(row, col + 1) = 255;
							}

						}
						else
						{
							horizontal.at<uchar>(row, col) = 0;
							horizontal.at<uchar>(row, col + 1) = 0;
						}
							int hola = 1;
					
				}

				//vertical
				
				if (num > gridW - 1)
				{
					row = (num - gridW) / gridW;
					col = (num - gridW) % gridW;
					if (abs(clpoints[num - gridW].z - points.z) < 0.05)
					{
						if (vertical.at<uchar>(row, col) != 0)
						{
							vertical.at<uchar>(row, col) = 255;
						}

						if (row == gridH - 2)
						{
							vertical.at<uchar>(row+1, col) = 255;
						}
					}
					else
					{
						vertical.at<uchar>(row, col) = 0;
						vertical.at<uchar>(row+1, col) = 0;
					}
				}

				*/

				////////////////
				////SEGMENTATION USING MAT
				/////////////////
				num++;




				int offset1 = c / gridSize + (r / gridSize)*gridW;
				if (offset1 > gridW && offset1%gridW != 0)
				{
					int offset2 = offset1 - gridW - 1;
					points = clpoints[offset1] - clpoints[offset2];
					diagonals.push_back(points);
				}

			}
		}
		once = true;
	}


	//////////////////////
	///////BACKGROUND
	//////////////////////

	cv::Mat segmentation = cv::Mat::zeros(gridH, gridW, CV_8UC1);
	if (!backReady)
	{
		backFrames++;
		for (int n = 0; n < clpoints.size(); n++)
		{
			if (backFrames == 1)
			{
				backgroundSum = clpoints;
				background = backgroundSum;
				break;
			}
			else
			{
				backgroundSum[n] += clpoints[n];
				background[n] = backgroundSum[n] / (float)backFrames;
			}
				
		}

		if (backFrames == 4)
		{
			backReady = true;
			calcNormal(background, gridW, gridH, normalBg);
			for (int n = 0; n < background.size(); n++)
			{
				if (bminXYZ.x > background[n].x)
					bminXYZ.x = background[n].x;
				if (bminXYZ.y > background[n].y)
					bminXYZ.y = background[n].y;
				if (bminXYZ.z > background[n].z)
					bminXYZ.z = background[n].z;

				if (bmaxXYZ.x < background[n].x)
					bmaxXYZ.x = background[n].x;
				if (bmaxXYZ.y < background[n].y)
					bmaxXYZ.y = background[n].y;
				if (bmaxXYZ.z < background[n].z)
					bmaxXYZ.z = background[n].z;

			}
			normals.push_back(normalBg);
			minXYZ.push_back(bminXYZ);
			maxXYZ.push_back(bmaxXYZ);
			

		}
			

		regions.push_back(background);
		

	}
	else
	{
		regions.push_back(background);
		normals.push_back(normalBg);
		minXYZ.push_back(bminXYZ);
		maxXYZ.push_back(bmaxXYZ);
		std::vector<glm::vec3> temp_region;
		glm::vec3 rminXYZ = glm::vec3(100.0, 100.0, 100.0);
		glm::vec3 rmaxXYZ = glm::vec3(0.0, 0.0, 0.0);
		glm::vec2 cornerA = glm::vec2(100.0, 100.0);
		glm::vec2 cornerB = glm::vec2(0.0, 0.0);
		glm::vec2 dimWH;
		for (int r = 0; r < gridH; r++)
		{
			for (int c = 0; c < gridW; c++)
			{
				int ix = r*gridW + c;
				if (abs(clpoints[ix].z - background[ix].z) > 0.1)
				{
					////It means there is an object in the scene
					segmentation.at<uchar>(r, c) = 255;
					pixelObj++;

					//Store corners
					cornerB.y = r;

					if (cornerA.x > c)
						cornerA.x = c;
					if (cornerA.y > r)
						cornerA.y = r;
					if (cornerB.x < c)
						cornerB.x = c;

				}

			}
		}

		

		

		if (pixelObj > 300) //~10% of the scene
		{
			dimWH.x = cornerB.x - cornerA.x + 1;
			dimWH.y = cornerB.y - cornerA.y + 1;

			for (int r = cornerA.y; r < cornerB.y + 1; r++)
			{
				for (int c = cornerA.x; c < cornerB.x + 1; c++)
				{
						int ix = r*gridW + c;
						temp_region.push_back(clpoints[ix]);

						if (rminXYZ.x > clpoints[ix].x)
							rminXYZ.x = clpoints[ix].x;
						if (rminXYZ.y > clpoints[ix].y)
							rminXYZ.y = clpoints[ix].y;
						if (rmaxXYZ.x < clpoints[ix].x)
							rmaxXYZ.x = clpoints[ix].x;
						if (rmaxXYZ.y < clpoints[ix].y)
							rmaxXYZ.y = clpoints[ix].y;


						if (rminXYZ.z > clpoints[ix].z)
							rminXYZ.z = clpoints[ix].z;
						if (rmaxXYZ.z < clpoints[ix].z)
							rmaxXYZ.z = clpoints[ix].z;
				}
			}
			std::vector<glm::vec3> regNormal;
			regions.push_back(temp_region);
			calcNormal(temp_region, dimWH.x, dimWH.y, regNormal);
			normals.push_back(regNormal);
			minXYZ.push_back(rminXYZ);
			maxXYZ.push_back(rmaxXYZ);
		}

		

	}

	//////////////////////
	///////BACKGROUND FINISH
	//////////////////////




	
	//cv::bitwise_and(horizontal, vertical, segmentation);
	//cv::Mat surfaces3;
	//cv::resize(segmentation, surfaces3, cv::Size((gridW) * 4, (gridH) * 4));
	//cv::imshow("Segmentation", surfaces3);

	//////////Region retrieving
	

	////////////////
	//// MAKING REGIONS FROM SEGMENTATION
	////////////////

	/////////////////////////////////////
	///////////////////TESTING MAT
	//////////////////////////////////
	/*
	cv::Mat segmentation2 = cv::Mat::zeros(5, 5, CV_8UC1);

	segmentation2.at<uchar>(0, 1) = 255;
	segmentation2.at<uchar>(0, 2) = 255;
	segmentation2.at<uchar>(0, 4) = 255;

	segmentation2.at<uchar>(1, 1) = 255;
	segmentation2.at<uchar>(1, 2) = 255;
	segmentation2.at<uchar>(1, 3) = 255;
	segmentation2.at<uchar>(1, 4) = 255;

	segmentation2.at<uchar>(2, 0) = 255;
	segmentation2.at<uchar>(2, 4) = 255;

	segmentation2.at<uchar>(3, 1) = 255;
	segmentation2.at<uchar>(3, 2) = 255;
	segmentation2.at<uchar>(3, 3) = 255;

	segmentation2.at<uchar>(4, 0) = 255;
	segmentation2.at<uchar>(4, 1) = 255;
	segmentation2.at<uchar>(4, 3) = 255;
	segmentation2.at<uchar>(4, 4) = 255;

	gridW = 5;
	gridH = 5;
	*/
	/////////////////////////////////////
	///////////////////TESTING MAT
	//////////////////////////////////


	///////Region map

	
	/*

	
	int regionNum = 0;
	
	for (int r = 0; r < depth->height; r++)
	{
		bool regionFound = false;
		bool partofaRegion = false;
		regionNum++;
		//int que = segmentation.at<uchar>(3, 2);
		//int que2 = segmentation.at<uchar>(3, 18);
		for (int c = 0; c < depth->width; c++)
		{
			if (r == 0)
			{
				if (!regionFound)
				{
					if (segmentation.at<float>(r, c) > 500.0)
					{
						segmentation.at<float>(r, c) = regionNum;
						regionFound = true;
					}
				}
				else
				{
					if (segmentation.at<float>(r, c) > 500.0 && segmentation.at<float>(r, c - 1) == regionNum)
					{
						segmentation.at<float>(r, c) = regionNum;
					}
					else
					{
						regionNum++;
						regionFound = false;
					}
			}
			
			}
			else
			{
				if (!regionFound)
				{
					if (segmentation.at<float>(r, c) > 500.0)
					{
						if (segmentation.at<float>(r - 1, c) != 0)
						{
							segmentation.at<float>(r, c) = segmentation.at<float>(r - 1, c);
							partofaRegion = true;
						}
						else
						{
							segmentation.at<float>(r, c) = regionNum;
						}
						regionFound = true;
					}
				}
				else
				{
					if (segmentation.at<float>(r, c) > 500.0)
					{
						if (partofaRegion)
						{
							segmentation.at<float>(r, c) = segmentation.at<float>(r, c - 1);
							if (segmentation.at<float>(r - 1, c) != 0)
							{

								if (segmentation.at<float>(r, c - 1) != segmentation.at<float>(r - 1, c))
								{
									int err = segmentation.at<float>(r - 1, c);
									int fix = segmentation.at<float>(r, c - 1);
									for (int r2 = 0; r2 < r; r2++)
									{
										for (int c2 = 0; c2 < depth->width; c2++)
										{
											if (segmentation.at<float>(r2, c2) == err)
												segmentation.at<float>(r2, c2) = fix;
										}
									}
								}
							}
						}
						else
						{
							if (segmentation.at<float>(r - 1, c) != 0.0)
							{
								int err = regionNum;
								int fix = segmentation.at<float>(r - 1, c);
								segmentation.at<float>(r, c) = segmentation.at<float>(r - 1, c);
								for (int c2 = 0; c2 < c; c2++)
								{
									if (segmentation.at<float>(r, c2) == err)
										segmentation.at<float>(r, c2) = fix;
								}
								partofaRegion = true;
								regionNum--;
							}
							else
							{
								segmentation.at<float>(r, c) = regionNum;
							}
						}


					}
					else
					{

						regionNum++;
						regionFound = false;
						partofaRegion = false;
					}
				}
			}
		}
	}

	/*
	std::vector <float> regions2;
	for (int r = 0; r < depth->height; r++)
	{
		for (int c = 0; c < depth->width; c++)
		{

			regions2.push_back(segmentation.at<float>(r, c));
		}
	}*/

	


	///////////////SETTING REGIONS
	//////////////GETTING HOW MANY REGIONS ARE AND THE CORNES OF EACH REGION.

	/*

	std::vector <float> regions2;
	std::vector <float> regions3;
	std::vector <glm::vec3> maxXYZ;
	std::vector <glm::vec3> minXYZ;
	std::vector <glm::vec2> dimWH;

	for (int r = 0; r < depth->height; r++)
	{
		for (int c = 0; c < depth->width; c++)
		{

			regions2.push_back(segmentation.at<float>(r, c));
			if (segmentation.at<float>(r, c) != 0.0)
			{
				if (regions3.empty())
				{
					regions3.push_back(segmentation.at<float>(r, c));
					maxXYZ.push_back(glm::vec3(c, r, -100.0f));
					minXYZ.push_back(glm::vec3(c, r, 100.0f));
					dimWH.push_back(glm::vec2(0.0, 0.0));
				}
				else
				{
					std::vector<float>::iterator it = std::find(regions3.begin(), regions3.end(), segmentation.at<float>(r, c));
					if (it != regions3.end())
					{
						///////////FIND MIN X, AND MAX X AND Y
						if (c < minXYZ[it - regions3.begin()].x)
						{
							minXYZ[it - regions3.begin()].x = c;
						}

						if (c > maxXYZ[it - regions3.begin()].x)
						{
							maxXYZ[it - regions3.begin()].x = c;
						}

						if (r > maxXYZ[it - regions3.begin()].y)
						{
							maxXYZ[it - regions3.begin()].y = r;
						}

					}
					else
					{
						regions3.push_back(segmentation.at<float>(r, c));
						maxXYZ.push_back(glm::vec3(c, r, -100.0f));
						minXYZ.push_back(glm::vec3(c, r, 100.0f));
						dimWH.push_back(glm::vec2(0.0, 0.0));
					}
				}
			}
		}
	}

	int nothing = 0;


	*/

	////////////////
	//////////////TEST 235 BEGIN
	/////////////////////////////

	/*



	*/


	////////////////
	//////////////TEST 235 FINISH
	/////////////////////////////






	/*
	std::vector<std::vector<glm::vec3>> regions;
	std::vector<cv::Mat> regmask;
	std::vector<int> noRegions;
	///////////CREATING ALL REGIONS IN VECTORES STORING 3D POINTS.
	for (int n = 0; n < regions3.size() ; n++)
	{
		if ((maxXYZ[n].x - minXYZ[n].x > 2) && (maxXYZ[n].y - minXYZ[n].y > 2))
		{
			std::vector<glm::vec3> temp_region;
			dimWH[n].x = maxXYZ[n].x - minXYZ[n].x + 1;
			dimWH[n].y = maxXYZ[n].y - minXYZ[n].y + 1;
			//cv::Mat temp_mask = cv::Mat::zeros(dimWH[n].y, dimWH[n].x, CV_8UC1);
			cv::Mat temp_mask = cv::Mat::zeros(gridH, gridW, CV_8UC1);
			for (int r = minXYZ[n].y; r < maxXYZ[n].y+1; r++)
			{
				for (int c = minXYZ[n].x; c < maxXYZ[n].x + 1; c++)
				{
					if (segmentation.at<uchar>(r, c) == regions3[n])
					{
						int ix = r*gridW + c;
						temp_region.push_back(clpoints[ix]);
						if (minXYZ[n].z > clpoints[ix].z)
							minXYZ[n].z = clpoints[ix].z;
						if (maxXYZ[n].z < clpoints[ix].z)
							maxXYZ[n].z = clpoints[ix].z;

						//MASK
						//temp_mask.at<uchar>(r- minXYZ[n].y, c- minXYZ[n].x) = 255;
						temp_mask.at<uchar>(r, c) = 255;

					}
					else
					{
						temp_region.push_back(nullval);
					}
				}
			}
			regions.push_back(temp_region);
			cv::dilate(temp_mask, temp_mask, cv::Mat());
			regmask.push_back(temp_mask);
		}
		else
		{
			noRegions.push_back(n);
		}
	}
	
	if (!noRegions.empty())
	{
		int nfix = 0;
		for (int n = 0; n < noRegions.size(); n++)
		{
			int no = noRegions[n]-nfix;
			minXYZ.erase(minXYZ.begin() + no);
			maxXYZ.erase(maxXYZ.begin() + no);
			dimWH.erase(dimWH.begin() + no);
			nfix++;
		}
	}
	
	*/
	
	///////////
	/////TESTING COLOR IMAGES IN CV::MAT
	////////////

	/*
	cv::Mat clrImage = cv::Mat(rgb->height, rgb->width, CV_8UC4, rgb->data);
	cv::Mat isolate, isoImage;
	cv::resize(regmask[0], isolate, cv::Size(rgb->width, rgb->height));
	isoImage = cv::Mat::zeros(cv::Size(rgb->width, rgb->height), CV_8UC4);
	

	for (int r = 0; r < rgb->height; r++)
	{
		for (int c = 0; c < rgb->width; c++)
		{
			if (isolate.at<uchar>(r, c) == 255)
			{
				isoImage.at<cv::Vec4b>(r, c) = clrImage.at<cv::Vec4b>(r, c);
			}
		}
	}

	//cv::imshow("Segmentation", surfaces3);
	cv::imshow("Color", isoImage);
	*/
	///////////
	/////TESTING COLOR IMAGES IN CV::MAT FNISH
	////////////


	/////////////
	////INDICES//////////////
	///////////
	
	/*
		std::vector<std::vector<int>> r_indices;

		for (int j = 0; j < regions.size(); j++)
		{
			int gridW1 = dimWH[j].x;
			int limit = (dimWH[j].x - 1)*(dimWH[j].y - 1) * 6;
			int triangleVertice = 0;
			int squareGrid = 0;
			std::vector<int> temp_ind;
			for (int n = 0; n < limit; n++)
			{
				if (squareGrid != 0 && (squareGrid + 1) % gridW1 == 0)
					squareGrid++;

				switch (triangleVertice)
				{
				case 0: temp_ind.push_back(squareGrid);
					triangleVertice++;
					break;
				case 1: temp_ind.push_back( temp_ind[n - 1] + gridW1);
					triangleVertice++;
					break;
				case 2: temp_ind.push_back(temp_ind[n - 2] + gridW1 + 1);
					triangleVertice++;
					break;
				case 3: temp_ind.push_back(temp_ind[n - 3]);
					triangleVertice++;
					break;
				case 4: temp_ind.push_back(temp_ind[n - 2]);
					triangleVertice++;
					break;
				case 5: temp_ind.push_back(temp_ind[n - 5] + 1);
					squareGrid++;
					triangleVertice = 0;
					break;
				}
			}
			r_indices.push_back(temp_ind);
		}

		int nothing = 0;


		*/
	

	/////////////
	////INDICES FINISH//////////////
	///////////


	//////////////////////
	////////COLORED REGIONS
	//////////////////////

	/*
	

	cv::Mat Colorsegmentation(depth->height, depth->width, CV_8UC3);
	int n_regions = regions3.size();
	std::vector<cv::Vec3b> colors;
	for (int n = 0; n < regions3.size(); n++)
	{
		cv::Vec3b color2(rand() & 0xFF, rand() & 0xFF, rand() & 0xFF);
		colors.push_back(color2);
	}

	for (int r = 0; r < depth->height; r++)
	{
		for (int c = 0; c < depth->width; c++)
		{
			if (segmentation.at<float>(r, c) != 0.0)
			{
				cv::Vec3b tempColor = colors[std::find(regions3.begin(), regions3.end(), segmentation.at<float>(r, c)) - regions3.begin()];
				Colorsegmentation.at<cv::Vec3b>(r, c) = tempColor;
			}
			else
			{
				Colorsegmentation.at<cv::Vec3b>(r, c)[0] = 0;
				Colorsegmentation.at<cv::Vec3b>(r, c)[1] = 0;
				Colorsegmentation.at<cv::Vec3b>(r, c)[2] = 0;
			}
			
		}
	}

	//cv::Mat surfaces4;
	//cv::resize(Colorsegmentation, surfaces4, cv::Size((gridW) * 4, (gridH) * 4));
	cv::imshow("Segmentation2", Colorsegmentation);
	
	*/

	//////////////////////
	////////COLORED REGIONS FINISH
	//////////////////////





	////////////////
	//// MAKING REGIONS FROM SEGMENTATION FINISH
	////////////////


	///////////////////////////
	//NORMAL CALCULATION
	//////////////////////////////

		/*

	for (int n = 0; n < clpoints.size(); n++)
	{
		glm::vec3 up;
		glm::vec3 left;
		glm::vec3 right;
		glm::vec3 down;
		glm::vec3 diagonal;
		glm::vec3 normal;
		int nrmPoints = 0;

		if (n < gridW)
		{
			//Primera fila
			down = clpoints[n + gridW]-clpoints[n];
			nrmPoints++;
			if (n == 0)
			{
				//Primer punto, primera fila         x o o o o o
				diagonal = clpoints[n + gridW + 1] - clpoints[n];
				nrmPoints++;
				right = clpoints[n + 1] - clpoints[n];
				nrmPoints++;
				normal = glm::cross(diagonal, right) + glm::cross(down, diagonal);
			}
			else if (n == gridW - 1)
			{
				//Ultimo punto, primera fila        o o o o o x
				diagonal = clpoints[n + gridW - 1] - clpoints[n];
				nrmPoints++;
				left = clpoints[n - 1] - clpoints[n];
				nrmPoints++;
				normal = glm::cross(left, diagonal) + glm::cross(diagonal, down);
			}
			else
			{
				//Resto de la primera fila    o x x x x o
				right = clpoints[n + 1] - clpoints[n];
				nrmPoints++;
				left = clpoints[n - 1] - clpoints[n];
				nrmPoints++;
				normal = glm::cross(left, down) + glm::cross(down, right);
			}
		}
		else if (n > clpoints.size() - gridW - 1)
		{
			//Ultima fila
			up = clpoints[n - gridW] - clpoints[n];
			nrmPoints++;
			if (n%gridW == 0)
			{
				//primer punto, ultima fila
				diagonal = clpoints[n - gridW + 1] - clpoints[n];
				nrmPoints++;
				right = clpoints[n + 1] - clpoints[n];
				nrmPoints++;
				normal = glm::cross(right, diagonal) + glm::cross(diagonal, up);
			}
			else
				if ((n+1) %gridW == 0)
				{
					//ultimo punto, ultima fila
					diagonal = clpoints[n - gridW - 1] - clpoints[n];
					nrmPoints++;
					left = clpoints[n - 1] - clpoints[n];
					nrmPoints++;
					normal = glm::cross(up, diagonal) + glm::cross(diagonal, left);
				}
				else
				{
					//Resto de la ultima fila
					right = clpoints[n + 1] - clpoints[n];
					nrmPoints++;
					left = clpoints[n - 1] - clpoints[n];
					nrmPoints++;
					normal = glm::cross(right, up) + glm::cross(up, left);
				}	
		}
		else
		{
			up = clpoints[n - gridW] - clpoints[n];
			nrmPoints++;
			down = clpoints[n + gridW] - clpoints[n];
			nrmPoints++;
			if (n % gridW == 0)
			{
				//Primera columna. Excepto primera y ultima fila
				right = clpoints[n + 1] - clpoints[n];
				nrmPoints++;
				normal = glm::cross(down, right) + glm::cross(right, up);
			}
			else if (n % (gridW - 1) == 0)
			{
				//ultima columna. Excepto primera y ultima fila
				left = clpoints[n - 1] - clpoints[n];
				nrmPoints++;
				normal = glm::cross(up, left) + glm::cross(left, down);

			}
			else
			{
				//Resto de puntos
				right = clpoints[n + 1] - clpoints[n];
				nrmPoints++;
				left = clpoints[n - 1] - clpoints[n];
				nrmPoints++;
				normal = glm::cross(up, left) + glm::cross(left, down)+ glm::cross(down, right) + glm::cross(right, up);
			}
		}
			
		normal = glm::normalize(normal);
		normals.push_back(normal);

	}

	*/
	///////////////////////////
	//NORMAL CALCULATION FINISH
	//////////////////////////////


	//////////////////////////MESH RECONSTRUCTION
	//WITH TRIANGLE MESH USING INDICES TO DRAW THEM.
	

	/*
	if (once)
	{
		int triangleVertice = 0;
		int squareGrid = 0;
		for (int n = 0; n < (sizeof(indices1)/sizeof(int)); n++)
		{
			if (squareGrid != 0 && squareGrid % (gridW1 - 1) == 0)
				squareGrid++;

			switch (triangleVertice)
			{
			case 0: indices1[n] = squareGrid;
				triangleVertice++;
				break;
			case 1: indices1[n] = indices1[n - 1] + gridW1;
				triangleVertice++;
				break;
			case 2: indices1[n] = indices1[n - 2] + gridW1 + 1;
				triangleVertice++;
				break;
			case 3: indices1[n] = indices1[n - 3];
				triangleVertice++;
				break;
			case 4: indices1[n] = indices1[n - 2];
				triangleVertice++;
				break;
			case 5: indices1[n] = indices1[n - 5] + 1;
				squareGrid++;
				triangleVertice = 0;
				break;
			}
		}
		once = false;
	}*/
	
	
	


	/* MESH CONSTRUCTION WITH OPENCV             COMMENT 1A
	
	float error;
	int area = (gridH - 1)*(gridW - 1);
	float tolerance = 0.0005;
	/////check diagonals
	glm::vec3 diagonal1;


	
	int conection[13335];
	for (int n = 0; n < area-gridW-1; n++)
	{
		if(n%(gridW-1)!=0)
		{
			int offset2 = n + gridW;
			diagonal1 = diagonals[offset2] - diagonals[n];
			error = diagonal1.x*diagonal1.x + diagonal1.y*diagonal1.y + diagonal1.z*diagonal1.z;
			if (error < tolerance)
				conection[n] = 1;
			else
				conection[n] = 0;

		}
	}

	///////////////////TRYING WITH OPENCV
	cv::Mat diagonal2(gridH - 1, gridW - 1, CV_8UC1);
	diagonal2 = cv::Mat::zeros(diagonal2.size(), CV_8UC1);
	for (int n = 0; n < area - gridW - 1; n++)
	{
		int col = n % (gridW - 1);
		if (col != 0)
		{
			int offset2 = n + gridW;
			int row = n / (gridW - 1);
			
			diagonal1 = diagonals[offset2] - diagonals[n];
			error = diagonal1.x*diagonal1.x + diagonal1.y*diagonal1.y + diagonal1.z*diagonal1.z;
			if (error < tolerance)
				diagonal2.at<uchar>(row, col) = 255;
			else
				diagonal2.at<uchar>(row, col) = 0;

		}
	}


	cv::Mat diagonal3;
	cv::resize(diagonal2, diagonal3, cv::Size((gridW-1)*4, (gridH-1)*4));
	cv::imshow("connections", diagonal3);
	//std::vector<std::vector<int>> surfaces;
	//std::vector<int> countours;

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;
	cv::Mat surfaces = diagonal2.clone();
	cv::findContours(surfaces, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
	cv::Mat surfaces2;
	cv::resize(surfaces, surfaces2, cv::Size((gridW - 1) * 4, (gridH - 1) * 4));
	cv::imshow("contours", surfaces2);

	cv::Mat DisplayContours = cv::Mat::zeros(diagonal2.size(), CV_8UC3);
	int counter = 0;
	for (int n = 0; (n < (int)contours.size()); n++)
	{
		if (cv::contourArea(contours[n]) > 50)
		{
			cv::Scalar colour(counter*5, counter*3+13, 255-(counter*2));
			cv::drawContours(DisplayContours, contours, n, colour, CV_FILLED, 8, hierarchy);
			counter++;
		}
		
	}
	cv::Mat surfaces3;
	cv::resize(DisplayContours, surfaces3, cv::Size((gridW - 1) * 4, (gridH - 1) * 4));
	cv::imshow("surfaces", surfaces3);
	*/       //COMMENT 1A
	

	/* COMMENT 1B
	if (scanner > depth->height - 20)
		scanner = 0;
	else
		scanner++;*/ //COMMENT 1B

	
	
	viewer.addPointCloud(regions);

	if (backReady)
	{
		viewer.addNormals(normals);
		viewer.addMin(minXYZ);
		viewer.addMax(maxXYZ);
		viewer.ready = true;
	}
	//viewer.addIndices(r_indices);
	//viewer.addMin(minXYZ);
	//viewer.addMask(regmask);



/// [loop start]

	
    if (enable_rgb && enable_depth)
    {
/// [registration]
     // registration->apply(rgb, depth, &undistorted, &registered);
		//registration->undistortDepth(depth, &undistorted);
/// [registration]
    }

    framecount++;
    if (!viewer_enabled)
    {
      if (framecount % 100 == 0)
        std::cout << "The viewer is turned off. Received " << framecount << " frames. Ctrl-C to stop." << std::endl;
      listener.release(frames);
      continue;
    }

#ifdef EXAMPLES_WITH_OPENGL_SUPPORT
	
    if (enable_rgb)
    {
      viewer.addFrame("RGB", rgb);
    }
	
    if (enable_depth)
    {
      //viewer.addFrame("ir", ir);
      viewer.addFrame("depth", depth);
    }
	/*
    if (enable_rgb && enable_depth)
    {
      viewer.addFrame("registered", &undistorted);
    }
	*/
    protonect_shutdown = protonect_shutdown || viewer.render();
#endif

/// [loop end]
    listener.release(frames);
    /** libfreenect2::this_thread::sleep_for(libfreenect2::chrono::milliseconds(100)); */
  }
/// [loop end]

  // TODO: restarting ir stream doesn't work!
  // TODO: bad things will happen, if frame listeners are freed before dev->stop() :(
/// [stop]
  dev->stop();
  dev->close();
/// [stop]

  delete registration;

  return 0;
}

void calcNormal(std::vector<glm::vec3> &points, int w, int h, std::vector<glm::vec3> &nrmals)
{
	///////////////////////////
	//NORMAL CALCULATION
	//////////////////////////////

		

	for (int n = 0; n < points.size(); n++)
	{
		glm::vec3 up;
		glm::vec3 left;
		glm::vec3 right;
		glm::vec3 down;
		glm::vec3 diagonal;
		glm::vec3 normal;
		int nrmPoints = 0;

		if (n < w)
		{
			//Primera fila
			down = points[n + w]-points[n];
			nrmPoints++;
			if (n == 0)
			{
				//Primer punto, primera fila         x o o o o o
				diagonal = points[n + w + 1] - points[n];
				nrmPoints++;
				right = points[n + 1] - points[n];
				nrmPoints++;
				normal = glm::cross(diagonal, right) + glm::cross(down, diagonal);
			}
			else if (n == w - 1)
			{
				//Ultimo punto, primera fila        o o o o o x
				diagonal = points[n + w - 1] - points[n];
				nrmPoints++;
				left = points[n - 1] - points[n];
				nrmPoints++;
				normal = glm::cross(left, diagonal) + glm::cross(diagonal, down);
			}
			else
			{
				//Resto de la primera fila    o x x x x o
				right = points[n + 1] - points[n];
				nrmPoints++;
				left = points[n - 1] - points[n];
				nrmPoints++;
				normal = glm::cross(left, down) + glm::cross(down, right);
			}
		}
		else if (n > points.size() - w - 1)
		{
			//Ultima fila
			up = points[n - w] - points[n];
			nrmPoints++;
			if (n%w == 0)
			{
				//primer punto, ultima fila
				diagonal = points[n - w + 1] - points[n];
				nrmPoints++;
				right = points[n + 1] - points[n];
				nrmPoints++;
				normal = glm::cross(right, diagonal) + glm::cross(diagonal, up);
			}
			else
				if ((n+1) %w == 0)
				{
					//ultimo punto, ultima fila
					diagonal = points[n - w - 1] - points[n];
					nrmPoints++;
					left = points[n - 1] - points[n];
					nrmPoints++;
					normal = glm::cross(up, diagonal) + glm::cross(diagonal, left);
				}
				else
				{
					//Resto de la ultima fila
					right = points[n + 1] - points[n];
					nrmPoints++;
					left = points[n - 1] - points[n];
					nrmPoints++;
					normal = glm::cross(right, up) + glm::cross(up, left);
				}	
		}
		else
		{
			up = points[n - w] - points[n];
			nrmPoints++;
			down = points[n + w] - points[n];
			nrmPoints++;
			if (n % w == 0)
			{
				//Primera columna. Excepto primera y ultima fila
				right = points[n + 1] - points[n];
				nrmPoints++;
				normal = glm::cross(down, right) + glm::cross(right, up);
			}
			else if (n % (w - 1) == 0)
			{
				//ultima columna. Excepto primera y ultima fila
				left = points[n - 1] - points[n];
				nrmPoints++;
				normal = glm::cross(up, left) + glm::cross(left, down);

			}
			else
			{
				//Resto de puntos
				right = points[n + 1] - points[n];
				nrmPoints++;
				left = points[n - 1] - points[n];
				nrmPoints++;
				normal = glm::cross(up, left) + glm::cross(left, down)+ glm::cross(down, right) + glm::cross(right, up);
			}
		}
			
		normal = glm::normalize(normal);
		nrmals.push_back(normal);

	}

	///////////////////////////
	//NORMAL CALCULATION FINISH
	//////////////////////////////
}
