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
    };
};



#ifdef RAYLIB_TEXTURE_UTILS_IMPL

RlTextureUtils::RenderTextureWrapper::RenderTextureWrapper() : valid(false) {

}
RlTextureUtils::RenderTextureWrapper::RenderTextureWrapper(int width, int height) : valid(true), tex(LoadRenderTexture(width, height)) {

}
RlTextureUtils::RenderTextureWrapper::RenderTextureWrapper(const RenderTextureWrapper& other) {
    DU_ASSERT(other.valid);

    if (valid)
        UnloadRenderTexture(tex);

    tex = LoadRenderTexture(other.tex.texture.width, other.tex.texture.height);
    valid = true;

    // todo copy data maybe?
}
RlTextureUtils::RenderTextureWrapper::RenderTextureWrapper(RenderTextureWrapper&& other) {
    DU_ASSERT(other.valid);
    tex = other.tex;
    valid = true;
    other.valid = false;
}
RlTextureUtils::RenderTextureWrapper::~RenderTextureWrapper() {
    if (valid)
        UnloadRenderTexture(tex);
}

#endif

#endif