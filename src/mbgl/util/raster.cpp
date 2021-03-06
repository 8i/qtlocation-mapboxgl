#include <mbgl/platform/platform.hpp>
#include <mbgl/gl/gl.hpp>
#include <mbgl/gl/context.hpp>
#include <mbgl/platform/log.hpp>

#include <mbgl/util/raster.hpp>

#include <cassert>
#include <cstring>

namespace mbgl {

bool Raster::isLoaded() const {
    return loaded;
}

gl::TextureID Raster::getID() const {
    return texture ? *texture : 0;
}

std::array<uint16_t, 2> Raster::getSize() const {
    return size;
}

void Raster::load(PremultipliedImage image, uint32_t mipmapLevel) {
    if (images.size() <= mipmapLevel) {
        images.resize(mipmapLevel + 1);
    }
    images.at(mipmapLevel) = std::move(image);

    loaded = true;
}

void Raster::bind(gl::Context& context,
                  uint32_t unit,
                  Scaling newFilter,
                  MipMap newMipMap) {
    bool filterNeedsUpdate = false;

    if (!texture) {
        if (images.empty()) {
            Log::Error(Event::OpenGL, "trying to bind texture without images");
            return;
        } else {
            upload(context, unit);
            filterNeedsUpdate = true;
        }
    } else {
        if (context.texture[unit] != *texture) {
            context.activeTexture = unit;
            context.texture[unit] = *texture;
        }
        filterNeedsUpdate = (filter != newFilter || mipmap != newMipMap);
    }

    if (filterNeedsUpdate) {
        filter = newFilter;
        mipmap = newMipMap;
        updateFilter();
    }
}

void Raster::updateFilter() {
    MBGL_CHECK_ERROR(glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
        filter == Scaling::Linear
            ? (mipmap == MipMap::Yes ? GL_LINEAR_MIPMAP_NEAREST : GL_LINEAR)
            : (mipmap == MipMap::Yes ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST)));
    MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                                     filter == Scaling::Linear ? GL_LINEAR : GL_NEAREST));
}

void Raster::upload(gl::Context& context, uint32_t unit) {
    if (!images.empty() && !texture) {
        texture = context.createTexture();
        context.activeTexture = unit;
        context.texture[unit] = *texture;
        updateFilter();
#if not MBGL_USE_GLES2
        MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(images.size())));
#endif // MBGL_USE_GLES2
        MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        MBGL_CHECK_ERROR(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GLint level = 0;
        for (auto& img : images) {
            MBGL_CHECK_ERROR(glTexImage2D(
                GL_TEXTURE_2D, level++, GL_RGBA, static_cast<GLsizei>(img.width),
                static_cast<GLsizei>(img.height), 0, GL_RGBA, GL_UNSIGNED_BYTE, img.data.get()));
        }
        size = { { images.front().width, images.front().height } };
        images.clear();
        images.shrink_to_fit();
    }
}

} // namespace mbgl
