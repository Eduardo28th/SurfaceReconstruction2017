
#include "viewer.h"
#include <cstdlib>

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


Viewer::Viewer() : shader_folder("src/shader/"), 
                   win_width(300),
                   win_height(200)
{
}





static void glfwErrorCallback(int error, const char* description)
{
  std::cerr << "GLFW error " << error << " " << description << std::endl;
}

void Viewer::initialize()
{
    // init glfw - if already initialized nothing happens
    glfwInit();

    GLFWerrorfun prev_func = glfwSetErrorCallback(glfwErrorCallback);
    if (prev_func)
      glfwSetErrorCallback(prev_func);

    // setup context
    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
#ifdef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
#endif
    //glfwWindowHint(GLFW_VISIBLE, debug ? GL_TRUE : GL_FALSE);

    window = glfwCreateWindow(win_width*4, win_height*2, "Viewer (press ESC to exit)", 0, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to create opengl window." << std::endl;
        exit(-1);
    }


    glfwMakeContextCurrent(window);
    OpenGLBindings *b = new OpenGLBindings();
    flextInit(b);
    gl(b);

    std::string vertexshadersrc = ""
        "#version 330\n"
                                                
        "in vec3 Position;"
        "in vec2 TexCoord;"
        "uniform mat4 projection;"
		"uniform mat4 view;"
        "out VertexData{"
        "vec2 TexCoord;" 
        "} VertexOut;"  
                    
        "void main(void)"
        "{"
        "    gl_Position = projection* view* vec4(Position, 1.0);"
		//"    gl_Position =  vec4(Position, 1.0);"
        "    VertexOut.TexCoord = TexCoord;"
        "}";


	std::string vertexshadermdl = ""
		"#version 330\n"

		"in vec3 Position;"
		"in vec3 Normal;"
		"out vec3 FragPos;"
		"out vec3 normal;"
		"uniform mat4 projection;"
		"uniform mat4 view;"
		"uniform mat4 model;"

		"void main(void)"
		"{"
		"FragPos = vec3(model * vec4(Position, 1.0));"
		"normal = Normal;"
		"gl_Position = projection* view* vec4(FragPos, 1.0);"
		
		"}";

	std::string vertexshadersrc2 = ""
		"#version 330\n"

		"in vec2 Position;"
		"in vec2 TexCoord;"

		"out VertexData{"
		"vec2 TexCoord;"
		"} VertexOut;"

		"void main(void)"
		"{"
		"    gl_Position = vec4(Position, 0.0, 1.0);"
		"    VertexOut.TexCoord = TexCoord;"
		"}";

	std::string vertexshadersrc3 = ""
		"#version 330\n"

		"in vec2 Position;"
		"in vec2 TexCoord;"
		"in vec2 DepthCoord;"

		"out VertexData{"
		"vec2 TexCoord;"
		"vec2 DepthCoord;"
		"} VertexOut;"

		"void main(void)"
		"{"
		"    gl_Position = vec4(Position, 0.0, 1.0);"
		"    VertexOut.TexCoord = TexCoord;"
		"    VertexOut.DepthCoord = DepthCoord; "
		"}";


    std::string redfragmentshader = ""
        "#version 330\n"
        
        "uniform sampler2DRect Data;"
        
        "vec4 tempColor;"
        "in VertexData{"
        "    vec2 TexCoord;"
        "} FragmentIn;"
        
        "layout(location = 0) out vec4 Color;"
        
        "void main(void)"
        "{"
            //"ivec2 uv = ivec2(FragmentIn.TexCoord.x, FragmentIn.TexCoord.y);"
            //"tempColor = texelFetch(Data, uv);"
            //"Color = vec4(tempColor.x/4500, tempColor.x/4500, tempColor.x/4500, 1);"
		"Color = vec4(0.0, 0.0, 1.0, 0.0);"
        "}";

	std::string grayfragmentshader = ""
		"#version 330\n"

		"uniform sampler2DRect Data;"

		"vec4 tempColor;"
		"in VertexData{"
		"    vec2 TexCoord;"
		"} FragmentIn;"

		"layout(location = 0) out vec4 Color;"

		"void main(void)"
		"{"
		"ivec2 uv = ivec2(FragmentIn.TexCoord.x, FragmentIn.TexCoord.y);"
		"tempColor = texelFetch(Data, uv);"
		//"Color = vec4(tempColor.x/4500, tempColor.x/4500, tempColor.x/4500, 1);"
		"Color = vec4(1.0);"
		"}";


	std::string fragmentshader = ""
		"#version 330\n"

		"uniform sampler2DRect Data;"

		"in VertexData{"
		"    vec2 TexCoord;"
		"} FragmentIn;"

		"layout(location = 0) out vec4 Color;"

		"void main(void)"
		"{"
		"    ivec2 uv = ivec2(FragmentIn.TexCoord.x, FragmentIn.TexCoord.y);"

		"    Color = texelFetch(Data, uv);"
		"}";

	std::string fragmentshader2 = ""
		"#version 330\n"


		"uniform sampler2DRect Data;"
		"uniform sampler2DRect Data2;"
		"uniform vec3 cubePos;"

		"in VertexData{"
		"    vec2 TexCoord;"
		"    vec2 DepthCoord;"
		"} FragmentIn;"

		"layout(location = 0) out vec4 Color;"

		"void main(void)"
		"{"
		"    ivec2 uv2 = ivec2(FragmentIn.DepthCoord.x, FragmentIn.DepthCoord.y);"
		"    ivec2 uv = ivec2(FragmentIn.TexCoord.x, FragmentIn.TexCoord.y);"
		" vec4 clr = texelFetch(Data, uv); "
		" vec4 dp = texelFetch(Data2, uv2); "


		//SEVERAL CUBES TEST
		/*
		"if(cubePos.z > 1.0f)"
		" Color = vec4( 0.0, 0.0, 1.0, 1.0);"
		"else"
		" Color = vec4( 0.0, 0.0, 0.0, 1.0);"
		*/

		/*
		" if(dp.x > 0.0f)"
		"Color = vec4(1.0, 0.0, 0.0, 1.0);"
		"  else"
		"  Color = vec4(0.0, 0.0, 1.0, 1.0);"
		*/

		//"if( dp.x/999.999762f > cubePos.z && dp.x/4500.0 > 0.12998f)"

		/*
		"if( dp.x/999.999762f > cubePos.z && dp.x > 0.0f)"
		" Color = vec4(0.0f);"
		"else"
		"    Color = clr;"
		*/
		" if(dp.x > 0.0f)"
		"{"
		" if( dp.x/999.999762f > cubePos.z)"
		" Color = vec4(0.0f);"
		"else"
		"    Color = clr;"
		"}"
		"else "
		" Color = vec4(0.0f);"
		
		
		"}";

	
	std::string redfragmentmdl = ""
		"#version 330\n"

		"out vec4 Color;"

	"in vec3 normal;"
	"in vec3 FragPos;"

	"uniform vec3 lightPos;"
	"uniform vec3 lightColor;"
	"uniform vec3 objectColor;"

	"void main()"
	"{"
		// ambient
		"float ambientStrength = 0.1;"
		"vec3 ambient = ambientStrength * lightColor;"

		// diffuse 
		"vec3 norm = normalize(normal);"
		"vec3 lightDir = normalize(lightPos - FragPos);"
		"float diff = max(dot(norm, lightDir), 0.0);"
		"vec3 diffuse = diff * lightColor;"

		"vec3 result = (ambient + diffuse) * objectColor;"
		"Color = vec4(result, 1.0);"
		"}";

    renderShader.setVertexShader(vertexshadersrc2);
    renderShader.setFragmentShader(fragmentshader);
    renderShader.build();

    renderGrayShader.setVertexShader(vertexshadersrc);
    renderGrayShader.setFragmentShader(grayfragmentshader);
    renderGrayShader.build();

	renderRedShader.setVertexShader(vertexshadersrc);
	renderRedShader.setFragmentShader(redfragmentshader);
	renderRedShader.build();

	renderModel.setVertexShader(vertexshadermdl);
	renderModel.setFragmentShader(redfragmentmdl);
	renderModel.build();

	renderRegion.setVertexShader(vertexshadersrc3);
	renderRegion.setFragmentShader(fragmentshader2);
	renderRegion.build();


    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, Viewer::key_callbackstatic);
    glfwSetWindowSizeCallback(window, Viewer::winsize_callbackstatic);

    shouldStop = false;

	deltaTime = 0.0f;
	lastFrame = 0.0f;
	
	Virtual cube1, cube2;
	cube1.initializeR1();
	cube2.initialize4();
	cubes.push_back(cube1);
	cubes.push_back(cube2);
	/*
	//pos = glm::vec3(0.0, -0.5, 0.0);
	cube.pos = glm::vec3(0.0, 0.0, 0.82f);
	cube.vel = glm::vec3(0.13f, 0.30f, 0.37f);
	cube.angle = glm::vec3(0.57f, 0.57f, 0.57f);
	glm::vec3 angle = cube.angle;
	cube.qx = glm::vec4(sin(angle.x / 2.0f), 0.0f, 0.0f, cos(angle.x / 2.0f));
	cube.qy = glm::vec4(0.0f, sin(angle.y / 2.0f), 0.0f, cos(angle.y / 2.0f));
	cube.qz = glm::vec4(0.0f, 0.0f, sin(angle.z / 2.0f), cos(angle.z / 2.0f));

	glm::vec4 qx = cube.qx;
	glm::vec4 qy = cube.qy;
	glm::vec4 qz = cube.qz;

	cube.quatx = glm::mat4(
		1 - 2 * qx.y*qx.y - 2 * qx.z*qx.z, 2 * qx.x*qx.y - 2 * qx.z*qx.w, 2 * qx.x*qx.z + 2 * qx.y*qx.w, 0.0,
		2 * qx.x*qx.y + 2 * qx.w*qx.z, 1 - 2 * qx.x*qx.x - 2 * qx.z*qx.z, 2 * qx.y*qx.z - 2 * qx.w*qx.x, 0.0,
		2 * qx.x*qx.z - 2 * qx.w*qx.y, 2 * qx.y*qx.z + 2 * qx.w*qx.x, 1 - 2 * qx.x*qx.x - 2 * qx.y*qx.y, 0.0,
		0.0, 0.0, 0.0, 1.0
	);

	cube.quaty = glm::mat4(
		1 - 2 * qy.y*qy.y - 2 * qy.z*qy.z, 2 * qy.x*qy.y - 2 * qy.z*qy.w, 2 * qy.x*qy.z + 2 * qy.y*qy.w, 0.0,
		2 * qy.x*qy.y + 2 * qy.w*qy.z, 1 - 2 * qy.x*qy.x - 2 * qy.z*qy.z, 2 * qy.y*qy.z - 2 * qy.w*qy.x, 0.0,
		2 * qy.x*qy.z - 2 * qy.w*qy.y, 2 * qy.y*qy.z + 2 * qy.w*qy.x, 1 - 2 * qy.x*qy.x - 2 * qy.y*qy.y, 0.0,
		0.0, 0.0, 0.0, 1.0
	);

	cube.quatz = glm::mat4(
		1 - 2 * qz.y*qz.y - 2 * qz.z*qz.z, 2 * qz.x*qz.y - 2 * qz.z*qz.w, 2 * qz.x*qz.z + 2 * qz.y*qz.w, 0.0,
		2 * qz.x*qz.y + 2 * qz.w*qz.z, 1 - 2 * qz.x*qz.x - 2 * qz.z*qz.z, 2 * qz.y*qz.z - 2 * qz.w*qz.x, 0.0,
		2 * qz.x*qz.z - 2 * qz.w*qz.y, 2 * qz.y*qz.z + 2 * qz.w*qz.x, 1 - 2 * qz.x*qz.x - 2 * qz.y*qz.y, 0.0,
		0.0, 0.0, 0.0, 1.0
	);

	cube.orient = cube.quatx*cube.quaty*cube.quatz;
	cube.Ibody = glm::mat4(
		0.00375f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.00375f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.00375f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	);
	cube.Ibodyinv = glm::inverse(cube.Ibody);
	cube.torque = glm::vec4(0.57f, 0.3f, 0.1f, 1.0f);
	cube.omg = glm::vec4(0.0f);*/

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	adj1 = 75.2525f*1.3f;
	adj2 = 75.2525f*0.7f;
	ready = false;
}

