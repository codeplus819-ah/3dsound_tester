#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <filesystem>

#include "scanner.h"
#include "audio.h"
#include "ui.h"

struct SoundEntry {
    std::string filepath;
    std::string filename;
    audio::Sound sound;
    float x = 0.0f, y = 0.0f, z = 0.0f;
    float delay = 0.0f;

    std::unique_ptr<ui::TextBox> boxX;
    std::unique_ptr<ui::TextBox> boxY;
    std::unique_ptr<ui::TextBox> boxZ;
    std::unique_ptr<ui::TextBox> boxDelay;
    std::unique_ptr<ui::Button>  btnPlay;
};

static sf::Font g_font;
static std::vector<SoundEntry> g_sounds;

bool setupSounds(const std::string& assetsPath) {
    auto files = scanner::scan(assetsPath);

    std::cout << "Found " << files.size() << " file(s)\n";
    for (const auto& f : files) {
        std::cout << "  " << f << "\n";
    }

    float y = 100.0f;
    const float rowHeight = 40.0f;

    for (const auto& path : files) {
        SoundEntry entry;
        entry.filepath = path;
        entry.filename = std::filesystem::path(path).filename().string();
        entry.sound = audio::createSound(path);

        if (!entry.sound.valid) {
            std::cerr << "  Skipped: " << path << "\n";
            continue;
        }

        float x = 350.0f;

        entry.boxX = std::make_unique<ui::TextBox>(g_font, sf::Vector2f{x, y}, sf::Vector2f{80, 30});
        entry.boxX->numeric = true;
        entry.boxX->setText("0");
        x += 90;

        entry.boxY = std::make_unique<ui::TextBox>(g_font, sf::Vector2f{x, y}, sf::Vector2f{80, 30});
        entry.boxY->numeric = true;
        entry.boxY->setText("0");
        x += 90;

        entry.boxZ = std::make_unique<ui::TextBox>(g_font, sf::Vector2f{x, y}, sf::Vector2f{80, 30});
        entry.boxZ->numeric = true;
        entry.boxZ->setText("0");
        x += 90;

        entry.boxDelay = std::make_unique<ui::TextBox>(g_font, sf::Vector2f{x, y}, sf::Vector2f{80, 30});
        entry.boxDelay->numeric = true;
        entry.boxDelay->setText("0");
        x += 90;

        entry.btnPlay = std::make_unique<ui::Button>(g_font, "Play",
            sf::Vector2f{x, y}, sf::Vector2f{70, 30});

        g_sounds.push_back(std::move(entry));
        y += rowHeight;
    }

    return !g_sounds.empty();
}

void drawSounds(sf::RenderWindow& window, sf::Vector2f mousePos) {
    for (auto& s : g_sounds) {
        sf::Text name(g_font, s.filename, 16);
        name.setFillColor(sf::Color::White);
        name.setPosition({20.0f, s.boxX->box.getPosition().y + 5.0f});
        window.draw(name);

        s.boxX->draw(window);
        s.boxY->draw(window);
        s.boxZ->draw(window);
        s.boxDelay->draw(window);
        s.btnPlay->draw(window);
    }
}

void handleEvents(sf::RenderWindow& window, sf::Vector2f mousePos) {
    while (const std::optional event = window.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window.close();
            return;
        }

        for (auto& s : g_sounds) {
            s.boxX->handleEvent(*event);
            s.boxY->handleEvent(*event);
            s.boxZ->handleEvent(*event);
            s.boxDelay->handleEvent(*event);
            s.btnPlay->handleEvent(*event, mousePos);
        }

        if (const auto* mouseReleased = event->getIf<sf::Event::MouseButtonReleased>()) {
            if (mouseReleased->button == sf::Mouse::Button::Left) {
                for (auto& s : g_sounds) {
                    if (s.btnPlay->pressed && s.btnPlay->hovered) {
                        audio::play(s.sound,
                                    s.boxX->getFloat(),
                                    s.boxY->getFloat(),
                                    s.boxZ->getFloat(),
                                    0.0f);
                    }
                    s.btnPlay->pressed = false;
                }
            }
        }
    }
}

int main() {
    sf::RenderWindow window(sf::VideoMode({1000, 700}), "3D Sound Test");
    window.setFramerateLimit(60);

    if (!g_font.openFromFile("assets/alibaba.ttf")) {
        std::cerr << "Could not load font: assets/alibaba.ttf\n";
        return -1;
    }

    if (!audio::init()) {
        std::cerr << "OpenAL failed to initialize\n";
        return -1;
    }

    audio::setListenerPosition(0.0f, 0.0f, 0.0f);
    audio::setListenerOrientation(0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);

    if (!setupSounds("assets/sounds")) {
        std::cerr << "No sounds found in assets/sounds\n";
        audio::shutdown();
        return -1;
    }

    ui::Button playAllBtn(g_font, "Play All",
        sf::Vector2f{400.0f, 620.0f}, sf::Vector2f{150.0f, 45.0f});

    sf::Clock clock;
    bool playAllWasPressed = false;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        sf::Vector2f mousePos = {
            static_cast<float>(sf::Mouse::getPosition(window).x),
            static_cast<float>(sf::Mouse::getPosition(window).y)
        };

        handleEvents(window, mousePos);
        playAllBtn.updateHover(mousePos);

        bool mouseDown = sf::Mouse::isButtonPressed(sf::Mouse::Button::Left);
        bool hoveringPlayAll = playAllBtn.contains(mousePos);

        if (mouseDown && hoveringPlayAll && !playAllWasPressed) {
            std::cout << "Play All triggered\n";
            for (auto& s : g_sounds) {
                audio::play(s.sound,
                            s.boxX->getFloat(),
                            s.boxY->getFloat(),
                            s.boxZ->getFloat(),
                            s.boxDelay->getFloat());
            }
            playAllWasPressed = true;
        }
        if (!mouseDown) {
            playAllWasPressed = false;
        }

        audio::update(dt);

        window.clear(sf::Color(30, 30, 40));

        sf::Text header(g_font, "3D Sound Test", 24);
        header.setFillColor(sf::Color::White);
        header.setPosition({20.0f, 20.0f});
        window.draw(header);

        sf::Text colX(g_font, "X", 14);
        colX.setFillColor(sf::Color(180, 180, 200));
        colX.setPosition({355.0f, 75.0f});
        window.draw(colX);

        sf::Text colY(g_font, "Y", 14);
        colY.setFillColor(sf::Color(180, 180, 200));
        colY.setPosition({445.0f, 75.0f});
        window.draw(colY);

        sf::Text colZ(g_font, "Z", 14);
        colZ.setFillColor(sf::Color(180, 180, 200));
        colZ.setPosition({535.0f, 75.0f});
        window.draw(colZ);

        sf::Text colD(g_font, "Delay", 14);
        colD.setFillColor(sf::Color(180, 180, 200));
        colD.setPosition({620.0f, 75.0f});
        window.draw(colD);

        drawSounds(window, mousePos);
        playAllBtn.draw(window);

        window.display();
    }

    for (auto& s : g_sounds) {
        audio::destroySound(s.sound);
    }
    audio::shutdown();

    return 0;
}