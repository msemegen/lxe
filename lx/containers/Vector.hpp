#pragma once

/*
 *	Name: Vector.hpp
 *  Copyright (c) Mateusz Semegen and contributors. All rights reserved.
 */

// std
#include <array>
#include <cassert>
#include <cstddef>
#include <memory>
#include <span>

namespace lx::containers {
template<typename Type, std::size_t capacity = 0u> class Vector
{
public:
    Vector() = default;
    Vector(Vector<Type, capacity>&&) = default;

    Vector(const Vector<Type, capacity>& other_a)
    {
        this->length = other_a.get_length();
        std::copy(other_a.get_buffer(), other_a.get_buffer() + this->length, this->buffer);
    }
    Vector(std::initializer_list<Type> list_a)
        : length(list_a.size())
    {
        assert(this->length <= capacity);

        for (std::size_t i = 0; i < this->get_length(); i++)
        {
            this->buffer[i] = *(list_a.begin() + i);
        }
    }
    Vector(std::span<const Type> data_a)
        : Vector()
    {
        std::size_t count = std::min(data_a.size(), capacity - 1u);
        std::copy(data_a.begin(), data_a.begin() + count, this->buffer);
    }

    bool push_back(const Type& data_a)
    {
        if (this->get_length() < this->get_capacity())
        {
            this->buffer[this->length++] = data_a;
            return true;
        }

        return false;
    }
    bool push_back(Type&& data_a)
    {
        if (this->get_length() < this->get_capacity())
        {
            this->buffer[this->length++] = std::move(data_a);
            return true;
        }

        return false;
    }
    bool push_back(std::span<const Type> data_a)
    {
        if (this->get_length() + data_a.size() <= this->get_capacity())
        {
            for (std::size_t i = 0; i < data_a.size(); i++)
            {
                this->buffer[this->length++] = data_a[i];
            }

            return true;
        }

        return false;
    }

    template<typename... Arg> void emplace_back(Arg&&... args_a)
    {
        assert(this->length < capacity);

        new (&(this->buffer[this->length++])) Type(std::forward<Arg>(args_a)...);
    }

    bool pop_back()
    {
        if (false == this->is_empty())
        {
            this->length--;

            return true;
        }

        return false;
    }

    bool reserve(std::size_t length_a)
    {
        if (capacity >= length_a)
        {
            this->length = length_a;

            return true;
        }

        return false;
    }

    void erase(std::size_t index_a)
    {
        assert(index_a < this->length);

        this->buffer[index_a].~Type();

        for (size_t i = index_a; i < this->length - 1; i++)
        {
            new (&(this->buffer[i])) Type(std::move(this->buffer[i + 1]));
            this->buffer[i + 1].~Type();
        }

        this->length--;
    }

    std::size_t get_length() const
    {
        return this->length;
    }
    std::size_t get_capacity() const
    {
        return capacity;
    }

    const Type& get_back() const
    {
        assert(false == this->is_empty());

        return this->buffer[this->length - 1u];
    }
    Type& get_back()
    {
        assert(false == this->is_empty());

        return this->buffer[this->length - 1u];
    }

    bool is_empty() const
    {
        return 0u == this->get_length();
    }
    const Type* get_buffer() const
    {
        return this->buffer;
    }
    Type* get_buffer()
    {
        return this->buffer;
    }

    const Type& operator[](std::size_t index_a) const
    {
        assert(index_a < this->get_length());

        return this->buffer[index_a];
    }
    Type& operator[](std::size_t index_a)
    {
        assert(index_a < this->get_length());

        return this->buffer[index_a];
    }

    Vector<Type, capacity>& operator=(const Vector<Type, capacity>& other_a)
    {
        return {};
    }

    operator std::span<const Type>() const
    {
        return std::span<const Type> { buffer, length };
    }

private:
    Type buffer[capacity] = {};
    std::size_t length = 0u;
};
template<typename Type> class Vector<Type, 0u>
{
public:
    Vector() = default;
    Vector(const Vector<Type, 0u>& other_a)
        : capacity(other_a.capacity)
        , length(other_a.length)
        , buffer(std::make_unique<Type[]>(other_a.capacity))
    {
        for (std::size_t i = 0; i < this->get_length(); i++)
        {
            this->buffer[i] = other_a.buffer[i];
        }
    }
    Vector(Vector<Type, 0u>&& other_a) noexcept
        : capacity(other_a.capacity)
        , length(other_a.length)
        , buffer(std::move(other_a.buffer))
    {
        other_a.capacity = 0u;
        other_a.length = 0u;
    }
    Vector(std::size_t capacity_a)
        : capacity(capacity_a)
        , length(0)
        , buffer(std::make_unique<Type[]>(capacity_a))
    {
        assert(capacity_a > 0u);
    }
    Vector(std::initializer_list<Type> list_a)
        : capacity(list_a.size())
        , length(list_a.size())
        , buffer(std::make_unique<Type[]>(list_a.size()))
    {
        assert(list_a.size() > 0u);

        for (std::size_t i = 0; i < this->get_length(); i++)
        {
            this->buffer[i] = *(list_a.begin() + i);
        }
    }
    Vector(std::span<const Type> data_a)
        : Vector(data_a.size())
    {
        assert(false == data_a.empty());

        this->length = data_a.size();
        std::copy(data_a.begin(), data_a.begin() + this->length, this->buffer.get());
    }
    Vector(std::size_t count_a, Type data_a)
        : Vector(count_a)
    {
        for (std::size_t i = 0; i < this->capacity; i++)
        {
            this->buffer[i] = data_a;
        }

        this->length = count_a;
    }

    void push_back(const Type& data_a)
    {
        this->reserve(this->get_length() + 2u);
        this->buffer[this->length++] = data_a;
    }
    void push_back(Type&& data_a)
    {
        this->reserve(this->get_length() + 2u);
        this->buffer[this->length++] = std::move(data_a);
    }
    void push_back(std::span<const Type> data_a)
    {
        this->reserve(this->get_length() + data_a.size());

        for (std::size_t i = 0; i < data_a.size(); i++)
        {
            this->buffer[this->length++] = data_a[i];
        }
    }

    template<typename... Arg> void emplace_back(Arg&&... args_a)
    {
        this->reserve(this->get_length() + 2u);
        new (&(this->buffer[this->length++])) Type(std::forward<Arg>(args_a)...);
    }

    bool pop_back()
    {
        if (this->length >= 0u)
        {
            this->length--;
            return true;
        }

        return false;
    }

    void reserve(std::size_t capacity_a)
    {
        if (this->capacity >= capacity_a)
        {
            return;
        }

        std::unique_ptr<Type[]> new_buffer = std::make_unique<Type[]>(capacity_a);

        for (size_t i = 0; i < this->get_length(); i++)
        {
            new (&(new_buffer[i])) Type(std::move(this->buffer[i]));
        }

        this->buffer = std::move(new_buffer);
        this->capacity = capacity_a;
    }
    void resize(std::size_t length_a)
    {
        this->reserve(length_a);
        this->length = length_a;
    }

    void erase(std::size_t index_a)
    {
        assert(index_a < this->length);

        this->buffer[index_a].~Type();

        for (size_t i = index_a; i < this->length - 1; i++)
        {
            new (&(this->buffer[i])) Type(std::move(this->buffer[i + 1]));
            this->buffer[i + 1].~Type();
        }

        this->length--;
    }

    void swap(std::size_t index_left_a, std::size_t index_right_a)
    {
        std::swap(this->buffer[index_left_a], this->buffer[index_right_a]);
    }

    void shrink_to_fit()
    {
        if (this->get_capacity() > this->get_length())
        {
            std::unique_ptr<Type[]> new_buffer = std::make_unique<Type[]>(this->get_length());

            for (size_t i = 0; i < this->get_length(); i++)
            {
                new (&(new_buffer[i])) Type(this->buffer[i]);
            }

            this->buffer = std::move(new_buffer);
            this->capacity = this->get_length();
        }
    }

    void clear()
    {
        this->buffer.release();
        this->capacity = 0u;
        this->length = 0u;
    }

    std::size_t get_length() const
    {
        return this->length;
    }
    std::size_t get_capacity() const
    {
        return this->capacity;
    }

    const Type& get_back() const
    {
        assert(false == this->is_empty());

        return this->buffer[this->length - 1u];
    }
    Type& get_back()
    {
        assert(false == this->is_empty());

        return this->buffer[this->length - 1u];
    }

    bool is_empty() const
    {
        return 0u == this->length;
    }
    const Type* get_buffer() const
    {
        return this->buffer.get();
    }
    Type* get_buffer() 
    {
        return this->buffer.get();
    }

    const Type& operator[](std::size_t index_a) const
    {
        assert(index_a < this->get_length());

        return this->buffer[index_a];
    }
    Type& operator[](std::size_t index_a)
    {
        assert(index_a < this->get_length());

        return this->buffer[index_a];
    }
    Vector<Type, 0u>& operator=(const Vector<Type, 0u>& other_a)
    {
        std::unique_ptr<Type[]> new_buffer = std::make_unique<Type[]>(other_a.get_capacity());

        for (size_t i = 0; i < other_a.get_length(); i++)
        {
            new_buffer[i] = other_a[i];
        }

        this->buffer.release();

        this->buffer = std::move(new_buffer);
        this->capacity = other_a.get_capacity();
        this->length = other_a.get_length();

        return *this;
    }
    Vector<Type, 0u>& operator=(Vector<Type, 0u>&& other_a)
    {
        this->buffer.release();

        this->buffer = std::move(other_a.buffer);
        this->capacity = other_a.get_capacity();
        this->length = other_a.get_length();

        other_a.capacity = 0u;
        other_a.length = 0u;

        return *this;
    }

    operator std::span<const Type>() const
    {
        return std::span<const Type> { this->buffer.get(), this->length };
    }

private:
    std::size_t capacity = 0u;
    std::size_t length = 0u;
    std::unique_ptr<Type[]> buffer;
};

template<typename Type> Type* begin(Vector<Type>& vec_a)
{
    return vec_a.get_buffer();
}

template<typename Type> Type* end(Vector<Type>& vec_a)
{
    return vec_a.get_buffer() + vec_a.get_length();
}

template<typename Type> const Type* begin(const Vector<Type>& vec_a)
{
    return vec_a.get_buffer();
}

template<typename Type> const Type* end(const Vector<Type>& vec_a)
{
    return vec_a.get_buffer() + vec_a.get_length();
}
} // namespace lx::containers