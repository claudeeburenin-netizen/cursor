#include "entry_state.h"

#include <cstdint>
#include <limits>

#include <gtest/gtest.h>

namespace {

TEST(EntryStateTest, DefaultConstructedFieldsAreZeroed) {
  EntryState state;
  EXPECT_EQ(state.version, 0u);
  EXPECT_EQ(state.count, 0);
  EXPECT_FALSE(state.multiple_versions);
}

TEST(EntryStateTest, FirstAddStoresVersionWhenNotReusable) {
  EntryState state;
  state.add(/*new_version=*/42, /*is_reusable_size=*/false);

  EXPECT_EQ(state.version, 42u);
  EXPECT_EQ(state.count, 1);
  // The very first add cannot produce multiple_versions: the count==0 branch
  // overwrites `version` to `new_version`, so the equality check that follows
  // always succeeds.
  EXPECT_FALSE(state.multiple_versions);
}

TEST(EntryStateTest, FirstAddStoresVersionWhenReusable) {
  EntryState state;
  state.add(/*new_version=*/7, /*is_reusable_size=*/true);

  EXPECT_EQ(state.version, 7u);
  EXPECT_EQ(state.count, 1);
  EXPECT_FALSE(state.multiple_versions);
}

TEST(EntryStateTest, FirstAddCanStoreZeroVersion) {
  EntryState state;
  state.add(/*new_version=*/0, /*is_reusable_size=*/false);

  EXPECT_EQ(state.version, 0u);
  EXPECT_EQ(state.count, 1);
  EXPECT_FALSE(state.multiple_versions);
}

TEST(EntryStateTest, CountIncrementsOnEveryAdd) {
  EntryState state;
  for (int i = 1; i <= 5; ++i) {
    state.add(/*new_version=*/100, /*is_reusable_size=*/true);
    EXPECT_EQ(state.count, i);
  }
}

TEST(EntryStateTest, VersionIsNotOverwrittenAfterFirstAdd) {
  EntryState state;
  state.add(/*new_version=*/10, /*is_reusable_size=*/true);
  state.add(/*new_version=*/20, /*is_reusable_size=*/true);
  state.add(/*new_version=*/30, /*is_reusable_size=*/false);

  EXPECT_EQ(state.version, 10u);
  EXPECT_EQ(state.count, 3);
}

TEST(EntryStateTest, SameVersionDoesNotSetMultipleVersionsFlag) {
  EntryState state;
  state.add(/*new_version=*/55, /*is_reusable_size=*/false);
  state.add(/*new_version=*/55, /*is_reusable_size=*/false);
  state.add(/*new_version=*/55, /*is_reusable_size=*/true);

  EXPECT_EQ(state.version, 55u);
  EXPECT_EQ(state.count, 3);
  EXPECT_FALSE(state.multiple_versions);
}

TEST(EntryStateTest, DifferentVersionWithReusableSizeKeepsFlagFalse) {
  EntryState state;
  state.add(/*new_version=*/1, /*is_reusable_size=*/true);
  state.add(/*new_version=*/2, /*is_reusable_size=*/true);
  state.add(/*new_version=*/3, /*is_reusable_size=*/true);

  EXPECT_EQ(state.version, 1u);
  EXPECT_EQ(state.count, 3);
  EXPECT_FALSE(state.multiple_versions);
}

TEST(EntryStateTest, DifferentVersionWithoutReusableSizeSetsFlag) {
  EntryState state;
  state.add(/*new_version=*/1, /*is_reusable_size=*/false);
  state.add(/*new_version=*/2, /*is_reusable_size=*/false);

  EXPECT_EQ(state.version, 1u);
  EXPECT_EQ(state.count, 2);
  EXPECT_TRUE(state.multiple_versions);
}

TEST(EntryStateTest, MultipleVersionsFlagIsSticky) {
  EntryState state;
  state.add(/*new_version=*/1, /*is_reusable_size=*/false);
  state.add(/*new_version=*/2, /*is_reusable_size=*/false);
  ASSERT_TRUE(state.multiple_versions);

  // Subsequent calls that on their own would not set the flag must not clear
  // it either.
  state.add(/*new_version=*/1, /*is_reusable_size=*/false);
  EXPECT_TRUE(state.multiple_versions);

  state.add(/*new_version=*/2, /*is_reusable_size=*/true);
  EXPECT_TRUE(state.multiple_versions);

  state.add(/*new_version=*/1, /*is_reusable_size=*/true);
  EXPECT_TRUE(state.multiple_versions);
}

TEST(EntryStateTest, ReusableSizeMasksDifferingVersion) {
  EntryState state;
  state.add(/*new_version=*/10, /*is_reusable_size=*/false);
  // Different version but reusable size: flag must stay false.
  state.add(/*new_version=*/11, /*is_reusable_size=*/true);

  EXPECT_EQ(state.version, 10u);
  EXPECT_EQ(state.count, 2);
  EXPECT_FALSE(state.multiple_versions);
}

TEST(EntryStateTest, NonReusableSameVersionKeepsFlagFalse) {
  EntryState state;
  state.add(/*new_version=*/10, /*is_reusable_size=*/true);
  // !is_reusable_size is true, but versions match, so flag stays false.
  state.add(/*new_version=*/10, /*is_reusable_size=*/false);

  EXPECT_EQ(state.version, 10u);
  EXPECT_EQ(state.count, 2);
  EXPECT_FALSE(state.multiple_versions);
}

TEST(EntryStateTest, FlagSetOnlyWhenBothConditionsHold) {
  // Truth table for the predicate `!is_reusable_size && new_version != version`
  // after a first add that pins version to 100.
  struct Case {
    uint32_t new_version;
    bool is_reusable_size;
    bool expected_flag;
  };
  const Case cases[] = {
      {100, true, false},   // same version, reusable     -> false
      {100, false, false},  // same version, not reusable -> false
      {200, true, false},   // diff version, reusable     -> false
      {200, false, true},   // diff version, not reusable -> true
  };

  for (const Case& c : cases) {
    EntryState state;
    state.add(/*new_version=*/100, /*is_reusable_size=*/true);
    ASSERT_FALSE(state.multiple_versions);

    state.add(c.new_version, c.is_reusable_size);
    EXPECT_EQ(state.multiple_versions, c.expected_flag)
        << "new_version=" << c.new_version
        << ", is_reusable_size=" << c.is_reusable_size;
    EXPECT_EQ(state.version, 100u);
    EXPECT_EQ(state.count, 2);
  }
}

TEST(EntryStateTest, HandlesMaxUint32Version) {
  EntryState state;
  const uint32_t kMax = std::numeric_limits<uint32_t>::max();
  state.add(/*new_version=*/kMax, /*is_reusable_size=*/false);
  EXPECT_EQ(state.version, kMax);
  EXPECT_EQ(state.count, 1);
  EXPECT_FALSE(state.multiple_versions);

  state.add(/*new_version=*/kMax - 1, /*is_reusable_size=*/false);
  EXPECT_EQ(state.version, kMax);
  EXPECT_EQ(state.count, 2);
  EXPECT_TRUE(state.multiple_versions);
}

TEST(EntryStateTest, ManyAddsAccumulateCountAndPreserveFirstVersion) {
  EntryState state;
  const int kIterations = 1000;
  for (int i = 0; i < kIterations; ++i) {
    state.add(/*new_version=*/500, /*is_reusable_size=*/true);
  }
  EXPECT_EQ(state.version, 500u);
  EXPECT_EQ(state.count, kIterations);
  EXPECT_FALSE(state.multiple_versions);
}

TEST(EntryStateTest, IndependentInstancesDoNotShareState) {
  EntryState a;
  EntryState b;

  a.add(/*new_version=*/1, /*is_reusable_size=*/false);
  a.add(/*new_version=*/2, /*is_reusable_size=*/false);

  b.add(/*new_version=*/9, /*is_reusable_size=*/true);

  EXPECT_EQ(a.version, 1u);
  EXPECT_EQ(a.count, 2);
  EXPECT_TRUE(a.multiple_versions);

  EXPECT_EQ(b.version, 9u);
  EXPECT_EQ(b.count, 1);
  EXPECT_FALSE(b.multiple_versions);
}

}  // namespace
