#pragma once

#include <mbgl/util/color.hpp>
#include <mbgl/util/range.hpp>
#include <mbgl/style/position.hpp>

#include <array>
#include <vector>
#include <string>
#include <type_traits>
#include <utility>

namespace mbgl {
namespace util {

float interpolationFactor(float base, Range<float> range, float z);

template <class T, class Enabled = void>
struct Interpolator;

template <typename T>
T interpolate(const T& a, const T& b, const double t) {
    return Interpolator<T>()(a, b, t);
}


template <class T, class Enabled>
struct Interpolator {
    T operator()(const T& a, const T& b, const double t) const {
        return a * (1.0 - t) + b * t;
    }
};

template <class T, std::size_t N>
struct Interpolator<std::array<T, N>> {
private:
    using Array = std::array<T, N>;

    template <std::size_t... I>
    Array operator()(const Array& a, const Array& b, const double t, std::index_sequence<I...>) {
        return {{ interpolate(a[I], b[I], t)... }};
    }

public:
    Array operator()(const Array& a, const Array& b, const double t) {
        return operator()(a, b, t, std::make_index_sequence<N>());
    }
};

// Only safe if vectors are guaranteed at runtime to be the same length.
template<>
struct Interpolator<std::vector<float>> {
    std::vector<float> operator()(const std::vector<float>& a,
                                  const std::vector<float>& b,
                                  const double t) const {
        assert(a.size() == b.size());
        if (a.size() == 0) return {};
        std::vector<float> result;
        for (std::size_t i = 0; i < a.size(); i++) {
            result.push_back(interpolate(a[i], b[i], t));
        }
        return result;
    }
};

template <>
struct Interpolator<style::Position> {
public:
    style::Position operator()(const style::Position& a, const style::Position& b, const double t) {
        auto pos = style::Position();
        auto interpolated = interpolate(a.getCartesian(), b.getCartesian(), t);
        pos.setCartesian(interpolated);
        return { pos };
    }
};

template <>
struct Interpolator<Color> {
public:
    Color operator()(const Color& a, const Color& b, const double t) {
        return {
            interpolate(a.r, b.r, t),
            interpolate(a.g, b.g, t),
            interpolate(a.b, b.b, t),
            interpolate(a.a, b.a, t)
        };
    }
};

struct Uninterpolated {
    template <class T>
    T operator()(const T& a, const T&, const double) const {
        return a;
    }
};

template <>
struct Interpolator<bool>
    : Uninterpolated {};

template <class T>
struct Interpolator<T, typename std::enable_if_t<std::is_enum<T>::value>>
    : Uninterpolated {};

template <>
struct Interpolator<std::string>
    : Uninterpolated {};

template <class T>
struct Interpolator<std::vector<T>>
    : Uninterpolated {};

template <class T>
struct Interpolatable
    : std::conditional_t<
      !std::is_base_of<Uninterpolated, Interpolator<T>>::value,
      std::true_type,
      std::false_type> {};



} // namespace util
} // namespace mbgl
