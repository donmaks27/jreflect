// Copyright © 2024 Leonov Maksim. All Rights Reserved.

#pragma once

#include "class_type.h"

namespace jreflect
{
    jutils::jarray<jreflect::class_type*> get_all_class_types();

    class database
    {
    private:
        database() { initDatabase(); }
        ~database() = default;

    public:
        
        static void CreateInstance() noexcept
        {
            if (Instance == nullptr)
            {
                Instance = new database();
            }
        }
        [[nodiscard]] static database* GetInstanse() noexcept
        {
            CreateInstance();
            return Instance;
        }
        static void ClearInstance() noexcept
        {
            if (Instance != nullptr)
            {
                delete Instance;
                Instance = nullptr;
            }
        }

        [[nodiscard]] const auto& getClassTypes() const { return m_ClassTypes; }

    private:

        inline static database* Instance = nullptr;

        jutils::jmap<jutils::jstringID, jreflect::class_type*> m_ClassTypes;


        void initDatabase()
        {
            const auto classTypes = jreflect::get_all_class_types();
            for (const auto& classType : classTypes)
            {
                assert(classType != nullptr);
                if (classType != nullptr)
                {
                    m_ClassTypes.add(classType->getName(), classType);
                }
            }
        }
    };
}
