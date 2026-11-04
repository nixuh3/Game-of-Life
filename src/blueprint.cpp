#include "blueprint.hpp"
#include <cctype>
#include <cstdio>
#include <fstream>
#include <sstream>

#include <raygui.h>
#include <raylib.h>


namespace gol
{

void BlueprintLoader::LoadAllFiles(const std::string& path)
{
    FilePathList blueprintFiles = LoadDirectoryFilesEx(path.c_str(), ".rle", false);
    std::ostringstream listStream;

    for (unsigned int i = 0; i < blueprintFiles.count; i++) {
        std::string fullPath = blueprintFiles.paths[i];
        std::string fileName = GetFileName(fullPath.c_str());

        if (LoadFile(fullPath)) {
            m_fullPaths.push_back(fullPath);
            m_fileNames.push_back(fileName);

            if (listStream.tellp() > 0) {
                listStream << ";";
            }
            listStream << fileName;
        }
    }

    UnloadDirectoryFiles(blueprintFiles);

    m_listBufferStr = listStream.str();
    m_listBuffer    = m_listBufferStr.c_str();
}

bool BlueprintLoader::LoadFile(const std::string& path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        return false;
    }

    std::streamsize size = file.tellg();
    if (size <= 0) {
        return false;
    }
    file.seekg(0, std::ios::beg);

    std::string buffer;
    buffer.resize(static_cast<size_t>(size));
    if (!file.read(buffer.data(), size)) {
        return false;
    }

    std::string fileName = GetFileName(path.c_str());

    Blueprint bp;
    bp.width  = 0;
    bp.height = 0;
    bp.name   = fileName; // Default fallback name if no #N comment exists

    std::istringstream stream(buffer);
    std::string line;
    bool headerFound    = false;
    std::string rleData = "";

    while (std::getline(stream, line)) {
        if (line.empty()) {
            continue;
        }

        // Extract pattern name if provided via standard "#N <Name>" comment
        if (line[0] == '#') {
            if (line.size() > 2 && line[1] == 'N') {
                size_t firstChar = line.find_first_not_of(" \t", 2);
                if (firstChar != std::string::npos) {
                    bp.name = line.substr(firstChar);
                    // Strip optional trailing carriage return from text lines
                    if (!bp.name.empty() && bp.name.back() == '\r') {
                        bp.name.pop_back();
                    }
                }
            }
            continue;
        }

        // Parse Header: x = ..., y = ...
        if (!headerFound) {
            char rule[128] = {};
            int fieldsRead = std::sscanf(line.c_str(), "x = %d, y = %d, rule = %127s", &bp.width, &bp.height, rule);
            if (fieldsRead >= 2) {
                if (fieldsRead == 3) {
                    bp.rule = rule;
                }
                headerFound = true;
                bp.cells.assign(bp.height, std::vector<bool>(bp.width, false));
            }
            continue;
        }

        // Collect body data
        rleData += line;
    }

    if (!headerFound) {
        return false;
    }

    // 4. Decode the run-length encoded stream
    int x        = 0;
    int y        = 0;
    int runCount = 0;

    for (char c : rleData) {
        if (std::isdigit(static_cast<unsigned char>(c))) {
            runCount = runCount * 10 + (c - '0');
        } else {
            int count = (runCount == 0) ? 1 : runCount;
            runCount  = 0;

            if (c == 'b') { // Dead cells
                x += count;
            } else if (c == 'o') { // Alive cells
                for (int i = 0; i < count; ++i) {
                    if (y < bp.height && x < bp.width) {
                        bp.cells[y][x] = true;
                    }
                    x++;
                }
            } else if (c == '$') { // Next row(s)
                y += count;
                x = 0;
            } else if (c == '!') { // End of pattern data
                break;
            }
        }
    }

    m_blueprints[fileName] = std::move(bp);
    return true;
}

std::optional<std::reference_wrapper<Blueprint>> BlueprintLoader::GetBlueprint(const std::string& name)
{
    if (m_blueprints.contains(name)) {
        return m_blueprints[name];
    }
    return {};
}

std::string BlueprintLoader::GetSelectedBlueprintName() const
{
    return m_selectedBlueprint;
}

void BlueprintLoader::Render()
{
    GuiListView({24, 60, 300, 300}, m_listBuffer, &m_listScroll, &m_listViewActive);

    if (m_listViewActive >= 0 && static_cast<size_t>(m_listViewActive) < m_fileNames.size()) {
        m_selectedBlueprint = m_fileNames[m_listViewActive];
    }
}

} // namespace gol
