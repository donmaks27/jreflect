// Copyright © 2024 Leonov Maksim. All Rights Reserved.

#pragma once

#include "class_field.h"

#include <cassert>

#include <jutils/jmap.h>
#include <jutils/marco_wrap.h>

namespace jreflect
{
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
        static auto* _helper_get_class_type(jutils::int32) { return T1::GetClassType(); }
        template<typename T1>
        static class_type* _helper_get_class_type(jutils::int8) { return nullptr; }

    public:
        using class_type_t = _helper_class_type_t<T>;
        static constexpr bool has_class_type = !std::is_same_v<_helper_class_type_t<T>, class_type>;

        static auto* get_class_type() { return _helper_get_class_type<T>(0); }
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

        virtual jutils::jstringID getName() const = 0;
        virtual class_type* getParent() const = 0;

        bool isDerived(const class_type* type) const { return (type != nullptr) && isDerivedFromClass(type); }
        JUTILS_TEMPLATE_CONDITION(has_class_type_v<T>, typename T)
        bool isDerived() const { return this->isDerived(class_type_info<T>::get_class_type()); }

        const auto& getFields() const { return m_Fields; }

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
        void createClassField(const jutils::jstringID& name, const std::size_t offset)
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
                m_FieldsOrder.remove(name);
            }

            class_field* field = CreateClassField<T>(name, offset);
            const bool fieldCreated = field != nullptr;
            assert(fieldCreated);
            if (!fieldCreated)
            {
                return;
            }

            m_Fields.add(name, field);
            m_FieldsOrder.addUnique(name);
        }
        template<typename T>
        void initClassField(const jutils::jstringID& name)
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
                m_FieldsOrder.remove(name);

                delete field;
            }
        }
        
        template<typename T>
        static class_field* CreateClassField(const jutils::jstringID& name, const std::size_t offset)
        {
            if constexpr (class_field_primitive<T>::value)
            {
                return new class_field_primitive<T>(name, offset);
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

    private:

        jutils::jmap<jutils::jstringID, class_field*> m_Fields;
        jutils::jarray<jutils::jstringID> m_FieldsOrder;

        template<typename T>
        static bool InitClassField(class_field* field)
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
        JUTILS_TEMPLATE_CONDITION(has_class_type_v<T>, typename T)
        static bool InitClassField(class_field_object_ptr* field, jutils::int32)
        {
            field->m_ObjectType = class_type_info<T>::get_class_type();
            return true;
        }
        template<typename T>
        static bool InitClassField(class_field_object_ptr* field, jutils::int8) { return false; }
    };

    class class_interface
    {
    public:
        class_interface() = default;
        virtual ~class_interface() = default;

        using this_t = class_interface;
        using class_type_t = class_type;

        virtual class_type* getClassType() const = 0;
    };

#define JREFLECT_CLASS_FIELD_NAMED(Field, FieldName)                            \
(jreflect::class_type::create_class_field_info<is_init, decltype(type::Field)>{ \
    .name = (FieldName), .offset = offsetof(type, Field)                        \
})
#define JREFLECT_CLASS_FIELD(Field) JREFLECT_CLASS_FIELD_NAMED(Field, #Field)

#define JREFLECT_HELPER_CREATE_CLASS_FIELD(Info) {                                      \
    auto createInfo = Info;                                                             \
    createClassField<decltype(createInfo)::type>(createInfo.name, createInfo.offset);   \
}
#define JREFLECT_CLASS_TYPE(ClassName, ...)                                                                 \
public:                                                                                                     \
    using parent_t = this_t;                                                                                \
    using this_t = ClassName;                                                                               \
    class class_type_##ClassName : public parent_t::class_type_t                                            \
    {                                                                                                       \
    public:                                                                                                 \
        using type = this_t;                                                                                \
        class_type_##ClassName() { __VA_OPT__(                                                              \
            static constexpr bool is_init = false;                                                          \
            JUTILS_WRAP(JREFLECT_HELPER_CREATE_CLASS_FIELD, __VA_ARGS__)                                    \
            initClassFields_##ClassName();                                                                  \
        ) }                                                                                                 \
        static jutils::jstringID GetName() { return #ClassName; }                                           \
        virtual jutils::jstringID getName() const override { return GetName(); }                            \
        static auto* GetParent() { return jreflect::class_type_info<parent_t>::get_class_type(); }          \
        virtual jreflect::class_type* getParent() const override { return GetParent(); }                    \
    protected:                                                                                              \
        virtual bool isDerivedFromClass(const jreflect::class_type* type) const override                    \
            { return (type::GetClassType() == this) || parent_t::class_type_t::isDerivedFromClass(type); }  \
    private:                                                                                                \
        __VA_OPT__(void initClassFields_##ClassName();)                                                     \
    };                                                                                                      \
    using class_type_t = class_type_##ClassName;                                                            \
    static class_type_t* GetClassType() { static class_type_t classType; return &classType; }               \
    virtual jreflect::class_type* getClassType() const override { return GetClassType(); }
    
#define JREFLECT_HELPER_INIT_CLASS_FIELD(Info) {                \
    auto createInfo = Info;                                     \
    initClassField<decltype(createInfo)::type>(createInfo.name);\
}
#define JREFLECT_INIT_CLASS_TYPE(Namespace, ClassName, ...)                         \
__VA_OPT__(void Namespace::ClassName::class_type_t::initClassFields_##ClassName() {  \
    static constexpr bool is_init = true;                                           \
    JUTILS_WRAP(JREFLECT_HELPER_INIT_CLASS_FIELD, __VA_ARGS__)                      \
})
}
