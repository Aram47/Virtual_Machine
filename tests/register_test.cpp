#include <gtest/gtest.h>

#include "vm/cpu/cpu_context.hpp"
#include "vm/cpu/register_file.hpp"

using namespace vm;

TEST(RegisterFileTest, SetAndGet) {
    RegisterFile registers;
    registers.set(0, 42);
    EXPECT_EQ(registers.get(0), 42u);
}

TEST(RegisterFileTest, CompareUpdatesFlags) {
    RegisterFile registers;
    registers.updateFlagsSub(5, 10, static_cast<Word>(5 - 10));
    EXPECT_FALSE(registers.flags().zero);
    EXPECT_TRUE(registers.flags().sign);
}

TEST(CpuContextTest, SaveAndRestore) {
    CpuContext source;
    source.registers().set(1, 100);
    source.registers().setPc(7);
    source.setTimeSliceLeft(42);

    CpuContext target;
    source.saveTo(target);

    EXPECT_EQ(target.registers().get(1), 100u);
    EXPECT_EQ(target.registers().pc(), 7u);
    EXPECT_EQ(target.timeSliceLeft(), 42u);
}
