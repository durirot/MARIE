#include <cstdint>
#include <concepts>
#include <initializer_list>
#include <array>

template <typename Type, auto Hashfunc>
concept is_hash_function = requires(Type key) {
    {
        Hashfunc(key)
    } -> std::same_as<std::size_t>;
};

template <typename Key, typename Cargo, std::size_t Size, auto Hash>
    requires is_hash_function<Key, Hash>
struct static_hashtable {
    struct Type {
        Key first;
        Cargo second;
    };
    constexpr static_hashtable() = default;

    constexpr static_hashtable(std::initializer_list<Type> input)
    {
        for (auto& obj : input) {
            insert(obj);
        }
    }

    constexpr void insert(Type input)
    {
        auto hash = Hash(input.first) % Size;

        std::size_t count = 0;
        while (_buffer[hash].first != default_value.first) {
            if (count >= Size) {
                return;
            }
            hash = (hash + 1) % Size;
            count++;
        }
        _buffer[hash] = input;
        contained++;
    }

    [[nodiscard]] constexpr auto search(Key key, std::size_t max_count) const -> const Type*
    {
        auto hash = Hash(key) % Size;
        std::size_t count = 0;
        while (_buffer[hash].first != key) {
            if (count >= max_count) {
                return nullptr;
            }
            hash = (hash + 1) % Size;
            count++;
        }
        return &_buffer[hash];
    }

    constexpr void remove(Key key)
    {
        auto* search_result = search(key, contained);
        if (search_result == nullptr) {
            return;
        } else {
            *search_result = default_value;
            contained--;
        }
    }

    [[nodiscard]] constexpr auto get(Key key) const -> const Cargo&
    {
        auto* search_result = search(key, contained);
        if (search_result == nullptr) {
            return default_value.second;
        } else {
            return search_result->second;
        }
    }

    [[nodiscard]] constexpr auto operator[](Key key) const -> const Cargo&
    {
        return get(key);
    }

private:
    std::array<Type, Size> _buffer {};
    std::size_t contained = 0;
    Type default_value {};
};

