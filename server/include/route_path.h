#pragma once

#include <format>
#include <string_view>
#include <vector>

namespace pine
{
  /// @brief Represents a route path. Used to validate paths at compile time
  /// and to store paths as string views. Also implements helper functions.
  class route_path
  {
  public:
    template <typename T>
    explicit(false) consteval route_path(const T& path)
      : path_{ path }
    {
      if (!validate_path(path_))
      {
        throw std::format_error("Invalid path");
      }
    }

    /// @brief Get the parts of the path as a range of string views.
    constexpr std::vector<std::string_view> parts() const noexcept
    {
      size_t start = 1;
      std::vector<std::string_view> parts;

      for (size_t i = 1; i < path_.size(); i++)
      {
        if (path_[i] == '/')
        {
          parts.push_back(path_.substr(start, i - start));
          start = i + 1;
        }
      }

      if (start < path_.size())
      {
        parts.push_back(path_.substr(start));
      }

      return parts;
    }

    /// @brief Validates a path. A path must start with a forward slash and may
    /// contain only the following characters:
    /// 
    /// - Lowercase letters (a-z)
    /// 
    /// - Uppercase letters (A-Z)
    /// 
    /// - Digits (0-9)
    /// 
    /// - Following special characters: - _ . ~ ! $ & ' ( ) * + , ; = : @ /
    /// 
    /// @param path The path to validate.
    /// @return True if the path is valid; false otherwise.
    static constexpr bool validate_path(std::string_view path)
    {
      if (path.empty())
        return false;

      if (path[0] != '/')
        return false;

      for (size_t i = 1; i < path.size(); ++i)
      {
        if ((path[i] < 'a' || path[i] > 'z') &&
            (path[i] < 'A' || path[i] > 'Z') &&
            (path[i] < '0' || path[i] > '9') &&
            path[i] != '-' &&
            path[i] != '_' &&
            path[i] != '.' &&
            path[i] != '~' &&
            path[i] != '!' &&
            path[i] != '$' &&
            path[i] != '&' &&
            path[i] != '\'' &&
            path[i] != '(' &&
            path[i] != ')' &&
            path[i] != '*' &&
            path[i] != '+' &&
            path[i] != ',' &&
            path[i] != ';' &&
            path[i] != '=' &&
            path[i] != ':' &&
            path[i] != '@' &&
            path[i] != '/')
          return false;
      }

      return true;
    }

    /// @brief Get the path as a string view.
    constexpr auto get() const noexcept
    {
      return path_;
    }

  private:
    std::string_view path_;
  };
}