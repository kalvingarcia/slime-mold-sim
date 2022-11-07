#pragma once
// defining some error constants inorder to find where the code is exiting upon error
#define VERTEX_SHADER_FAIL -4
#define FRAGMENT_SHADER_FAIL -5
#define VERTEX_FRAGMENT_LINK_FAIL -6
#define COMPUTE_SHADER_FAIL -7
#define COMPUTE_LINK_FAIL -8
#define FILE_READ_FAIL -9

#include <stdio.h>

#include <string>
#include <fstream>
#include <sstream>
#include <vector>

#define GLEW_STATIC
#include <glew.h>
#include <glfw3.h>

/*
	DisplayShader  struct (default public class)

	description:
		this class houses the vertex and fragment shader compilation and linking
		it also allows us to use the shader

	member variables:
		program_id
*/
struct DisplayShader {
	GLuint program_id;

	/*
			DisplayShader contructor

			description:
				opens the shader files and compiles them
				links them into a program and sets the program_id
	*/
	DisplayShader() {
		std::string vshader_code, fshader_code;
		std::ifstream vshader_fin("./vertex.glsl"), fshader_fin("./fragment.glsl");
		if (!vshader_fin || !fshader_fin) {
			fprintf(stderr, "Could not open vertex shader or fragment shader.\n");
			exit(FILE_READ_FAIL);
		}

		// reading vertex shader
		std::stringstream sout;
		sout << vshader_fin.rdbuf();
		vshader_code = sout.str();
		vshader_fin.close();
		// reading fragment shader
		sout.str(std::string());
		sout << fshader_fin.rdbuf();
		fshader_code = sout.str();
		fshader_fin.close();

		GLint result = GL_FALSE;
		int info_length;

		// Compile Vertex Shader
		char const* vshader_source = vshader_code.c_str();
		GLuint vshader_id = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vshader_id, 1, &vshader_source, NULL);
		glCompileShader(vshader_id);
		// Check Vertex Shader
		glGetShaderiv(vshader_id, GL_COMPILE_STATUS, &result);
		glGetShaderiv(vshader_id, GL_INFO_LOG_LENGTH, &info_length);
		if (result != GL_TRUE) {
			std::vector<char> v_error_message(info_length + 1);
			glGetShaderInfoLog(vshader_id, info_length, NULL, &v_error_message[0]);
			printf("%s\n", &v_error_message[0]);
			exit(VERTEX_SHADER_FAIL);
		}

		// Compile Fragment Shader
		char const* fshader_source = fshader_code.c_str();
		GLuint fshader_id = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fshader_id, 1, &fshader_source, NULL);
		glCompileShader(fshader_id);
		// Check Fragment Shader
		glGetShaderiv(fshader_id, GL_COMPILE_STATUS, &result);
		glGetShaderiv(fshader_id, GL_INFO_LOG_LENGTH, &info_length);
		if (result != GL_TRUE) {
			std::vector<char> f_error_message(info_length + 1);
			glGetShaderInfoLog(fshader_id, info_length, NULL, &f_error_message[0]);
			printf("%s\n", &f_error_message[0]);
			exit(FRAGMENT_SHADER_FAIL);
		}

		// Link the program
		program_id = glCreateProgram();
		glAttachShader(program_id, vshader_id);
		glAttachShader(program_id, fshader_id);
		glLinkProgram(program_id);

		// Check the program
		glGetProgramiv(program_id, GL_LINK_STATUS, &result);
		glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_length);
		if (result != GL_TRUE) {
			std::vector<char> p_error_message(info_length + 1);
			glGetProgramInfoLog(program_id, info_length, NULL, &p_error_message[0]);
			printf("%s\n", &p_error_message[0]);
			exit(VERTEX_FRAGMENT_LINK_FAIL);
		}

		glDetachShader(program_id, vshader_id);
		glDetachShader(program_id, fshader_id);

		glDeleteShader(vshader_id);
		glDeleteShader(fshader_id);
	}

	/*
		use funciton

		description:
			sets the active gl program to the program_id
	*/
	void use() {
		glUseProgram(program_id);
	}

	// util functions
	void set_bool(const std::string &name, bool value) const {
		glUniform1i(glGetUniformLocation(program_id, name.c_str()), (int)value);
	}
	void set_int(const std::string& name, int value) const {
		glUniform1i(glGetUniformLocation(program_id, name.c_str()), value);
	}
	void set_float(const std::string& name, float value) const {
		glUniform1f(glGetUniformLocation(program_id, name.c_str()), value);
	}
	void set_vec4(const std::string& name, float value1, float value2, float value3, float value4) const {
		glUniform4f(glGetUniformLocation(program_id, name.c_str()), value1, value2, value3, value4);
	}
};

/*
	ComputeShader struct (default public class)

	description:
		this class houses the compute shader compilation and linking
		it also contains the use and dispatch for the shader

	member variables:
		program_id
*/
struct ComputeShader {
	GLuint program_id;

	/*
			ComputeShader contructor

			description:
				opens the shader file and compiles it
				links it into a program and sets the program_id
	*/
	ComputeShader() {
		std::string compute_code;
		std::ifstream compute_fin("./slime_mold.glsl");
		if (!compute_fin) {
			fprintf(stderr, "Could not open compute shader.\n");
			exit(FILE_READ_FAIL);
		}

		// reading compute shader
		std::stringstream sout;
		sout << compute_fin.rdbuf();
		compute_code = sout.str();
		compute_fin.close();

		GLuint compute_id;
		GLint result;
		int info_length;

		const char* compute_source = compute_code.c_str();

		// compile compute shader
		compute_id = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(compute_id, 1, &compute_source, NULL);
		glCompileShader(compute_id);
		// print compile errors
		glGetShaderiv(compute_id, GL_COMPILE_STATUS, &result);
		glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_length);
		if (result != GL_TRUE) {
			std::vector<char> c_error_message(info_length + 1);
			glGetShaderInfoLog(compute_id, info_length, NULL, &c_error_message[0]);
			printf("%s", &c_error_message[0]);
			exit(COMPUTE_SHADER_FAIL);
		}

		// compute shader Program
		program_id = glCreateProgram();
		glAttachShader(program_id, compute_id);
		glLinkProgram(program_id);
		// print linking errors if any
		glGetProgramiv(program_id, GL_LINK_STATUS, &result);
		glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &info_length);
		if (result != GL_TRUE) {
			std::vector<char> c_error_message(info_length + 1);
			glGetProgramInfoLog(program_id, info_length, NULL, &c_error_message[0]);
			printf("%s", &c_error_message[0]);
			exit(COMPUTE_SHADER_FAIL);
		}

		glDeleteShader(compute_id);
	}

	/*
		use funciton

		description:
			sets the active gl program to the program_id
	*/
	void use() {
		glUseProgram(program_id);
	}

	/*
		dispatch function

		description:
			dispatches the little computers in the gpu to do the work!
	*/
	void dispatch(int width, int height) {
		glDispatchCompute(width, height, 1);
	}

	// util functions
	void set_bool(const std::string& name, bool value) const {
		glUniform1i(glGetUniformLocation(program_id, name.c_str()), (int)value);
	}
	void set_int(const std::string& name, int value) const {
		glUniform1i(glGetUniformLocation(program_id, name.c_str()), value);
	}
	void set_float(const std::string& name, float value) const {
		glUniform1f(glGetUniformLocation(program_id, name.c_str()), value);
	}
	void set_vec4(const std::string& name, float value1, float value2, float value3, float value4) const {
		glUniform4f(glGetUniformLocation(program_id, name.c_str()), value1, value2, value3, value4);
	}
};