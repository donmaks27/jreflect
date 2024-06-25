// Copyright © 2024 Leonov Maksim. All Rights Reserved.

#pragma once

#include <cassert>

#include <jutils/jmap.h>
#include <jutils/jstringID.h>

namespace jreflect
{
    class class_type;
    class value;



    enum class value_type : jutils::uint8
    {
        none,
        boolean, int8, uint8, int16, uint16, int32, uint32, int64, uint64, string,
        object, object_ptr,
        array, array_bool
    };
    [[nodiscard]] constexpr const char* value_type_to_string(const value_type type)
    {
        switch (type)
        {
        case value_type::boolean:    return "boolean";
        case value_type::int8:       return "int8";
        case value_type::uint8:      return "uint8";
        case value_type::int16:      return "int16";
        case value_type::uint16:     return "uint16";
        case value_type::int32:      return "int32";
        case value_type::uint32:     return "uint32";
        case value_type::int64:      return "int64";
        case value_type::uint64:     return "uint64";
        case value_type::string:     return "string";
        case value_type::object:     return "object";
        case value_type::object_ptr: return "object_ptr";
        case value_type::array:      return "array";
        case value_type::array_bool: return "array_bool";
        default: ;
        }
        return "NONE";
    }

    template<value_type Type>
    struct value_type_info
    {
        using type = value;
    };
    template<typename T>
    struct value_info : std::integral_constant<value_type, value_type::none>
    {
        static jreflect::value* create() { return nullptr; }
    };
    template<value_type Type>
    using value_t = typename value_type_info<Type>::type;
    template<typename T>
    static constexpr value_type value_type_v = value_info<T>::value;
    template<typename T>
    [[nodiscard]] jreflect::value* create_value() { return value_info<T>::create(); }
    
    class value
    {
    protected:
        value(const value_type type) : m_Type(type) {}
    public:
        value(const value&) = delete;
        value(value&&) noexcept = delete;
        virtual ~value() = default;

        value& operator=(const value&) = delete;
        value& operator=(value&&) noexcept = delete;

        [[nodiscard]] value_type getType() const { return m_Type; }

        template<value_type Type>
        [[nodiscard]] const value_t<Type>* cast() const
        {
            if constexpr (Type == value_type::none)
            {
                return nullptr;
            }
            return Type == getType() ? dynamic_cast<const value_t<Type>*>(this) : nullptr;
        }

    private:

        value_type m_Type = value_type::none;
    };

    

    class class_interface
    {
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
    };



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

        JUTILS_TEMPLATE_CONDITION(_helper_has_class_type<T1>, typename T1, bool Raw)
        [[nodiscard]] static auto* _helper_get_class_type(jutils::int32) { return Raw ? T1::GetClassType_Raw() : T1::GetClassType(); }
        template<typename T1, bool>
        [[nodiscard]] static class_type* _helper_get_class_type(jutils::int8) { return nullptr; }

    public:
        static constexpr bool has_class_type = _helper_has_class_type<jutils::remove_cvref_t<T>>;//!std::is_same_v<_helper_class_type_t<jutils::remove_cvref_t<T>>, class_type>;
        using class_type_t = _helper_class_type_t<jutils::remove_cvref_t<T>>;
        
