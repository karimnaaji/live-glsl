#pragma once

#include <string>
#include <vector>

#include "renderpass.h"
#include "gui.h"

bool ShaderParserParse(const std::string& base_path, const std::string& path, std::vector<std::string>& watches, std::vector<RenderPass>& render_passes, std::vector<GUIComponent>& components, std::string& read_file_error);