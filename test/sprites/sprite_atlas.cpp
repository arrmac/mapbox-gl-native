#include "../fixtures/util.hpp"
#include "../fixtures/fixture_log_observer.hpp"

#include <mbgl/util/exception.hpp>

#include <mbgl/geometry/sprite_atlas2.hpp>

using namespace mbgl;

TEST(Sprites, Atlas) {
    SpriteStore store(2);
    SpriteAtlas2 atlas(512, 512, store);

    store.setSprite("test", std::make_shared<SpriteImage>(8, 8, 2, std::string(16 * 16 * 4, '\0')));

    const auto pos = atlas.get(0, "test");
    EXPECT_EQ(0, pos.x);
    EXPECT_EQ(0, pos.y);
    EXPECT_EQ(16, pos.originalW);
    EXPECT_EQ(16, pos.originalH);
    EXPECT_EQ(20, pos.w);
    EXPECT_EQ(20, pos.h);

    // Get the same sprite again => expect same position
    const auto pos2 = atlas.get(0, "test");
    EXPECT_EQ(0, pos2.x);
    EXPECT_EQ(0, pos2.y);
    EXPECT_EQ(16, pos2.originalW);
    EXPECT_EQ(16, pos2.originalH);
    EXPECT_EQ(20, pos2.w);
    EXPECT_EQ(20, pos2.h);

    store.setSprite("test2", std::make_shared<SpriteImage>(12, 12, 2, std::string(24 * 24 * 4, '\0')));

    // New sprite should be added next to the existing sprite
    const auto pos3 = atlas.get(1, "test2");
    EXPECT_EQ(20, pos3.x);
    EXPECT_EQ(0, pos3.y);
    EXPECT_EQ(24, pos3.originalW);
    EXPECT_EQ(24, pos3.originalH);
    EXPECT_EQ(28, pos3.w);
    EXPECT_EQ(28, pos3.h);

    atlas.abandon(0);

    // Reuse the position
    const auto pos4 = atlas.get(1, "test");
    EXPECT_EQ(0, pos4.x);
    EXPECT_EQ(0, pos4.y);
    EXPECT_EQ(16, pos4.originalW);
    EXPECT_EQ(16, pos4.originalH);
    EXPECT_EQ(20, pos4.w);
    EXPECT_EQ(20, pos4.h);

    // Use the same sprite, but in a different Tile ID/pointer.
    const auto pos5 = atlas.get(2, "test");
    EXPECT_EQ(0, pos5.x);
    EXPECT_EQ(0, pos5.y);
    EXPECT_EQ(16, pos5.originalW);
    EXPECT_EQ(16, pos5.originalH);
    EXPECT_EQ(20, pos5.w);
    EXPECT_EQ(20, pos5.h);

    atlas.abandon(1);
    atlas.abandon(2);
}


TEST(Sprites, AtlasOverflow) {
    SpriteStore store(2);
    SpriteAtlas2 atlas(20, 20, store);

    store.setSprite("test", std::make_shared<SpriteImage>(8, 8, 2, std::string(16 * 16 * 4, '\0')));

    const auto pos = atlas.get(0, "test");
    EXPECT_EQ(0, pos.x);
    EXPECT_EQ(0, pos.y);
    EXPECT_EQ(16, pos.originalW);
    EXPECT_EQ(16, pos.originalH);
    EXPECT_EQ(20, pos.w);
    EXPECT_EQ(20, pos.h);

    FixtureLog log;

    store.setSprite("test2", std::make_shared<SpriteImage>(8, 8, 2, std::string(16 * 16 * 4, '\0')));
    const auto pos2 = atlas.get(1, "test2");
    EXPECT_EQ(0, pos2.x);
    EXPECT_EQ(0, pos2.y);
    EXPECT_EQ(0, pos2.w);
    EXPECT_EQ(0, pos2.h);

    EXPECT_EQ(1u, log.count({
                      EventSeverity::Warning,
                      Event::Sprite,
                      int64_t(-1),
                      "Atlas bitmap doesn't have enough space",
                  }));

    atlas.abandon(0);

    store.setSprite("test2", std::make_shared<SpriteImage>(8, 8, 2, std::string(16 * 16 * 4, '\0')));
    const auto pos3 = atlas.get(1, "test2");
    EXPECT_EQ(0, pos3.x);
    EXPECT_EQ(0, pos3.y);
    EXPECT_EQ(16, pos3.originalW);
    EXPECT_EQ(16, pos3.originalH);
    EXPECT_EQ(20, pos3.w);
    EXPECT_EQ(20, pos3.h);

    atlas.abandon(1);
}


TEST(Sprites, AtlasNonEmptyDestruction) {
    SpriteStore store(2);
    store.setSprite("test", std::make_shared<SpriteImage>(8, 8, 2, std::string(16 * 16 * 4, '\0')));

    auto atlas = std::make_unique<SpriteAtlas2>(512, 512, store);
    atlas->get(0, "test");
    EXPECT_DEATH(atlas.reset(), "failed assertion `textures.empty\\(\\)'");

    atlas->abandon(0);
}
