#include <gtest/gtest.h>

#include <filesystem>

#define private public
#define protected public
#include "framework/graphics/graphics.h"
#undef private
#undef protected

#include "framework/core/logger.h"
#include "framework/core/resourcemanager.h"
#include "framework/graphics/bitmapfont.h"
#include "framework/graphics/fontmanager.h"
#include "framework/graphics/texture.h"

namespace {

std::filesystem::path g_repoRoot;

std::filesystem::path findRepoRoot()
{
    auto current = std::filesystem::current_path();
    for (int i = 0; i < 8; ++i) {
        if (std::filesystem::exists(current / "data" / "fonts" / "Verdana.ttf")) {
            return current;
        }
        if (!current.has_parent_path())
            break;
        current = current.parent_path();
    }
    return std::filesystem::current_path();
}

class GraphicsTestEnvironment : public testing::Environment
{
public:
    void SetUp() override
    {
        m_previousLogLevel = g_logger.getLevel();
        g_logger.setLevel(Fw::LogFatal);

        g_resources.init(".");

        g_repoRoot = findRepoRoot();
        if (!std::filesystem::exists(g_repoRoot / "data" / "fonts" / "Verdana.ttf")) {
            ADD_FAILURE() << "Unable to locate data/fonts/Verdana.ttf under " << g_repoRoot.string();
        }

        const auto rootPath = g_repoRoot.generic_string();
        const auto dataPath = (g_repoRoot / "data").generic_string();
        const auto fontsPath = (g_repoRoot / "data" / "fonts").generic_string();

        if (!g_resources.addSearchPath(rootPath, true))
            ADD_FAILURE() << "Failed to mount search path: " << rootPath;
        if (!g_resources.addSearchPath(dataPath, true))
            ADD_FAILURE() << "Failed to mount search path: " << dataPath;
        if (!g_resources.addSearchPath(fontsPath, true))
            ADD_FAILURE() << "Failed to mount search path: " << fontsPath;

        g_graphics.m_maxTextureSize = 8192;
        g_graphics.m_ok = true;
    }

    void TearDown() override
    {
        g_fonts.clearFonts();
        g_resources.terminate();
        g_logger.setLevel(m_previousLogLevel);
    }

private:
    Fw::LogLevel m_previousLogLevel{ Fw::LogFatal };
};

[[maybe_unused]] testing::Environment* const g_graphicsEnv =
    testing::AddGlobalTestEnvironment(new GraphicsTestEnvironment);

class FontTest : public testing::Test
{
public:
    void SetUp() override
    {
        g_fonts.clearFonts();
    }
};

TEST_F(FontTest, BitmapFontLoadTrueTypeFromDataFolder)
{
    const std::string relativePath = "data/fonts/Verdana.ttf";
    ASSERT_TRUE(std::filesystem::exists(g_repoRoot / relativePath));

    std::string fontPath = relativePath;
    if (!g_resources.fileExists(fontPath))
        fontPath = "/" + relativePath;
    ASSERT_TRUE(g_resources.fileExists(fontPath));

    BitmapFont font("Verdana");
    EXPECT_TRUE(font.loadTrueType(fontPath, 12));
    ASSERT_NE(nullptr, font.getTexture());
    EXPECT_GT(font.getTexture()->getSize().width(), 0);
}

TEST_F(FontTest, FontManagerCanResolveFamilySizeToken)
{
    g_fonts.clearFonts();

    const auto font = g_fonts.getFont("Verdana-12px");
    ASSERT_NE(nullptr, font);
    ASSERT_NE(nullptr, font->getTexture());
    EXPECT_GT(font->getTexture()->getSize().width(), 0);

    const auto cached = g_fonts.getFont("Verdana-12px");
    EXPECT_EQ(font, cached);
}

} // namespace
