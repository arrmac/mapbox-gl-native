#ifndef MBGL_GEOMETRY_TEXTURE_ATLAS_IMPL
#define MBGL_GEOMETRY_TEXTURE_ATLAS_IMPL

#include "texture_atlas.hpp"

#include <mbgl/platform/log.hpp>

#include <mbgl/util/assert.hpp>

namespace mbgl {

template <typename Atlas, typename Pixel, typename Key, typename Texture>
TextureAtlas<Atlas, Pixel, Key, Texture>::TextureAtlas(Dim width, Dim height)
    : data(std::make_unique<Pixel[]>(width * height)), bin(width, height) {
}

template <typename Atlas, typename Pixel, typename Key, typename Texture>
TextureAtlas<Atlas, Pixel, Key, Texture>::~TextureAtlas() {
    assert_always(textures.empty());
}

template <typename Atlas, typename Pixel, typename Key, typename Texture>
TextureAtlas<Atlas, Pixel, Key, Texture>::Holder::Holder(
    const std::shared_ptr<const Texture>& texture_,
    const std::set<uintptr_t>& references_,
    const Rect<Dim>& pos_)
    : texture(texture_), references(references_), pos(pos_) {
}

template <typename Atlas, typename Pixel, typename Key, typename Texture>
TextureAtlas<Atlas, Pixel, Key, Texture>::Holder::Holder(Holder&& h)
    : texture(std::move(h.texture)), references(std::move(h.references)), pos(h.pos) {
}

namespace {

template <typename Dim>
Rect<Dim> allocateImage(BinPack<Dim>& bin, const Dim pixelWidth, const Dim pixelHeight) {
    // Increase to next number divisible by 4, but at least 1.
    // This is so we can scale down the texture coordinates and pack them
    // into 2 bytes rather than 4 bytes.
    const Dim packWidth = (pixelWidth + 1) + (4 - (pixelWidth + 1) % 4);
    const Dim packHeight = (pixelHeight + 1) + (4 - (pixelHeight + 1) % 4);

    // We have to allocate a new area in the bin, and store an empty image in it.
    // Add a 1px border around every image.
    Rect<Dim> rect = bin.allocate(packWidth, packHeight);
    if (rect.w == 0) {
        return rect;
    }

    rect.originalW = pixelWidth;
    rect.originalH = pixelHeight;

    return rect;
}

} // anonymous namespace

// public:

template <typename Atlas, typename Pixel, typename Key, typename Texture>
Rect<typename TextureAtlas<Atlas, Pixel, Key, Texture>::Dim>
TextureAtlas<Atlas, Pixel, Key, Texture>::get(uintptr_t uid, const Key& key) {
    std::lock_guard<std::mutex> lock(mtx);

    auto it = textures.find(key);
    if (it != textures.end()) {
        // We already have the texture in our sprite atlas. We're just going to update the reference
        // count and return the position.
        it->second.references.insert(uid);
        return it->second.pos;
    }

    // We don't have this object yet, so try to obtain it from the store.
    std::shared_ptr<const Texture> texture = static_cast<Atlas&>(*this).getTexture(key);
    if (!texture) {
        Log::Warning(Event::Sprite, "Texture store doesn't have texture with key '%s'",
                     std::string(key).c_str());
        return {};
    }

    Rect<Dim> pos = allocateImage(bin, texture->pixelWidth, texture->pixelHeight);
    if (pos.w == 0) {
        Log::Warning(Event::Sprite, "Atlas bitmap doesn't have enough space");
        return {};
    }

    // TODO: copy the texture data to the correct location

    textures.emplace(key, Holder{ texture, { uid }, pos });
    dirty = true;

    return pos;

}

template <typename Atlas, typename Pixel, typename Key, typename Texture>
void TextureAtlas<Atlas, Pixel, Key, Texture>::abandon(uintptr_t uid) {
    std::lock_guard<std::mutex> lock(mtx);

    const auto end = textures.end();
    for (auto it = textures.begin(); it != end;) {
        if (it->second.references.erase(uid) > 0 && it->second.references.empty()) {
            bin.release(it->second.pos);
            textures.erase(it++);
            dirty = true;
        } else {
            ++it;
        }
    }
}

} // namespace mbgl

#endif
