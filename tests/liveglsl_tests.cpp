#include "arguments.h"
#include "utils.h"
#include "filewatcher.h"
#include "gui.h"
#include "renderpass.h"
#include "shaderparser.h"
#include "utest.h"

#include <string.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

#define T(b) EXPECT_TRUE(b)
#define TSTR(s0, s1) EXPECT_TRUE(0 == strcmp(s0,s1))

UTEST(arguments, parse_0) {
    Arguments arguments;

    const char* args[] = {
        "liveglsl",
        "--input",
        "shader.frag",
        "--width",
        "300",
        "--height",
        "200"
    };

    T(ArgumentsParse(ARRAY_LENGTH(args), args, arguments));
    TSTR(arguments.Input.c_str(), "shader.frag");
    T(arguments.Width == 300);
    T(arguments.Height == 200);
}

UTEST(arguments, parse_1) {
    Arguments arguments;

    const char* args[] = {
        "liveglsl",
        "shader.frag",
    };

    T(!ArgumentsParse(ARRAY_LENGTH(args), args, arguments));
}

void OnFileChanged(void* user_data, const char* file_path) {
    bool* callback_called = (bool*)user_data;
    *callback_called = true;
}

UTEST(filewatcher, add_watch_modify_0) {
    const char* file_path = "test_file.txt";
    std::ofstream file(file_path);
    file << "File dummy content";
    file.close();

    bool callback_called = false;
    HFileWatcher watcher = FileWatcherCreate(OnFileChanged, &callback_called, 100);
    FileWatcherAddWatch(watcher, file_path);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::ofstream modified_file(file_path);
    modified_file << "Modified content";
    modified_file.close();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    T(callback_called);

    FileWatcherRemoveWatch(watcher, file_path);
    FileWatcherDestroy(watcher);

    std::remove(file_path);
}

UTEST(filewatcher, add_watch_modify_1) {
    const char* file_path = "test_file.txt";
    std::ofstream file(file_path);
    file << "File dummy content";
    file.close();

    bool callback_called = false;
    HFileWatcher watcher = FileWatcherCreate(OnFileChanged, &callback_called, 16);
    FileWatcherAddWatch(watcher, file_path);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    T(!callback_called);

    FileWatcherRemoveWatch(watcher, file_path);
    FileWatcherDestroy(watcher);

    std::remove(file_path);
}

UTEST(filewatcher, add_watch_modify_2) {
    const char* file_path = "test_file.txt";
    std::ofstream file(file_path);
    file << "File dummy content";
    file.close();

    bool callback_called = false;
    HFileWatcher watcher = FileWatcherCreate(OnFileChanged, &callback_called, 100);
    FileWatcherAddWatch(watcher, file_path);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    FileWatcherRemoveWatch(watcher, file_path);

    std::ofstream modified_file(file_path);
    modified_file << "Modified content";
    modified_file.close();

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    T(!callback_called);

    FileWatcherDestroy(watcher);

    std::remove(file_path);
}

UTEST(gui, component_parse_invalid) {
    std::string line, uniform_line, error;
    GUIComponent component;

    line = "invalid_gui(-1, 2.5)";
    uniform_line = "uniform float v0;";
    T(!GUIComponentParse(0, line, uniform_line, {}, component, error));
    TSTR(error.c_str(), "Invalid GUI type name 'invalid_gui' at line 0");

    line = "invalid_gui()";
    uniform_line = "uniform float v0;";
    T(!GUIComponentParse(0, line, uniform_line, {}, component, error));
    TSTR(error.c_str(), "Invalid GUI type name 'invalid_gui' at line 0");
}

