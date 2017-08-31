#ifndef VIRTUAL_H
#define VIRTUAL_H

#include <libfreenect2/config.h>
#include <libfreenect2/frame_listener.hpp>
#include <iostream>
#include <string>
#include <map>

#include "flextGL.h"
#include <GLFW/glfw3.h>

#include <vector>

#include "opencv2/core.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/video.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


class Virtual
{
	public:
		glm::vec3 pos;
		glm::vec3 vel;
		glm::vec3 acc;
		glm::vec3 angle;
		glm::vec4 qx;
		glm::vec4 qy;
		glm::vec4 qz;
		glm::mat4 quatx;
		glm::mat4 quaty;
		glm::mat4 quatz;
		glm::vec4 omg;
		glm::mat4 orient;
		glm::mat4 Ibody;
		glm::mat4 Ibodyinv;
		glm::vec4 torque;

		void initialize();
		void initialize2();
		void initialize3();
		void initialize4();
		void Virtual::initializeR1();
};

#endif
