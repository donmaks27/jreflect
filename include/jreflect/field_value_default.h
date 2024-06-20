// Copyright © 2024 Leonov Maksim. All Rights Reserved.

#pragma once

#include "class_type.h"

namespace jreflect
{
    template<field_value_type T>
    struct field_value_default
    {
        using type = field_value_none;
    };
    template<typename T>
    struct field_value_info_default : std::integral_constant<field_value_type, field_value_type::none>
    {
        template<typename Factory>
        static field_value* create() { return nullptr; }
    };


#define JREFLECT_HELPER_DECLARE_DEFAULT_PRIMITIVE_FIELD_VALUE(Enum, Type)                                               \
    class field_value_default_##Enum : public field_value                                                               \
    {                                                                                                                   \
    public:                                                                                                             \
        field_value_default_##Enum() : field_value(field_value_type::Enum) {}                                           \
        virtual ~field_value_default_##Enum() override = default;                                                       \
        bool get(const void* valuePtr, Type& outValue) const                                                            \
        {                                                                                                               \
            if (valuePtr == nullptr) return false;                                                                      \
            outValue = *static_cast<const Type*>(valuePtr);                                                             \
            return true;                                                                                                \
        }                                                                                                               \
        bool set(void* valuePtr, std::conditional_t<std::is_arithmetic_v<Type>, const Type, const Type&> value) const   \
        {                                                                                                               \
            if (valuePtr == nullptr) return false;                                                                      \
            *static_cast<Type*>(valuePtr) = value;                                                                      \
            return true;                                                                                                \
        }                                                                                                               \
    };                                                                                                                  \
    template<> struct field_value_default<field_value_type::Enum> { using type = field_value_default_##Enum; };         \
    template<> struct field_value_info_default<Type> : std::integral_constant<field_value_type, field_value_type::Enum> \
    {                                                                                                                   \
        template<typename Factory> static field_value* create() { return new field_value_default_##Enum(); }            \
    }

    JREFLECT_HELPER_DECLARE_DEFAULT_PRIMITIVE_FIELD_VALUE(boolean,            bool);
    JREFLECT_HELPER_DECLARE_DEFAULT_PRIMITIVE_FIELD_VALUE(   int8, jutils::   int8);
    JREFLECT_HELPER_DECLARE_DEFAULT_PRIMITIVE_FIELD_VALUE(  uint8, jutils::  uint8);
    JREFLECT_HELPER_DECLARE_DEFAULT_PRIMITIVE_FIELD_VALUE(  int16, jutils::  int16);
    JREFLECT_HELPER_DECLARE_DEFAULT_PRIMITIVE_FIELD_VALUE( uint16, jutils:: uint16);
    JREFLECT_HELPER_DECLARE_DEFAULT_PRIMITIVE_FIELD_VALUE(  int32, jutils::  int32);
    JREFLECT_HELPER_DECLARE_DEFAULT_PRIMITIVE_FIELD_VALUE( uint32, jutils:: uint32);
    JREFLECT_HELPER_DECLARE_DEFAULT_PRIMITIVE_FIELD_VALUE(  int64, jutils::  int64);
    JREFLECT_HELPER_DECLARE_DEFAULT_PRIMITIVE_FIELD_VALUE( uint64, jutils:: uint64);
    JREFLECT_HELPER_DECLARE_DEFAULT_PRIMITIVE_FIELD_VALUE( string, jutils::jstring);

    class field_value_default_object : public field_value_object
    {
    public:
        field_value_default_object(class_type* objectType)
            : field_value_object(field_value_type::object, objectType)
        {}
        virtual ~field_value_default_object() override = default;

        bool get(void* valuePtr, class_interface*& outValue) const
        {
            if ((valuePtr == nullptr) || (getObjectType() == nullptr))
            {
                return false;
            }
            outValue = static_cast<class_interface*>(valuePtr);
            return true;
        }
        bool get(const void* valuePtr, const class_interface*& outValue) const
        {
            if ((valuePtr == nullptr) || (getObjectType() == nullptr))
            {
                return false;
            }
            outValue = static_cast<const class_interface*>(valuePtr);
            return true;
        }

        bool set(void* valuePtr, const class_interface& value) const
        {
            if ((valuePtr == nullptr) || (getObjectType() == nullptr))
            {
                return false;
            }
            return copyObject(static_cast<class_interface*>(valuePtr), value);
        }
        bool set(void* valuePtr, class_interface&& value) const
        {
            if ((valuePtr == nullptr) || (getObjectType() == nullptr))
            {
                return false;
            }
            return copyObject(static_cast<class_interface*>(valuePtr), std::move(value));
        }
    };
    template<> struct field_value_default<field_value_type::object>
    {
        using type = field_value_default_object;
    };
    JUTILS_TEMPLATE_CONDITION((!std::is_pointer_v<T> && has_class_type_v<T>), typename T)
    struct field_value_info_default<T> : std::integral_constant<field_value_type, field_value_type::object>
    {
        template<typename Factory>
        static field_value* create()
        {
            return new field_value_default_object(get_class_type_raw<T>());
        }
    };
    
    class field_value_default_object_ptr : public field_value_object
    {
    public:
        field_value_default_object_ptr(class_type* objectType)
            : field_value_object(field_value_type::object_ptr, objectType)
        {}
        virtual ~field_value_default_object_ptr() override = default;

        bool get(const void* valuePtr, class_interface*& outValue) const
        {
            if ((valuePtr == nullptr) || (getObjectType() == nullptr))
            {
                return false;
            }
            outValue = *static_cast<class_interface* const*>(valuePtr);
            return true;
        }
        bool set(void* valuePtr, class_interface* value) const
        {
            if ((valuePtr == nullptr) || (getObjectType() == nullptr))
            {
                return false;
            }
            if (value != nullptr)
            {
                const class_type* valueType = value->getClassType();
                if ((valueType == nullptr) || !valueType->isDerivedFrom(getObjectType()))
                {
                    return false;
                }
            }
            *static_cast<class_interface**>(valuePtr) = value;
            return true;
        }
    };
    template<> struct field_value_default<field_value_type::object_ptr>
    {
        using type = field_value_default_object_ptr;
    };
    JUTILS_TEMPLATE_CONDITION((std::is_pointer_v<T>), typename T)
    struct field_value_info_default<T> : std::integral_constant<field_value_type, field_value_type::object_ptr>
    {
        template<typename Factory>
        static field_value* create()
        {
            return new field_value_default_object_ptr(get_class_type_raw<std::remove_pointer_t<T>>());
        }
    };

    class field_value_default_array : public field_value
    {
    protected:
        field_value_default_array(field_value* elementValue)
            : field_value(field_value_type::array), m_ElementValue(elementValue)
        {}
    public:
        virtual ~field_value_default_array() override = default;

        [[nodiscard]] field_value* getElementFieldValue() const { return m_ElementValue; }

        [[nodiscard]] virtual jutils::index_type getSize(const void* valuePtr) const = 0;

        [[nodiscard]] virtual void* get(void* valuePtr, jutils::index_type i) const = 0;
        [[nodiscard]] virtual const void* get(const void* valuePtr, jutils::index_type i) const = 0;

        virtual void* add(void* valuePtr, jutils::index_type i = jutils::index_invalid) const = 0;

        virtual void remove(void* valuePtr, jutils::index_type i) const = 0;

        virtual void clear(void* valuePtr) const = 0;

    private:

        field_value* m_ElementValue = nullptr;
    };
    template<typename T>
    class field_value_default_array_impl : public field_value_default_array
    {
    public:
        field_value_default_array_impl(field_value* elementValue)
            : field_value_default_array(elementValue)
        {}
        virtual ~field_value_default_array_impl() override = default;

        virtual jutils::index_type getSize(const void* valuePtr) const override
        {
            return valuePtr != nullptr ? GetArray(valuePtr)->getSize() : jutils::index_invalid;
        }

        virtual void* get(void* valuePtr, const jutils::index_type i) const override
        {
            if (valuePtr == nullptr)
            {
                return nullptr;
            }
            auto* valueArray = GetArray(valuePtr);
            if (!valueArray->isValidIndex(i))
            {
                return nullptr;
            }
            return &valueArray->get(i);
        }
        virtual const void* get(const void* valuePtr, const jutils::index_type i) const override
        {
            if (valuePtr == nullptr)
            {
                return nullptr;
            }
            const auto* valueArray = GetArray(valuePtr);
            if (!valueArray->isValidIndex(i))
            {
                return nullptr;
            }
            return &valueArray->get(i);
        }

        virtual void* add(void* valuePtr, const jutils::index_type i) const override
        {
            return valuePtr != nullptr ? &GetArray(valuePtr)->addDefaultAt(i) : nullptr;
        }

        virtual void remove(void* valuePtr, const jutils::index_type i) const override
        {
            if (valuePtr != nullptr)
            {
                GetArray(valuePtr)->removeAt(i);
            }
        }

        virtual void clear(void* valuePtr) const override
        {
            if (valuePtr != nullptr)
            {
                GetArray(valuePtr)->clear();
            }
        }

    private:

        static jutils::jarray<T>* GetArray(void* valuePtr) { return static_cast<jutils::jarray<T>*>(valuePtr); }
        static const jutils::jarray<T>* GetArray(const void* valuePtr) { return static_cast<const jutils::jarray<T>*>(valuePtr); }
    };
    template<> struct field_value_default<field_value_type::array>
    {
        using type = field_value_default_array;
    };
    template<typename ElemType>
    struct field_value_info_default<jarray<ElemType>> : std::integral_constant<field_value_type, field_value_type::array>
    {
        template<typename Factory>
        static field_value* create()
        {
            return new field_value_default_array_impl<ElemType>(create_field_value<ElemType, Factory>());
        }
    };


    struct field_value_factory_default
    {
        template<field_value_type Type>
        using field_value_t = typename field_value_default<Type>::type;
        template<typename T>
        using field_value_into = field_value_info_default<T>;
        template<typename T>
        [[nodiscard]] static field_value* create() { return field_value_into<T>::template create<field_value_factory_default>(); }
    };
    template<field_value_type Type>
    using field_value_default_t = field_value_t<Type, field_value_factory_default>;
    // field_value_info_default
    template<typename T>
    [[nodiscard]] field_value* create_field_value_default() { return create_field_value<T, field_value_factory_default>(); }
}
