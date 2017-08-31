#ifndef VIEWER_H
#define VIEWER_H

#include <libfreenect2/config.h>
#include <libfreenect2/frame_listener.hpp>
#include <iostream>
#include <string>
#include <map>

#include "flextGL.h"
#include "virtual.h"
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


struct Vertex
{
    float x, y;
    float u, v;
};

class WithOpenGLBindings
{
private:
    OpenGLBindings *bindings;
protected:
    WithOpenGLBindings() : bindings(0) {}
    virtual ~WithOpenGLBindings() {}

    virtual void onOpenGLBindingsChanged(OpenGLBindings *b) { }
public:
    void gl(OpenGLBindings *bindings)
    {
        this->bindings = bindings;
        onOpenGLBindingsChanged(this->bindings);
    }

    OpenGLBindings *gl()
    {
        return bindings;
    }
};

template<size_t TBytesPerPixel, GLenum TInternalFormat, GLenum TFormat, GLenum TType>
struct ImageFormat
{
    static const size_t BytesPerPixel = TBytesPerPixel;
    static const GLenum InternalFormat = TInternalFormat;
    static const GLenum Format = TFormat;
    static const GLenum Type = TType;
};

typedef ImageFormat<1, GL_R8UI, GL_RED_INTEGER, GL_UNSIGNED_BYTE> U8C1;
typedef ImageFormat<2, GL_R16I, GL_RED_INTEGER, GL_SHORT> S16C1;
typedef ImageFormat<2, GL_R16UI, GL_RED_INTEGER, GL_UNSIGNED_SHORT> U16C1;
typedef ImageFormat<4, GL_R32F, GL_RED, GL_FLOAT> F32C1;
typedef ImageFormat<8, GL_RG32F, GL_RG, GL_FLOAT> F32C2;
typedef ImageFormat<12, GL_RGB32F, GL_RGB, GL_FLOAT> F32C3;
typedef ImageFormat<4, GL_RGBA, GL_BGRA, GL_UNSIGNED_BYTE> F8C4;
typedef ImageFormat<16, GL_RGBA32F, GL_RGBA, GL_FLOAT> F32C4;

template<typename FormatT>
struct Texture : public WithOpenGLBindings
{
protected:
    size_t bytes_per_pixel, height, width;

public:
    GLuint texture;
    unsigned char *data;
    size_t size;

    Texture() : bytes_per_pixel(FormatT::BytesPerPixel), height(0), width(0), texture(0), data(0), size(0)
    {
    }

    void bindToUnit(GLenum unit)
    {
        gl()->glActiveTexture(unit);
        glBindTexture(GL_TEXTURE_RECTANGLE, texture);
    }

    void allocate(size_t new_width, size_t new_height)
    {
        width = new_width;
        height = new_height;
        size = height * width * bytes_per_pixel;
        data = new unsigned char[size];

        glGenTextures(1, &texture);
        bindToUnit(GL_TEXTURE0);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_RECTANGLE, 0, FormatT::InternalFormat, width, height, 0, FormatT::Format, FormatT::Type, 0);
    }



    void deallocate()
    {
        glDeleteTextures(1, &texture);
        delete[] data;
    }

    void upload()
    {
        bindToUnit(GL_TEXTURE0);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_RECTANGLE, /*level*/0, /*xoffset*/0, /*yoffset*/0, width, height, FormatT::Format, FormatT::Type, data);
    }

    void download()
    {
        downloadToBuffer(data);
    }

    void downloadToBuffer(unsigned char *data)
    {
        glReadPixels(0, 0, width, height, FormatT::Format, FormatT::Type, data);
    }

    void flipY()
    {
        flipYBuffer(data);
    }

    void flipYBuffer(unsigned char *data)
    {
        typedef unsigned char type;

        size_t linestep = width * bytes_per_pixel / sizeof(type);

        type *first_line = reinterpret_cast<type *>(data), *last_line = reinterpret_cast<type *>(data) + (height - 1) * linestep;

        for (size_t y = 0; y < height / 2; ++y)
        {
            for (size_t x = 0; x < linestep; ++x, ++first_line, ++last_line)
            {
                std::swap(*first_line, *last_line);
            }
            last_line -= 2 * linestep;
        }
    }

    libfreenect2::Frame *downloadToNewFrame()
    {
        libfreenect2::Frame *f = new libfreenect2::Frame(width, height, bytes_per_pixel);
        downloadToBuffer(f->data);
        flipYBuffer(f->data);

        return f;
    }
};


