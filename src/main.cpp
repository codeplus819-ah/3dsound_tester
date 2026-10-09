#include <SFML/Graphics.hpp>
#include <iostream>
#include "scanner.h"

int main() {
    std::string assetsPath = "assets/sounds";
    auto files = scanner::scan(assetsPath);

    for (const auto& f : files) {
        std::cout << "  " << f << "\n";
    }

    sf::RenderWindow window(sf::VideoMode({900, 600}), "3D Sound Test");

    sf::Font font;
    if (!font.openFromFile("assets/alibaba.ttf")) {
        std::cerr << "Could not find alibaba.ttf font\n";
        return -1;
    }

    sf::Text text(font);
    text.setCharacterSize(18);
    text.setFillColor(sf::Color::White);

    std::string listStr;
    for (size_t i = 0; i < files.size(); ++i) {
        listStr += std::to_string(i + 1) + ". " + files[i] + "\n";
    }
    text.setString(listStr);

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear(sf::Color(30, 30, 40));
        window.draw(text);
        window.display();
    }

    return 0;
}