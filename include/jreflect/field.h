// Copyright © 2024 Leonov Maksim. All Rights Reserved.

#pragma once

#include "field_value.h"

#include <jutils/jstringID.h>

namespace jreflect
{
    class field
    {
    public:
        field(field_value* fieldValue, const jutils::jstringID& name, const std::size_t offset)
            : m_FieldValue(fieldValue), m_Name(name), m_Offset(offset)
        {}
        field(const field&) = delete;
        field(field&& otherField) noexcept
            : m_FieldValue(otherField.m_FieldValue)
            , m_Name(otherField.m_Name)
            , m_Offset(otherField.m_Offset)
        {
            otherField.m_FieldValue = nullptr;
        }
        ~field()
        {
            delete m_FieldValue;
            m_FieldValue = nullptr;
        }

        field& operator=(const field&) = delete;
        field& operator=(field&& otherField) noexcept
        {
            delete m_FieldValue;
            m_FieldValue = otherField.m_FieldValue;
            m_Name = otherField.m_Name;
            m_Offset = otherField.m_Offset;
            otherField.m_FieldValue = nullptr;
            return *this;
        }

        [[nodiscard]] field_value* getFieldValue() const { return m_FieldValue; }
        [[nodiscard]] jutils::jstringID getName() const { return m_Name; }
        [[nodiscard]] std::size_t getOffset() const { return m_Offset; }

        [[nodiscard]] field_value_type getValueType() const
        {
            return m_FieldValue != nullptr ? m_FieldValue->getType() : field_value_type::none;
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

        field_value* m_FieldValue = nullptr;
        jutils::jstringID m_Name = jutils::jstringID_NONE;
        std::size_t m_Offset = 0;
    };
}
