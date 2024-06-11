// Copyright © 2024 Leonov Maksim. All Rights Reserved.

#pragma once

#include <cassert>

#include <jutils/jmap.h>
#include <jutils/jstringID.h>

namespace jreflect
{
    class class_type;
    class class_interface;

    enum class class_field_type : jutils::uint8
    {
        boolean, int8, uint8, int16, uint16, int32, uint32, int64, uint64, string, object, object_ptr
    };
    constexpr const char* class_field_type_to_string(const class_field_type type)
    {
        switch (type)
        {
        case class_field_type::boolean:    return "boolean";
        case class_field_type::int8:       return "int8";
        case class_field_type::uint8:      return "uint8";
        case class_field_type::int16:      return "int16";
        case class_field_type::uint16:     return "uint16";
        case class_field_type::int32:      return "int32";
        case class_field_type::uint32:     return "uint32";
        case class_field_type::int64:      return "int64";
        case class_field_type::uint64:     return "uint64";
        case class_field_type::string:     return "string";
        case class_field_type::object:     return "object";
        case class_field_type::object_ptr: return "object_ptr";
        default: ;
        }
        return "NONE";
    }



    template<class_field_type Type>
    struct class_field_info
    {
        using type = std::false_type;
    };
    template<class_field_type Type>
    using class_field_t = typename class_field_info<Type>::type;

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


        template<typename T>
        T* getValuePtr(class_interface* object) const
        {
            return object != nullptr ? reinterpret_cast<T*>((reinterpret_cast<jutils::uint8*>(object) + m_Offset)) : nullptr;
        }
        template<typename T>
        const T* getValuePtr(const class_interface* object) const
        {
            return object != nullptr ? reinterpret_cast<const T*>((reinterpret_cast<const jutils::uint8*>(object) + m_Offset)) : nullptr;
        }
    };

    class class_field_object_ptr;
    class class_field_object;



    template<typename T>
    struct class_type_info
    {
    private:
        JUTILS_TEMPLATE_CONDITION((
            std::is_base_of_v<class_interface, T1> &&
            std::is_same_v<T1, typename T1::class_type_t::type>
        ), typename T1)
        static constexpr typename T1::class_type_t* _helper_class_type(jutils::int32) { return nullptr; }
        template<typename T1>
        static constexpr class_type* _helper_class_type(jutils::int8) { return nullptr; }

        template<typename T1>
        using _helper_class_type_t = std::remove_pointer_t<decltype(_helper_class_type<T1>(0))>;
        template<typename T1>
        static constexpr bool _helper_has_class_type = !std::is_same_v<_helper_class_type_t<T1>, class_type>;

        JUTILS_TEMPLATE_CONDITION(_helper_has_class_type<T1>, typename T1)
        [[nodiscard]] static auto* _helper_get_class_type(jutils::int32) { return T1::GetClassType(); }
        template<typename T1>
        [[nodiscard]] static class_type* _helper_get_class_type(jutils::int8) { return nullptr; }

    public:
        using class_type_t = _helper_class_type_t<T>;
        static constexpr bool has_class_type = !std::is_same_v<_helper_class_type_t<T>, class_type>;

        [[nodiscard]] static auto* get_class_type() { return _helper_get_class_type<T>(0); }
    };
    template<typename T>
    using get_class_type_t = typename class_type_info<T>::class_type_t;
    template<typename T>
    constexpr bool has_class_type_v = class_type_info<T>::has_class_type;

    class class_type
    {
    public:
        class_type() = default;
        virtual ~class_type() = default;

        [[nodiscard]] virtual jutils::jstringID getName() const = 0;
        [[nodiscard]] virtual class_type* getParent() const = 0;

        [[nodiscard]] bool isDerivedFrom(const class_type* type) const { return (type != nullptr) && isDerivedFromClass(type); }
        JUTILS_TEMPLATE_CONDITION(has_class_type_v<T>, typename T)
        [[nodiscard]] bool isDerivedFrom() const { return this->isDerivedFrom(class_type_info<T>::get_class_type()); }

        [[nodiscard]] const auto& getFields() const { return m_Fields; }

    protected:

        template<bool IsInit, typename T>
        struct create_class_field_info
        {
            using type = T;

            jutils::jstringID name = jutils::jstringID_NONE;
            std::size_t offset = 0;
        };

        virtual bool isDerivedFromClass(const class_type* type) const { return false; }
        
        template<typename T>
        void createClassField(const jutils::jstringID& name, std::size_t offset);
        template<typename T>
        void initClassField(const jutils::jstringID& name);

    private:

        jutils::jmap<jutils::jstringID, class_field*> m_Fields;
        
        template<typename T>
        static class_field* CreateClassField(const jutils::jstringID& name, std::size_t offset);
        template<typename T>
        static bool InitClassField(class_field* field);
        JUTILS_TEMPLATE_CONDITION(has_class_type_v<T>, typename T)
        static bool InitClassField(class_field_object_ptr* field, jutils::int32);
        template<typename T>
        static bool InitClassField(class_field_object_ptr* field, jutils::int8) { return false; }
    };

    class class_interface
    {
        friend class_field_object;

    public:
        class_interface() = default;
        class_interface(const class_interface&) = default;
        class_interface(class_interface&&) noexcept = default;
        virtual ~class_interface() = default;

        using this_t = class_interface;
        using class_type_t = class_type;

        class_interface& operator=(const class_interface&) = default;
        class_interface& operator=(class_interface&&) noexcept = default;

        [[nodiscard]] virtual class_type* getClassType() const = 0;

    protected:

        virtual bool copyFromInternal(const class_interface& value) { return false; }
        virtual bool moveFromInternal(class_interface&& value) { return copyFromInternal(value); }

    private:

        bool copyFrom(const class_interface& value)
        {
            return (&value != this) && (value.getClassType() == getClassType()) && copyFromInternal(value);
        }
        bool moveFrom(class_interface&& value)
        {
            return (&value != this) && (value.getClassType() == getClassType()) && moveFromInternal(std::move(value));
        }
    };



    template<typename T>
    class class_field_simple : public std::false_type {};

