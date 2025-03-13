#include <SFML/Graphics.hpp>
#include <iostream>
#include "imgui.h"
#include "imgui-SFML.h"
#include <cmath>
#include <cstdlib>
#include <ctime>

const float GRAVITY = 9.81f;
const float SCALE = 10.f; 
const float WINDOW_WIDTH = 800.f;
const float WINDOW_HEIGHT = 600.f;
const float AIR_RESISTANCE = 0.995f; 
const float RESTITUTION = 0.75f; 

struct Projectile {
    sf::CircleShape shape;
    sf::Vector2f velocity;
    sf::Vector2f position;
    bool isLaunched = false;
    bool canLaunch = true;
    sf::RectangleShape angleIndicator;

    Projectile() {
        shape.setRadius(5.f);
        shape.setFillColor(sf::Color::Red);
        position = { 50.f, 450.f }; 
        shape.setPosition(position);
        angleIndicator.setSize({ 30.f, 2.f });
        angleIndicator.setFillColor(sf::Color::Yellow);
        angleIndicator.setOrigin(0.f, 1.f);
        updateIndicator(45.f);
    }

    void updateIndicator(float angle) {
        if (isLaunched) return;
        angleIndicator.setPosition(position.x + shape.getRadius(), position.y + shape.getRadius());
        angleIndicator.setRotation(-angle);
    }

    void launch(float angle, float speed) {
        if (!canLaunch) return;
        velocity.x = speed * std::cos(angle * 3.14159f / 180.f);
        velocity.y = -speed * std::sin(angle * 3.14159f / 180.f);
        isLaunched = true;
        canLaunch = false;
    }

    void update(float dt) {
        if (!isLaunched) return;
        velocity.y += GRAVITY * dt; 
        velocity *= AIR_RESISTANCE;
        position += velocity * dt * SCALE;

        if (position.x <= 0.f || position.x >= WINDOW_WIDTH - shape.getRadius() * 2) {
            velocity.x = -velocity.x * RESTITUTION;
        }

        if (position.y <= 0.f || position.y >= WINDOW_HEIGHT - shape.getRadius() * 2) {
            velocity.y = -velocity.y * RESTITUTION;
        }

        if (std::hypot(velocity.x, velocity.y) < 1.f) {
            isLaunched = false;
            position = { 50.f, 450.f };
            canLaunch = true;
        }

        shape.setPosition(position);
    }
};

struct Basket {
    sf::RectangleShape shape;
    sf::Vector2f position;

    Basket() {
        shape.setSize({ 40.f, 10.f });
        shape.setFillColor(sf::Color::White);
        resetPosition();
    }

    void resetPosition() {
        position.x = static_cast<float>(std::rand() % static_cast<int>(WINDOW_WIDTH - shape.getSize().x));
        position.y = static_cast<float>(std::rand() % static_cast<int>(WINDOW_HEIGHT / 2));
        shape.setPosition(position);
        shape.setRotation(static_cast<float>(std::rand() % 180 - 90));
    }

    bool checkCollision(const Projectile& projectile) {
        return shape.getGlobalBounds().contains(projectile.shape.getPosition());
    }
};

int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));
    sf::RenderWindow window(sf::VideoMode(800, 600), "Launch!!!");
    window.setFramerateLimit(60);

    ImGui::SFML::Init(window);
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.FontGlobalScale = 1.5f;

    Projectile ball;
    Basket basket;
    sf::Clock clock;
    float angle = 45.f, speed = 50.f;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(window, event);
            if (event.type == sf::Event::Closed) window.close();
        }

        float dt = clock.restart().asSeconds();
        ball.update(dt);

        if (basket.checkCollision(ball)) {
            basket.resetPosition();
            ball.isLaunched = false;
            ball.position = { 50.f, 450.f };
            ball.shape.setPosition(ball.position);
            ball.canLaunch = true;
        }

        ImGui::SFML::Update(window, sf::seconds(dt));
        ImGui::Begin("Controls");
        ImGui::SliderFloat("Angle", &angle, -90.f, 90.f);
        ImGui::SliderFloat("Speed", &speed, 10.f, 100.f);
        if (ImGui::Button("Launch")) ball.launch(angle, speed);
        ImGui::End();

        ball.updateIndicator(angle);

        window.clear(sf::Color::Black);
        if (!ball.isLaunched) window.draw(ball.angleIndicator);
        window.draw(ball.shape);
        window.draw(basket.shape);
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown();
    return 0;
}
