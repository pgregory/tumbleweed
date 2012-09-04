#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

sf::Texture* texture;
sf::Sprite* sprite;

extern "C" sf::RenderWindow* windowCreate(int width, int height, int bitsPerPixel, const char* name)
{
    // Create the window
    sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode(width, height, bitsPerPixel), name);

    return window;
}

extern "C" void windowDisplay(sf::RenderWindow* window)
{
    window->display();
}

extern "C" void windowClear(sf::RenderWindow* window)
{
    window->clear();
}

extern "C" void windowDraw(sf::RenderWindow* window, sf::Drawable* drawable)
{
    window->draw(*drawable);
}

extern "C" sf::Texture* textureCreate()
{
    sf::Texture* texture = new sf::Texture();

    return texture;
}

extern "C" int textureLoadFromFile(sf::Texture* texture, const char* file)
{
    if(!texture->loadFromFile(file))
        return 0;
    else
        return 1;
}

extern "C" sf::Sprite* spriteCreate()
{
    sf::Sprite* sprite = new sf::Sprite();

    return sprite;
}

extern "C" void spriteSetTexture(sf::Sprite* sprite, const sf::Texture* texture)
{
    sprite->setTexture(*texture);
}

extern "C" void spriteSetPosition(sf::Sprite* sprite, float xpos, float ypos)
{
    sprite->setPosition(xpos, ypos);
}

extern "C" void spriteMove(sf::Sprite* sprite, float xoffset, float yoffset)
{
    sprite->move(xoffset, yoffset);
}

extern "C" void spriteRotate(sf::Sprite* sprite, float angle)
{
    sprite->rotate(angle);
}

extern "C" sf::Font* fontCreate()
{
    sf::Font* font = new sf::Font();

    return font;
}

extern "C" int fontLoadFromFile(sf::Font* font, const char* file)
{
    if(!font->loadFromFile(file))
        return 0;
    else
        return 1;
}

extern "C" sf::Text* textCreate()
{
    sf::Text* text = new sf::Text();

    return text;
}

extern "C" void textSetString(sf::Text* text, const char* string)
{
    text->setString(string);
}


extern "C" void textSetFont(sf::Text* text, sf::Font* font)
{
    text->setFont(*font);
}

extern "C" void textSetCharacterSize(sf::Text* text, int size)
{
    text->setCharacterSize(size);
}

extern "C" int windowPollEvent(sf::RenderWindow* window)
{
    sf::Event event;
    bool result = window->pollEvent(event);
    if(result && (event.type == sf::Event::KeyReleased))
        return event.key.code;
    else
        return 0;
}