UTEST(gui, component_parse_slider_1) {
    std::string line, uniform_line, error;
    GUIComponent component;

    line = "slider1(-1, 2.5)";
    uniform_line = "uniform float v0;";
    T(GUIComponentParse(0, line, uniform_line, {}, component, error));
    T(component.SliderRange.Start == -1);
    T(component.SliderRange.End == 2.5);
    T(component.UniformType == EGUIUniformTypeFloat);
    T(error.empty());

    line = "slider1()";
    uniform_line = "uniform float v0;";
    T(!GUIComponentParse(0, line, uniform_line, {}, component, error));
    TSTR(error.c_str(), "Invalid format for GUI component data '()'\nFormat should be @slider1(start_range, end_range) at line 0");

    line = "slider1(5.0)";
    uniform_line = "uniform float v0;";
    T(!GUIComponentParse(2, line, uniform_line, {}, component, error));
    TSTR(error.c_str(), "Invalid format for GUI component data '(5.0)'\nFormat should be @slider1(start_range, end_range) at line 2");

    line = "slider1(-1, 2.5)";
    for (const auto& datatype : {"vec2", "vec3"}) {
        uniform_line = "uniform " + std::string(datatype) + " v0;";
        T(!GUIComponentParse(5, line, uniform_line, {}, component, error));
        TSTR(error.c_str(), "GUI component count does not match uniform component count at line 5");
    }
}

UTEST(gui, component_parse_slider_2) {
    std::string line, uniform_line, error;
    GUIComponent component;

    line = "slider2(-1, 2.5)";
    uniform_line = "uniform vec2 v0;";
    T(GUIComponentParse(0, line, uniform_line, {}, component, error));
    T(component.SliderRange.Start == -1);
    T(component.SliderRange.End == 2.5);
    T(component.UniformType == EGUIUniformTypeVec2);
    T(error.empty());

    line = "slider2()";
    uniform_line = "uniform vec2 v0;";
    T(!GUIComponentParse(0, line, uniform_line, {}, component, error));
    TSTR(error.c_str(), "Invalid format for GUI component data '()'\nFormat should be @slider2(start_range, end_range) at line 0");

    line = "slider2(5.0)";
    uniform_line = "uniform vec v0;";
    T(!GUIComponentParse(2, line, uniform_line, {}, component, error));
    TSTR(error.c_str(), "Invalid format for GUI component data '(5.0)'\nFormat should be @slider2(start_range, end_range) at line 2");

    line = "slider2(-1, 2.5)";
    for (const auto& datatype : {"float", "vec3"}) {
        uniform_line = "uniform " + std::string(datatype) + " v0;";
        T(!GUIComponentParse(5, line, uniform_line, {}, component, error));
        TSTR(error.c_str(), "GUI component count does not match uniform component count at line 5");
    }
}

UTEST(gui, component_parse_slider_3) {
    std::string line, uniform_line, error;
    GUIComponent component;

    line = "slider3(-1, 2.5)";
    uniform_line = "uniform vec3 v0;";
    T(GUIComponentParse(0, line, uniform_line, {}, component, error));
    T(component.SliderRange.Start == -1);
    T(component.SliderRange.End == 2.5);
    T(component.UniformType == EGUIUniformTypeVec3);
    T(error.empty());

    line = "slider3()";
    uniform_line = "uniform vec3 v0;";
    T(!GUIComponentParse(0, line, uniform_line, {}, component, error));
    TSTR(error.c_str(), "Invalid format for GUI component data '()'\nFormat should be @slider3(start_range, end_range) at line 0");

    line = "slider3(5.0)";
    uniform_line = "uniform vec3 v0;";
    T(!GUIComponentParse(2, line, uniform_line, {}, component, error));
    TSTR(error.c_str(), "Invalid format for GUI component data '(5.0)'\nFormat should be @slider3(start_range, end_range) at line 2");

    line = "slider3(-1, 2.5)";
    for (const auto& datatype : {"float", "vec2"}) {
        uniform_line = "uniform " + std::string(datatype) + " v0;";
        T(!GUIComponentParse(5, line, uniform_line, {}, component, error));
        TSTR(error.c_str(), "GUI component count does not match uniform component count at line 5");
    }
}