//////////TEXTURE 2
template<typename FormatT>
struct Texture2 : public WithOpenGLBindings
{
protected:
	size_t bytes_per_pixel2, height2, width2;

public:
	GLuint texture2;
	unsigned char *data2;
	size_t size2;

	Texture2() : bytes_per_pixel2(FormatT::BytesPerPixel), height2(0), width2(0), texture2(0), data2(0), size2(0)
	{
	}

	void bindToUnit2(GLenum unit)
	{
		gl()->glActiveTexture(unit);
		glBindTexture(GL_TEXTURE_RECTANGLE, texture2);
	}

	void allocate2(size_t new_width, size_t new_height)
	{
		width2 = new_width;
		height2 = new_height;
		size2 = height2 * width2 * bytes_per_pixel2;
		data2 = new unsigned char[size2];

		glGenTextures(1, &texture2);
		bindToUnit2(GL_TEXTURE1);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_RECTANGLE, 0, FormatT::InternalFormat, width2, height2, 0, FormatT::Format, FormatT::Type, 0);
	}



	void deallocate2()
	{
		glDeleteTextures(1, &texture2);
		delete[] data2;
	}

	void upload2()
	{
		bindToUnit2(GL_TEXTURE1);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glTexSubImage2D(GL_TEXTURE_RECTANGLE, /*level*/0, /*xoffset*/0, /*yoffset*/0, width2, height2, FormatT::Format, FormatT::Type, data2);
	}

	void download2()
	{
		downloadToBuffer2(data2);
	}

	void downloadToBuffer2(unsigned char *data)
	{
		glReadPixels(0, 0, width2, height2, FormatT::Format, FormatT::Type, data);
	}

	void flipY2()
	{
		flipYBuffer2(data2);
	}

	void flipYBuffer2(unsigned char *data)
	{
		typedef unsigned char type;

		size_t linestep = width2 * bytes_per_pixel2 / sizeof(type);

		type *first_line = reinterpret_cast<type *>(data), *last_line = reinterpret_cast<type *>(data) + (height2 - 1) * linestep;

		for (size_t y = 0; y < height2 / 2; ++y)
		{
			for (size_t x = 0; x < linestep; ++x, ++first_line, ++last_line)
			{
				std::swap(*first_line, *last_line);
			}
			last_line -= 2 * linestep;
		}
	}

	libfreenect2::Frame *downloadToNewFrame()
	{
		libfreenect2::Frame *f = new libfreenect2::Frame(width2, height2, bytes_per_pixel2);
		downloadToBuffer2(f->data2);
		flipYBuffer2(f->data2);

		return f;
	}
};

/////////TEXTURE 2

struct ShaderProgram : public WithOpenGLBindings
{
    GLuint program, vertex_shader, fragment_shader;

    char error_buffer[2048];

    ShaderProgram() :
        program(0),
        vertex_shader(0),
        fragment_shader(0)
    {
    }

    void setVertexShader(const std::string& src)
    {
        const char* src_ = src.c_str();
        int length_ = src.length();
        vertex_shader = gl()->glCreateShader(GL_VERTEX_SHADER);
        gl()->glShaderSource(vertex_shader, 1, &src_, &length_);
    }

    void setFragmentShader(const std::string& src)
    {
        const char* src_ = src.c_str();
        int length_ = src.length();
        fragment_shader = gl()->glCreateShader(GL_FRAGMENT_SHADER);
        gl()->glShaderSource(fragment_shader, 1, &src_, &length_);
    }

    void build()
    {
        GLint status;

        gl()->glCompileShader(vertex_shader);
        gl()->glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &status);

        if (status != GL_TRUE)
        {
            gl()->glGetShaderInfoLog(vertex_shader, sizeof(error_buffer), NULL, error_buffer);

            std::cerr << "failed to compile vertex shader!" << std::endl << error_buffer << std::endl;
        }

