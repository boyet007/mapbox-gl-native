#pragma once

#include <mbgl/gl/types.hpp>
#include <mbgl/gl/object.hpp>
#include <mbgl/util/noncopyable.hpp>
#include <mbgl/gl/shader_parameters.hpp>

namespace mbgl {
namespace gl {

class Context;

class Shader : private util::noncopyable {
public:
    ~Shader();
    const char* name;

    ProgramID getID() const {
        return program.get();
    }

    AttributeLocation getAttributeLocation(const char* uniform) const;
    UniformLocation getUniformLocation(const char* uniform) const;

protected:
    
    Shader(const char* name_,
           const char* vertex,
           const char* fragment,
           Context&,
           gl::ShaderParameters& );

private:
    bool compileShader(UniqueShader&, const char *source);

    UniqueProgram program;
    UniqueShader vertexShader;
    UniqueShader fragmentShader;
};

} // namespace gl
} // namespace mbgl
