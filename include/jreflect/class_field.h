// Copyright © 2024 Leonov Maksim. All Rights Reserved.

#pragma once

#include <jutils/jstringID.h>

namespace jreflect
{
    class class_type;
    class class_interface;

    enum class class_field_type : jutils::uint8
    {
        boolean, int8, uint8, int16, uint16, int32, uint32, int64, uint64, string, object, object_ptr
    };

    class class_field
    {
    public:
        class_field() = default;
        virtual ~class_field() = default;

        [[nodiscard]] class_field_type getType() const { return m_Type; }
        [[nodiscard]] jutils::jstringID getName() const { return m_Name; }

    protected:

        jutils::jstringID m_Name = jutils::jstringID_NONE;
        std::size_t m_Offset = 0;
        class_field_type m_Type = class_field_type::boolean;
    };

    template<typename T>
    class class_field_primitive : public std::false_type {};

#define JREFLECT_HELPER_DECLARE_PRIMITIVE_CLASS_FIELD(Enum, Type)                       \
    template<>                                                                          \
    class class_field_primitive<Type> : public std::true_type, public class_field       \
    {                                                                                   \
    public:                                                                             \
        class_field_primitive(const jutils::jstringID& name, const std::size_t offset)  \
        {                                                                               \
            m_Name = name;                                                              \
            m_Offset = offset;                                                          \
            m_Type = class_field_type::Enum;                                            \
        }                                                                               \
        virtual ~class_field_primitive() override = default;                            \
    };                                                                                  \
    using class_field_##Enum = class_field_primitive<Type>

    JREFLECT_HELPER_DECLARE_PRIMITIVE_CLASS_FIELD(boolean,           bool);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_CLASS_FIELD(   int8, jutils::  int8);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_CLASS_FIELD(  uint8, jutils:: uint8);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_CLASS_FIELD(  int16, jutils:: int16);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_CLASS_FIELD( uint16, jutils::uint16);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_CLASS_FIELD(  int32, jutils:: int32);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_CLASS_FIELD( uint32, jutils::uint32);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_CLASS_FIELD(  int64, jutils:: int64);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_CLASS_FIELD( uint64, jutils::uint64);
    
    template<>
    class class_field_primitive<jutils::jstring> : public std::true_type, public class_field
    {
    public:
        class_field_primitive(const jutils::jstringID& name, const std::size_t offset)
        {
            m_Name = name;
            m_Offset = offset;
            m_Type = class_field_type::string;
        }
        virtual ~class_field_primitive() override = default;
    };
    using class_field_string = class_field_primitive<jutils::jstring>;

    class class_field_object : public class_field
    {
    public:
        class_field_object(const jutils::jstringID& name, const std::size_t offset, class_type* objectType)
        {
            m_Name = name;
            m_Offset = offset;
            m_Type = class_field_type::object;
            m_ObjectType = objectType;
        }
        virtual ~class_field_object() override = default;

        [[nodiscard]] class_type* getObjectType() const { return m_ObjectType; }

    private:

        class_type* m_ObjectType = nullptr;
    };

    class class_field_object_ptr : public class_field
    {
        friend class_type;

    public:
        class_field_object_ptr(const jutils::jstringID& name, const std::size_t offset)
        {
            m_Name = name;
            m_Offset = offset;
            m_Type = class_field_type::object_ptr;
        }
        virtual ~class_field_object_ptr() override = default;

        [[nodiscard]] class_type* getObjectType() const { return m_ObjectType; }

    private:

        class_type* m_ObjectType = nullptr;
    };
}
