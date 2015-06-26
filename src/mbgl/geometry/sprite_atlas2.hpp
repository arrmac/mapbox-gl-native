#ifndef MBGL_GEOMETRY_SPRITE_ATLAS2
#define MBGL_GEOMETRY_SPRITE_ATLAS2

#include "texture_atlas.hpp"
#include <mbgl/annotation/sprite_store.hpp>

#include <mutex>

namespace mbgl {



class SpriteAtlas2 : public TextureAtlas<SpriteAtlas2, uint32_t, std::string, SpriteImage> {
    friend class TextureAtlas<SpriteAtlas2, uint32_t, std::string, SpriteImage>;

public:
    SpriteAtlas2(uint16_t width, uint16_t height, SpriteStore& store_);

private:
    std::shared_ptr<const SpriteImage> getTexture(const std::string& name) {
        return store.getSprite(name);
    }

private:
    SpriteStore& store;
};

} // namespace mbgl

#endif