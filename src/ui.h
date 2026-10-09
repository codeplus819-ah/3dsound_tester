#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

namespace ui {

struct TextBox {
    sf::RectangleShape box;
    std::string text;
    sf::Text display;
    bool active = false;
    bool numeric = false;
    TextBox(const sf::Font& font, sf::Vector2f pos, sf::Vector2f size);
    void handleEvent(const sf::Event& event);
    void draw(sf::RenderWindow& window);
    float getFloat() const;
    void setText(const std::string& s);
    bool contains(sf::Vector2f point) const;
};

struct Button {
    sf::RectangleShape box;
    sf::Text label;
    bool hovered = false;
    bool pressed = false;

    Button(const sf::Font& font, const std::string& text,
           sf::Vector2f pos, sf::Vector2f size);
    void handleEvent(const sf::Event& event, sf::Vector2f mousePos);
    void draw(sf::RenderWindow& window);
    bool contains(sf::Vector2f point) const;
};

}