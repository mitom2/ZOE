#include "gpuModule.hpp"

bool GpuModule::createCharacterFile(char character, int x, int y)
{
    std::string nm = "gpuCharacters/" + std::to_string(character) + ".gpu";
    std::ofstream charFile(nm);
    if (charFile.good() == false)
    {
        charFile.close();
        throw std::exception("GPU could not create character file");
        return true;
    }
    sf::Font font;
    if (font.loadFromFile("gpuCharacters/ChivoMono-Regular.ttf") == false)
    {
        charFile.close();
        throw std::exception("GPU could load character definition");
        return true;
    }
    std::string c = "_";
    c[0] = character;
    sf::Text ch(c, font, characterSizeY);
    sf::RenderTexture rendT;
    if (rendT.create(characterSizeX, characterSizeY) == false)
    {
        throw std::exception("GPU could not generate character");
        return true;
    }
    ch.setOrigin(0, 0);
    ch.setPosition(0, -characterSizeY / 5);
    ch.setFillColor(sf::Color::Red);
    rendT.clear(sf::Color::Black);
    rendT.draw(ch);
    rendT.display();
    sf::Image img = rendT.getTexture().copyToImage();
    bool res = true;
    for (int iy = 0; iy < img.getSize().y; iy++)
    {
        for (int ix = 0; ix < img.getSize().x; ix++)
        {
            if (img.getPixel(ix, iy).r > gpuCharGenThr)
            {
                charFile << c << " ";
                if (x == ix && y == iy)
                    res = true;
            }
            else
            {
                if (character == '0')
                    charFile << "_ ";
                else
                    charFile << "0 ";
                if (x == ix && y == iy)
                    res = false;
            }
        }
        charFile << std::endl;
    }
    charFile.close();
    return res;
}

bool GpuModule::characterPixel(char character, int x, int y)
{
    std::string nm = "gpuCharacters/" + std::to_string(character) + ".gpu";
    std::ifstream charFile(nm);
    if (charFile.good() == false)
    {
        charFile.close();
        return GpuModule::createCharacterFile(character, x, y);
    }
    char in = 0;
    for (int i = 0; i < characterSizeX * characterSizeY; i++)
    {
        charFile >> in;
        if (charFile.eof() == true)
            break;
        if (i == y * characterSizeX + x)
        {
            return in == character;
        }
    }
    charFile.close();
    return true;
}