void Viewer::winsize_callbackstatic(GLFWwindow* window, int w, int h)
{
    Viewer* viewer = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(window));
    viewer->winsize_callback(window, w, h);
}

void Viewer::winsize_callback(GLFWwindow* window, int w, int h)
{
    win_width = w/2;
    win_height = h/2;
}

void Viewer::key_callbackstatic(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Viewer* viewer = reinterpret_cast<Viewer*>(glfwGetWindowUserPointer(window));
    viewer->key_callback(window, key, scancode, action, mods);
}

void Viewer::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        shouldStop = true;
}

void Viewer::onOpenGLBindingsChanged(OpenGLBindings *b)
{
    renderShader.gl(b);
    renderGrayShader.gl(b);
	renderRedShader.gl(b);
	renderModel.gl(b);
	renderRegion.gl(b);
    rgb.gl(b);
    ir.gl(b);
}

bool Viewer::render()
{
    // wipe the drawing surface clear
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    GLint x = 0, y = 0;
    int fb_width, fb_width_half, fb_height, fb_height_half;

    std::map<std::string, libfreenect2::Frame*>::iterator iter;
	GLfloat currentFrame = glfwGetTime();

	GLfloat c_vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};
	

    //for (iter = frames.begin(); iter != frames.end(); ++iter)
	for (int screens = 0; screens < 2; screens++)
	{
		if(screens == 0)
		{
			glStencilFunc(GL_ALWAYS, 1, 0xFF);
			//  libfreenect2::Frame* frame = iter->second;
			
			// Using the frame buffer size to account for screens where window.size != framebuffer.size, e.g. retina displays
			glfwGetFramebufferSize(window, &fb_width, &fb_height);
			fb_width_half = (fb_width + 1) / 2;
			glViewport(0, 0, fb_width_half, fb_height);
		
			
			//glDepthMask(GL_TRUE);

			for (int n = 0; n < vertices.size(); n++)
			{
				std::vector<glm::vec3>& ver = vertices[n];
				//std::vector<int>& indcs = indices[n];


				gl()->glGenBuffers(1, &triangle_vbo);
				gl()->glGenBuffers(1, &triangle_ebo);
				gl()->glGenVertexArrays(1, &triangle_vao);

				gl()->glBindVertexArray(triangle_vao);
				gl()->glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
				gl()->glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*ver.size(), &ver[0], GL_STATIC_DRAW);

				//gl()->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangle_ebo);
				//gl()->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int)*indcs.size(), &indcs[0], GL_STATIC_DRAW);

				GLint position_attr = renderShader.getAttributeLocation("Position");
				gl()->glVertexAttribPointer(position_attr, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (GLvoid*)0);
				gl()->glEnableVertexAttribArray(position_attr);

				//GLint texcoord_attr = renderShader.getAttributeLocation("TexCoord");
				//gl()->glVertexAttribPointer(texcoord_attr, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(2 * sizeof(float)));
				//gl()->glEnableVertexAttribArray(texcoord_attr);


				//if (iter->first == "RGB" || iter->first == "registered")
				//{
				renderGrayShader.use();

				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				glm::mat4 projection;
				glm::mat4 view;
				////FRONT VIEW
				view = glm::rotate(view, 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				view = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.5f));

				//SIDE VIEW
				//view = glm::rotate(view, 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				//view = glm::translate(view, glm::vec3(2.0f, 0.0f, -1.0f));

				projection = glm::perspective(45.0f, (float)fb_width_half / (float)fb_height, 0.1f, 100.0f);
				gl()->glUniformMatrix4fv(gl()->glGetUniformLocation(renderGrayShader.program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
				gl()->glUniformMatrix4fv(gl()->glGetUniformLocation(renderGrayShader.program, "view"), 1, GL_FALSE, glm::value_ptr(view));

				//glDrawArrays(GL_POINTS, 0, vertices.size());


				glDrawArrays(GL_POINTS, 0, ver.size());
				//glDrawElements(GL_TRIANGLES, indcs.size(), GL_UNSIGNED_INT, 0);
				gl()->glBindVertexArray(0);

				gl()->glDeleteBuffers(1, &triangle_vbo);
				gl()->glDeleteBuffers(1, &triangle_ebo);
				gl()->glDeleteVertexArrays(1, &triangle_vao);
			}

			

			////////////////////////////
			/////////Draw a cube////////
			////////////////////////////

			/*
			renderModel.use();

			GLuint cubeVAO, cubeVBO;
			gl()->glGenBuffers(1, &cubeVBO);
			gl()->glGenVertexArrays(1, &cubeVAO);

			gl()->glBindVertexArray(cubeVAO);
			gl()->glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
			gl()->glBufferData(GL_ARRAY_BUFFER, sizeof(c_vertices), &c_vertices, GL_STATIC_DRAW);


			
			gl()->glVertexAttribPointer(position_attr, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
			gl()->glEnableVertexAttribArray(position_attr);

			GLint normal_attr = renderModel.getAttributeLocation("Normal");
			gl()->glVertexAttribPointer(normal_attr, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
			gl()->glEnableVertexAttribArray(normal_attr);

			glm::mat4 model;

			deltaTime = currentFrame - lastFrame;
			lastFrame = currentFrame;
			if (pos.y < -2.0f || pos.y > 2.0f)
			{
				pos = glm::vec3(0.0, -0.5, 0.0);
				vel = glm::vec3(0.13f, 0.30f, 0.37f);
				torque = glm::vec4(0.57f, 0.3f, 0.1f, 1.0f);
			}

			//Check for collision broad phase
			bool collision = false;
			for (int n = 0; n < vertices.size(); n++)
			{
				glm::vec3 p = vertices[n];
				float dis = (pos.x - p.x)*(pos.x - p.x) + (pos.y - p.y)*(pos.y - p.y) + (pos.z - p.z)*(pos.z - p.z);
				if (dis <= 0.012)
				{
					collision = true;
					cv = n;
					break;
				}
			}

			if (!collision)
			{
				pos += vel*deltaTime;
				vel += glm::vec3(0.0f, -0.098f, 0.0f)*deltaTime;
				glm::mat4 Itensor_inv = glm::transpose(orient)*Ibodyinv*orient;
				omg += Itensor_inv*torque*deltaTime;
				torque = glm::vec4(0.0);
				omg = glm::normalize(omg);
				glm::mat4 omg_star = glm::mat4(
					0.0, -omg.z, omg.y, 0.0,
					omg.z, 0.0, omg.x, 0.0,
					omg.y, omg.x, 0.0, 0.0,
					0.0, 0.0, 0.0, 1.0
				);
				orient += omg_star*orient*deltaTime;
				orient = normMatrix(orient);

			}
			else
			{
				//pos = glm::vec3(0.0, -0.5, 0.0);
				vel = vnormals[cv];
				pos += vel*deltaTime;
				collision = false;
				//torque = glm::vec4(0.57f, 0.3f, 0.1f, 1.0f);
			}
			


			model = glm::translate(model, pos);
			model = model * orient;
			model = glm::scale(model, glm::vec3(0.15f, 0.15, 0.15f));

			gl()->glUniformMatrix4fv(gl()->glGetUniformLocation(renderModel.program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
			gl()->glUniformMatrix4fv(gl()->glGetUniformLocation(renderModel.program, "view"), 1, GL_FALSE, glm::value_ptr(view));
			gl()->glUniformMatrix4fv(gl()->glGetUniformLocation(renderModel.program, "model"), 1, GL_FALSE, glm::value_ptr(model));
			gl()->glUniform3f(gl()->glGetUniformLocation(renderModel.program, "objectColor"), 1.0f, 0.5f, 0.31f);
			gl()->glUniform3f(gl()->glGetUniformLocation(renderModel.program, "lightColor"), 1.0f, 1.0f, 1.0f);
			gl()->glUniform3f(gl()->glGetUniformLocation(renderModel.program, "lightPos"), 0.5f, 1.0f, 0.3f);

			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDrawArrays(GL_TRIANGLES, 0, 36);
			gl()->glBindVertexArray(0);

			gl()->glDeleteBuffers(1, &cubeVBO);
			gl()->glDeleteVertexArrays(1, &cubeVAO);

			*/
			
		


			//glDepthMask(GL_FALSE);
				

				

		}
		else
		//////////////////////////////////////////////////////
		//////////////////////////////COLOR STREAM
		///////////////////////////////////////////////////////

		{


			/////////////////////////////////////////////
			////////////COLOR STREAM/////////////////////
			/////////////////////////////////////////////
			
			glStencilMask(0x00);
			iter = frames.begin();
			libfreenect2::Frame* frame = iter->second;

			// Using the frame buffer size to account for screens where window.size != framebuffer.size, e.g. retina displays
			glfwGetFramebufferSize(window, &fb_width, &fb_height);
			//fb_width_half = (fb_width + 1) / 2;
			//fb_height_half = (fb_height + 1) / 2;
			glViewport(fb_width_half, 0, fb_width_half, fb_height);
			//glViewport(x, y, fb_width_half, fb_height_half);
			

			float w = static_cast<float>(frame->width);
			float h = static_cast<float>(frame->height);

			glDepthMask(GL_FALSE);
			Vertex bl = { -1.0f, -1.0f, 0.0f, 0.0f };
			Vertex br = { 1.0f, -1.0f, w, 0.0f };
			Vertex tl = { -1.0f, 1.0f, 0.0f, h };
			Vertex tr = { 1.0f, 1.0f, w, h };
			Vertex screen[] = {
				bl, tl, tr,
				tr, br, bl
			};

			
			


			gl()->glGenBuffers(1, &triangle_vbo);
			gl()->glGenVertexArrays(1, &triangle_vao);

			gl()->glBindVertexArray(triangle_vao);
			gl()->glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
			gl()->glBufferData(GL_ARRAY_BUFFER, sizeof(screen), screen, GL_STATIC_DRAW);

			GLint position_attr = renderShader.getAttributeLocation("Position");
			gl()->glVertexAttribPointer(position_attr, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
			gl()->glEnableVertexAttribArray(position_attr);

			GLint texcoord_attr = renderShader.getAttributeLocation("TexCoord");
			gl()->glVertexAttribPointer(texcoord_attr, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)(2 * sizeof(float)));
			gl()->glEnableVertexAttribArray(texcoord_attr);

			renderShader.use();

			rgb.allocate(frame->width, frame->height);
			std::copy(frame->data, frame->data + frame->width * frame->height * frame->bytes_per_pixel, rgb.data);
			rgb.flipY();
			rgb.upload();
			gl()->glUniform1i(gl()->glGetUniformLocation(renderRegion.program, "Data"), 0);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glDrawArrays(GL_TRIANGLES, 0, 6);

		


			//////////////////////
			/////////SEVERAL CUBES
			//////////////////////

			if (ready)
			{
				iter++;
				libfreenect2::Frame* frame2 = iter->second;

				ir.allocate2(frame2->width, frame2->height);
				std::copy(frame2->data, frame2->data + frame2->width * frame2->height * frame2->bytes_per_pixel, ir.data2);
				ir.flipY2();
				ir.upload2();

				GLuint cubeVAO, cubeVBO;
				gl()->glGenBuffers(1, &cubeVBO);
				gl()->glGenVertexArrays(1, &cubeVAO);

				gl()->glBindVertexArray(cubeVAO);
				gl()->glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
				gl()->glBufferData(GL_ARRAY_BUFFER, sizeof(c_vertices), &c_vertices, GL_STATIC_DRAW);



				gl()->glVertexAttribPointer(position_attr, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
				gl()->glEnableVertexAttribArray(position_attr);

				GLint normal_attr = renderModel.getAttributeLocation("Normal");
				gl()->glVertexAttribPointer(normal_attr, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
				gl()->glEnableVertexAttribArray(normal_attr);

				deltaTime = currentFrame - lastFrame;
				lastFrame = currentFrame;

				glm::mat4 projection;
				glm::mat4 view;
				////FRONT VIEW
				view = glm::rotate(view, 180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				view = glm::translate(view, glm::vec3(0.0f, 0.0f, 0.5f));

				//SIDE VIEW
				//view = glm::rotate(view, 90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
				//view = glm::translate(view, glm::vec3(2.0f, 0.0f, -1.0f));

				projection = glm::perspective(45.0f, (float)fb_width_half / (float)fb_height, 0.1f, 100.0f);

				for (int n = 0; n < cubes.size(); n++)
				{
					Virtual& cube = cubes[n];

					glStencilFunc(GL_ALWAYS, 1, 0xFF);
					glStencilMask(0xFF);


					glDepthMask(GL_TRUE);
					glEnable(GL_DEPTH_TEST);
					renderModel.use();
					gl()->glBindVertexArray(cubeVAO);


					glm::mat4 model;
					
					if (cube.pos.z > (maximum[0].z+ 0.5f) || cube.pos.z < 0.0 || cube.pos.x > maximum[0].x + 1.5 || cube.pos.x < minimum[0].x-1.5)
					{
						//It means is beyond the background or behind the camera
						if (n == 0)
							cube.initializeR1();
						else
							cube.initialize4();
					} 
					else
					{
						bool collision = false;
						for (int m = 0; (m < vertices.size()) && !collision; m++)
						{
							if (cube.pos.z > minimum[m].z && cube.pos.z < maximum[m].z)
							{
								if (cube.pos.x > minimum[m].x && cube.pos.x < maximum[m].x)
								{
									for (int q = 0; q < vertices[m].size(); q++)
									{
										glm::vec3 &p = vertices[m][q];
										float dis = (cube.pos.x - p.x)*(cube.pos.x - p.x) + (cube.pos.y - p.y)*(cube.pos.y - p.y) + (cube.pos.z - p.z)*(cube.pos.z - p.z);
										if (dis <= 0.012)
										{
											collision = true;
											cr = m;
											cv = q;
											break;
										}
									}
								}
							}
						}

						
						if (!collision)
						{
							cube.pos += cube.vel*deltaTime;
							cube.vel += glm::vec3(0.0f, -0.098f, 0.0f)*deltaTime;
							glm::mat4 Itensor_inv = glm::transpose(cube.orient)*cube.Ibodyinv*cube.orient;
							cube.omg += Itensor_inv*cube.torque*deltaTime;
							cube.torque = glm::vec4(0.0);
							cube.omg = glm::normalize(cube.omg);
							glm::vec4 omg = cube.omg;
							glm::mat4 omg_star = glm::mat4(
								0.0, -omg.z, omg.y, 0.0,
								omg.z, 0.0, omg.x, 0.0,
								omg.y, omg.x, 0.0, 0.0,
								0.0, 0.0, 0.0, 1.0
							);
							cube.orient += omg_star*cube.orient*deltaTime;
							cube.orient = normMatrix(cube.orient);
						}
						else
						{
							cube.vel = vnormals[cr][cv];
							std::cout << "new vel = " << cube.vel.x <<", "<< cube.vel.y <<", " << cube.vel.z << std::endl;
							if (isnan(cube.vel.x) == 1)
								cube.vel = glm::vec3(0.0, 0.0, -1.0);
							cube.pos += cube.vel*deltaTime;
							collision = false;
						}

						
					}

				


					////////////////CHANGE///////////////////


					model = glm::translate(model, cube.pos);
					model = model * cube.orient;
					model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
					gl()->glUniformMatrix4fv(gl()->glGetUniformLocation(renderModel.program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
					gl()->glUniformMatrix4fv(gl()->glGetUniformLocation(renderModel.program, "view"), 1, GL_FALSE, glm::value_ptr(view));
					gl()->glUniformMatrix4fv(gl()->glGetUniformLocation(renderModel.program, "model"), 1, GL_FALSE, glm::value_ptr(model));
					gl()->glUniform3f(gl()->glGetUniformLocation(renderModel.program, "objectColor"), 1.0f, 0.5f, 0.31f);
					gl()->glUniform3f(gl()->glGetUniformLocation(renderModel.program, "lightColor"), 1.0f, 1.0f, 1.0f);
					gl()->glUniform3f(gl()->glGetUniformLocation(renderModel.program, "lightPos"), 0.5f, 1.0f, 0.3f);

					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glDrawArrays(GL_TRIANGLES, 0, 36);







					float w2 = static_cast<float>(frame2->width);
					float h2 = static_cast<float>(frame2->height);



					glEnable(GL_BLEND);
					glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


					glStencilFunc(GL_EQUAL, 1, 0xFF);
					glStencilMask(0xFF);



					float screen2[] = {
						-1.0f, -1.18f,	 0.0f, -97.2f,		-adj1, 40.0f,
						-1.0f, 1.18f,	 0.0f, h + 97.2f,	-adj1, h2,
						1.0f, 1.18f,	 w, h + 97.2f,		w2 + adj2, h2,
						1.0f, 1.18f,	 w, h + 97.2f,		w2 + adj2, h2,
						1.0f, -1.18f,	 w, 0.0f,			w2 + adj2, 40.0f,
						-1.0f, -1.18f,	 0.0f, -97.2f,		-adj1, 40.0f
					};


					gl()->glGenBuffers(1, &triangle_vbo);
					gl()->glGenVertexArrays(1, &triangle_vao);

					gl()->glBindVertexArray(triangle_vao);
					gl()->glBindBuffer(GL_ARRAY_BUFFER, triangle_vbo);
					gl()->glBufferData(GL_ARRAY_BUFFER, sizeof(screen2), &screen2, GL_STATIC_DRAW);

					//GLint position_attr = renderShader.getAttributeLocation("Position");
					gl()->glVertexAttribPointer(position_attr, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid*)0);
					gl()->glEnableVertexAttribArray(position_attr);

					GLint texcoord_attr2 = renderRegion.getAttributeLocation("TexCoord");
					gl()->glVertexAttribPointer(texcoord_attr2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid*)(2 * sizeof(float)));
					gl()->glEnableVertexAttribArray(texcoord_attr2);

					GLint depth_attr = renderRegion.getAttributeLocation("DepthCoord");
					gl()->glVertexAttribPointer(depth_attr, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (GLvoid*)(4 * sizeof(float)));
					gl()->glEnableVertexAttribArray(depth_attr);


					renderRegion.use();
					glm::vec3 posi = cube.pos;
					gl()->glUniform3f(gl()->glGetUniformLocation(renderRegion.program, "cubePos"), posi.x, posi.y, posi.z);

					gl()->glUniform1i(gl()->glGetUniformLocation(renderRegion.program, "Data2"), 1);
					//rgb.upload();


					glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
					glDrawArrays(GL_TRIANGLES, 0, 6);




					glDisable(GL_BLEND);


					glClear(GL_STENCIL_BUFFER_BIT);


				}


				ir.deallocate2();

				gl()->glDeleteBuffers(1, &cubeVBO);
				gl()->glDeleteVertexArrays(1, &cubeVAO);
			}

			
			rgb.deallocate();

			gl()->glBindVertexArray(0);

			

			

			
			
			
			///////////////////////////////////////////
			/////////////////////CUBE RENDERING
			///////////////////////////////////////////

			gl()->glDeleteBuffers(1, &triangle_vbo);
			gl()->glDeleteBuffers(1, &triangle_ebo);
			gl()->glDeleteVertexArrays(1, &triangle_vao);


		}


	}
    










    // put the stuff we've been drawing onto the display
    glfwSwapBuffers(window);
    // update other events like input handling 
    glfwPollEvents();

    return shouldStop || glfwWindowShouldClose(window);
}

void Viewer::addFrame(std::string id, libfreenect2::Frame* frame)
{
    frames[id] = frame;
}

void Viewer::addPointCloud(std::vector<std::vector<glm::vec3>> points)
{
	//copy(points.begin(), points.end(), vertices.begin());
	vertices = points;
}


void Viewer::addNormals(std::vector<std::vector<glm::vec3>> points)
{
	//copy(points.begin(), points.end(), vertices.begin());
	vnormals = points;
}

void Viewer::addIndices(std::vector<std::vector<int>> ind)
{
	indices = ind;
}

void Viewer::addMin(std::vector<glm::vec3> points)
{
	minimum = points;
}

void Viewer::addMax(std::vector<glm::vec3> points)
{
	maximum = points;
}

void Viewer::addMask(std::vector<cv::Mat> mask)
{
	regionMask = mask;
}

glm::mat4 Viewer::normMatrix(glm::mat4 matrix)
{
	//std::cout << matrix[1][1] << std::endl;
	glm::vec3 cx = glm::vec3(matrix[0][0], matrix[1][0], matrix[2][0]);
	glm::vec3 cy = glm::vec3(matrix[0][1], matrix[1][1], matrix[2][1]);
	glm::vec3 cz = glm::vec3(matrix[0][2], matrix[1][2], matrix[2][2]);
	cx = glm::normalize(cx);
	cy = glm::cross(cz, cx);
	cy = glm::normalize(cy);
	cz = glm::cross(cx, cy);
	cz = glm::normalize(cz);
	matrix = glm::mat4(
		cx.x, cx.y, cx.z, 0.0,
		cy.x, cy.y, cy.z, 0.0, 
		cz.x, cz.y, cz.z, 0.0,
		0.0, 0.0, 0.0, 1.0);
	matrix = glm::transpose(matrix);
	//std::cout << matrix[1][1] << std::endl;
	return matrix;
}

