//----------------------------------------------------------------------------//
//                                                                            //
// ozz-animation is hosted at http://github.com/guillaumeblanc/ozz-animation  //
// and distributed under the MIT License (MIT).                               //
//                                                                            //
// Copyright (c) 2019 Guillaume Blanc                                         //
//                                                                            //
// Permission is hereby granted, free of charge, to any person obtaining a    //
// copy of this software and associated documentation files (the "Software"), //
// to deal in the Software without restriction, including without limitation  //
// the rights to use, copy, modify, merge, publish, distribute, sublicense,   //
// and/or sell copies of the Software, and to permit persons to whom the      //
// Software is furnished to do so, subject to the following conditions:       //
//                                                                            //
// The above copyright notice and this permission notice shall be included in //
// all copies or substantial portions of the Software.                        //
//                                                                            //
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR //
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,   //
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL    //
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER //
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING    //
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER        //
// DEALINGS IN THE SOFTWARE.                                                  //
//                                                                            //
//----------------------------------------------------------------------------//

#include "ozz/animation/offline/animation_optimizer.h"

#include "gtest/gtest.h"

#include "ozz/base/maths/math_constant.h"

#include "ozz/animation/offline/animation_builder.h"
#include "ozz/animation/offline/raw_animation.h"

#include "ozz/animation/offline/raw_skeleton.h"
#include "ozz/animation/offline/skeleton_builder.h"
#include "ozz/animation/runtime/skeleton.h"

using ozz::animation::Skeleton;
using ozz::animation::offline::AnimationOptimizer;
using ozz::animation::offline::RawAnimation;
using ozz::animation::offline::RawSkeleton;
using ozz::animation::offline::SkeletonBuilder;

TEST(Error, AnimationOptimizer) {
  AnimationOptimizer optimizer;

  {  // NULL output.
    RawAnimation input;
    Skeleton skeleton;
    EXPECT_TRUE(input.Validate());

    // Builds animation
    EXPECT_FALSE(optimizer(input, skeleton, NULL));
  }

  {  // Invalid input animation.
    RawSkeleton raw_skeleton;
    raw_skeleton.roots.resize(1);
    SkeletonBuilder skeleton_builder;
    Skeleton* skeleton = skeleton_builder(raw_skeleton);
    ASSERT_TRUE(skeleton != NULL);

    RawAnimation input;
    input.duration = -1.f;
    EXPECT_FALSE(input.Validate());

    // Builds animation
    RawAnimation output;
    output.duration = -1.f;
    output.tracks.resize(1);
    EXPECT_FALSE(optimizer(input, *skeleton, &output));
    EXPECT_FLOAT_EQ(output.duration, RawAnimation().duration);
    EXPECT_EQ(output.num_tracks(), 0);

    ozz::memory::default_allocator()->Delete(skeleton);
  }

  {  // Invalid skeleton.
    Skeleton skeleton;

    RawAnimation input;
    input.tracks.resize(1);
    EXPECT_TRUE(input.Validate());

    // Builds animation
    RawAnimation output;
    EXPECT_FALSE(optimizer(input, skeleton, &output));
    EXPECT_FLOAT_EQ(output.duration, RawAnimation().duration);
    EXPECT_EQ(output.num_tracks(), 0);
  }
}

TEST(Name, AnimationOptimizer) {
  // Prepares a skeleton.
  RawSkeleton raw_skeleton;
  SkeletonBuilder skeleton_builder;
  Skeleton* skeleton = skeleton_builder(raw_skeleton);
  ASSERT_TRUE(skeleton != NULL);

  AnimationOptimizer optimizer;

  RawAnimation input;
  input.name = "Test_Animation";
  input.duration = 1.f;

  ASSERT_TRUE(input.Validate());

  RawAnimation output;
  ASSERT_TRUE(optimizer(input, *skeleton, &output));
  EXPECT_EQ(output.num_tracks(), 0);
  EXPECT_STRCASEEQ(output.name.c_str(), "Test_Animation");

  ozz::memory::default_allocator()->Delete(skeleton);
}

