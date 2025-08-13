#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <cmath>
#include "fractal.h" // sudah ada generate_serial, generate_parallel, generate_julia_serial, generate_julia_parallel, generate_gpu

#define WINDOW_W 1280
#define WINDOW_H 720
#define UI_W 320
#define FRACTAL_W (WINDOW_W - UI_W)
#define FRACTAL_H WINDOW_H

struct AppState {
    int width = FRACTAL_W;
    int height = FRACTAL_H;
    int max_iter = 1000;
    double center_x = -0.5;
    double center_y = 0.0;
    double scale = 3.5;
    double time_serial = 0.0;
    double time_parallel = 0.0;
    double ratio = 0.0;
    double time_gpu = 0.0;
    double ratio_gpu_cpu = 0.0;
    double ratio_gpu_serial = 0.0;
    bool juliaMode = false;
    double c_real = 0.285;
    double c_imag = 0.01;
};

struct InputField {
    sf::FloatRect rect;
    std::string text;
    bool focused = false;
};

struct Button {
    sf::FloatRect rect;
    std::string label;
    bool pressed = false;
    sf::Color normal;
    sf::Color active;
};

static void centerText(sf::Text &text, sf::FloatRect area) {
    sf::FloatRect bounds = text.getLocalBounds();
    text.setPosition(area.left + (area.width - bounds.width) / 2.0f,
                     area.top + (area.height - bounds.height) / 2.0f - bounds.top);
}