void GpuModule::interpretCommand(sf::Image& pixels, float& cx, float& cy, uint32_t& color, float& dx, float& dy, float& step, float& ctr)
{
    if (GpuModule::commands.front() == cmd_ClearScreen)
    {
        if (ctr == 0)
        {
            cx = 0;
            cy = 0;
        }
        if (cx < 640 && cy < 480 && cx >= 0 && cy >= 0)
            pixels.setPixel(cx, cy, sf::Color(color));
        cx++;
        if (cx >= 640)
        {
            cy++;
            cx = 0;
        }
        ctr++;
        if (cy >= 480)
        {
            cx = 0;
            cy = 0;
            ctr = 0;
            GpuModule::commands.pop_front();
        }
    }
    else if (GpuModule::commands.front() == cmd_SetPointer)
    {
        if (GpuModule::commands.size() < 5)
            return;
        GpuModule::commands.pop_front();
        int x = 0, y = 0;
        x=GpuModule::commands.front();
        GpuModule::commands.pop_front();
        x += GpuModule::commands.front() * 0b100000000;
        GpuModule::commands.pop_front();
        y = GpuModule::commands.front();
        GpuModule::commands.pop_front();
        y += GpuModule::commands.front() * 0b100000000;
        GpuModule::commands.pop_front();
        cx = x;
        cy = y;
    }
    else if (GpuModule::commands.front() == cmd_MovePointerLeftUp)
    {
        if (GpuModule::commands.size() < 5)
            return;
        GpuModule::commands.pop_front();
        int x = 0, y = 0;
        x = GpuModule::commands.front();
        GpuModule::commands.pop_front();
        x += GpuModule::commands.front() << 8;
        GpuModule::commands.pop_front();
        y = GpuModule::commands.front();
        GpuModule::commands.pop_front();
        y += GpuModule::commands.front() << 8;
        GpuModule::commands.pop_front();
        cx -= x;
        cy -= y;
    }
    else if (GpuModule::commands.front() == cmd_MovePointerRightDown)
    {
        if (GpuModule::commands.size() < 5)
            return;
        GpuModule::commands.pop_front();
        int x = 0, y = 0;
        x = GpuModule::commands.front();
        GpuModule::commands.pop_front();
        x += GpuModule::commands.front() << 8;
        GpuModule::commands.pop_front();
        y = GpuModule::commands.front();
        GpuModule::commands.pop_front();
        y += GpuModule::commands.front() << 8;
        GpuModule::commands.pop_front();
        cx += x;
        cy += y;
    }
    else if (GpuModule::commands.front() == cmd_setColor)
    {
        if (GpuModule::commands.size() < 2)
            return;
        GpuModule::commands.pop_front();
        color = GpuModule::commands.front();
        uint32_t r2 = color & 0b100000;
        uint32_t r1 = color & 0b010000;
        uint32_t g2 = color & 0b001000;
        uint32_t g1 = color & 0b000100;
        uint32_t b2 = color & 0b000010;
        uint32_t b1 = color & 0b000001;
        color = 255;
        if (r1 > 0 && r2 > 0)
            color += 0b11111111 << 24;
        else if (r1 > 0)
            color += 0b01010101 << 24;
        else if (r2 > 0)
            color += 0b10101010 << 24;
        if (g1 > 0 && g2 > 0)
            color += 0b11111111 << 16;
        else if (g1 > 0)
            color += 0b01010101 << 16;
        else if (g2 > 0)
            color += 0b10101010 << 16;
        if (b1 > 0 && b2 > 0)
            color += 0b11111111 << 8;
        else if (b1 > 0)
            color += 0b01010101 << 8;
        else if (b2 > 0)
            color += 0b10101010 << 8;
        GpuModule::commands.pop_front();
    }
    else if (GpuModule::commands.front() == cmd_drawPixel)
    {
        GpuModule::commands.pop_front();
        if (cx < 640 && cy < 480 && cx >= 0 && cy >= 0)
            pixels.setPixel(cx, cy, sf::Color(color));
        cx++;
        if (cx >= 640)
        {
            cy++;
            cx = 0;
        }
    }
    else if (GpuModule::commands.front() == cmd_drawLine)
    {
        if (GpuModule::commands.size() < 5)
            return;
        int x = 0, y = 0, st = 0;
        for (auto i : GpuModule::commands)
        {
            st++;
            if (st == 2)
                x = i;
            else if (st == 3)
                x += i * 0b100000000;
            else if (st == 4)
                y = i;
            else if (st == 5)
                y += i * 0b100000000;
            else if (st > 5)
                break;
        }
        if (ctr == 0)
        {
            dy = y - cy;
            dx = x - cx;
        }
        if (step == 0)
        {
            step = std::max(std::fabs(dx), std::fabs(dy));
            dx /= step;
            dy /= step;
        }
        if (ctr <= step)
        {
            if (cx < 640 && cy < 480 && cx >= 0 && cy >= 0)
                pixels.setPixel(cx, cy, sf::Color(color));
            cx += dx;
            cy += dy;
            ctr++;
        }
        else
        {
            step = 0;
            ctr = 0;
            GpuModule::commands.pop_front();
            GpuModule::commands.pop_front();
            GpuModule::commands.pop_front();
            GpuModule::commands.pop_front();
            GpuModule::commands.pop_front();
        }
    }
    else if (GpuModule::commands.front() == cmd_drawRectangle)
    {
        if (GpuModule::commands.size() < 5)
            return;
        int x = 0, y = 0, st = 0;
        if (ctr == 0)
        {
            dx = cx;
            dy = cy;
            ctr++;
        }
            for (auto i : GpuModule::commands)
            {
                st++;
                if (st == 2)
                    x = i;
                else if (st == 3)
                    x += i * 0b100000000;
                else if (st == 4)
                    y = i;
                else if (st == 5)
                    y += i * 0b100000000;
                else if (st > 5)
                    break;
            }
            if (cx < 640 && cy < 480 && cx >= 0 && cy >= 0)
                pixels.setPixel(cx, cy, sf::Color(color));
        if (x < dx)
        {
            cx--;
        }
        else if (x > dx)
        {
            cx++;
        }
        if (cx == x)
        {
            cx = dx;
            if (cy == y)
            {
                ctr = 0;
                cx = x;
                GpuModule::commands.pop_front();
                GpuModule::commands.pop_front();
                GpuModule::commands.pop_front();
                GpuModule::commands.pop_front();
                GpuModule::commands.pop_front();
            }
            else
            {
                if (y < dy)
                {
                    cy--;
                }
                else if (y > cy)
                {
                    cy++;
                }
            }
        }
    }
    else if (GpuModule::commands.front() == cmd_drawChar)
    {
        if (GpuModule::commands.size() < 2)
            return;
        int st = 0;
        char character = 0;
        if (ctr == 0)
        {
            dx = cx;
            dy = cy;
            step = 0;
        }
        for (auto i : GpuModule::commands)
        {
            st++;
            if (st == 2)
                character = i;
            else if (st > 2)
                break;
        }
        if (GpuModule::characterPixel(character, (int)ctr % characterSizeX, ctr / characterSizeX) == true)
        {
            if (cx < 640 && cy < 480 && cx >= 0 && cy >= 0)
                pixels.setPixel(cx, cy, sf::Color(color));
        }
        ctr++;
        cx++;
        step++;
        if (step >= characterSizeX)
        {
            cx = dx;
            cy++;
            step = 0;
        }
        if (ctr + 1 > characterSizeX * characterSizeY)
        {
            ctr = 0;
            step = 0;
            cy = dy;
            cx = dx + characterSizeX + 1;
            if (cx >= 640)
            {
                cx = 0;
                cy += characterSizeY + 1;
            }
            GpuModule::commands.pop_front();
            GpuModule::commands.pop_front();
        }
    }
    else
    {
        GpuModule::commands.pop_front();
    }
}

