// Copyright © 2024 Leonov Maksim. All Rights Reserved.

#pragma once

#include <jutils/format.h>

namespace jreflect
{
    class class_interface;

    enum class field_value_type : jutils::uint8
    {
        none,
        boolean, int8, uint8, int16, uint16, int32, uint32, int64, uint64, string,
        object, object_ptr,
        array
    };
    [[nodiscard]] constexpr const char* field_value_type_to_string(const field_value_type type)
    {
        switch (type)
        {
        case field_value_type::boolean:    return "boolean";
        case field_value_type::int8:       return "int8";
        case field_value_type::uint8:      return "uint8";
        case field_value_type::int16:      return "int16";
        case field_value_type::uint16:     return "uint16";
        case field_value_type::int32:      return "int32";
        case field_value_type::uint32:     return "uint32";
        case field_value_type::int64:      return "int64";
        case field_value_type::uint64:     return "uint64";
        case field_value_type::string:     return "string";
        case field_value_type::object:     return "object";
        case field_value_type::object_ptr: return "object_ptr";
        default: ;
        }
        return "NONE";
    }

    class field_value
    {
    protected:
        field_value(const field_value_type type) : m_Type(type) {}
    public:
        virtual ~field_value() = default;

        [[nodiscard]] field_value_type getType() const { return m_Type; }

    private:

        field_value_type m_Type = field_value_type::none;
    };
    class field_value_none : public field_value
    {
    public:
        field_value_none() : field_value(field_value_type::none) {}
    };
    
    template<field_value_type Type, typename Factory>
    using field_value_t = typename Factory::template field_value_t<Type>;
    template<typename T, typename Factory>
    using field_value_into = typename Factory::template field_value_into<T>;
    template<typename T, typename Factory>
    [[nodiscard]] field_value* create_field_value() { return Factory::template create<T>(); }
}

JUTILS_STRING_FORMATTER_CONSTEXPR(jreflect::field_value_type, jreflect::field_value_type_to_string);