#define JREFLECT_HELPER_DECLARE_PRIMITIVE_CLASS_FIELD(Enum, Type)                           \
    template<>                                                                              \
    class class_field_simple<Type> : public std::true_type, public class_field              \
    {                                                                                       \
    public:                                                                                 \
        class_field_simple(const jutils::jstringID& name, const std::size_t offset)         \
        {                                                                                   \
            m_Name = name;                                                                  \
            m_Offset = offset;                                                              \
            m_Type = class_field_type::Enum;                                                \
        }                                                                                   \
        virtual ~class_field_simple() override = default;                                   \
        bool get(const class_interface* object, Type& outValue) const                       \
        {                                                                                   \
            auto* valuePtr = getValuePtr<Type>(object);                                     \
            if (valuePtr == nullptr)                                                        \
                return false;                                                               \
            outValue = *valuePtr;                                                           \
            return true;                                                                    \
        }                                                                                   \
        bool set(class_interface* object,                                                   \
            std::conditional_t<std::is_arithmetic_v<Type>, Type, const Type&> value) const  \
        {                                                                                   \
            auto* valuePtr = getValuePtr<Type>(object);                                     \
            if (valuePtr == nullptr)                                                        \
                return false;                                                               \
            *valuePtr = value;                                                              \
            return true;                                                                    \
        }                                                                                   \
    };                                                                                      \
    using class_field_##Enum = class_field_simple<Type>;                                    \
    template<> struct class_field_info<class_field_type::Enum>                              \
        { using type = class_field_##Enum; }
    
    JREFLECT_HELPER_DECLARE_PRIMITIVE_CLASS_FIELD(boolean,            bool);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_CLASS_FIELD(   int8, jutils::   int8);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_CLASS_FIELD(  uint8, jutils::  uint8);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_CLASS_FIELD(  int16, jutils::  int16);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_CLASS_FIELD( uint16, jutils:: uint16);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_CLASS_FIELD(  int32, jutils::  int32);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_CLASS_FIELD( uint32, jutils:: uint32);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_CLASS_FIELD(  int64, jutils::  int64);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_CLASS_FIELD( uint64, jutils:: uint64);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_CLASS_FIELD( string, jutils::jstring);

    class class_field_object : public class_field, public std::true_type
    {
    public:
        class_field_object(const jutils::jstringID& name, const std::size_t offset, class_type* classType)
        {
            m_Name = name;
            m_Offset = offset;
            m_Type = class_field_type::object;
            m_ClassType = classType;
        }
        virtual ~class_field_object() override = default;

        [[nodiscard]] class_type* getClassType() const { return m_ClassType; }
        
        bool get(class_interface* object, class_interface*& outValue) const
        {
            auto* valuePtr = m_ClassType != nullptr ? getValuePtr<class_interface>(object) : nullptr;
            if (valuePtr == nullptr)
            {
                return false;
            }
            outValue = valuePtr;
            return true;
        }
        bool get(const class_interface* object, const class_interface*& outValue) const
        {
            auto* valuePtr = m_ClassType != nullptr ? getValuePtr<class_interface>(object) : nullptr;
            if (valuePtr == nullptr)
            {
                return false;
            }
            outValue = valuePtr;
            return true;
        }
        bool set(class_interface* object, const class_interface& value) const
        {
            auto* valuePtr = m_ClassType != nullptr ? getValuePtr<class_interface>(object) : nullptr;
            if (valuePtr == nullptr)
            {
                return false;
            }
            return valuePtr->copyFrom(value);
        }
        bool set(class_interface* object, class_interface&& value) const
        {
            auto* valuePtr = m_ClassType != nullptr ? getValuePtr<class_interface>(object) : nullptr;
            if (valuePtr == nullptr)
            {
                return false;
            }
            return valuePtr->moveFrom(std::move(value));
        }

    private:

        class_type* m_ClassType = nullptr;
    };
    template<> struct class_field_info<class_field_type::object> { using type = class_field_object; };
    
    // TODO: How to handle fields like object_ptr<objType>, not just objType*?
    class class_field_object_ptr : public class_field, public std::true_type
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

        [[nodiscard]] class_type* getClassType() const { return m_ClassType; }

        bool get(const class_interface* object, class_interface*& outValue) const
        {
            auto* valuePtr = m_ClassType != nullptr ? getValuePtr<class_interface*>(object) : nullptr;
            if (valuePtr == nullptr)
            {
                return false;
            }
            outValue = *valuePtr;
            return true;
        }
        bool set(class_interface* object, class_interface* value) const
        {
            auto* valuePtr = m_ClassType != nullptr ? getValuePtr<class_interface*>(object) : nullptr;
            if (valuePtr == nullptr)
            {
                return false;
            }
            if (value != nullptr)
            {
                class_type* valueType = value->getClassType();
                if ((valueType == nullptr) || !valueType->isDerivedFrom(m_ClassType))
                {
                    return false;
                }
            }
            *valuePtr = value;
            return true;
        }

    private:

        class_type* m_ClassType = nullptr;
    };
    template<> struct class_field_info<class_field_type::object_ptr> { using type = class_field_object_ptr; };



    template <typename T>
    void class_type::createClassField(const jutils::jstringID& name, const std::size_t offset)
    {
        if (!name.isValid())
        {
            return;
        }

        const bool uniqueField = !m_Fields.contains(name);
        assert(uniqueField);
        if (!uniqueField)
        {
            delete m_Fields[name];
            m_Fields.remove(name);
        }

        class_field* field = CreateClassField<T>(name, offset);
        const bool fieldCreated = field != nullptr;
        assert(fieldCreated);
        if (!fieldCreated)
        {
            return;
        }

        m_Fields.add(name, field);
    }
    template <typename T>
    void class_type::initClassField(const jutils::jstringID& name)
    {
        class_field** fieldPtr = m_Fields.find(name);
        if (fieldPtr == nullptr)
        {
            return;
        }

        class_field* field = *fieldPtr;
        const bool fieldInitialized = InitClassField<T>(field);
        assert(fieldInitialized);

        if (!fieldInitialized)
        {
            m_Fields.remove(name);
            delete field;
        }
    }

    template <typename T>
    class_field* class_type::CreateClassField(const jutils::jstringID& name, const std::size_t offset)
    {
        if constexpr (class_field_simple<T>::value)
        {
            return new class_field_simple<T>(name, offset);
        }
        else if constexpr (!std::is_pointer_v<T> && has_class_type_v<T>)
        {
            class_type* type = class_type_info<T>::get_class_type();
            if (type != nullptr)
            {
                return new class_field_object(name, offset, type);
            }
        }
        else if constexpr (std::is_pointer_v<T>)
        {
            return new class_field_object_ptr(name, offset);
        }
        return nullptr;
    }
    template <typename T>
    bool class_type::InitClassField(class_field* field)
    {
        if (field == nullptr)
        {
            return false;
        }
        if (field->getType() != class_field_type::object_ptr)
        {
            return true;
        }
        auto* objectField = dynamic_cast<class_field_object_ptr*>(field);
        return (objectField != nullptr) && InitClassField<std::remove_pointer_t<T>>(objectField, 0);
    }
    JUTILS_TEMPLATE_CONDITION_IMPL(has_class_type_v<T>, typename T)
    bool class_type::InitClassField(class_field_object_ptr* field, jutils::int32)
    {
        field->m_ClassType = class_type_info<T>::get_class_type();
        return true;
    }
}

JUTILS_STRING_FORMATTER_CONSTEXPR(jreflect::class_field_type, jreflect::class_field_type_to_string);
