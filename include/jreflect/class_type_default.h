// Copyright © 2024 Leonov Maksim. All Rights Reserved.

#pragma once

#include "class_type.h"

#include <jutils/marco_wrap.h>

#define JREFLECT_CLASS_FIELD_NAMED(Field, FieldName)                            \
(jreflect::class_type::create_class_field_info<is_init, decltype(type::Field)>{ \
    .name = (FieldName), .offset = offsetof(type, Field)                        \
})
#define JREFLECT_CLASS_FIELD(Field) JREFLECT_CLASS_FIELD_NAMED(Field, #Field)

#define JREFLECT_HELPER_CREATE_CLASS_FIELD(Info) {                                      \
    auto createInfo = Info;                                                             \
    createClassField<decltype(createInfo)::type>(createInfo.name, createInfo.offset);   \
}
#define JREFLECT_CLASS_TYPE(ClassName, ...)                                                                     \
public:                                                                                                         \
    using parent_t = this_t;                                                                                    \
    using this_t = ClassName;                                                                                   \
    class class_type_##ClassName : public parent_t::class_type_t                                                \
    {                                                                                                           \
    public:                                                                                                     \
        using type = this_t;                                                                                    \
        class_type_##ClassName() { __VA_OPT__(                                                                  \
            static constexpr bool is_init = false;                                                              \
            JUTILS_WRAP(JREFLECT_HELPER_CREATE_CLASS_FIELD, __VA_ARGS__)                                        \
            initClassFields_##ClassName();                                                                      \
        ) }                                                                                                     \
        [[nodiscard]] static jutils::jstringID GetName() { return #ClassName; }                                 \
        [[nodiscard]] virtual jutils::jstringID getName() const override { return GetName(); }                  \
        [[nodiscard]] static auto* GetParent() { return jreflect::class_type_info<parent_t>::get_class_type(); }\
        [[nodiscard]] virtual jreflect::class_type* getParent() const override { return GetParent(); }          \
    protected:                                                                                                  \
        virtual bool isDerivedFromClass(const jreflect::class_type* type) const override                        \
            { return (type::GetClassType() == this) || parent_t::class_type_t::isDerivedFromClass(type); }      \
    private:                                                                                                    \
        __VA_OPT__(void initClassFields_##ClassName();)                                                         \
    };                                                                                                          \
    using class_type_t = class_type_##ClassName;                                                                \
    [[nodiscard]] static class_type_t* GetClassType() { static class_type_t classType; return &classType; }     \
    [[nodiscard]] virtual jreflect::class_type* getClassType() const override { return GetClassType(); }        \
protected:                                                                                                      \
    virtual bool copyFromInternal(const jreflect::class_interface& value) override                              \
    {                                                                                                           \
        if constexpr (std::is_copy_assignable_v<this_t>)                                                        \
        {                                                                                                       \
            *this = dynamic_cast<const this_t&>(value);                                                         \
            return true;                                                                                        \
        }                                                                                                       \
        assert(std::is_copy_assignable_v<ClassName>);                                                           \
        return false;                                                                                           \
    }                                                                                                           \
    virtual bool moveFromInternal(class_interface&& value)                                                      \
    {                                                                                                           \
        if constexpr (std::is_move_assignable_v<this_t>)                                                        \
        {                                                                                                       \
            *this = std::move(dynamic_cast<this_t&&>(value));                                                   \
            return true;                                                                                        \
        }                                                                                                       \
        return copyFromInternal(value);                                                                         \
    }                                                                                                           \
public:
    
#define JREFLECT_HELPER_INIT_CLASS_FIELD(Info) {                    \
        auto createInfo = Info;                                     \
        initClassField<decltype(createInfo)::type>(createInfo.name);\
    }
#define JREFLECT_INIT_CLASS_TYPE(Namespace, ClassName, ...)                             \
    __VA_OPT__(void Namespace::ClassName::class_type_t::initClassFields_##ClassName() { \
        static constexpr bool is_init = true;                                           \
        JUTILS_WRAP(JREFLECT_HELPER_INIT_CLASS_FIELD, __VA_ARGS__)                      \
    })