TEST(Optimize, AnimationOptimizer) {
  // Prepares a skeleton.
  RawSkeleton raw_skeleton;
  raw_skeleton.roots.resize(1);
  SkeletonBuilder skeleton_builder;
  Skeleton* skeleton = skeleton_builder(raw_skeleton);
  ASSERT_TRUE(skeleton != NULL);

  AnimationOptimizer optimizer;

  RawAnimation input;
  input.duration = 1.f;
  input.tracks.resize(1);
  {
    RawAnimation::TranslationKey key = {0.f, ozz::math::Float3(0.f, 0.f, 0.f)};
    input.tracks[0].translations.push_back(key);
  }
  {
    RawAnimation::TranslationKey key = {
        .25f, ozz::math::Float3(.1f, 0.f, 0.f)};  // Not interpolable.
    input.tracks[0].translations.push_back(key);
  }
  {
    RawAnimation::TranslationKey key = {.5f, ozz::math::Float3(0.f, 0.f, 0.f)};
    input.tracks[0].translations.push_back(key);
  }
  {
    RawAnimation::TranslationKey key = {
        .625f, ozz::math::Float3(.1f, 0.f, 0.f)};  // Interpolable.
    input.tracks[0].translations.push_back(key);
  }
  {
    RawAnimation::TranslationKey key = {
        .75f, ozz::math::Float3(.21f, 0.f, 0.f)};  // Interpolable.
    input.tracks[0].translations.push_back(key);
  }
  {
    RawAnimation::TranslationKey key = {
        .875f, ozz::math::Float3(.29f, 0.f, 0.f)};  // Interpolable.
    input.tracks[0].translations.push_back(key);
  }
  {
    RawAnimation::TranslationKey key = {.9999f,
                                        ozz::math::Float3(.4f, 0.f, 0.f)};
    input.tracks[0].translations.push_back(key);
  }
  {
    RawAnimation::TranslationKey key = {
        1.f, ozz::math::Float3(0.f, 0.f, 0.f)};  // Last key.
    input.tracks[0].translations.push_back(key);
  }
  {
    RawAnimation::RotationKey key = {0.f, ozz::math::Quaternion::identity()};
    input.tracks[0].rotations.push_back(key);
  }
  {
    RawAnimation::RotationKey key = {
        .5f,
        ozz::math::Quaternion::FromEuler(1.1f * ozz::math::kPi / 180.f, 0, 0)};
    input.tracks[0].rotations.push_back(key);
  }
  {
    RawAnimation::RotationKey key = {
        1.f,
        ozz::math::Quaternion::FromEuler(2.f * ozz::math::kPi / 180.f, 0, 0)};
    input.tracks[0].rotations.push_back(key);
  }

  ASSERT_TRUE(input.Validate());

  // Builds animation with very little tolerance.
  {
    optimizer.translation_tolerance = 0.f;
    optimizer.rotation_tolerance = 0.f;
    optimizer.scale_tolerance = 0.f;

    // Disables hierarchical tolerance.
    optimizer.hierarchical_tolerance = 1e9f;

    RawAnimation output;
    ASSERT_TRUE(optimizer(input, *skeleton, &output));
    EXPECT_EQ(output.num_tracks(), 1);

    const RawAnimation::JointTrack::Translations& translations =
        output.tracks[0].translations;
    ASSERT_EQ(translations.size(), 8u);
    EXPECT_FLOAT_EQ(translations[0].time, 0.f);     // Track 0 begin.
    EXPECT_FLOAT_EQ(translations[1].time, .25f);    // Track 0 at .25f.
    EXPECT_FLOAT_EQ(translations[2].time, .5f);     // Track 0 at .5f.
    EXPECT_FLOAT_EQ(translations[3].time, .625f);   // Track 0 at .625f.
    EXPECT_FLOAT_EQ(translations[4].time, .75f);    // Track 0 at .75f.
    EXPECT_FLOAT_EQ(translations[5].time, .875f);   // Track 0 at .875f.
    EXPECT_FLOAT_EQ(translations[6].time, .9999f);  // Track 0 ~end.
    EXPECT_FLOAT_EQ(translations[7].time, 1.f);     // Track 0 end.

    const RawAnimation::JointTrack::Rotations& rotations =
        output.tracks[0].rotations;
    ASSERT_EQ(rotations.size(), 3u);
    EXPECT_FLOAT_EQ(rotations[0].time, 0.f);  // Track 0 begin.
    EXPECT_FLOAT_EQ(rotations[1].time, .5f);  // Track 0 at .5f.
    EXPECT_FLOAT_EQ(rotations[2].time, 1.f);  // Track 0 end.
  }
  {
    // Rebuilds with tolerance.
    optimizer.translation_tolerance = .02f;
    optimizer.rotation_tolerance = .2f * 3.14159f / 180.f;  // .2 degree.
    optimizer.hierarchical_tolerance = .02f;
    RawAnimation output;
    ASSERT_TRUE(optimizer(input, *skeleton, &output));
    EXPECT_EQ(output.num_tracks(), 1);

    const RawAnimation::JointTrack::Translations& translations =
        output.tracks[0].translations;
    ASSERT_EQ(translations.size(), 5u);
    EXPECT_FLOAT_EQ(translations[0].time, 0.f);     // Track 0 begin.
    EXPECT_FLOAT_EQ(translations[1].time, .25f);    // Track 0 at .25f.
    EXPECT_FLOAT_EQ(translations[2].time, .5f);     // Track 0 at .5f.
    EXPECT_FLOAT_EQ(translations[3].time, .9999f);  // Track 0 at ~1.f.
    EXPECT_FLOAT_EQ(translations[4].time, 1.f);     // Track 0 end.

    const RawAnimation::JointTrack::Rotations& rotations =
        output.tracks[0].rotations;
    ASSERT_EQ(rotations.size(), 2u);
    EXPECT_FLOAT_EQ(rotations[0].time, 0.f);  // Track 0 begin.
    EXPECT_FLOAT_EQ(rotations[1].time, 1.f);  // Track 0 end.
  }

  ozz::memory::default_allocator()->Delete(skeleton);
}

