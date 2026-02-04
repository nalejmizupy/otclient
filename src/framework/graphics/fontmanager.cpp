/*
 * Copyright (c) 2010-2026 OTClient <https://github.com/edubart/otclient>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "fontmanager.h"

#include "framework/core/resourcemanager.h"
#include "framework/otml/otmldocument.h"

FontManager g_fonts;

void FontManager::terminate() { clearFonts(); }

void FontManager::clearFonts() {
    m_fonts.clear();
    m_defaultFont = nullptr;
    m_defaultWidgetFont = nullptr;
}

bool FontManager::importFont(const std::string& file)
{
    const auto& path = g_resources.guessFilePath(file, "otfont");
    try {
        const auto& doc = OTMLDocument::parse(path);
        const auto& fontNode = doc->at("Font");
        const auto& name = fontNode->valueAt("name");

        // remove any font with the same name
        for (auto it = m_fonts.begin(); it != m_fonts.end(); ++it) {
            if ((*it)->getName() == name) {
                m_fonts.erase(it);
                break;
            }
        }

        const auto& font(std::make_shared<BitmapFont>(name));
        font->load(fontNode);
        m_fonts.emplace_back(font);

        // set as default if needed
        if (!m_defaultFont || fontNode->valueAt<bool>("default", false))
            m_defaultFont = font;
        else if (!m_defaultWidgetFont || fontNode->valueAt<bool>("widget-default", false))
            m_defaultWidgetFont = font;

        return true;
    } catch (const stdext::exception& e) {
        g_logger.error("Unable to load font from file '{}': {}", path, e.what());
        return false;
    }
}

bool FontManager::fontExists(const std::string_view fontName)
{
    for (const auto& font : m_fonts) {
        if (font->getName() == fontName)
            return true;
    }
    return false;
}

BitmapFontPtr FontManager::getFont(const std::string_view fontName)
{
    // find font by name
    for (const auto& font : m_fonts) {
        if (font->getName() == fontName)
            return font;
    }

    // Try to load TTF dynamically: "Family-SizePx" or "Family-Size"
    std::string name(fontName);
    int size = 0;
    std::string family;

    size_t dashPos = name.rfind('-');
    if (dashPos != std::string::npos && dashPos + 1 < name.size()) {
        std::string suffix = name.substr(dashPos + 1);

        // Strip "px" if present
        if (suffix.size() > 2 && suffix.substr(suffix.size() - 2) == "px") {
            suffix = suffix.substr(0, suffix.size() - 2);
        }

        // Check if suffix is a number
        bool isNum = !suffix.empty();
        for (char c : suffix) {
            if (c < '0' || c > '9') {
                isNum = false;
                break;
            }
        }

        if (isNum) {
            size = std::atoi(suffix.c_str());
            family = name.substr(0, dashPos);
        }
    }

    if (size > 0 && !family.empty()) {
        std::vector<std::string> candidates;
        candidates.push_back(family);
        candidates.push_back(name);
        // Try paths with leading slash as seen in styles.lua ('/fonts/...')
        candidates.push_back("/fonts/" + family);
        candidates.push_back("/data/fonts/" + family);

        // Also try relative (just in case)
        candidates.push_back("fonts/" + family);
        candidates.push_back("data/fonts/" + family);

        // Try lower-case version of family if different
        std::string lowerFamily = family;
        std::transform(lowerFamily.begin(), lowerFamily.end(), lowerFamily.begin(), ::tolower);
        if (lowerFamily != family) {
            candidates.push_back(lowerFamily);
            candidates.push_back("/fonts/" + lowerFamily);
            candidates.push_back("/data/fonts/" + lowerFamily);
            candidates.push_back("fonts/" + lowerFamily);
            candidates.push_back("data/fonts/" + lowerFamily);
        }

        std::string path;
        for (const auto& cand : candidates) {
            std::string guess = g_resources.guessFilePath(cand, "ttf");
            g_logger.info("Checking font candidate: '{}' -> '{}'", cand, guess);
            if (!guess.empty() && g_resources.fileExists(guess)) {
                path = guess;
                break;
            }
        }

        if (!path.empty()) {
            g_logger.info("Loading TrueType font '{}' from path '{}'", name, path);
            BitmapFontPtr font = std::make_shared<BitmapFont>(name);
            if (font->loadTrueType(path, size)) {
                m_fonts.push_back(font);
                return font;
            } else {
                g_logger.error("Failed to load TrueType font content from '{}'", path);
            }
        } else {
            g_logger.error("Could not find any TrueType font file for '{}' (Size: {})", name, size);
            for (const auto& cand : candidates) {
                g_logger.error("  Tried: '{}' -> '{}'", cand, g_resources.guessFilePath(cand, "ttf"));
            }
        }
    }

    // when not found, fallback to default font
    g_logger.error("font '{}' not found", fontName);
    return m_defaultFont;
}
