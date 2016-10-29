#pragma once

#include <mbgl/gl/types.hpp>
#include <mbgl/util/optional.hpp>

#include <array>
#include <functional>
#include <tuple>

namespace mbgl {
namespace gl {

template <class T>
void bindUniform(UniformLocation, const T&);

template <class Tag, class T>
class UniformValue {
public:
    explicit UniformValue(T t_) : t(std::move(t_)) {}
    T t;
};

template <class Tag, class T>
class Uniform {
public:
    using Value = UniformValue<Tag, T>;

    class State {
    public:
        void operator=(const Value& value) {
            if (!current || *current != value.t) {
                current = value.t;
                bindUniform(location, value.t);
            }
        }

        UniformLocation location;
        optional<T> current = {};
    };
};

template <class Tag, class T>
using UniformScalar = Uniform<Tag, T>;

template <class Tag, class T, size_t N>
using UniformVector = Uniform<Tag, std::array<T, N>>;

template <class Tag, class T, size_t N>
using UniformMatrix = Uniform<Tag, std::array<T, N*N>>;

#define MBGL_DEFINE_UNIFORM_SCALAR(type_, name_) \
    struct name_ : ::mbgl::gl::UniformScalar<name_, type_> { static constexpr auto name = #name_; }

#define MBGL_DEFINE_UNIFORM_VECTOR(type_, n_, name_) \
    struct name_ : ::mbgl::gl::UniformVector<name_, type_, n_> { static constexpr auto name = #name_; }

#define MBGL_DEFINE_UNIFORM_MATRIX(type_, n_, name_) \
    struct name_ : ::mbgl::gl::UniformMatrix<name_, type_, n_> { static constexpr auto name = #name_; }

UniformLocation uniformLocation(ProgramID, const char * name);

template <class... Us>
class Uniforms {
public:
    using State = std::tuple<typename Us::State...>;
    using Values = std::tuple<typename Us::Value...>;

    static State state(const ProgramID& id) {
        return State { { uniformLocation(id, Us::name) }... };
    }

    static std::function<void ()> binder(State& state, Values&& values_) {
        return [&state, values = std::move(values_)] () mutable {
            noop((std::get<typename Us::State>(state) = std::get<typename Us::Value>(values), 0)...);
        };
    }

private:
    // This exists only to provide a varags context for unpacking the assignments in `binder`.
    template <int...> static void noop(int...) {}
};

} // namespace gl
} // namespace mbgl
