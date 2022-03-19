#pragma once

#include <string>

namespace {
    // "Магическое" число, используемое для отслеживания живости объекта
    inline const uint32_t DEFAULT_COOKIE = 0xdeadbeef;

    struct TestObj {
        TestObj() = default;

        TestObj(const TestObj& other) = default;

        TestObj& operator=(const TestObj& other) = default;

        TestObj(TestObj&& other) = default;

        TestObj& operator=(TestObj&& other) = default;

        ~TestObj();

        [[nodiscard]] bool IsAlive() const noexcept;

        uint32_t cookie = DEFAULT_COOKIE;
    };

    struct Obj {
        Obj();

        explicit Obj(int id);

        Obj(int id, std::string name);

        Obj(const Obj& other);

        Obj(Obj&& other) noexcept;

        Obj& operator=(const Obj& other);

        Obj& operator=(Obj&& other) noexcept;

        ~Obj();

        static int GetAliveObjectCount();

        static void ResetCounters();

        bool throw_on_copy = false;
        int id = 0;
        std::string name;

        static inline int default_construction_throw_countdown = 0;
        static inline int num_default_constructed = 0;
        static inline int num_constructed_with_id = 0;
        static inline int num_constructed_with_id_and_name = 0;
        static inline int num_copied = 0;
        static inline int num_moved = 0;
        static inline int num_destroyed = 0;
        static inline int num_assigned = 0;
        static inline int num_move_assigned = 0;
    };
}

void Test1();

void Test2();

void Test3();

void Test4();

void Test5();

void Test6();

namespace {
    struct C {
        C() noexcept;

        C(const C& /*other*/) noexcept;

        C(C&& /*other*/) noexcept;

        C& operator=(const C& other) noexcept;

        C& operator=(C&& /*other*/) noexcept;

        ~C();

        static void Reset();

        inline static size_t def_ctor = 0;
        inline static size_t copy_ctor = 0;
        inline static size_t move_ctor = 0;
        inline static size_t copy_assign = 0;
        inline static size_t move_assign = 0;
        inline static size_t dtor = 0;
    };
}

void Dump();

void Benchmark();