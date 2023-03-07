#include "shaderparser.h"

#include "utils.h"

#include <fstream>
#include <sstream>
#include <regex>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

typedef std::function<void(const std::string&, uint32_t)> FErrorReport;

bool ShaderParserAmalgamate(const std::string& base_path, const std::string& path, std::vector<std::string>& watches, std::string& read_file_error, std::string& amalgamate) {
    std::ifstream file;

    file.open(path.c_str());

    if (!file.is_open()) {
        read_file_error = "Unable read file at path " + path;
        return false;
    }

    watches.push_back(path);

    std::string curr_buffer;
    while (!file.eof()) {
        getline(file, curr_buffer);
        uint32_t current_char = 0;

        while (isspace(curr_buffer[current_char])) {
            ++current_char;
        }

        std::regex include_regex("#include\\s+\"([^\"]+)\"");
        std::smatch include_match;

        if (std::regex_search(curr_buffer, include_match, include_regex)) {
            std::string include = base_path + PATH_DELIMITER + std::string(include_match[1]);

            if (!ShaderParserAmalgamate(ExtractBasePath(include), include, watches, read_file_error, amalgamate)) {
                return false;
            }
        } else {
            amalgamate += curr_buffer + '\n';
        }
    }

    file.close();

    return true;
}

bool ShaderParserParseTextures(const std::string& base_path, const std::string& prev_line, const std::string& line, uint32_t current_char, uint32_t line_number, FErrorReport report_error, std::vector<Texture>& textures) {
    int last_parenthesis_index = -1;
    int first_parenthesis_index = -1;

    std::string path = prev_line.substr(current_char + 5, std::string::npos);

    for (int i = 0; i < path.size(); ++i) {
        if (path[i] == '(') first_parenthesis_index = i;
        if (path[i] == ')') last_parenthesis_index = i;
    }

    if (last_parenthesis_index <= first_parenthesis_index) {
        report_error("Path format should be @path(path): " + path, line_number);
        return false;
    }

    path = path.substr(first_parenthesis_index + 1, last_parenthesis_index - 1);

    std::vector<std::string> uniform_tokens = SplitString(line,  ' ');
    if (uniform_tokens.size() != 3) {
        return false;
    }

    std::string uniform_type = uniform_tokens[1];
    if (uniform_type != "sampler2D") {
        report_error("@path syntax is only supported on sampler2D uniform types", line_number);
        return false;
    }

    stbi_set_flip_vertically_on_load(true);

    Texture texture;
    std::string full_path = base_path + PATH_DELIMITER + path;
    texture.Data = stbi_load(full_path.c_str(), &texture.Width, &texture.Height, &texture.Channels, 4);

    if (!texture.Data) {
        report_error("Failed to load texture at path " + path, line_number);
        return false;
    }

    textures.push_back(texture);

    return true;
}

bool ShaderParserParseRenderPass(const std::string& prev_line, const std::string& line, uint32_t current_char, uint32_t line_number, FErrorReport report_error, std::vector<Texture>& textures, RenderPass& pass) {
    char input[64] = {0};
    char output[64] = {0};

    uint32_t width = 0;
    uint32_t height = 0;

    std::string args = prev_line.substr(current_char + 5, std::string::npos);

    int scanned = sscanf(args.c_str(), "(%[^,], %[^,], %d, %d)", output, input, &width, &height);
    if (scanned != 4) {
        width = height = 0;

        memset(input, 0x0, sizeof(input));
        memset(output, 0x0, sizeof(output));

        scanned = sscanf(args.c_str(), "(%[^,], %d, %d)", output, &width, &height);

        if (scanned != 3) {
            width = height = 0;

            memset(input, 0x0, sizeof(input));
            memset(output, 0x0, sizeof(output));

            scanned = sscanf(args.c_str(), "(%[^,], %[^)])", output, input);

            if (scanned != 2) {
                report_error("Render pass format should be @pass(output, input, width, height), @pass(output, input) or @pass(output, width, height)", line_number);
                return false;
            }
        }
    }

    pass.Input = input;
    pass.Output = output;
    pass.Width = width;
    pass.Height = height;
    pass.Textures = textures;

    if (pass.Output == "main") {
        pass.IsMain = true;
    }

    textures.clear();

    return true;
}

bool ShaderParserParse(const std::string& base_path, const std::string& path, std::vector<std::string>& watches, std::vector<RenderPass>& render_passes, std::vector<GUIComponent>& components, std::string& read_file_error) {
    std::string amalgamate;

    if (!ShaderParserAmalgamate(base_path, path, watches, read_file_error, amalgamate)) {
        return false;
    }

    std::vector<GUIComponent> previous_components = components;
    components.clear();

    uint32_t line_number = 0;
    std::string source;
    std::string prev_line;
    std::vector<Texture> textures;
    RenderPass* pass = nullptr;

    auto report_error = [&](const std::string& error, uint32_t line_number) {
        read_file_error = error + " at line " + std::to_string(line_number);
    };

    for (size_t i = 0; i < amalgamate.size(); ++i) {
        std::string line;

        while (amalgamate[i] != '\n') {
            line += amalgamate[i];
            ++i;
        }

        uint32_t current_char = 0;
        while (isspace(line[current_char])) {
            ++current_char;
        }

        if (current_char < prev_line.length() && prev_line[current_char] == '@') {
            if (prev_line.substr(current_char + 1, current_char + 4) == "path") {
                if (!ShaderParserParseTextures(base_path, prev_line, line, current_char, line_number, report_error, textures)) {
                    return false;
                }
            } else if (prev_line.substr(current_char + 1, current_char + 8) == "pass_end") {
                if (!pass) {
                    report_error("@pass_end was declared without @pass", line_number);
                    return false;
                }

                pass->ShaderSource = std::move(source);
                source.clear();
                pass = nullptr;
            } else if (prev_line.substr(current_char + 1, current_char + 4) == "pass") {
                RenderPass new_pass;
                if (!ShaderParserParseRenderPass(prev_line, line, current_char, line_number, report_error, textures, new_pass)) {
                    return false;
                }

                render_passes.push_back(new_pass);
                pass = &render_passes.back();
            } else {
                GUIComponent component;
                std::string gui_component_line = prev_line.substr(current_char + 1, std::string::npos);
                if (!GUIComponentParse(line_number, gui_component_line, line, previous_components, component, read_file_error)) {
                    return false;
                }

                components.push_back(component);
            }
        }

        if (line[current_char] != '@') {
            source += line + "\n";
        }

        prev_line = line;
        ++line_number;
    }

    if (render_passes.empty()) {
        render_passes.emplace_back();
        render_passes.back().ShaderSource = std::move(source);
        render_passes.back().IsMain = true;
        render_passes.back().Textures = textures;
    }

    return true;
}
