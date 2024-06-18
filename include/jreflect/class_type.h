// Copyright © 2024 Leonov Maksim. All Rights Reserved.

#pragma once

#include "field.h"

#include <cassert>

#include <jutils/jmap.h>

namespace jreflect
{
    class class_type;
    class class_interface;
    class field_value_object;

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
        static constexpr bool has_class_type = !std::is_same_v<_helper_class_type_t<jutils::remove_cvref_t<T>>, class_type>;
        using class_type_t = _helper_class_type_t<jutils::remove_cvref_t<T>>;
        
        [[nodiscard]] static auto* get_class_type_raw() { return _helper_get_class_type<jutils::remove_cvref_t<T>, true>(0); }
        [[nodiscard]] static auto* get_class_type() { return _helper_get_class_type<jutils::remove_cvref_t<T>, false>(0); }
    };
    template<typename T>
    using class_type_t = typename class_type_info<T>::class_type_t;
    template<typename T>
    constexpr bool has_class_type_v = class_type_info<T>::has_class_type;
    template<typename T>
    [[nodiscard]] auto* get_class_type_raw() { return class_type_info<T>::get_class_type_raw(); }
    template<typename T>
    [[nodiscard]] auto* get_class_type() { return class_type_info<T>::get_class_type(); }

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

        template<bool IsInit, typename T>
        struct create_field_info
        {
            using type = T;

            jutils::jstringID name = jutils::jstringID_NONE;
            std::size_t offset = 0;
        };

        virtual void initializeClassType() {}

        virtual bool isDerivedFromClass(const class_type* type) const { return false; }

        virtual class_interface* createObjectInternal() const { return nullptr; }
        
        template<typename T, typename FieldValueFactory>
        void createField(const jutils::jstringID& name, std::size_t offset);
        template<typename T>
        void initField(const jutils::jstringID& name);

    private:

        jutils::jmap<jutils::jstringID, field> m_Fields;
        bool m_Initialized = false;
        
        JUTILS_TEMPLATE_CONDITION(has_class_type_v<T>, typename T)
        static bool InitField(field_value_object* field, jutils::int32);
        template<typename T>
        static bool InitField(field_value_object* field, jutils::int8) { return false; }
    };

    class class_interface
    {
        friend field_value_object;

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
        virtual bool copyFromInternal(class_interface&& value) { return copyFromInternal(value); }

    private:

        bool copyFrom(const class_interface& value)
        {
            return (&value != this) && (value.getClassType() == getClassType()) && copyFromInternal(value);
        }
        bool copyFrom(class_interface&& value)
        {
            return (&value != this) && (value.getClassType() == getClassType()) && copyFromInternal(std::move(value));
        }
    };

    class field_value_object : public field_value
    {
        friend class_type;

    protected:
        field_value_object(const field_value_type type, class_type* objectType = nullptr)
            : field_value(type), m_ObjectType(objectType)
        {}
    public:

        [[nodiscard]] class_type* getObjectType() const { return m_ObjectType; }

    protected:

        static bool copyObject(class_interface* dst, const class_interface& src) { return dst->copyFrom(src); }
        static bool copyObject(class_interface* dst, class_interface&& src) { return dst->copyFrom(std::move(src)); }

    private:

        class_type* m_ObjectType = nullptr;
    };

    template<typename T, typename FieldValueFactory>
    void class_type::createField(const jutils::jstringID& name, const std::size_t offset)
    {
        if (!name.isValid())
        {
            return;
        }

        const bool uniqueField = !m_Fields.contains(name);
        assert(uniqueField);
        field_value* fieldValue = create_field_value<T, FieldValueFactory>();
        assert(fieldValue != nullptr);
        if (fieldValue == nullptr)
        {
            return;
        }

        m_Fields.put(name, fieldValue, name, offset);
    }

    template<typename T>
    void class_type::initField(const jutils::jstringID& name)
    {
        auto* fieldPtr = m_Fields.find(name);
        field_value_object* fieldValueObject = fieldPtr != nullptr ? dynamic_cast<field_value_object*>(fieldPtr->getFieldValue()) : nullptr;
        if (fieldValueObject == nullptr)
        {
            return;
        }
        if (fieldValueObject->m_ObjectType != nullptr)
        {
            return;
        }

        const bool fieldInitialized = InitField<std::remove_pointer_t<T>>(fieldValueObject, 0);
        assert(fieldInitialized);
        if (!fieldInitialized)
        {
            m_Fields.remove(name);
        }
    }
    JUTILS_TEMPLATE_CONDITION_IMPL(has_class_type_v<T>, typename T)
    bool class_type::InitField(field_value_object* field, jutils::int32)
    {
        field->m_ObjectType = class_type_info<T>::get_class_type_raw();
        return true;
    }
}
