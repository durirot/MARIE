#pragma once

template <typename T>
[[nodiscard]] std::vector<T> fileToVector(const char* fileName)
{
    std::vector<T> data;
    std::fstream file(fileName, std::ios::in);
    std::size_t size = std::filesystem::file_size(fileName);

    data.resize(size / sizeof(T));
    file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size));

    return data;
}

template <typename T>
void dataToFile(const char* fileName, const std::span<T>& data)
{
    std::fstream file(fileName, std::ios::out);
    file.write(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size() * sizeof(T)));
}
