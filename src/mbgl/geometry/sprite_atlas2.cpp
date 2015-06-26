#include "sprite_atlas2.hpp"
#include "texture_atlas_impl.hpp"

namespace mbgl {

SpriteAtlas2::SpriteAtlas2(uint16_t width, uint16_t height, SpriteStore& store_)
    : TextureAtlas(width, height), store(store_) {
}

// Explicit template instantiation
template TextureAtlas<SpriteAtlas2, uint32_t, std::string, SpriteImage>::~TextureAtlas();
template Rect<TextureAtlas<SpriteAtlas2, uint32_t, std::string, SpriteImage>::Dim>
TextureAtlas<SpriteAtlas2, uint32_t, std::string, SpriteImage>::get(uintptr_t, const std::string&);
template void TextureAtlas<SpriteAtlas2, uint32_t, std::string, SpriteImage>::abandon(uintptr_t);
template TextureAtlas<SpriteAtlas2, uint32_t, std::string, SpriteImage>::Holder::Holder(
    const std::shared_ptr<const SpriteImage>&, const std::set<uintptr_t>&, const Rect<Dim>&);
template TextureAtlas<SpriteAtlas2, uint32_t, std::string, SpriteImage>::Holder::Holder(Holder&&);

} // namespace mbgl
