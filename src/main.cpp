#include <SFML/Graphics.hpp>
#include <mutex>
#include <string>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <imgui-SFML.h>
#include <optional>
#include <thread>
#include <vector>
#include <cmath>

#include "parser.hpp"

int main() {
    sf::ContextSettings settings;
    settings.antiAliasingLevel = 8.0;
    sf::RenderWindow window(sf::VideoMode({1600, 900}), "Graphing Calculator", sf::Style::Default, sf::State::Windowed, settings);
    window.setVerticalSyncEnabled(true);

    ImGui::SFML::Init(window, true);

    ExpressionParser parser;

    sf::Vector2f origin(window.getSize().x / 2.0f, window.getSize().y / 2.0f);
    float gridSpacing = 40.0f;

    sf::VertexArray gridLines(sf::PrimitiveType::Lines);
    // Vertical grid lines
    for (float x = origin.x; x < window.getSize().x; x += gridSpacing) {
        gridLines.append({{x, 0}, sf::Color(80, 80, 80)});
        gridLines.append({{x, static_cast<float>(window.getSize().y)}, sf::Color(80, 80, 80)});
    }
    for (float x = origin.x - gridSpacing; x > 0; x -= gridSpacing) {
        gridLines.append({{x, 0}, sf::Color(80, 80, 80)});
        gridLines.append({{x, static_cast<float>(window.getSize().y)}, sf::Color(80, 80, 80)});
    }

    // Horizontal grid lines
    for (float y = origin.y; y < window.getSize().y; y += gridSpacing) {
        gridLines.append({{0, y}, sf::Color(80, 80, 80)});
        gridLines.append({{static_cast<float>(window.getSize().x), y}, sf::Color(80, 80, 80)});
    }
    for (float y = origin.y - gridSpacing; y > 0; y -= gridSpacing) {
        gridLines.append({{0, y}, sf::Color(80, 80, 80)});
        gridLines.append({{static_cast<float>(window.getSize().x), y}, sf::Color(80, 80, 80)});
    }

    sf::VertexArray axes(sf::PrimitiveType::Lines, 4);
    axes[0].position = {0, origin.y};
    axes[1].position = {static_cast<float>(window.getSize().x), origin.y};
    axes[2].position = {origin.x, 0};
    axes[3].position = {origin.x, static_cast<float>(window.getSize().y)};

    sf::Clock deltaClock;

    float color[3] = { 1.0f, 1.0f, 1.0f };
    std::string text;
    std::mutex functionsMutex;
    std::vector<sf::VertexArray> functions;
    std::vector<sf::Color> functionColors;
    bool plottingComplete = true;

    while (window.isOpen()) {
        while (const std::optional event = window.pollEvent()) {
            ImGui::SFML::ProcessEvent(window, *event);
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        ImGui::SFML::Update(window, deltaClock.restart());
        ImGui::Begin("Plot function");
        ImGui::InputText("Function", &text);
        ImGui::ColorEdit3("Function color", color);

        if (ImGui::Button("Plot function") && plottingComplete) {
            plottingComplete = false;
            sf::Color functionColor(color[0] * 255, color[1] * 255, color[2] * 255);
            std::string functionText = text; // Copy the text to avoid race conditions
            std::thread plotThread([&]() {
                sf::VertexArray newFunction(sf::PrimitiveType::LineStrip);
                for (float x = -50; x < 50; x += 0.01) {
                    double yValue = parser.parseExpression(functionText, 'x', x);

                    if (!std::isnan(yValue) && !std::isinf(yValue)) {
                        sf::Vertex v{{x * 20.0f + static_cast<float>(window.getSize().x) / 2,
                                        static_cast<float>(-1.0f * yValue * 20.0f + static_cast<float>(window.getSize().y) / 2)},
                                        functionColor};
                        newFunction.append(v);
                    }
                }
                functionsMutex.lock();
                functions.push_back(newFunction);
                functionColors.push_back(functionColor);
                functionsMutex.unlock();
                plottingComplete = true;
            });
            plotThread.detach();
        }

        if (ImGui::Button("Clear Functions")) {
            functionsMutex.lock();
            functions.clear();
            functionColors.clear();
            functionsMutex.unlock();
        }

        ImGui::End();

        window.clear();
        window.draw(gridLines);
        window.draw(axes);
        functionsMutex.lock();
        for (auto& function : functions)
            window.draw(function);
        functionsMutex.unlock();
        ImGui::SFML::Render(window);
        window.display();
    }

    ImGui::SFML::Shutdown(window);
    return 0;
}
