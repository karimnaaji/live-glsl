#include "shaderparser.h"

#include "utils.h"

#include <fstream>
#include <regex>

bool ShaderParserAmalgamate(const std::string& base_path, const std::string& path, std::vector<std::string>& watches, std::string& read_file_error, std::string& amalgamate) {
    std::ifstream file;
    std::string curr_buffer;
    std::string prev_buffer;
    file.open(path.c_str());

    if (!file.is_open()) {
        read_file_error = "Unable read file at path " + path;
        return false;
    }

    watches.push_back(path);

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

bool ShaderParserParse(const std::string& base_path, const std::string& path, std::vector<std::string>& watches, std::vector<RenderPass>& render_passes, std::vector<GUIComponent>& components, std::string& read_file_error) {
    std::string amalgamate;

    if (!ShaderParserAmalgamate(base_path, path, watches, read_file_error, amalgamate)) {
        read_file_error = "Unable read file at path " + path;
        return false;
    }

    std::vector<GUIComponent> previous_components = components;
    components.clear();
    uint32_t line_number = 0;
    RenderPass* pass = nullptr;
    std::string shader_source;
    std::string prev_buffer;

    auto ReportError = [&](const std::string& error, uint32_t line_number) {
        char buffer[33];
        sprintf(buffer, "%d", line_number);
        read_file_error = error + " at line " + buffer;
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

        if (current_char < prev_buffer.length() && prev_buffer[current_char] == '@') {
            if (prev_buffer.substr(current_char + 1, current_char + 4) == "path") {

            } else if (prev_buffer.substr(current_char + 1, current_char + 8) == "pass_end") {
                pass->ShaderSource = shader_source;
                shader_source = "";
                pass = nullptr;
            } else if (prev_buffer.substr(current_char + 1, current_char + 4) == "pass") {
                render_passes.emplace_back();
                pass = &render_passes.back();

                char input_name[64] = {0};
                char output_name[64] = {0};
                uint32_t width = 0;
                uint32_t height = 0;
                std::string render_pass_args = prev_buffer.substr(current_char + 5, std::string::npos);
                int scanned = 0;
                scanned = sscanf(render_pass_args.c_str(), "(%[^,], %[^,], %d, %d)", output_name, input_name, &width, &height);
                if (scanned != 4) {
                    width = height = 0;
                    memset(input_name, 0x0, sizeof(input_name));
                    memset(output_name, 0x0, sizeof(output_name));
                    scanned = sscanf(render_pass_args.c_str(), "(%[^,], %d, %d)", output_name, &width, &height);
                    if (scanned != 3) {
                        width = height = 0;
                        memset(input_name, 0x0, sizeof(input_name));
                        memset(output_name, 0x0, sizeof(output_name));
                        scanned = sscanf(render_pass_args.c_str(), "(%[^,], %[^)])", output_name, input_name);
                        if (scanned != 2) {
                            std::string error;
                            error += "Render pass format should be @pass(output, input, width, height), @pass(output, input) or @pass(output, width, height)";
                            ReportError(error, line_number);
                            return false;
                        }
                    }
                }

                pass->Input = input_name;
                pass->Output = output_name;
                pass->Width = width;
                pass->Height = height;
                if (pass->Output == "main") {
                    pass->IsMain = true;
                }
            } else {
                GUIComponent component;
                std::string gui_component_line = prev_buffer.substr(current_char + 1, std::string::npos);
                if (!GUIComponentParse(line_number, gui_component_line, line, previous_components, component, read_file_error)) {
                    return false;
                }

                components.push_back(component);
            }
        }

        if (line[current_char] != '@') {
            shader_source += line + "\n";
        }

        prev_buffer = line;
        ++line_number;
    }

    if (render_passes.empty()) {
        render_passes.emplace_back();
        render_passes.back().ShaderSource = shader_source;
        render_passes.back().IsMain = true;
    }

    return true;
}