int main() {
    sf::RenderWindow window(sf::VideoMode(WINDOW_W, WINDOW_H), "Mandelbrot / Julia Explorer");
    window.setFramerateLimit(60);

    sf::Font font;
    font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf");

    AppState state;
    sf::Texture fractal_tex;
    sf::Sprite fractal_sprite;

    bool popupActive = false;
    std::string fileNameInput;

    auto draw_image = [&](std::vector<unsigned char>& imgData) {
        sf::Image imgFull;
        imgFull.create(state.width, state.height);
        for (int y = 0; y < state.height; y++) {
            for (int x = 0; x < state.width; x++) {
                int idx = (y * state.width + x) * 3;
                imgFull.setPixel(x, y, sf::Color(imgData[idx], imgData[idx+1], imgData[idx+2]));
            }
        }
        fractal_tex.loadFromImage(imgFull);
        fractal_sprite.setTexture(fractal_tex, true);
        float scaleX = FRACTAL_W / static_cast<float>(state.width);
        float scaleY = FRACTAL_H / static_cast<float>(state.height);
        float scale = std::min(scaleX, scaleY);
        fractal_sprite.setScale(scale, scale);
        fractal_sprite.setPosition(0, 0);
    };

    auto regenerate_full = [&](bool saveFull = false, const std::string &saveName = "") {
        std::vector<unsigned char> imgData(state.width * state.height * 3);

        // Serial
        {
            sf::Clock clk;
            if (!state.juliaMode)
                generate_serial(imgData.data(), state.width, state.height, state.max_iter, state.center_x, state.center_y, state.scale);
            else
                generate_julia_serial(imgData.data(), state.width, state.height, state.max_iter, state.center_x, state.center_y, state.scale, state.c_real, state.c_imag);
            state.time_serial = clk.getElapsedTime().asSeconds();
        }
        // Parallel
        {
            sf::Clock clk;
            if (!state.juliaMode)
                generate_parallel(imgData.data(), state.width, state.height, state.max_iter, state.center_x, state.center_y, state.scale);
            else
                generate_julia_parallel(imgData.data(), state.width, state.height, state.max_iter, state.center_x, state.center_y, state.scale, state.c_real, state.c_imag);
            state.time_parallel = clk.getElapsedTime().asSeconds();
        }
        if (state.time_parallel > 0)
            state.ratio = state.time_serial / state.time_parallel;

        if (saveFull && !saveName.empty()) {
            char path[512];
            snprintf(path, sizeof(path), "image/%s.png", saveName.c_str());
            save_png(path, imgData.data(), state.width, state.height);
        }

        draw_image(imgData);
    };

    auto regenerate_gpu = [&]() {
        std::vector<unsigned char> imgData(state.width * state.height * 3);
        sf::Clock clk;
        generate_gpu(imgData.data(), state.width, state.height, state.max_iter,
                     state.center_x, state.center_y, state.scale,
                     state.juliaMode, state.c_real, state.c_imag);
        state.time_gpu = clk.getElapsedTime().asSeconds();
        if (state.time_parallel > 0) state.ratio_gpu_cpu = state.time_parallel / state.time_gpu;
        if (state.time_serial > 0) state.ratio_gpu_serial = state.time_serial / state.time_gpu;
        draw_image(imgData);
    };

    InputField fieldWidth{{FRACTAL_W + 20, 40, UI_W - 40, 32}, std::to_string(state.width)};
    InputField fieldHeight{{FRACTAL_W + 20, 90, UI_W - 40, 32}, std::to_string(state.height)};
    Button btnGenerate{{FRACTAL_W + 20, 190, UI_W - 40, 40}, "Generate CPU", false, {66,133,244}, {46,92,184}};
    Button btnGenerateGPU{{FRACTAL_W + 20, 240, UI_W - 40, 40}, "Generate GPU", false, {255,165,0}, {200,120,0}};
    Button btnSave{{FRACTAL_W + 20, 290, UI_W - 40, 40}, "Save", false, {46,204,113}, {36,150,83}};
    Button btnToggle{{FRACTAL_W + 20, 340, UI_W - 40, 40}, "Mode: Mandelbrot", false, {155,89,182}, {115,59,142}};

    regenerate_full();

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            sf::Vector2f mpos(event.mouseButton.x, event.mouseButton.y);

            if (popupActive) {
                if (event.type == sf::Event::TextEntered) {
                    if (event.text.unicode == 8) {
                        if (!fileNameInput.empty()) fileNameInput.pop_back();
                    } else if (std::isalnum(event.text.unicode) || event.text.unicode == '_') {
                        fileNameInput += static_cast<char>(event.text.unicode);
                    }
                } else if (event.type == sf::Event::MouseButtonPressed) {
                    sf::FloatRect rectOk(FRACTAL_W + 20, 200, 80, 32);
                    sf::FloatRect rectCancel(FRACTAL_W + 120, 200, 80, 32);
                    if (rectOk.contains(mpos)) {
                        regenerate_full(true, fileNameInput);
                        popupActive = false;
                    }
                    if (rectCancel.contains(mpos)) {
                        popupActive = false;
                    }
                }
                continue;
            }

            if (event.type == sf::Event::MouseButtonPressed) {
                fieldWidth.focused = fieldWidth.rect.contains(mpos);
                fieldHeight.focused = fieldHeight.rect.contains(mpos);
                if (btnGenerate.rect.contains(mpos)) btnGenerate.pressed = true;
                if (btnGenerateGPU.rect.contains(mpos)) btnGenerateGPU.pressed = true;
                if (btnSave.rect.contains(mpos)) btnSave.pressed = true;
                if (btnToggle.rect.contains(mpos)) btnToggle.pressed = true;
            }

            if (event.type == sf::Event::MouseButtonReleased) {
                if (btnGenerate.pressed) {
                    btnGenerate.pressed = false;
                    state.width = std::stoi(fieldWidth.text);
                    state.height = std::stoi(fieldHeight.text);
                    regenerate_full();
                }
                if (btnGenerateGPU.pressed) {
                    btnGenerateGPU.pressed = false;
                    state.width = std::stoi(fieldWidth.text);
                    state.height = std::stoi(fieldHeight.text);
                    regenerate_gpu();
                }
                if (btnSave.pressed) {
                    btnSave.pressed = false;
                    fileNameInput.clear();
                    popupActive = true;
                }
                if (btnToggle.pressed) {
                    btnToggle.pressed = false;
                    state.juliaMode = !state.juliaMode;
                    btnToggle.label = std::string("Mode: ") + (state.juliaMode ? "Julia" : "Mandelbrot");
                    regenerate_full();
                }
            }

            if (event.type == sf::Event::TextEntered) {
                if (std::isdigit(event.text.unicode) || event.text.unicode == 8) {
                    auto handle = [&](InputField &f) {
                        if (f.focused) {
                            if (event.text.unicode == 8) {
                                if (!f.text.empty()) f.text.pop_back();
                            } else {
                                f.text += static_cast<char>(event.text.unicode);
                            }
                        }
                    };
                    handle(fieldWidth);
                    handle(fieldHeight);
                }
            }
        }

        window.clear(sf::Color(20,20,20));
        window.draw(fractal_sprite);

        sf::RectangleShape panel(sf::Vector2f(UI_W, WINDOW_H));
        panel.setFillColor(sf::Color(30,30,30));
        panel.setPosition(FRACTAL_W,0);
        window.draw(panel);

        auto drawField = [&](const InputField &f, const std::string &label) {
            sf::RectangleShape box(sf::Vector2f(f.rect.width, f.rect.height));
            box.setPosition(f.rect.left, f.rect.top);
            box.setFillColor(f.focused ? sf::Color(60, 60, 60) : sf::Color(40, 40, 40));
            window.draw(box);
            sf::Text txt(label + ": " + f.text, font, 16);
            txt.setFillColor(sf::Color::White);
            txt.setPosition(f.rect.left + 6, f.rect.top + 6);
            window.draw(txt);
        };

        drawField(fieldWidth, "Width");
        drawField(fieldHeight, "Height");

        auto drawButton = [&](const Button &b) {
            sf::RectangleShape btn(sf::Vector2f(b.rect.width, b.rect.height));
            btn.setPosition(b.rect.left, b.rect.top);
            btn.setFillColor(b.pressed ? b.active : b.normal);
            window.draw(btn);
            sf::Text btnTxt(b.label, font, 18);
            btnTxt.setFillColor(sf::Color::White);
            centerText(btnTxt, b.rect);
            window.draw(btnTxt);
        };

        drawButton(btnGenerate);
        drawButton(btnGenerateGPU);
        drawButton(btnSave);
        drawButton(btnToggle);

        std::ostringstream oss;
        oss << "Serial: " << std::fixed << std::setprecision(3) << state.time_serial << "s\n"
            << "Parallel: " << state.time_parallel << "s (" << state.ratio << "x)\n"
            << "GPU: " << state.time_gpu << "s\n"
            << "GPU vs CPU: " << state.ratio_gpu_cpu << "x\n"
            << "GPU vs Serial: " << state.ratio_gpu_serial << "x";
        sf::Text statTxt(oss.str(), font, 14);
        statTxt.setFillColor(sf::Color(200, 200, 200));
        statTxt.setPosition(FRACTAL_W + 20, btnToggle.rect.top + btnToggle.rect.height + 20);
        window.draw(statTxt);

        window.display();
    }
}

