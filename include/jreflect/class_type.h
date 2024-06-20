// Copyright © 2024 Leonov Maksim. All Rights Reserved.

#pragma once

#include "field.h"

#include <cassert>

#include <jutils/jmap.h>

namespace jreflect
{
    class class_type;
    class field_value_object;

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

        inline bool copyFrom(const class_interface& value);
        inline bool copyFrom(class_interface&& value);
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
        
        template<typename T, typename FieldValueFactory>
        void createField(const jutils::jstringID& name, std::size_t offset)
        {
            if (!name.isValid())
            {
                return;
            }

            assert(!m_Fields.contains(name));

            field_value* createdFieldValue = create_field_value<T, FieldValueFactory>();
            assert(createdFieldValue != nullptr);
            if (createdFieldValue == nullptr)
            {
                return;
            }

            m_Fields.put(name, createdFieldValue, name, offset);
        }

    private:

        jutils::jmap<jutils::jstringID, field> m_Fields;
        bool m_Initialized = false;
    };

    class field_value_object : public field_value
    {
    protected:
        field_value_object(const field_value_type type, class_type* objectType)
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

    inline bool class_interface::copyFrom(const class_interface& value)
    {
        return (&value != this) && (value.getClassType() == getClassType()) && copyFromInternal(value);
    }
    inline bool class_interface::copyFrom(class_interface&& value)
    {
        return (&value != this) && (value.getClassType() == getClassType()) && copyFromInternal(std::move(value));
    }
}
