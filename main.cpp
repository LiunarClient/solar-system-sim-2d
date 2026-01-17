#include <SFML/Graphics.hpp>
#include <utility>
#include <cmath>
#include <vector>

struct sliderButton
{
    sf::Vector2f position;
    sf::Vector2f scale;
    sf::RectangleShape track;
    sf::RectangleShape button;
    float trackSize = 200.f;
    float buttonSize = 24.f;

    sliderButton() {

        track.setSize({trackSize, 16.f});
        track.setOrigin({trackSize / 2.f, 16.f / 2.f});
        button.setSize({buttonSize, buttonSize});
        button.setOrigin({buttonSize / 2.f, buttonSize / 2.f});
        button.setPosition({track.getPosition().x, track.getPosition().y});

        track.setFillColor(sf::Color(100, 100, 100, 100));
        button.setFillColor(sf::Color::White);
    }
};

sf::Font font("Arvo-Regular.ttf");


struct listElement
{
    int index;
    float separation;
    sf::Vector2f buttonSize;
    sf::RectangleShape button;
    sf::Text text;

    listElement(int i,
                const std::string& name,
                const sf::Font& font)
        : index(i),
          separation(50.f),
          buttonSize{150.f, 36.f},
          text(font)
    {
        text.setFont(font);
        text.setString(name);

        button.setSize(buttonSize);
        button.setOrigin(button.getSize() / 2.f);
        button.setFillColor(sf::Color(100, 100, 200, 150));
    }
};

struct body
{
    std::string name;
    sf::RectangleShape square;
    sf::Vector2f velocity;
    sf::Vector2f position;
    float mass;
};
// When calculating Gravitational constant and other measurements, use these:
// In the case of Earth:
// M (sun's mass) = 1 (Solar mass)
// r (distance to sun) = 1 (Astronomical unit)
// T (time) = 1 (year)
// G = 4PI^2
//
// Radius is also calculated in kilometers/100

const double PI = 3.1415926535;
const float G = 4*PI*PI;
const float M = 1.f; // 1 SM
const int HEIGHT = 924; // pixels
const int WIDTH = 1650; // pixels
const float dt = 0.00001f; // 1 year per frame
constexpr float AU_TO_WORLD = 50000.f;
constexpr float MOON_RAD_SCALE = 1.05f;


/*
    1 - object from which angle is drawn
    2 - object to which angle is drawn
    x1, y1 should be the object of which angle of force should be calculated
    x2, y2 should be the object that's pulling the object of interest
*/
float Gravity (float x1, float x2, float y1, float y2, float m1, float m2)
{
    float radius = sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
    float force = (G * m1 * m2) / (radius * radius);
    return force;
}
float Angle (float x1, float x2, float y1, float y2)
{
    return atan2((y2-y1),(x2-x1));
}
bool mouseOver(sf::RectangleShape object, sf::Vector2i mousePosI)
{
    sf::Vector2f mousePosF = {static_cast<float>(mousePosI.x), static_cast<float>(mousePosI.y)};
    bool mouseOverButton =
        mousePosF.x >= object.getPosition().x - object.getSize().x/2 &&
        mousePosF.x <= object.getPosition().x + object.getSize().x/2 &&
        mousePosF.y >= object.getPosition().y - object.getSize().y/2 &&
        mousePosF.y <= object.getPosition().y + object.getSize().y/2;
    return mouseOverButton;
}

body CreatePlanet(const std::string& name,
                  float radius,
                  float mass,
                  float distance,
                  sf::Color& color,
                  bool moon = false)
{
    body planet;
    planet.name = name;
    float sizeMult = 1.f;
    if (planet.name != "Sun")
    {
         sizeMult = 20.f;
    }
    float renderRadius = radius*2 * sizeMult;
    planet.square.setSize({renderRadius, renderRadius});
    planet.square.setOrigin({renderRadius/2, renderRadius/2});
    planet.square.setFillColor(color);
    planet.position = {distance, 0.f};
    planet.square.setPosition({ WIDTH/2 + planet.position.x * AU_TO_WORLD, HEIGHT/2 + planet.position.y * AU_TO_WORLD});
    if(distance == 0)
    {
        planet.velocity = {0.f, 0.f};
    }
    else
    {
        // Planet starts in vertical velocity, due to in-line starting body position
        planet.velocity = {0.f, -sqrtf((M*G)/distance)};
    }
    planet.mass = mass;
    return planet;
}