        [[nodiscard]] static auto* get_class_type_raw() { return _helper_get_class_type<jutils::remove_cvref_t<T>, true>(0); }
        [[nodiscard]] static auto* get_class_type() { return _helper_get_class_type<jutils::remove_cvref_t<T>, false>(0); }
    };
    template<typename T>
    using class_type_t = typename class_type_info<T>::class_type_t;
    template<typename T>
    constexpr bool has_class_type_v = class_type_info<T>::has_class_type;
    // TODO: Get rid of raw functions, move initialization to generated file
    template<typename T>
    [[nodiscard]] auto* get_class_type_raw() { return class_type_info<T>::get_class_type_raw(); }
    template<typename T>
    [[nodiscard]] auto* get_class_type() { return class_type_info<T>::get_class_type(); }



    class class_field
    {
    public:
        class_field(value* fieldValue, const jutils::jstringID& name, const std::size_t offset)
            : m_Value(fieldValue), m_Name(name), m_Offset(offset)
        {}
        class_field(const class_field&) = delete;
        class_field(class_field&& otherField) noexcept
            : m_Value(otherField.m_Value)
            , m_Name(otherField.m_Name)
            , m_Offset(otherField.m_Offset)
        {
            otherField.m_Value = nullptr;
        }
        ~class_field()
        {
            delete m_Value;
            m_Value = nullptr;
        }

        class_field& operator=(const class_field&) = delete;
        class_field& operator=(class_field&& otherField) noexcept
        {
            delete m_Value;
            m_Value = otherField.m_Value;
            m_Name = otherField.m_Name;
            m_Offset = otherField.m_Offset;
            otherField.m_Value = nullptr;
            return *this;
        }

        [[nodiscard]] value* getValue() const { return m_Value; }
        [[nodiscard]] jutils::jstringID getName() const { return m_Name; }
        [[nodiscard]] std::size_t getOffset() const { return m_Offset; }

        [[nodiscard]] value_type getValueType() const
        {
            return m_Value != nullptr ? m_Value->getType() : value_type::none;
        }
        [[nodiscard]] void* getValuePtr(class_interface* object) const
        {
            return object != nullptr ? (reinterpret_cast<jutils::uint8*>(object) + getOffset()) : nullptr;
        }
        [[nodiscard]] const void* getValuePtr(const class_interface* object) const
        {
            return object != nullptr ? (reinterpret_cast<const jutils::uint8*>(object) + getOffset()) : nullptr;
        }

    private:

        value* m_Value = nullptr;
        jutils::jstringID m_Name = jutils::jstringID_NONE;
        std::size_t m_Offset = 0;
    };

    class class_type
    {
    public:
        class_type() = default;
        virtual ~class_type() = default;

        void initialize()
        {
            if (!m_Initialized)
            {
                m_Initialized = true;
                initializeClassType();
            }
        }

        [[nodiscard]] virtual jutils::jstringID getName() const = 0;
        [[nodiscard]] virtual class_type* getParent() const = 0;

        [[nodiscard]] bool isDerivedFrom(const class_type* type) const { return (type != nullptr) && isDerivedFromClass(type); }
        JUTILS_TEMPLATE_CONDITION(has_class_type_v<T>, typename T)
        [[nodiscard]] bool isDerivedFrom() const { return this->isDerivedFrom(class_type_info<T>::get_class_type()); }

        [[nodiscard]] const auto& getFields() const { return m_Fields; }

    protected:

        template<typename T>
        struct create_field_info
        {
            using type = T;

            jutils::jstringID name = jutils::jstringID_NONE;
            std::size_t offset = 0;
        };

        virtual void initializeClassType() {}

        virtual bool isDerivedFromClass(const class_type* type) const { return false; }

        virtual class_interface* createObjectInternal() const { return nullptr; }
        
        template<typename T>
        void createField(const jutils::jstringID& name, const std::size_t offset)
        {
            if (!name.isValid())
            {
                return;
            }

            assert(!m_Fields.contains(name));

            value* createdValue = create_value<T>();
            assert(createdValue != nullptr);
            if (createdValue == nullptr)
            {
                return;
            }

            m_Fields.put(name, createdValue, name, offset);
        }

    private:

        jutils::jmap<jutils::jstringID, class_field> m_Fields;
        bool m_Initialized = false;
    };