        gl()->glCompileShader(fragment_shader);

        gl()->glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &status);
        if (status != GL_TRUE)
        {
            gl()->glGetShaderInfoLog(fragment_shader, sizeof(error_buffer), NULL, error_buffer);

            std::cerr << "failed to compile fragment shader!" << std::endl << error_buffer << std::endl;
        }

        program = gl()->glCreateProgram();
        gl()->glAttachShader(program, vertex_shader);
        gl()->glAttachShader(program, fragment_shader);

        gl()->glLinkProgram(program);

        gl()->glGetProgramiv(program, GL_LINK_STATUS, &status);

        if (status != GL_TRUE)
        {
            gl()->glGetProgramInfoLog(program, sizeof(error_buffer), NULL, error_buffer);
            std::cerr << "failed to link shader program!" << std::endl << error_buffer << std::endl;
        }
    }

    GLint getAttributeLocation(const std::string& name)
    {
        return gl()->glGetAttribLocation(program, name.c_str());
    }

    void setUniform(const std::string& name, GLint value)
    {
        GLint idx = gl()->glGetUniformLocation(program, name.c_str());
        if (idx == -1) return;

        gl()->glUniform1i(idx, value);
    }

    void setUniform(const std::string& name, GLfloat value)
    {
        GLint idx = gl()->glGetUniformLocation(program, name.c_str());
        if (idx == -1) return;

        gl()->glUniform1f(idx, value);
    }

    void setUniformVector3(const std::string& name, GLfloat value[3])
    {
        GLint idx = gl()->glGetUniformLocation(program, name.c_str());
        if (idx == -1) return;

        gl()->glUniform3fv(idx, 1, value);
    }

    void setUniformMatrix3(const std::string& name, GLfloat value[9])
    {
        GLint idx = gl()->glGetUniformLocation(program, name.c_str());
        if (idx == -1) return;

        gl()->glUniformMatrix3fv(idx, 1, false, value);
    }

    void use()
    {
        gl()->glUseProgram(program);
    }
};

class Viewer : WithOpenGLBindings {
private:
    bool shouldStop;
    GLFWwindow* window;
    GLuint triangle_vbo, triangle_vao, triangle_ebo;
    ShaderProgram renderShader;
    ShaderProgram renderGrayShader;
	ShaderProgram renderRedShader;
	ShaderProgram renderModel;
	ShaderProgram renderRegion;
	std::string shader_folder;
    std::map<std::string,libfreenect2::Frame*> frames;
    Texture<F8C4> rgb;
    Texture2<F32C1> ir;
    int win_width;
    int win_height;
	

	//////////////////
	std::vector<std::vector<glm::vec3>> vertices;	//changed to vector
	std::vector < std::vector<int>> indices;
	std::vector <std::vector<glm::vec3>> vnormals;
	std::vector<glm::vec3> minimum;
	std::vector<glm::vec3> maximum;
	std::vector<cv::Mat> regionMask;
	GLfloat deltaTime; 
	GLfloat lastFrame;
	std::vector <Virtual> cubes;
	//Virtual cube;
	/*
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
	*/
	int cv; //collision vertice
	int cr; //collision region
	float adj1, adj2;
	
	

public:
    Viewer();
	
    void initialize();
    virtual void onOpenGLBindingsChanged(OpenGLBindings *b);
    bool render();
    void addFrame(std::string id,libfreenect2::Frame* frame);
    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void winsize_callback(GLFWwindow* window, int w, int h);
    static void key_callbackstatic(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void winsize_callbackstatic(GLFWwindow* window, int w, int h);
	void addPointCloud(std::vector<std::vector<glm::vec3>> points);  //changed to vector
	void addNormals(std::vector<std::vector<glm::vec3>> points);
	void addMin(std::vector<glm::vec3> points);
	void addMax(std::vector<glm::vec3> points);
	void addIndices(std::vector<std::vector<int>> ind);
	void addMask(std::vector<cv::Mat> mask);
	glm::mat4 Viewer::normMatrix(glm::mat4 matrix);
	bool ready;
};

#endif
