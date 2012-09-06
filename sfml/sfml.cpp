#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>

#if defined WIN32
#define EXPORT extern "C" __declspec(dllexport)
#else
#define EXPORT extern "C"
#endif

sf::Texture* texture;
sf::Sprite* sprite;

extern "C" EXPORT sf::RenderWindow* windowCreate(int width, int height, int bitsPerPixel, const char* name)
{
    // Create the window
    sf::RenderWindow* window = new sf::RenderWindow(sf::VideoMode(width, height, bitsPerPixel), name);

    return window;
}

EXPORT void windowDisplay(sf::RenderWindow* window)
{
    window->display();
}

EXPORT void windowClear(sf::RenderWindow* window)
{
    window->clear();
}

EXPORT void windowDraw(sf::RenderWindow* window, sf::Drawable* drawable)
{
    window->draw(*drawable);
}

EXPORT sf::Texture* textureCreate()
{
    sf::Texture* texture = new sf::Texture();

    return texture;
}

EXPORT int textureLoadFromFile(sf::Texture* texture, const char* file)
{
    if(!texture->loadFromFile(file))
        return 0;
    else
        return 1;
}

EXPORT sf::Sprite* spriteCreate()
{
    sf::Sprite* sprite = new sf::Sprite();

    return sprite;
}

EXPORT void spriteSetTexture(sf::Sprite* sprite, const sf::Texture* texture)
{
    sprite->setTexture(*texture);
}

EXPORT void spriteSetPosition(sf::Sprite* sprite, float xpos, float ypos)
{
    sprite->setPosition(xpos, ypos);
}

EXPORT void spriteMove(sf::Sprite* sprite, float xoffset, float yoffset)
{
    sprite->move(xoffset, yoffset);
}

EXPORT void spriteRotate(sf::Sprite* sprite, float angle)
{
    sprite->rotate(angle);
}

EXPORT sf::Font* fontCreate()
{
    sf::Font* font = new sf::Font();

    return font;
}

EXPORT int fontLoadFromFile(sf::Font* font, const char* file)
{
    if(!font->loadFromFile(file))
        return 0;
    else
        return 1;
}

EXPORT sf::Text* textCreate()
{
    sf::Text* text = new sf::Text();

    return text;
}

EXPORT void textSetString(sf::Text* text, const char* string)
{
    text->setString(string);
}


EXPORT void textSetFont(sf::Text* text, sf::Font* font)
{
    text->setFont(*font);
}

EXPORT void textSetCharacterSize(sf::Text* text, int size)
{
    text->setCharacterSize(size);
}

EXPORT int windowPollEvent(sf::RenderWindow* window)
{
    sf::Event event;
    bool result = window->pollEvent(event);
    if(result && (event.type == sf::Event::KeyReleased))
        return event.key.code;
    else
        return 0;
}
