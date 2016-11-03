#include <mbgl/gl/shader.hpp>
#include <mbgl/gl/gl.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/util/stopwatch.hpp>
#include <mbgl/util/exception.hpp>
#include <mbgl/platform/log.hpp>
#include <mbgl/platform/platform.hpp>

#include <cstring>
#include <cassert>
#include <iostream>
#include <string>
#include <fstream>
#include <cstdio>
#include <cassert>

namespace mbgl {
namespace gl {

Shader::Shader(const char* name_,
               const char* vertexSource,
               const char* fragmentSource,
               Context& context,
               gl::ShaderParameters& shaderParameters)
    : name(name_),
      program(context.createProgram()),
      vertexShader(context.createVertexShader()),
      fragmentShader(context.createFragmentShader()) {
    util::stopwatch stopwatch("shader compilation", Event::Shader);

    std::string vertex(vertexSource);
    vertex.replace(vertex.find_first_of('\n'), 1,"\n#define DEVICE_PIXEL_RATIO " + std::to_string(shaderParameters.pixelRatio) + "\n");
    if (!compileShader(vertexShader, vertex.c_str())) {
        Log::Error(Event::Shader, "Vertex shader %s failed to compile: %s", name, vertexSource);
        throw util::ShaderException(std::string { "Vertex shader " } + name + " failed to compile");
    }


    std::string fragment(fragmentSource);
    fragment.replace(fragment.find_first_of('\n'), 1,"\n#define DEVICE_PIXEL_RATIO " + std::to_string(shaderParameters.pixelRatio) + "\n");
    if ((strcmp(name, "collision_box") != 0) & shaderParameters.overdraw) {
        assert(fragment.find("#ifdef OVERDRAW_INSPECTOR") != std::string::npos);
        fragment.replace(fragment.find_first_of('\n'), 1, "\n#define OVERDRAW_INSPECTOR\n");
    }

    if (!compileShader(fragmentShader, fragment.c_str())) {
        Log::Error(Event::Shader, "Fragment shader %s failed to compile: %s", name, fragmentSource);
        throw util::ShaderException(std::string { "Fragment shader " } + name + " failed to compile");
    }

    // Attach shaders
    MBGL_CHECK_ERROR(glAttachShader(program.get(), vertexShader.get()));
    MBGL_CHECK_ERROR(glAttachShader(program.get(), fragmentShader.get()));

    // Link program
    GLint status;
     MBGL_CHECK_ERROR(glLinkProgram(program.get()));

    MBGL_CHECK_ERROR(glGetProgramiv(program.get(), GL_LINK_STATUS, &status));
    if (status == 0) {
        GLint logLength;
        MBGL_CHECK_ERROR(glGetProgramiv(program.get(), GL_INFO_LOG_LENGTH, &logLength));
        const auto log = std::make_unique<GLchar[]>(logLength);
        if (logLength > 0) {
            MBGL_CHECK_ERROR(glGetProgramInfoLog(program.get(), logLength, &logLength, log.get()));
            Log::Error(Event::Shader, "Program failed to link: %s", log.get());
        }
        throw util::ShaderException(std::string { "Program " } + name + " failed to link: " + log.get());
    }
}

bool Shader::compileShader(UniqueShader& shader, const GLchar *source) {
    GLint status = 0;

    const GLsizei lengths = static_cast<GLsizei>(std::strlen(source));
    MBGL_CHECK_ERROR(glShaderSource(shader.get(), 1, &source, &lengths));

    MBGL_CHECK_ERROR(glCompileShader(shader.get()));

    MBGL_CHECK_ERROR(glGetShaderiv(shader.get(), GL_COMPILE_STATUS, &status));
    if (status == 0) {
        GLint logLength;
        MBGL_CHECK_ERROR(glGetShaderiv(shader.get(), GL_INFO_LOG_LENGTH, &logLength));
        if (logLength > 0) {
            const auto log = std::make_unique<GLchar[]>(logLength);
            MBGL_CHECK_ERROR(glGetShaderInfoLog(shader.get(), logLength, &logLength, log.get()));
            Log::Error(Event::Shader, "Shader failed to compile: %s", log.get());
        }
        return false;
    }

    MBGL_CHECK_ERROR(glGetShaderiv(shader.get(), GL_COMPILE_STATUS, &status));
    if (status == GL_FALSE) {
        Log::Error(Event::Shader, "Shader %s failed to compile.", name);
        return false;
    }

    return true;
}

Shader::~Shader() {
    if (program.get()) {
        MBGL_CHECK_ERROR(glDetachShader(program.get(), vertexShader.get()));
        MBGL_CHECK_ERROR(glDetachShader(program.get(), fragmentShader.get()));
    }
}

UniformLocation Shader::getUniformLocation(const char* uniform) const {
    return MBGL_CHECK_ERROR(glGetUniformLocation(program.get(), uniform));
}

AttributeLocation Shader::getAttributeLocation(const char* attribute) const {
    return MBGL_CHECK_ERROR(glGetAttribLocation(program.get(), attribute));
}

} // namespace gl
} // namespace mbgl
