#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cstdint>
#include <unordered_map>
#include "logging.h"

void parseConfigLine(std::string line, std::unordered_map<std::string, std::string>& options);

enum class CustomAlign {
    LEFT,
    RIGHT
};

struct CustomCol {
    std::string text;
    CustomAlign align = CustomAlign::LEFT;
    uint32_t color = 0xFFFFFFFF;
    std::string font_style = "";
    float opacity = 1.0f;
    bool is_separator = false;
    bool use_brand_color = false;
};

struct CustomRow {
    bool is_separator = false;
    std::vector<CustomCol> cols;
};

struct CustomWindow {
    bool is_custom_config = false;
    std::string anchor = "top-right";
    int offset_x = 24;
    int offset_y = 24;
    int width = 300;
    int round = 8;
    uint32_t background = 0xD0101010;
};

struct CustomHUD {
    CustomWindow window;
    std::vector<CustomRow> rows;
};

inline CustomHUD g_custom_hud;

inline uint32_t parse_hex_color(std::string hex) {
    if (hex.empty()) return 0xFFFFFFFF;
    if (hex[0] == '#') hex = hex.substr(1);

    unsigned int r = 255, g = 255, b = 255, a = 255;
    if (hex.length() == 6) {
        sscanf(hex.c_str(), "%02x%02x%02x", &r, &g, &b);
    } else if (hex.length() == 8) {
        sscanf(hex.c_str(), "%02x%02x%02x%02x", &r, &g, &b, &a);
    }
    return (a << 24) | (b << 16) | (g << 8) | r;
}

inline std::string trim_str(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

inline bool load_custom_hud_config(const std::string& filepath, std::unordered_map<std::string, std::string>& options) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    std::string line;
    bool has_custom_tags = false;
    while (std::getline(file, line)) {
        if (line.find("[window]") != std::string::npos || line.find("[layout]") != std::string::npos) {
            has_custom_tags = true;
            break;
        }
    }

    if (!has_custom_tags) return false;

    file.clear();
    file.seekg(0);

    g_custom_hud = CustomHUD();
    g_custom_hud.window.is_custom_config = true;

    enum Section { GLOBAL, WINDOW, LAYOUT } current_section = GLOBAL;
    CustomRow current_row;
    bool inside_row = false;

    while (std::getline(file, line)) {
        std::string trimmed = trim_str(line);
        if (trimmed.empty() || trimmed[0] == '#' || trimmed.rfind("//", 0) == 0) {
            continue;
        }

        if (trimmed == "[window]") {
            current_section = WINDOW;
            continue;
        }
        if (trimmed == "[layout]") {
            current_section = LAYOUT;
            continue;
        }

        if (current_section == GLOBAL) {
            parseConfigLine(trimmed, options);
        } else if (current_section == WINDOW) {
            size_t eq = trimmed.find('=');
            if (eq != std::string::npos) {
                std::string key = trim_str(trimmed.substr(0, eq));
                std::string val = trim_str(trimmed.substr(eq + 1));
                if (key == "anchor") {
                    g_custom_hud.window.anchor = val;
                } else if (key == "offset_x") {
                    g_custom_hud.window.offset_x = std::stoi(val);
                } else if (key == "offset_y") {
                    g_custom_hud.window.offset_y = std::stoi(val);
                } else if (key == "width") {
                    g_custom_hud.window.width = std::stoi(val);
                } else if (key == "round") {
                    g_custom_hud.window.round = std::stoi(val);
                } else if (key == "background") {
                    g_custom_hud.window.background = parse_hex_color(val);
                }
            }
        } else if (current_section == LAYOUT) {
            if (trimmed == "separator") {
                CustomRow sep_row;
                sep_row.is_separator = true;
                g_custom_hud.rows.push_back(sep_row);
                continue;
            }

            if (trimmed.rfind("row", 0) == 0 && trimmed.find('{') != std::string::npos) {
                inside_row = true;
                current_row = CustomRow();
                continue;
            }

            if (trimmed == "}" && inside_row) {
                inside_row = false;
                g_custom_hud.rows.push_back(current_row);
                continue;
            }

            if (inside_row && trimmed.rfind("col", 0) == 0) {
                CustomCol col;
                size_t start = trimmed.find('{');
                size_t end = trimmed.rfind('}');
                if (start != std::string::npos && end != std::string::npos && end > start) {
                    std::string props_str = trimmed.substr(start + 1, end - start - 1);
                    std::stringstream ss(props_str);
                    std::string token;
                    while (std::getline(ss, token, ',')) {
                        size_t eq = token.find('=');
                        if (eq != std::string::npos) {
                            std::string k = trim_str(token.substr(0, eq));
                            std::string v = trim_str(token.substr(eq + 1));
                            if (k == "text") {
                                if (v.size() >= 2 && v.front() == '"' && v.back() == '"') {
                                    v = v.substr(1, v.size() - 2);
                                }
                                col.text = v;

                                if (v.find("{fps}") != std::string::npos) {
                                    options["fps"] = "";
                                }
                                if (v.find("{frametime}") != std::string::npos) {
                                    options["frametime"] = "";
                                }
                                if (v.find("{ram}") != std::string::npos) {
                                    options["ram"] = "";
                                }
                                if (v.find("{gpu_") != std::string::npos) {
                                    options["gpu_stats"] = "";
                                    if (v.find("{gpu_temp}") != std::string::npos) {
                                        options["gpu_temp"] = "";
                                    }
                                    if (v.find("{gpu_core_clock}") != std::string::npos) {
                                        options["gpu_core_clock"] = "";
                                    }
                                    if (v.find("{gpu_power}") != std::string::npos) {
                                        options["gpu_power"] = "";
                                    }
                                    if (v.find("{gpu_load}") != std::string::npos) {
                                        options["gpu_load_change"] = "";
                                    }
                                }
                            } else if (k == "color") {
                                if (v == "#brand" || v == "brand") {
                                    col.use_brand_color = true;
                                } else {
                                    col.color = parse_hex_color(v);
                                }
                            } else if (k == "align") {
                                if (v == "right") {
                                    col.align = CustomAlign::RIGHT;
                                }
                            } else if (k == "font") {
                                col.font_style = v;
                            } else if (k == "opacity") {
                                col.opacity = std::stof(v);
                            } else if (k == "type") {
                                if (v == "separator") {
                                    col.is_separator = true;
                                }
                            }
                        }
                    }
                    current_row.cols.push_back(col);
                }
            }
        }
    }

    SPDLOG_INFO("Custom layout detected and parsed! Injected telemetry flags.");
    return true;
}
