#include "ui.h"
#include <iostream>
#include <cstdlib>

namespace ui {

TextBox::TextBox(const sf::Font& font, sf::Vector2f pos, sf::Vector2f size)
    : display(font)
{
    box.setPosition(pos);
    box.setSize(size);
    box.setFillColor(sf::Color(50, 50, 60));
    box.setOutlineColor(sf::Color(120, 120, 140));
    box.setOutlineThickness(1.0f);

    display.setCharacterSize(16);
    display.setFillColor(sf::Color::White);
    display.setPosition({pos.x + 5.0f, pos.y + 5.0f});
}

void TextBox::handleEvent(const sf::Event& event) {
    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left) {
            sf::Vector2f mp(static_cast<float>(mousePressed->position.x),
                            static_cast<float>(mousePressed->position.y));
            active = contains(mp);
        }
    }

    if (const auto* textEntered = event.getIf<sf::Event::TextEntered>()) {
        if (!active) return;
        char32_t c = textEntered->unicode;
        if (c == 8) {
            if (!text.empty()) text.pop_back();
        }
        else if (c == 13 || c == 10) {
            active = false;
        }
        else if (c >= 32 && c < 127) {
            char ch = static_cast<char>(c);
            if (numeric) {
                if ((ch >= '0' && ch <= '9') || ch == '.' || ch == '-') {
                    text += ch;
                }
            } else {
                text += ch;
            }
        }
        display.setString(text);
    }
}

void TextBox::draw(sf::RenderWindow& window) {
    box.setOutlineColor(active ? sf::Color(100, 180, 255)
                               : sf::Color(120, 120, 140));
    window.draw(box);
    window.draw(display);
}

float TextBox::getFloat() const {
    if (text.empty()) return 0.0f;
    try {
        return std::stof(text);
    } catch (...) {
        return 0.0f;
    }
}

void TextBox::setText(const std::string& s) {
    text = s;
    display.setString(text);
}

bool TextBox::contains(sf::Vector2f point) const {
    return box.getGlobalBounds().contains(point);
}

Button::Button(const sf::Font& font, const std::string& text,
               sf::Vector2f pos, sf::Vector2f size)
    : label(font)
{
    box.setPosition(pos);
    box.setSize(size);
    box.setFillColor(sf::Color(70, 70, 90));
    box.setOutlineColor(sf::Color(120, 120, 140));
    box.setOutlineThickness(1.0f);

    label.setString(text);
    label.setCharacterSize(16);
    label.setFillColor(sf::Color::White);
    sf::FloatRect tb = label.getLocalBounds();
    label.setPosition({
        pos.x + (size.x - tb.size.x) / 2.0f - tb.position.x,
        pos.y + (size.y - tb.size.y) / 2.0f - tb.position.y
    });
}

void Button::handleEvent(const sf::Event& event, sf::Vector2f mousePos) {
    hovered = contains(mousePos);

    if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mousePressed->button == sf::Mouse::Button::Left && hovered) {
            pressed = true;
        }
    }
    if (const auto* mouseReleased = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mouseReleased->button == sf::Mouse::Button::Left) {
            if (pressed && hovered) {
            }
        }
    }
}

void Button::draw(sf::RenderWindow& window) {
    if (hovered) {
        box.setFillColor(sf::Color(90, 90, 120));
    } else {
        box.setFillColor(sf::Color(70, 70, 90));
    }
    window.draw(box);
    window.draw(label);
}

bool Button::contains(sf::Vector2f point) const {
    return box.getGlobalBounds().contains(point);
}

}