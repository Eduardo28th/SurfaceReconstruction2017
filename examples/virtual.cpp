
#include "virtual.h"
#include <cstdlib>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


void Virtual::initialize()
{
	pos = glm::vec3(0.15, 0.0, 0.55f);
	vel = glm::vec3(0.13f, 0.30f, 0.37f);
	angle = glm::vec3(0.57f, 0.57f, 0.57f);
	qx = glm::vec4(sin(angle.x / 2.0f), 0.0f, 0.0f, cos(angle.x / 2.0f));
	qy = glm::vec4(0.0f, sin(angle.y / 2.0f), 0.0f, cos(angle.y / 2.0f));
	qz = glm::vec4(0.0f, 0.0f, sin(angle.z / 2.0f), cos(angle.z / 2.0f));

	quatx = glm::mat4(
		1 - 2 * qx.y*qx.y - 2 * qx.z*qx.z, 2 * qx.x*qx.y - 2 * qx.z*qx.w, 2 * qx.x*qx.z + 2 * qx.y*qx.w, 0.0,
		2 * qx.x*qx.y + 2 * qx.w*qx.z, 1 - 2 * qx.x*qx.x - 2 * qx.z*qx.z, 2 * qx.y*qx.z - 2 * qx.w*qx.x, 0.0,
		2 * qx.x*qx.z - 2 * qx.w*qx.y, 2 * qx.y*qx.z + 2 * qx.w*qx.x, 1 - 2 * qx.x*qx.x - 2 * qx.y*qx.y, 0.0,
		0.0, 0.0, 0.0, 1.0
	);

	quaty = glm::mat4(
		1 - 2 * qy.y*qy.y - 2 * qy.z*qy.z, 2 * qy.x*qy.y - 2 * qy.z*qy.w, 2 * qy.x*qy.z + 2 * qy.y*qy.w, 0.0,
		2 * qy.x*qy.y + 2 * qy.w*qy.z, 1 - 2 * qy.x*qy.x - 2 * qy.z*qy.z, 2 * qy.y*qy.z - 2 * qy.w*qy.x, 0.0,
		2 * qy.x*qy.z - 2 * qy.w*qy.y, 2 * qy.y*qy.z + 2 * qy.w*qy.x, 1 - 2 * qy.x*qy.x - 2 * qy.y*qy.y, 0.0,
		0.0, 0.0, 0.0, 1.0
	);

	quatz = glm::mat4(
		1 - 2 * qz.y*qz.y - 2 * qz.z*qz.z, 2 * qz.x*qz.y - 2 * qz.z*qz.w, 2 * qz.x*qz.z + 2 * qz.y*qz.w, 0.0,
		2 * qz.x*qz.y + 2 * qz.w*qz.z, 1 - 2 * qz.x*qz.x - 2 * qz.z*qz.z, 2 * qz.y*qz.z - 2 * qz.w*qz.x, 0.0,
		2 * qz.x*qz.z - 2 * qz.w*qz.y, 2 * qz.y*qz.z + 2 * qz.w*qz.x, 1 - 2 * qz.x*qz.x - 2 * qz.y*qz.y, 0.0,
		0.0, 0.0, 0.0, 1.0
	);

	orient = quatx*quaty*quatz;
	Ibody = glm::mat4(
		0.00375f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.00375f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.00375f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	Ibodyinv = glm::inverse(Ibody);
	torque = glm::vec4(0.57f, 0.3f, 0.1f, 1.0f);
	omg = glm::vec4(0.0f);
}


void Virtual::initialize2()
{
	pos = glm::vec3(-0.15, 0.0, 1.1f);
	vel = glm::vec3(0.13f, 0.30f, 0.37f);
	angle = glm::vec3(0.30f, 0.57f, 0.84f);
	qx = glm::vec4(sin(angle.x / 2.0f), 0.0f, 0.0f, cos(angle.x / 2.0f));
	qy = glm::vec4(0.0f, sin(angle.y / 2.0f), 0.0f, cos(angle.y / 2.0f));
	qz = glm::vec4(0.0f, 0.0f, sin(angle.z / 2.0f), cos(angle.z / 2.0f));

	quatx = glm::mat4(
		1 - 2 * qx.y*qx.y - 2 * qx.z*qx.z, 2 * qx.x*qx.y - 2 * qx.z*qx.w, 2 * qx.x*qx.z + 2 * qx.y*qx.w, 0.0,
		2 * qx.x*qx.y + 2 * qx.w*qx.z, 1 - 2 * qx.x*qx.x - 2 * qx.z*qx.z, 2 * qx.y*qx.z - 2 * qx.w*qx.x, 0.0,
		2 * qx.x*qx.z - 2 * qx.w*qx.y, 2 * qx.y*qx.z + 2 * qx.w*qx.x, 1 - 2 * qx.x*qx.x - 2 * qx.y*qx.y, 0.0,
		0.0, 0.0, 0.0, 1.0
	);

	quaty = glm::mat4(
		1 - 2 * qy.y*qy.y - 2 * qy.z*qy.z, 2 * qy.x*qy.y - 2 * qy.z*qy.w, 2 * qy.x*qy.z + 2 * qy.y*qy.w, 0.0,
		2 * qy.x*qy.y + 2 * qy.w*qy.z, 1 - 2 * qy.x*qy.x - 2 * qy.z*qy.z, 2 * qy.y*qy.z - 2 * qy.w*qy.x, 0.0,
		2 * qy.x*qy.z - 2 * qy.w*qy.y, 2 * qy.y*qy.z + 2 * qy.w*qy.x, 1 - 2 * qy.x*qy.x - 2 * qy.y*qy.y, 0.0,
		0.0, 0.0, 0.0, 1.0
	);

	quatz = glm::mat4(
		1 - 2 * qz.y*qz.y - 2 * qz.z*qz.z, 2 * qz.x*qz.y - 2 * qz.z*qz.w, 2 * qz.x*qz.z + 2 * qz.y*qz.w, 0.0,
		2 * qz.x*qz.y + 2 * qz.w*qz.z, 1 - 2 * qz.x*qz.x - 2 * qz.z*qz.z, 2 * qz.y*qz.z - 2 * qz.w*qz.x, 0.0,
		2 * qz.x*qz.z - 2 * qz.w*qz.y, 2 * qz.y*qz.z + 2 * qz.w*qz.x, 1 - 2 * qz.x*qz.x - 2 * qz.y*qz.y, 0.0,
		0.0, 0.0, 0.0, 1.0
	);

	orient = quatx*quaty*quatz;
	Ibody = glm::mat4(
		0.00375f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.00375f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.00375f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	Ibodyinv = glm::inverse(Ibody);
	torque = glm::vec4(0.57f, 0.3f, 0.1f, 1.0f);
	omg = glm::vec4(0.0f);
}

void Virtual::initialize3()
{
	pos = glm::vec3(0.1, -0.5, 0.0f);
	vel = glm::vec3(0.13f, 0.30f, 0.37f);
	angle = glm::vec3(0.57f, 0.57f, 0.57f);
	qx = glm::vec4(sin(angle.x / 2.0f), 0.0f, 0.0f, cos(angle.x / 2.0f));
	qy = glm::vec4(0.0f, sin(angle.y / 2.0f), 0.0f, cos(angle.y / 2.0f));
	qz = glm::vec4(0.0f, 0.0f, sin(angle.z / 2.0f), cos(angle.z / 2.0f));

	quatx = glm::mat4(
		1 - 2 * qx.y*qx.y - 2 * qx.z*qx.z, 2 * qx.x*qx.y - 2 * qx.z*qx.w, 2 * qx.x*qx.z + 2 * qx.y*qx.w, 0.0,
		2 * qx.x*qx.y + 2 * qx.w*qx.z, 1 - 2 * qx.x*qx.x - 2 * qx.z*qx.z, 2 * qx.y*qx.z - 2 * qx.w*qx.x, 0.0,
		2 * qx.x*qx.z - 2 * qx.w*qx.y, 2 * qx.y*qx.z + 2 * qx.w*qx.x, 1 - 2 * qx.x*qx.x - 2 * qx.y*qx.y, 0.0,
		0.0, 0.0, 0.0, 1.0
	);

	quaty = glm::mat4(
		1 - 2 * qy.y*qy.y - 2 * qy.z*qy.z, 2 * qy.x*qy.y - 2 * qy.z*qy.w, 2 * qy.x*qy.z + 2 * qy.y*qy.w, 0.0,
		2 * qy.x*qy.y + 2 * qy.w*qy.z, 1 - 2 * qy.x*qy.x - 2 * qy.z*qy.z, 2 * qy.y*qy.z - 2 * qy.w*qy.x, 0.0,
		2 * qy.x*qy.z - 2 * qy.w*qy.y, 2 * qy.y*qy.z + 2 * qy.w*qy.x, 1 - 2 * qy.x*qy.x - 2 * qy.y*qy.y, 0.0,
		0.0, 0.0, 0.0, 1.0
	);

	quatz = glm::mat4(
		1 - 2 * qz.y*qz.y - 2 * qz.z*qz.z, 2 * qz.x*qz.y - 2 * qz.z*qz.w, 2 * qz.x*qz.z + 2 * qz.y*qz.w, 0.0,
		2 * qz.x*qz.y + 2 * qz.w*qz.z, 1 - 2 * qz.x*qz.x - 2 * qz.z*qz.z, 2 * qz.y*qz.z - 2 * qz.w*qz.x, 0.0,
		2 * qz.x*qz.z - 2 * qz.w*qz.y, 2 * qz.y*qz.z + 2 * qz.w*qz.x, 1 - 2 * qz.x*qz.x - 2 * qz.y*qz.y, 0.0,
		0.0, 0.0, 0.0, 1.0
	);

	orient = quatx*quaty*quatz;
	Ibody = glm::mat4(
		0.00375f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.00375f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.00375f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	Ibodyinv = glm::inverse(Ibody);
	torque = glm::vec4(0.57f, 0.3f, 0.1f, 1.0f);
	omg = glm::vec4(0.0f);
}

void Virtual::initialize4()
{
	pos = glm::vec3(-1.3, -0.3, 0.7f);
	vel = glm::vec3(0.30f, 0.33f, 0.11f);
	angle = glm::vec3(0.57f, 0.57f, 0.57f);
	qx = glm::vec4(sin(angle.x / 2.0f), 0.0f, 0.0f, cos(angle.x / 2.0f));
	qy = glm::vec4(0.0f, sin(angle.y / 2.0f), 0.0f, cos(angle.y / 2.0f));
	qz = glm::vec4(0.0f, 0.0f, sin(angle.z / 2.0f), cos(angle.z / 2.0f));

	quatx = glm::mat4(
		1 - 2 * qx.y*qx.y - 2 * qx.z*qx.z, 2 * qx.x*qx.y - 2 * qx.z*qx.w, 2 * qx.x*qx.z + 2 * qx.y*qx.w, 0.0,
		2 * qx.x*qx.y + 2 * qx.w*qx.z, 1 - 2 * qx.x*qx.x - 2 * qx.z*qx.z, 2 * qx.y*qx.z - 2 * qx.w*qx.x, 0.0,
		2 * qx.x*qx.z - 2 * qx.w*qx.y, 2 * qx.y*qx.z + 2 * qx.w*qx.x, 1 - 2 * qx.x*qx.x - 2 * qx.y*qx.y, 0.0,
		0.0, 0.0, 0.0, 1.0
	);

	quaty = glm::mat4(
		1 - 2 * qy.y*qy.y - 2 * qy.z*qy.z, 2 * qy.x*qy.y - 2 * qy.z*qy.w, 2 * qy.x*qy.z + 2 * qy.y*qy.w, 0.0,
		2 * qy.x*qy.y + 2 * qy.w*qy.z, 1 - 2 * qy.x*qy.x - 2 * qy.z*qy.z, 2 * qy.y*qy.z - 2 * qy.w*qy.x, 0.0,
		2 * qy.x*qy.z - 2 * qy.w*qy.y, 2 * qy.y*qy.z + 2 * qy.w*qy.x, 1 - 2 * qy.x*qy.x - 2 * qy.y*qy.y, 0.0,
		0.0, 0.0, 0.0, 1.0
	);

	quatz = glm::mat4(
		1 - 2 * qz.y*qz.y - 2 * qz.z*qz.z, 2 * qz.x*qz.y - 2 * qz.z*qz.w, 2 * qz.x*qz.z + 2 * qz.y*qz.w, 0.0,
		2 * qz.x*qz.y + 2 * qz.w*qz.z, 1 - 2 * qz.x*qz.x - 2 * qz.z*qz.z, 2 * qz.y*qz.z - 2 * qz.w*qz.x, 0.0,
		2 * qz.x*qz.z - 2 * qz.w*qz.y, 2 * qz.y*qz.z + 2 * qz.w*qz.x, 1 - 2 * qz.x*qz.x - 2 * qz.y*qz.y, 0.0,
		0.0, 0.0, 0.0, 1.0
	);

	orient = quatx*quaty*quatz;
	Ibody = glm::mat4(
		0.00375f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.00375f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.00375f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	Ibodyinv = glm::inverse(Ibody);
	torque = glm::vec4(0.57f, 0.3f, 0.1f, 1.0f);
	omg = glm::vec4(0.0f);
}

void Virtual::initializeR1()
{
	int x = rand() % 100 + 1;
	float x1 = (x / 100.0f) - 0.5f;
	float v1 = (50.0 - x)*0.009f;
	int z = rand() % 100 + 1;
	float vz = (z / 100.0f)*0.15f + 0.1f;
	pos = glm::vec3(x1, -0.5, 0.0f);
	vel = glm::vec3(v1, 0.33f, vz);
	angle = glm::vec3(0.57f, 0.57f, 0.57f);
	qx = glm::vec4(sin(angle.x / 2.0f), 0.0f, 0.0f, cos(angle.x / 2.0f));
	qy = glm::vec4(0.0f, sin(angle.y / 2.0f), 0.0f, cos(angle.y / 2.0f));
	qz = glm::vec4(0.0f, 0.0f, sin(angle.z / 2.0f), cos(angle.z / 2.0f));

	quatx = glm::mat4(
		1 - 2 * qx.y*qx.y - 2 * qx.z*qx.z, 2 * qx.x*qx.y - 2 * qx.z*qx.w, 2 * qx.x*qx.z + 2 * qx.y*qx.w, 0.0,
		2 * qx.x*qx.y + 2 * qx.w*qx.z, 1 - 2 * qx.x*qx.x - 2 * qx.z*qx.z, 2 * qx.y*qx.z - 2 * qx.w*qx.x, 0.0,
		2 * qx.x*qx.z - 2 * qx.w*qx.y, 2 * qx.y*qx.z + 2 * qx.w*qx.x, 1 - 2 * qx.x*qx.x - 2 * qx.y*qx.y, 0.0,
		0.0, 0.0, 0.0, 1.0
	);

	quaty = glm::mat4(
		1 - 2 * qy.y*qy.y - 2 * qy.z*qy.z, 2 * qy.x*qy.y - 2 * qy.z*qy.w, 2 * qy.x*qy.z + 2 * qy.y*qy.w, 0.0,
		2 * qy.x*qy.y + 2 * qy.w*qy.z, 1 - 2 * qy.x*qy.x - 2 * qy.z*qy.z, 2 * qy.y*qy.z - 2 * qy.w*qy.x, 0.0,
		2 * qy.x*qy.z - 2 * qy.w*qy.y, 2 * qy.y*qy.z + 2 * qy.w*qy.x, 1 - 2 * qy.x*qy.x - 2 * qy.y*qy.y, 0.0,
		0.0, 0.0, 0.0, 1.0
	);

	quatz = glm::mat4(
		1 - 2 * qz.y*qz.y - 2 * qz.z*qz.z, 2 * qz.x*qz.y - 2 * qz.z*qz.w, 2 * qz.x*qz.z + 2 * qz.y*qz.w, 0.0,
		2 * qz.x*qz.y + 2 * qz.w*qz.z, 1 - 2 * qz.x*qz.x - 2 * qz.z*qz.z, 2 * qz.y*qz.z - 2 * qz.w*qz.x, 0.0,
		2 * qz.x*qz.z - 2 * qz.w*qz.y, 2 * qz.y*qz.z + 2 * qz.w*qz.x, 1 - 2 * qz.x*qz.x - 2 * qz.y*qz.y, 0.0,
		0.0, 0.0, 0.0, 1.0
	);

	orient = quatx*quaty*quatz;
	Ibody = glm::mat4(
		0.00375f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.00375f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.00375f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	Ibodyinv = glm::inverse(Ibody);
	torque = glm::vec4(0.57f, 0.3f, 0.1f, 1.0f);
	omg = glm::vec4(0.0f);
}