body CreateMoon(const std::string& name,
                  float radius, // moon's radius in km/100
                  float mass, // moon's mass in Solar masses (SM)
                  float parentMass, // parent planet's mass in SM
                  float distance, // moon's distance to sun (0; 0)
                  float parentDistance, // parent planet's distance to sun (0; 0)
                  sf::Color& color)
{
    body moon;
    moon.name = name;
    float sizeMult = 20.f;
    float renderRadius = radius*2 * sizeMult;
    moon.square.setSize({renderRadius, renderRadius});
    moon.square.setOrigin({renderRadius/2, renderRadius/2});
    moon.square.setFillColor(color);
    moon.position = {distance, 0.f};
    moon.square.setPosition({ WIDTH/2 + moon.position.x * AU_TO_WORLD * MOON_RAD_SCALE, HEIGHT/2 + moon.position.y * AU_TO_WORLD * MOON_RAD_SCALE});
    // Velocity calculation
    float moonDistance = distance - parentDistance; // distance to parent
    float v_orbit = sqrt(G * parentMass / moonDistance); // orbital speed around parent
    // Moon starts in vertical velocity, due to in-line starting body position
    sf::Vector2f parentVelocity = {0.f, -1 * sqrtf(M * G / parentDistance)};
    moon.velocity = {0.f, parentVelocity.y +(-v_orbit)};
    moon.mass = mass;
    return moon;
}
void RenderPlanet(int i, std::vector<body>& Bodies)
{
    Bodies[i].square.setPosition(Bodies[i].position * AU_TO_WORLD);
}
void RenderMoon(sf::Vector2f pos, sf::Vector2f posParent, float exaggeration, int i, std::vector<body>& Bodies)
{
    sf::Vector2f mtp = posParent - pos;
    sf::Vector2f exagerratedOffset = mtp * exaggeration;
    sf::Vector2f renderPos = pos + exagerratedOffset;
    Bodies[i].square.setPosition(renderPos * AU_TO_WORLD);
}

