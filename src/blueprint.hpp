#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>


namespace gol
{

struct Blueprint {
    int width;
    int height;
    std::string name;
    std::string rule;
    std::vector<std::vector<bool>> cells;
};

class BlueprintLoader
{
  public:
    void LoadAllFiles(const std::string& path);
    bool LoadFile(const std::string& path);
    std::optional<std::reference_wrapper<Blueprint>> GetBlueprint(const std::string& name);
    std::string GetSelectedBlueprintName() const;

    void Render();

  private:
    std::unordered_map<std::string, Blueprint> m_blueprints;

    std::vector<std::string> m_fileNames;
    std::vector<std::string> m_fullPaths;

    std::string m_listBufferStr;
    const char* m_listBuffer;

    // GUI State variables
    int m_listScroll                = 0;
    int m_listViewActive            = -1; // -1 means nothing selected initially
    std::string m_selectedBlueprint = "None";
};

} // namespace gol