#define JREFLECT_HELPER_DECLARE_PRIMITIVE_VALUE(Enum, Type)                                     \
    class value_##Enum : public value                                                           \
    {                                                                                           \
    public:                                                                                     \
        value_##Enum() : value(value_type::Enum) {}                                             \
        virtual ~value_##Enum() override = default;                                             \
        bool get(const void* valuePtr, Type& outValue) const                                    \
        {                                                                                       \
            if (valuePtr == nullptr) return false;                                              \
            outValue = *static_cast<const Type*>(valuePtr);                                     \
            return true;                                                                        \
        }                                                                                       \
        bool set(void* valuePtr, const Type& value) const                                       \
        {                                                                                       \
            if (valuePtr == nullptr) return false;                                              \
            *static_cast<Type*>(valuePtr) = value;                                              \
            return true;                                                                        \
        }                                                                                       \
    };                                                                                          \
    template<> struct value_type_info<value_type::Enum> { using type = value_##Enum; };         \
    template<> struct value_info<Type> : std::integral_constant<value_type, value_type::Enum>   \
    {                                                                                           \
        static jreflect::value* create() { return new value_type_info<value>::type(); }         \
    };

    JREFLECT_HELPER_DECLARE_PRIMITIVE_VALUE(boolean,            bool);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_VALUE(   int8, jutils::   int8);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_VALUE(  uint8, jutils::  uint8);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_VALUE(  int16, jutils::  int16);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_VALUE( uint16, jutils:: uint16);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_VALUE(  int32, jutils::  int32);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_VALUE( uint32, jutils:: uint32);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_VALUE(  int64, jutils::  int64);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_VALUE( uint64, jutils:: uint64);
    JREFLECT_HELPER_DECLARE_PRIMITIVE_VALUE( string, jutils::jstring);

    class value_object : public value
    {
    protected:
        value_object(class_type* objectType)
            : value(value_type::object), m_ObjectType(objectType)
        {}
    public:
        virtual ~value_object() override = default;

        [[nodiscard]] class_type* getObjectType() const { return m_ObjectType; }

        bool get(void* valuePtr, class_interface*& outValue) const
        {
            if (valuePtr == nullptr)
            {
                return false;
            }
            outValue = static_cast<class_interface*>(valuePtr);
            return true;
        }
        bool get(const void* valuePtr, const class_interface*& outValue) const
        {
            if (valuePtr == nullptr)
            {
                return false;
            }
            outValue = static_cast<const class_interface*>(valuePtr);
            return true;
        }
        bool get(const void* valuePtr, class_interface& outValue) const
        {
            if ((valuePtr == nullptr) || (outValue.getClassType() != m_ObjectType))
            {
                return false;
            }
            return copyAssign(outValue, *static_cast<const class_interface*>(valuePtr));
        }

        bool set(void* valuePtr, const class_interface* v) const
        {
            return (v != nullptr) && set(valuePtr, *v);
        }
        bool set(void* valuePtr, const class_interface& v) const
        {
            if ((valuePtr == nullptr) || (v.getClassType() != m_ObjectType))
            {
                return false;
            }
            return copyAssign(*static_cast<class_interface*>(valuePtr), v);
        }
        bool set(void* valuePtr, class_interface&& v) const
        {
            if ((valuePtr == nullptr) || (v.getClassType() != m_ObjectType))
            {
                return false;
            }
            return moveAssign(*static_cast<class_interface*>(valuePtr), std::move(v));
        }

    protected:

        virtual bool copyAssign(class_interface& dst, const class_interface& src) const = 0;
        virtual bool moveAssign(class_interface& dst, class_interface&& src) const = 0;

    private:

        class_type* m_ObjectType = nullptr;
    };
    JUTILS_TEMPLATE_CONDITION((has_class_type_v<T>), typename T)
    class value_object_impl : public value_object
    {
    public:
        value_object_impl() : value_object(get_class_type_raw<T>()) {}

    protected:

        virtual bool copyAssign(class_interface& dst, const class_interface& src) const override
        {
            if constexpr (std::is_copy_assignable_v<T>)
            {
                dynamic_cast<T&>(dst) = dynamic_cast<const T&>(src);
                return true;
            }
            return false;
        }
        virtual bool moveAssign(class_interface& dst, class_interface&& src) const override
        {
            if constexpr (std::is_move_assignable_v<T>)
            {
                dynamic_cast<T&>(dst) = std::move(dynamic_cast<T&&>(src));
                return true;
            }
            return copyAssign(dst, src);
        }
    };
    template<> struct value_type_info<value_type::object> { using type = value_object; };
    JUTILS_TEMPLATE_CONDITION((has_class_type_v<T>), typename T)
    struct value_info<T> : std::integral_constant<value_type, value_type::object>
    {
        static jreflect::value* create() { return new value_object_impl<T>(); }
    };

    class value_object_ptr : public value
    {
    protected:
        value_object_ptr(class_type* objectType)
            : value(value_type::object_ptr), m_ObjectType(objectType)
        {}
    public:
        virtual ~value_object_ptr() override = default;

        [[nodiscard]] class_type* getObjectType() const { return m_ObjectType; }

        bool get(void* valuePtr, class_interface*& outValue) const
        {
            if (valuePtr == nullptr)
            {
                return false;
            }
            outValue = getObjectPtr(valuePtr);
            return true;
        }

        bool set(void* valuePtr, class_interface* v) const
        {
            if (valuePtr == nullptr)
            {
                return false;
            }
            if (v != nullptr)
            {
                const auto* classType = v->getClassType();
                if ((classType == nullptr) || !classType->isDerivedFrom(m_ObjectType))
                {
                    return false;
                }
            }
            setObjectPtr(valuePtr, v);
            return true;
        }

    protected:

        virtual class_interface* getObjectPtr(void* valuePtr) const = 0;
        virtual void setObjectPtr(void* valuePtr, class_interface* v) const = 0;

    private:

        class_type* m_ObjectType = nullptr;
    };
    template<typename T>
    class value_object_ptr_impl
    {
    public:
        static constexpr bool valid = false;
    };
    JUTILS_TEMPLATE_CONDITION((
        !has_class_type_v<T> && std::is_reference_v<decltype(*std::declval<T>())>
    ), typename T)
    class value_object_ptr_impl<T> : public value_object_ptr
    {
    public:
        using ptr_type = T;
        using type = jutils::remove_cvref_t<decltype(*std::declval<ptr_type>())>;

        static_assert(has_class_type_v<type>);
        static_assert(std::is_same_v<bool, decltype(std::declval<ptr_type>() == nullptr)>);
        static_assert(std::is_same_v<ptr_type, type*> || std::is_assignable_v<ptr_type, type*>);

        static constexpr bool valid = true;

        value_object_ptr_impl()
            : value_object_ptr(get_class_type_raw<type>())
        {}

    protected:

        virtual class_interface* getObjectPtr(void* valuePtr) const override
        {
            ptr_type& objectPtr = *static_cast<ptr_type*>(valuePtr);
            return !(objectPtr == nullptr) ? &*objectPtr : nullptr;
        }
        virtual void setObjectPtr(void* valuePtr, class_interface* v) const override
        {
            *static_cast<ptr_type*>(valuePtr) = dynamic_cast<type*>(v);
        }
    };
    template<> struct value_type_info<value_type::object_ptr> { using type = value_object_ptr; };
    JUTILS_TEMPLATE_CONDITION((value_object_ptr_impl<T>::valid), typename T)
    struct value_info<T> : std::integral_constant<value_type, value_type::object_ptr>
    {
        static jreflect::value* create() { return new value_object_ptr_impl<T>(); }
    };

    class value_array_bool : public value
    {
    public:
        value_array_bool() : value(value_type::array_bool) {}

        [[nodiscard]] jutils::index_type getSize(const void* valuePtr) const
        {
            return valuePtr != nullptr ? GetArray(valuePtr)->size() : jutils::index_invalid;
        }

        bool get(const void* valuePtr, const jutils::index_type index, bool& outValue) const
        {
            auto* valueArray = valuePtr != nullptr ? GetArray(valuePtr) : nullptr;
            if ((valueArray == nullptr) || (index < 0) || (index >= valueArray->size()))
            {
                return false;
            }
            outValue = valueArray->at(index);
            return true;
        }
        bool set(void* valuePtr, const jutils::index_type index, const bool newValue) const
        {
            auto* valueArray = valuePtr != nullptr ? GetArray(valuePtr) : nullptr;
            if ((valueArray == nullptr) || (index < 0) || (index >= valueArray->size()))
            {
                return false;
            }
            valueArray->at(index) = newValue;
            return true;
        }

        bool add(void* valuePtr, const jutils::index_type index = jutils::index_invalid, const bool newValue = false) const
        {
            auto* valueArray = valuePtr != nullptr ? GetArray(valuePtr) : nullptr;
            if (valueArray == nullptr)
            {
                return false;
            }
            valueArray->emplace(std::next(valueArray->begin(), jutils::math::min(index, valueArray->size())), newValue);
            return true;
        }
        void remove(void* valuePtr, const jutils::index_type index) const
        {
            auto* valueArray = valuePtr != nullptr ? GetArray(valuePtr) : nullptr;
            if ((valueArray != nullptr) && (index < valueArray->size()))
            {
                valueArray->erase(std::next(valueArray->begin(), index));
            }
        }
        void clear(void* valuePtr) const
        {
            auto* valueArray = valuePtr != nullptr ? GetArray(valuePtr) : nullptr;
            if (valueArray != nullptr)
            {
                valueArray->clear();
            }
        }

    private:

        static std::vector<bool>* GetArray(void* valuePtr) { return static_cast<std::vector<bool>*>(valuePtr); }
        static const std::vector<bool>* GetArray(const void* valuePtr) { return static_cast<const std::vector<bool>*>(valuePtr); }
    };
    template<> struct value_type_info<value_type::array_bool> { using type = value_array_bool; };
    template<> struct value_info<std::vector<bool>> : std::integral_constant<value_type, value_type::array_bool>
    {
        static jreflect::value* create() { return new value_array_bool(); }
    };

    class value_array : public value
    {
    protected:
        value_array(value* elementValue)
            : value(value_type::array), m_ElementValue(elementValue)
        {}
    public:

        [[nodiscard]] value* getElementValue() const { return m_ElementValue; }

        [[nodiscard]] virtual jutils::index_type getSize(const void* valuePtr) const = 0;

        [[nodiscard]] virtual void* get(void* valuePtr, jutils::index_type index) const = 0;
        [[nodiscard]] virtual const void* get(const void* valuePtr, jutils::index_type index) const = 0;

        virtual void* add(void* valuePtr, jutils::index_type index = jutils::index_invalid) const = 0;
        virtual void remove(void* valuePtr, jutils::index_type index) const = 0;
        virtual void clear(void* valuePtr) const = 0;

    private:

        value* m_ElementValue = nullptr;
    };
    template<typename T>
    class value_array_impl : public value_array
    {
    public:
        value_array_impl() : value_array(create_value<T>()) {}

        virtual jutils::index_type getSize(const void* valuePtr) const override
        {
            return valuePtr != nullptr ? GetArray(valuePtr)->getSize() : jutils::index_invalid;
        }

        virtual void* get(void* valuePtr, const jutils::index_type index) const override
        {
            if (valuePtr == nullptr)
            {
                return nullptr;
            }
            auto* valueArray = GetArray(valuePtr);
            return valueArray->isValidIndex(index) ? &valueArray->get(index) : nullptr;
        }
        virtual const void* get(const void* valuePtr, const jutils::index_type index) const override
        {
            if (valuePtr == nullptr)
            {
                return nullptr;
            }
            const auto* valueArray = GetArray(valuePtr);
            return valueArray->isValidIndex(index) ? &valueArray->get(index) : nullptr;
        }

        virtual void* add(void* valuePtr, const jutils::index_type index) const override
        {
            return valuePtr != nullptr ? &GetArray(valuePtr)->addDefaultAt(index) : nullptr;
        }
        virtual void remove(void* valuePtr, const jutils::index_type index) const override
        {
            if (valuePtr != nullptr)
            {
                GetArray(valuePtr)->removeAt(index);
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
    template<> struct value_type_info<value_type::array> { using type = value_array; };
    JUTILS_TEMPLATE_CONDITION((!std::is_same_v<T, bool> && (value_info<T>::value != value_type::none)), typename T)
    struct value_info<jutils::jarray<T>> : std::integral_constant<value_type, value_type::array>
    {
        static jreflect::value* create() { return new value_array_impl<T>(); }
    };
}

JUTILS_STRING_FORMATTER_CONSTEXPR(jreflect::value_type, jreflect::value_type_to_string);
