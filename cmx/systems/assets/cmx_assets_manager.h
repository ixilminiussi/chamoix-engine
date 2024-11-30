#pragma once

#include "cmx_asset.h"

// std
#include <memory>
#include <spdlog/spdlog.h>
#include <string>

#include <type_traits>
#include <unordered_map>

namespace cmx
{

class AssetsManager final
{
  public:
    static AssetsManager *getInstance()
    {
        if (instance)
        {
            instance = new AssetsManager();
        }
        return instance;
    }

    // TODO: implement
    void loadAssets(const std::string &filePath);

    template <typename T> std::weak_ptr<T> getAsset(const std::string &name);

  private:
    AssetsManager() = default;
    ~AssetsManager() = default;

    static AssetsManager *instance;

    std::unordered_map<std::string, std::unique_ptr<Asset>> assets{};
};

template <typename T> std::weak_ptr<T> AssetsManager::getAsset(const std::string &name)
{
    if constexpr (!std::is_base_of<Asset, T>::value)
    {
        spdlog::error(
            "AssetManager: '{0}' is not of base type 'Asset', 'getAsset<{1}>' will always return invalid pointer",
            typeid(T).name(), typeid(T).name());
        return std::weak_ptr<T>();
    }

    if (!assets.at(name))
    {
        spdlog::error("'AssetManager: '{0}' does not exist in list of loaded assets");
        return std::weak_ptr<T>();
    }

    if (auto asset = std::dynamic_pointer_cast<T>(assets.at(name)))
    {
        return asset;
    }

    return std::weak_ptr<T>();
}

} // namespace cmx