UTEST(gui, component_parse_color_3) {
    std::string line, uniform_line, error;
    GUIComponent component;

    line = "color3";
    uniform_line = "uniform vec3 v0;";
    T(GUIComponentParse(0, line, uniform_line, {}, component, error));
    T(component.UniformType == EGUIUniformTypeVec3);
    T(error.empty());

#if 0
    line = "color3()";
    uniform_line = "uniform vec3 v0;";
    T(GUIComponentParse(0, line, uniform_line, {}, component, error));

    line = "color3(5.0)";
    uniform_line = "uniform vec3 v0;";
    T(GUIComponentParse(2, line, uniform_line, {}, component, error));
#endif

    line = "color3(0.1, -1, 2.5)";
    for (const auto& datatype : {"float", "vec2"}) {
        uniform_line = "uniform " + std::string(datatype) + " v0;";
        T(!GUIComponentParse(5, line, uniform_line, {}, component, error));
        TSTR(error.c_str(), "GUI component count does not match uniform component count at line 5");
    }
}

UTEST(shader_parser, parse_0) {
    std::vector<std::string> watches;
    std::vector<RenderPass> render_passes;
    std::vector<GUIComponent> components;
    std::string error;

    T(ShaderParserParse("tests", "tests/shader0.frag", watches, render_passes, components, error));

    T(error.empty());
    T(render_passes.size() == 3);

    T(!render_passes[0].IsMain);
    T(render_passes[0].Input.empty());
    T(render_passes[0].Output == "pass0");
    T(render_passes[0].Height == 256);
    T(render_passes[0].Width == 256);
    T(!render_passes[0].ShaderSource.empty());
    T(!render_passes[0].Textures.empty());
    T(render_passes[0].Textures[0].Width == 1653);
    T(render_passes[0].Textures[0].Height == 1252)
    T(render_passes[0].Textures[0].Data);

    T(!render_passes[1].IsMain);
    T(render_passes[1].Input == "pass0");
    T(render_passes[1].Output == "pass1");
    T(render_passes[1].Height == 512);
    T(render_passes[1].Width == 512);
    T(!render_passes[1].ShaderSource.empty());
    T(render_passes[1].Textures.empty());

    T(render_passes[2].Input == "pass1");
    T(render_passes[2].Output == "main");
    T(render_passes[2].IsMain);
    T(!render_passes[2].ShaderSource.empty());
    T(!render_passes[2].Textures.empty());
    T(render_passes[2].Textures[0].Width == 2320);
    T(render_passes[2].Textures[0].Height == 1485)
    T(render_passes[2].Textures[0].Data);
}

UTEST(utils, utils_split_string) {
    std::vector<std::string> expected;
    expected = {"path", "to", "file.txt"};
    T(SplitString("/path/to/file.txt", '/') == expected);
    expected = {"example"};
    T(SplitString("example", '/') == expected);
    expected = {".hidden"};
    T(SplitString("/.hidden", '/') == expected);
    expected = {"path", "with", "no", "extension"};
    T(SplitString("path/with/no/extension/", '/') == expected);
    expected = {"hello", "world"};
    T(SplitString("hello\tworld", '\t') == expected);
}

UTEST(utils, utils_extract_base_path) {
    T(ExtractBasePath("/path/to/file.txt") == "/path/to");
    T(ExtractBasePath("/path/to/another.file.docx") == "/path/to");
    T(ExtractBasePath("example") == "");
    T(ExtractBasePath("example.txt") == "");
    T(ExtractBasePath("./filename.txt") == ".");
    T(ExtractBasePath("path/with/no/extension/") == "path/with/no/extension");
    T(ExtractBasePath("") == "");
}

UTEST(utils, utils_extract_filename_without_ext) {
    T(ExtractFilenameWithoutExt("/path/to/file.txt") == "file");
    T(ExtractFilenameWithoutExt("/path/to/another.file.docx") == "another.file");
    T(ExtractFilenameWithoutExt("example") == "example");
    T(ExtractFilenameWithoutExt("example.txt") == "example");
    T(ExtractFilenameWithoutExt("path/with/no/extension/") == "");
    T(ExtractFilenameWithoutExt("") == "");
}

UTEST_MAIN();