void GpuModule::display()
{
    sf::RenderWindow window(sf::VideoMode(640, 480), "Z80 PC", sf::Style::Resize);
    sf::Image screen;
    screen.create(640, 480, sf::Color::Black);
    sf::Texture tx;
    sf::Sprite displ;
    displ.setOrigin(0, 0);
    displ.setPosition(0, 0);
    window.setFramerateLimit(60);
    float cx = 0, cy = 0;
    uint32_t color = 0;
    float dx = 0.0, dy = 0.0, step = 0.0, ctr = 0.0;

    sf::RectangleShape cur;
    cur.setSize(sf::Vector2f(10, 10));
    cur.setOrigin(0, 0);
    cur.setPosition(0, 0);
    cur.setFillColor(sf::Color(0, 0, 255, 128));

    float fps;
    sf::Clock clock = sf::Clock::Clock();
    sf::Time previousTime = clock.getElapsedTime();
    sf::Time currentTime;

    while (window.isOpen())
    {
        GpuModule::listMtx.lock();
        for (int i = 0; i < gpuSpeed; i++)
        {
            if (GpuModule::commands.size() > 0)
                GpuModule::interpretCommand(screen, cx, cy, color, dx, dy, step, ctr);
            else break;
        }
        GpuModule::listMtx.unlock();

        sf::Event event;
        while (window.pollEvent(event))
        {
            //if (event.type == sf::Event::Closed)
                //window.close();
        }

        currentTime = clock.getElapsedTime();
        fps = 1.0f / (currentTime.asSeconds() - previousTime.asSeconds());
        previousTime = currentTime;
        window.setTitle("Z80 PC | " + std::to_string(fps) + " FPS");

        cur.setPosition(cx - 5, cy - 5);
        tx.loadFromImage(screen);
        displ.setTexture(tx, true);
        window.clear(sf::Color::Black);
        window.draw(displ);
        if (hideCur == false)
            window.draw(cur);
        window.display();
    }
}

void GpuModule::controlInput(unsigned short address, unsigned char value)
{
    if (address != gpuModulePort && address != gpuModulePort + 1)
    {
        throw std::exception("GPU module control access denied");
        return;
    }
    GpuModule::listMtx.lock();
    GpuModule::commands.push_back(value);
    GpuModule::listMtx.unlock();
}