TEST(OptimizeHierarchical, AnimationOptimizer) {
  // Prepares a skeleton.
  RawSkeleton raw_skeleton;
  raw_skeleton.roots.resize(1);
  raw_skeleton.roots[0].children.resize(1);
  raw_skeleton.roots[0].children[0].children.resize(1);
  raw_skeleton.roots[0].children[0].children[0].children.resize(2);
  SkeletonBuilder skeleton_builder;
  Skeleton* skeleton = skeleton_builder(raw_skeleton);
  ASSERT_TRUE(skeleton != NULL);

  // Disable non hierarchical optimizations
  AnimationOptimizer optimizer;
  optimizer.translation_tolerance = 1e9f;
  optimizer.rotation_tolerance = 1e9f;
  optimizer.scale_tolerance = 1e9f;

  RawAnimation input;
  input.duration = 1.f;
  input.tracks.resize(5);

  // Translations on track 0.
  {
    RawAnimation::TranslationKey key = {0.f, ozz::math::Float3(4.f, 0.f, 0.f)};
    input.tracks[0].translations.push_back(key);
  }

  // Translations on track 1.
  {
    RawAnimation::TranslationKey key = {0.f, ozz::math::Float3(0.f, 0.f, 0.f)};
    input.tracks[1].translations.push_back(key);
  }

  // Translations on track 2.
  {
    RawAnimation::TranslationKey key = {0.f, ozz::math::Float3(5.f, 0.f, 0.f)};
    input.tracks[2].translations.push_back(key);
  }
  {
    RawAnimation::TranslationKey key = {.1f, ozz::math::Float3(6.f, 0.f, 0.f)};
    input.tracks[2].translations.push_back(key);
  }
  {  // Creates an variation.
    RawAnimation::TranslationKey key = {.2f, ozz::math::Float3(7.1f, 0.f, 0.f)};
    input.tracks[2].translations.push_back(key);
  }
  {
    RawAnimation::TranslationKey key = {.3f, ozz::math::Float3(8.f, 0.f, 0.f)};
    input.tracks[2].translations.push_back(key);
  }

  // Translations on track 3.
  {
    RawAnimation::TranslationKey key = {0.f, ozz::math::Float3(16.f, 0.f, 0.f)};
    input.tracks[3].translations.push_back(key);
  }
  // Translations on track 4.
  {
    RawAnimation::TranslationKey key = {0.f, ozz::math::Float3(32.f, 0.f, 0.f)};
    input.tracks[4].translations.push_back(key);
  }

  ASSERT_TRUE(input.Validate());

  // Small translation tolerance -> all key maintained
  {
    RawAnimation output;
    optimizer.hierarchical_tolerance = .01f;
    ASSERT_TRUE(optimizer(input, *skeleton, &output));
    EXPECT_EQ(output.num_tracks(), 5);

    const RawAnimation::JointTrack::Translations& translations =
        output.tracks[2].translations;
    ASSERT_EQ(translations.size(), 4u);
    EXPECT_FLOAT_EQ(translations[0].time, 0.f);
    EXPECT_FLOAT_EQ(translations[1].time, .1f);
    EXPECT_FLOAT_EQ(translations[2].time, .2f);
    EXPECT_FLOAT_EQ(translations[3].time, .3f);
  }

  // High translation tolerance -> all key interpolated
  {
    RawAnimation output;
    optimizer.hierarchical_tolerance = .1f;
    ASSERT_TRUE(optimizer(input, *skeleton, &output));
    EXPECT_EQ(output.num_tracks(), 5);

    const RawAnimation::JointTrack::Translations& translations =
        output.tracks[2].translations;
    ASSERT_EQ(translations.size(), 2u);
    EXPECT_FLOAT_EQ(translations[0].time, 0.f);
    EXPECT_FLOAT_EQ(translations[1].time, .3f);
  }

  // Introduces a 10x scaling upstream that amplifies error
  // Scaling on track 0
  {
    RawAnimation::ScaleKey key = {0.f, ozz::math::Float3(10.f, 0.f, 0.f)};
    input.tracks[0].scales.push_back(key);
  }

  // High translation tolerance -> keys aren't interpolated because of scale
  // effect.
  {
    RawAnimation output;
    optimizer.hierarchical_tolerance = .1f;
    ASSERT_TRUE(optimizer(input, *skeleton, &output));
    EXPECT_EQ(output.num_tracks(), 5);

    const RawAnimation::JointTrack::Translations& translations =
        output.tracks[2].translations;
    ASSERT_EQ(translations.size(), 4u);
  }

  // Very high translation tolerance
  {
    RawAnimation output;
    optimizer.hierarchical_tolerance = 1.f;
    ASSERT_TRUE(optimizer(input, *skeleton, &output));
    EXPECT_EQ(output.num_tracks(), 5);

    const RawAnimation::JointTrack::Translations& translations =
        output.tracks[2].translations;
    ASSERT_EQ(translations.size(), 2u);
  }

  // Introduces a -10x scaling upstream that amplifies error
  // Scaling on track 0
  { input.tracks[0].scales[0].value = ozz::math::Float3(0.f, -10.f, 0.f); }

  // High translation tolerance -> keys aren't interpolated because of scale
  // effect.
  {
    RawAnimation output;
    optimizer.hierarchical_tolerance = .1f;
    ASSERT_TRUE(optimizer(input, *skeleton, &output));
    EXPECT_EQ(output.num_tracks(), 5);

    const RawAnimation::JointTrack::Translations& translations =
        output.tracks[2].translations;
    ASSERT_EQ(translations.size(), 4u);
    EXPECT_FLOAT_EQ(translations[0].time, 0.f);
    EXPECT_FLOAT_EQ(translations[1].time, .1f);
    EXPECT_FLOAT_EQ(translations[2].time, .2f);
    EXPECT_FLOAT_EQ(translations[3].time, .3f);
  }

  // Very high translation tolerance
  {
    RawAnimation output;
    optimizer.hierarchical_tolerance = 1.f;
    ASSERT_TRUE(optimizer(input, *skeleton, &output));
    EXPECT_EQ(output.num_tracks(), 5);

    const RawAnimation::JointTrack::Translations& translations =
        output.tracks[2].translations;
    ASSERT_EQ(translations.size(), 2u);
    EXPECT_FLOAT_EQ(translations[0].time, 0.f);
    EXPECT_FLOAT_EQ(translations[1].time, .3f);
  }

  // Compenstate scale on next joint
  {
    RawAnimation::ScaleKey key = {0.f, ozz::math::Float3(.1f, 0.f, 0.f)};
    input.tracks[1].scales.push_back(key);
  }

  // High translation tolerance -> keys ar interpolated because of scale
  // compensation.
  {
    RawAnimation output;
    optimizer.hierarchical_tolerance = 1.f;
    ASSERT_TRUE(optimizer(input, *skeleton, &output));
    EXPECT_EQ(output.num_tracks(), 5);

    const RawAnimation::JointTrack::Translations& translations =
        output.tracks[2].translations;
    ASSERT_EQ(translations.size(), 2u);
  }

  // Remove scaling compensation
  { input.tracks[1].scales.clear(); }

  // Introduces a .1x scaling upstream that amplifies error
  // Scaling on track 0
  { input.tracks[0].scales[0].value = ozz::math::Float3(0.f, 0.f, .1f); }

  // High translation tolerance -> keys aren't interpolated because of scale
  // effect.
  {
    RawAnimation output;
    optimizer.hierarchical_tolerance = .001f;
    ASSERT_TRUE(optimizer(input, *skeleton, &output));
    EXPECT_EQ(output.num_tracks(), 5);

    const RawAnimation::JointTrack::Translations& translations =
        output.tracks[2].translations;
    ASSERT_EQ(translations.size(), 4u);
    EXPECT_FLOAT_EQ(translations[0].time, 0.f);
    EXPECT_FLOAT_EQ(translations[1].time, .1f);
    EXPECT_FLOAT_EQ(translations[2].time, .2f);
    EXPECT_FLOAT_EQ(translations[3].time, .3f);
  }

  // Very high translation tolerance
  {
    RawAnimation output;
    optimizer.hierarchical_tolerance = .01f;
    ASSERT_TRUE(optimizer(input, *skeleton, &output));
    EXPECT_EQ(output.num_tracks(), 5);

    const RawAnimation::JointTrack::Translations& translations =
        output.tracks[2].translations;
    ASSERT_EQ(translations.size(), 2u);
    EXPECT_FLOAT_EQ(translations[0].time, 0.f);
    EXPECT_FLOAT_EQ(translations[1].time, .3f);
  }

  // Remove scaling
  { input.tracks[0].scales.clear(); }

  // Rotations on track 0.
  {
    RawAnimation::RotationKey key = {
        0.f, ozz::math::Quaternion::FromEuler(0.f, 0.f, 0.f)};
    input.tracks[0].rotations.push_back(key);
  }
  {                                     // Include error
    const float angle_error = 2.5e-3f;  // creates an arc of .1m at 40m.
    RawAnimation::RotationKey key = {
        .1f, ozz::math::Quaternion::FromEuler(ozz::math::kPi_4 + angle_error,
                                              0.f, 0.f)};
    input.tracks[0].rotations.push_back(key);
  }
  {
    RawAnimation::RotationKey key = {
        .2f, ozz::math::Quaternion::FromEuler(ozz::math::kPi_2, 0.f, 0.f)};
    input.tracks[0].rotations.push_back(key);
  }

  // Big enough translation tolerance -> keys rejected
  {
    RawAnimation output;
    optimizer.hierarchical_tolerance = .1f;
    ASSERT_TRUE(optimizer(input, *skeleton, &output));
    EXPECT_EQ(output.num_tracks(), 5);

    const RawAnimation::JointTrack::Rotations& rotations =
        output.tracks[0].rotations;
    ASSERT_EQ(rotations.size(), 2u);

    const RawAnimation::JointTrack::Translations& translations =
        output.tracks[2].translations;
    ASSERT_EQ(translations.size(), 2u);
  }

  // Small translation tolerance -> all key maintained
  {
    RawAnimation output;
    optimizer.hierarchical_tolerance = .01f;
    ASSERT_TRUE(optimizer(input, *skeleton, &output));
    EXPECT_EQ(output.num_tracks(), 5);

    const RawAnimation::JointTrack::Rotations& rotations =
        output.tracks[0].rotations;
    ASSERT_EQ(rotations.size(), 3u);

    const RawAnimation::JointTrack::Translations& translations =
        output.tracks[2].translations;
    ASSERT_EQ(translations.size(), 4u);
  }

  // Introduces a .1x scaling upstream that lowers error
  // Scaling on track 0
  {
    RawAnimation::ScaleKey key = {0.f, ozz::math::Float3(0.f, .1f, 0.f)};
    input.tracks[1].scales.push_back(key);
  }

  // Small translation tolerance, but scaled down -> keys rejected
  {
    RawAnimation output;
    optimizer.hierarchical_tolerance = .01f;
    ASSERT_TRUE(optimizer(input, *skeleton, &output));
    EXPECT_EQ(output.num_tracks(), 5);

    const RawAnimation::JointTrack::Rotations& rotations =
        output.tracks[0].rotations;
    ASSERT_EQ(rotations.size(), 2u);

    const RawAnimation::JointTrack::Translations& translations =
        output.tracks[2].translations;
    ASSERT_EQ(translations.size(), 2u);
  }

  // Remove scaling
  { input.tracks[2].scales.clear(); }

  ozz::memory::default_allocator()->Delete(skeleton);
}
