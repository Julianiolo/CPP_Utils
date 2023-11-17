#ifndef __RAYLIB_TEXTURE_UTILS_H__
#define __RAYLIB_TEXTURE_UTILS_H__

#include "raylib.h"
#include "../DataUtils.h"

namespace RlTextureUtils {
    class RenderTextureWrapper {
    private:
        bool valid = false;
    public:
        RenderTexture tex;

        RenderTextureWrapper();
        RenderTextureWrapper(int width, int height);
        RenderTextureWrapper(const RenderTextureWrapper& other);
        RenderTextureWrapper(RenderTextureWrapper&& other);
        ~RenderTextureWrapper();

        RenderTextureWrapper& operator=(const RenderTextureWrapper& other);
        RenderTextureWrapper& operator=(RenderTextureWrapper&& other);

        void clear();
    };
};



#ifdef RAYLIB_TEXTURE_UTILS_IMPL

RlTextureUtils::RenderTextureWrapper::RenderTextureWrapper() : valid(false) {

}
RlTextureUtils::RenderTextureWrapper::RenderTextureWrapper(int width, int height) : valid(true), tex(LoadRenderTexture(width, height)) {

}
RlTextureUtils::RenderTextureWrapper::RenderTextureWrapper(const RenderTextureWrapper& other) {
    *this = other;
}
RlTextureUtils::RenderTextureWrapper::RenderTextureWrapper(RenderTextureWrapper&& other) {
    *this = other;
}
RlTextureUtils::RenderTextureWrapper::~RenderTextureWrapper() {
    clear();
}

RlTextureUtils::RenderTextureWrapper& RlTextureUtils::RenderTextureWrapper::operator=(const RenderTextureWrapper& other) {
    clear();

    if (other.valid) {
        tex = LoadRenderTexture(other.tex.texture.width, other.tex.texture.height);
        valid = true;
    }
    return *this;
}

RlTextureUtils::RenderTextureWrapper& RlTextureUtils::RenderTextureWrapper::operator=(RenderTextureWrapper&& other) {
    clear();

    if (other.valid) {
        tex = other.tex;
        valid = true;
        other.valid = false;
    }
    return *this;
}

void RlTextureUtils::RenderTextureWrapper::clear() {
    if (valid)
        UnloadRenderTexture(tex);
    valid = false;
}

#endif

#endif