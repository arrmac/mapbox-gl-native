#ifndef MBGL_GEOMETRY_TEXTURE_ATLAS
#define MBGL_GEOMETRY_TEXTURE_ATLAS

#include <mbgl/geometry/binpack.hpp>

#include <mbgl/util/rect.hpp>
#include <mbgl/util/noncopyable.hpp>

#include <mutex>
#include <memory>
#include <map>
#include <set>

namespace mbgl {

template <typename Atlas, typename Pixel, typename Key, typename Texture>
class TextureAtlas : private util::noncopyable {
public:
    typedef uint16_t Dim;

    TextureAtlas(Dim width, Dim height);
    ~TextureAtlas();

private:
    struct Holder : private util::noncopyable {
        Holder(const std::shared_ptr<const Texture>&, const std::set<uintptr_t>&, const Rect<Dim>&);
        Holder(Holder&&);
        std::shared_ptr<const Texture> texture;
        std::set<uintptr_t> references;
        const Rect<Dim> pos;
    };

public:
    // Returns the position of the texture, and a token that automatically removes the texture from
    // the atlas when it goes out of scope.
    Rect<Dim> get(uintptr_t uid, const Key& key);

    // Remove all textures associated with the given uid, unless they're used elsewhere.
    void abandon(uintptr_t uid);

private:
    std::mutex mtx;

    // Texture data
    const std::unique_ptr<Pixel[]> data;
    BinPack<Dim> bin;
    std::map<Key, Holder> textures;
    bool dirty = true;
};

} // namespace mbgl

#endif