int main()
{

    sf::RenderWindow window(sf::VideoMode({WIDTH, HEIGHT}), "Solar System Simulator 2D");

    sf::View view;

    font.setSmooth(false);

    sliderButton zoomSlider;
    zoomSlider.track.setPosition({0.1f * WIDTH, 0.5f * HEIGHT});
    zoomSlider.button.setPosition({zoomSlider.track.getPosition().x, zoomSlider.track.getPosition().y});
    bool buttonHeld = false;

    const sf::Texture bgTexture("background.png");
    sf::Sprite background(bgTexture);
    background.setColor(sf::Color(255, 255, 255, 100));

    std::vector<body> Bodies;
    body Planet;

    // Sun setup
    sf::Color sunColor(sf::Color::Yellow);
    Bodies.push_back(CreatePlanet("Sun", 7000.f, 1.f, 0.f, sunColor));
    // Mercury setup
    sf::Color mercuryColor(100, 0, 10);
    Bodies.push_back(CreatePlanet("Mercury", 24.4f, 1.66e-7f, 0.4f, mercuryColor));
    // Venus setup
    sf::Color venusColor(100, 50, 10);
    Bodies.push_back(CreatePlanet("Venus", 60.52f, 2.44e-6f, 0.7f, venusColor));
    // Earth setup
    sf::Color earthColor(sf::Color::Blue);
    Bodies.push_back(CreatePlanet("Earth", 63.71f, 3.e-6f, 1.f, earthColor));
        // Moon setup
        float earthMass = Bodies[3].mass;
        float earthDistance = Bodies[3].position.x;
        sf::Color moonColor(sf::Color::White);
        Bodies.push_back(CreateMoon("Moon", 17.38f, 3.69e-8f, earthMass, 1.00257f, earthDistance, moonColor));

    // Mars setup
    sf::Color marsColor(sf::Color::Red);
    Bodies.push_back(CreatePlanet("Mars", 33.90f, 3.22e-7f, 1.5f, marsColor));
    // Jupiter setup
    sf::Color jupiterColor(160, 80, 40);
    Bodies.push_back(CreatePlanet("Jupiter", 699.11f, 9.5e-4f, 5.2f, jupiterColor));
        // Jupiter's moons
        // Ganymede setup
        float jupiterMass = Bodies[6].mass;
        float jupiterDistance = Bodies[6].position.x;
        sf::Color ganymedeColor(233, 220, 200);
        Bodies.push_back(CreateMoon("Ganymede", 26.341f, 9.9e-5f, jupiterMass, 5.215f, jupiterDistance, ganymedeColor));
        //Callisto setup
        sf::Color callistoColor(130, 120, 100);
        Bodies.push_back(CreateMoon("Callisto", 24.105f, 5.41e-6f, jupiterMass, 5.2055f, jupiterDistance, callistoColor));
    // Saturn setup
    sf::Color saturnColor(180, 140, 110);
    Bodies.push_back(CreatePlanet("Saturn", 582.32f, 2.86e-4f, 9.6f, saturnColor));
        // Saturn's moons
        // Titan setup
        float saturnMass = Bodies[9].mass;
        float saturnDistance = Bodies[9].position.x;
        sf::Color titanColor(250, 190, 70);
        Bodies.push_back(CreateMoon("Titan", 25.747f, 6.76e-8f, saturnMass, 9.61816f, saturnDistance, titanColor));
    // Uranus setup
    sf::Color uranusColor(170, 230, 240);
    Bodies.push_back(CreatePlanet("Uranus", 253.62f, 4.36e-5f, 19.2f, uranusColor));
    // Neptune setup
    sf::Color neptuneColor(120, 180, 190);
    Bodies.push_back(CreatePlanet("Neptune", 246.22f, 5.13e-5f, 30.f, neptuneColor));

    int n = Bodies.size();

    std::vector<listElement> focusElements;

    int focus = 0;
    view.setCenter(Bodies[0].position);

    for (int i = 0; i < n; i++)
    {
        listElement focus(i, Bodies[i].name, font);
        sf::FloatRect bounds = focus.text.getLocalBounds();
        focus.text.setOrigin(bounds.getCenter());
        focus.button.setPosition({WIDTH - 100.f, 0 + 50.f + focus.separation * focus.index});
        focus.text.setPosition(focus.button.getPosition());
        focusElements.push_back(focus);
    }

    // Game loop
    while (window.isOpen())
    {
        while (const std::optional event = window.pollEvent())
        {
            if (event -> is <sf::Event::Closed>() )
            {
                window.close();
            }
        }

        // Store accelerations (velocity verlet)
        std::vector<sf::Vector2f> Accels(n, {0.f, 0.f});


        // Calculate gravitational force
        for (int i = 0; i < n; i++)
        {
            for (int j = i+1; j < n; j++)
            {
//                if (i == 0 && j == 4) continue;
//                if (i == 0 && j == 3) continue;

                float force = Gravity(Bodies[i].position.x, Bodies[j].position.x,
                                      Bodies[i].position.y, Bodies[j].position.y,
                                      Bodies[i].mass, Bodies[j].mass);
                float angle = Angle(Bodies[i].position.x, Bodies[j].position.x,
                                    Bodies[i].position.y, Bodies[j].position.y);
                float ax = force * cos(angle) / Bodies[i].mass;
                float ay = force * sin(angle) / Bodies[i].mass;
                Accels[i].x += ax;
                Accels[i].y += ay;
                Accels[j].x -= ax * Bodies[i].mass / Bodies[j].mass;
                Accels[j].y -= ay * Bodies[i].mass / Bodies[j].mass;
            }
        }

        for (int i = 0; i < n; i++)
        {
            Bodies[i].position += Bodies[i].velocity * dt + 0.5f * Accels[i] * dt * dt;
        }
        for (int i = 0; i < n; i++)
        {
            Bodies[i].velocity += Accels[i] * dt;
        }

        // Change focus view to body[i]
        view.setSize({WIDTH, HEIGHT});
        float zoomLevel = 4.f;

        // Mouse on slider detection
        sf::Vector2f positionAtButtonPress;
        sf::Vector2i mousePosI = sf::Mouse::getPosition(window);
        sf::Vector2f mousePosF = {static_cast<float>(mousePosI.x), static_cast<float>(mousePosI.y)};
        bool mouseOverButton =
            mousePosF.x >= zoomSlider.button.getPosition().x - zoomSlider.buttonSize &&
            mousePosF.x <= zoomSlider.button.getPosition().x + zoomSlider.buttonSize &&
            mousePosF.y >= zoomSlider.button.getPosition().y - zoomSlider.buttonSize &&
            mousePosF.y <= zoomSlider.button.getPosition().y + zoomSlider.buttonSize;

        if (!buttonHeld && mouseOverButton && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
        {
                positionAtButtonPress = mousePosF;
                buttonHeld = true;
        }
        // Set up and set zoom level to slider position
        float trackLeft = zoomSlider.track.getPosition().x - zoomSlider.trackSize/2;
        float trackRight = zoomSlider.track.getPosition().x + zoomSlider.trackSize/2;
        float sliderX = std::max(trackLeft, std::min(mousePosF.x, trackRight));
        float sliderValue = ((WIDTH - zoomSlider.button.getPosition().x - (WIDTH - trackRight) + 5.f));
        zoomLevel = powf(10.f, sliderValue/50.f);
        // Move slider to mouse position
        if (buttonHeld)
        {
            zoomSlider.button.setPosition({sliderX, zoomSlider.button.getPosition().y});
            zoomSlider.button.setFillColor(sf::Color(200, 200, 200));

            if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
            {
                buttonHeld = false;
                zoomSlider.button.setFillColor(sf::Color::White);
            }
        }

        // Focus button click detecion

        for (int i = 0; i < n; i++)
        {
            mousePosI = sf::Mouse::getPosition(window);
            if (mouseOver(focusElements[i].button, mousePosI))
            {
                focusElements[i].button.setFillColor(sf::Color(255, 255, 255, 150));
                if (sf::Mouse::isButtonPressed(sf::Mouse::Button::Left))
                {
                    focus = i;
                    focusElements[i].button.setFillColor(sf::Color(100, 100, 100, 100));
                }
            }
            else
            {
                focusElements[i].button.setFillColor(sf::Color(100, 100, 200, 150));
            }

        }
        view.setCenter(Bodies[focus].square.getPosition());

        // Window drawing stage
        window.clear();

        // Default view (BG)
        window.setView(window.getDefaultView());
        window.draw(background);


        // Object view (bodies)
        view.zoom(zoomLevel);
        window.setView(view);
        for (int i = 0; i < n; i++)
        {
            if (Bodies[i].name == "Moon")
            {
                RenderMoon(Bodies[i].position, Bodies[i-1].position, 25.f, i, Bodies);
            }
            else if (Bodies[i].name == "Ganymede") //1st jupiter's moon
            {
                RenderMoon(Bodies[i].position, Bodies[i-1].position, 55.f, i, Bodies);
            }
            else if (Bodies[i].name == "Callisto") //2nd jupiter's moon
            {
                RenderMoon(Bodies[i].position, Bodies[i-2].position, 70.f, i, Bodies);
            }
            else if (Bodies[i].name == "Titan") // 1st saturn's moon
            {
                RenderMoon(Bodies[i].position, Bodies[i-1].position, 20.f, i, Bodies);
            }
            else
            {
                RenderPlanet(i, Bodies);
            }
            window.draw(Bodies[i].square);
        }

        // Default view (UI elements)
        window.setView(window.getDefaultView());
        window.draw(zoomSlider.track);
        window.draw(zoomSlider.button);
        for(int i = 0; i < n; i++)
        {
            window.draw(focusElements[i].button);
            window.draw(focusElements[i].text);
        }
        // Update display
        window.display();
    }
    return 0;
}
