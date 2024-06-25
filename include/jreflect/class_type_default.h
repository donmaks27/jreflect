// Copyright © 2024 Leonov Maksim. All Rights Reserved.

#pragma once

#include "class_type.h"

#include <jutils/marco_wrap.h>

#define JREFLECT_CLASS_FIELD_NAMED(Field, FieldName)            \
(jreflect::class_type::create_field_info<decltype(type::Field)>{\
    .name = (FieldName), .offset = offsetof(type, Field)        \
})
#define JREFLECT_CLASS_FIELD(Field) JREFLECT_CLASS_FIELD_NAMED(Field, #Field)

#define JREFLECT_CLASS_TYPE(ClassName, ...)                                                                         \
public:                                                                                                             \
    using parent_t = this_t;                                                                                        \
    using this_t = ClassName;                                                                                       \
    class class_type_##ClassName : public parent_t::class_type_t                                                    \
    {                                                                                                               \
    public:                                                                                                         \
        using type = this_t;                                                                                        \
        class_type_##ClassName() = default;                                                                         \
        [[nodiscard]] static jutils::jstringID GetName() { return #ClassName; }                                     \
        [[nodiscard]] virtual jutils::jstringID getName() const override { return GetName(); }                      \
        [[nodiscard]] static auto* GetParent() { return jreflect::class_type_info<parent_t>::get_class_type(); }    \
        [[nodiscard]] virtual jreflect::class_type* getParent() const override { return GetParent(); }              \
    protected:                                                                                                      \
        __VA_OPT__(virtual void initializeClassType() override {                                                    \
            parent_t::class_type_t::initializeClassType();                                                          \
            initFields_##ClassName();                                                                               \
        })                                                                                                          \
        virtual bool isDerivedFromClass(const jreflect::class_type* classType) const override                       \
            { return (type::GetClassType() == classType) || parent_t::class_type_t::isDerivedFromClass(classType); }\
        virtual class_interface* createObjectInternal() const override { return new type(); }                       \
    private:                                                                                                        \
        __VA_OPT__(void initFields_##ClassName();)                                                                  \
    };                                                                                                              \
    using class_type_t = class_type_##ClassName;                                                                    \
    [[nodiscard]] static class_type_t* GetClassType()  { static class_type_t classType; return &classType; }        \
    [[nodiscard]] virtual jreflect::class_type* getClassType() const override { return GetClassType(); }
    
#define JREFLECT_HELPER_INIT_CLASS_FIELD(Info) {                                \
    auto createInfo = Info;                                                     \
    createField<decltype(createInfo)::type>(createInfo.name, createInfo.offset);\
}
#define JREFLECT_INIT_CLASS_TYPE(Namespace, ClassName, ...)                     \
__VA_OPT__(void Namespace::ClassName::class_type_t::initFields_##ClassName() {  \
    JUTILS_WRAP(JREFLECT_HELPER_INIT_CLASS_FIELD, __VA_ARGS__)                  \
})