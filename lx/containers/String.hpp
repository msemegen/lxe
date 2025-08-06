#pragma once

/*
 *	Name: String.hpp
 *  Copyright (c) Mateusz Semegen and contributors. All rights reserved.
 */

// lx
#include <lx/containers/Vector.hpp>

// std
#include <cstddef>
#include <string_view>

namespace lx::containers {
template<typename Char, std::size_t capacity = 0u> class String
{
public:
    String()
        : buffer({ static_cast<Char>('\0') })
    {
    }
    String(std::basic_string_view<Char> string_a)
    {
        this->buffer.push_back(string_a);
        this->buffer.push_back('\0');
    }
    String(const String<Char, capacity>&) = default;
    String(String<Char, capacity>&&) = default;

    bool push_back(Char char_a)
    {
        if (this->buffer.get_capacity() >= this->buffer.get_length() + 1u)
        {
            const Char c[] = { char_a, static_cast<Char>('\0') };

            this->buffer.pop_back();
            this->buffer.push_back(std::span<const Char, 2u>(c));

            return true;
        }

        return false;
    }
    bool push_back(std::basic_string_view<Char> string_a)
    {
        if (this->buffer.get_capacity() >= this->get_length() + string_a.size() + 1u)
        {
            this->buffer.pop_back();
            this->buffer.push_back(std::span { string_a.begin(), string_a.size() });
            this->buffer.push_back('\0');

            return true;
        }

        return false;
    }

    bool reserve(std::size_t length)
    {
        return this->buffer.reserve(length + 1u);
    }

    std::size_t get_capacity() const
    {
        return this->buffer.get_capacity();
    }
    std::size_t get_length() const
    {
        return this->buffer.get_length() - 1u;
    }

    const Char* get_cstring() const
    {
        return this->buffer.get_buffer();
    }
    const Char* get_buffer() const
    {
        return this->buffer.get_buffer();
    }
    Char* get_buffer()
    {
        return this->buffer.get_buffer();
    }

    operator std::string_view() const
    {
        return { this->get_cstring(), this->get_length() };
    }

private:
    Vector<Char, capacity> buffer = {};
};

template<typename Char> class String<Char, 0u>
{
public:
    String()
        : buffer({ static_cast<Char>(0) })
    {
    }

    String(const Char* string_a)
    {
        this->buffer.push_back(std::span { string_a, string_a + std::strlen(string_a) });
        this->buffer.push_back(0);

        this->buffer.shrink_to_fit();  
    }
    String(const String<Char>&) = default;
    String(std::basic_string_view<Char> string_a)
        : buffer(string_a.size() + 2u)
    {
        this->buffer.push_back(std::span { string_a.begin(), string_a.size() });
        this->buffer.push_back(0);
    }

    void push_back(Char char_a)
    {
        this->buffer.pop_back();

        const Char c[] = { char_a, static_cast<Char>(0) };
        this->buffer.push_back(std::span<const Char, 2u>(c));
    }
    void push_back(std::string_view string_a)
    {
        this->buffer.pop_back();

        const std::size_t l = this->buffer.get_length();
        this->buffer.reserve(l + string_a.size() + 1u);

        std::memcpy(this->buffer.get_buffer() + l, string_a.data(), string_a.size());
        this->buffer[l + string_a.size()] = 0;
    }

    const Char* get_cstring() const
    {
        return this->buffer.get_buffer();
    }
    Char* get_cstring()
    {
        return this->buffer.get_buffer();
    }

    std::size_t get_capacity() const
    {
        return this->buffer.get_capacity();
    }

    std::size_t get_length() const
    {
        return this->buffer.get_length() - 1u;
    }

    bool is_empty() const
    {
        return 0u == this->get_length();
    }

    String<Char>& operator=(const Char* string_a)
    {
        this->buffer.clear();
        std::size_t length = std::strlen(string_a);
        this->buffer.resize(length + 1u);

        std::memcpy(this->buffer.get_buffer(), string_a, length);

        this->buffer[length] = 0;

        return *this;
    }
    String<Char>& operator=(const String<Char, 0u>& string_a)
    {
        this->buffer.clear();
        this->buffer.reseresizerve(string_a.size() + 1u);

        std::memcpy(this->buffer.get_buffer(), string_a.data(), string_a.size());
        this->buffer[string_a.size()] = 0;

        return *this;
    }
    String<Char>& operator=(std::basic_string_view<Char> string_a)
    {
        this->buffer.clear();
        this->buffer.resize(string_a.size() + 1u);

        std::memcpy(this->buffer.get_buffer(), string_a.data(), string_a.size());
        this->buffer[string_a.size()] = 0;

        return *this;
    }

    operator std::basic_string_view<Char>() const
    {
        return { this->get_cstring(), this->get_length() };
    }

private:
    Vector<Char> buffer;
};
} // namespace lx::containers