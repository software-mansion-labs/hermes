/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "hermes/VM/ArrayStorage.h"

#include "VMRuntimeTestHelpers.h"

using namespace hermes::vm;

namespace {

using ArrayTest = RuntimeTestFixture;

/// Test only the C++ API. We use the high-level API only when it is necessary
/// in order to access some part of the C++ API.
TEST_F(ArrayTest, CppAPITest) {
  struct : Locals {
    PinnedValue<JSArray> array;
  } lv;
  LocalsRAII lraii{runtime, &lv};
  // We allocate a lot of handles in row.
  GCScope scope(runtime, "ArrayTest.CppAPITest", 128);

  auto arrayRes = JSArray::create(runtime, 1, 0);
  ASSERT_FALSE(isException(arrayRes));
  lv.array = std::move(*arrayRes);

  // Make sure the beginning is at 0.
  ASSERT_EQ(0u, lv.array->getBeginIndex());
  // Make sure the length is 0.
  ASSERT_EQ(0u, lv.array->getEndIndex());

  // Call haveOwnIndexed() and getOwnIndexed() on a element not in the array.
  ComputedPropertyDescriptor desc;
  MutableHandle<SymbolID> tmpSymbolStorage{runtime};
  ASSERT_FALSE(*lv.array->getOwnComputedPrimitiveDescriptor(
      lv.array,
      runtime,
      runtime.makeHandle(100.0_hd),
      JSObject::IgnoreProxy::No,
      tmpSymbolStorage,
      desc));
  EXPECT_CALLRESULT_UNDEFINED(lv.array->getComputed_RJS(
      lv.array, runtime, runtime.makeHandle(100.0_hd)));

// Obtain the value a couple of different ways and check its value.
#define EXPECT_INDEX_VALUE(value, array, index)                               \
  EXPECT_EQ(value, array->at(runtime, index).unboxToHV(runtime));             \
  ASSERT_TRUE(*array->getOwnComputedDescriptor(                               \
      array,                                                                  \
      runtime,                                                                \
      runtime.makeHandle(HermesValue::encodeTrustedNumberValue(index)),       \
      tmpSymbolStorage,                                                       \
      desc));                                                                 \
  EXPECT_CALLRESULT_VALUE(                                                    \
      value,                                                                  \
      JSArray::getComputedPropertyValue_RJS(                                  \
          array,                                                              \
          runtime,                                                            \
          array,                                                              \
          tmpSymbolStorage,                                                   \
          desc,                                                               \
          runtime.makeHandle(HermesValue::encodeTrustedNumberValue(index)))); \
  EXPECT_CALLRESULT_VALUE(                                                    \
      value,                                                                  \
      array->getComputed_RJS(                                                 \
          array,                                                              \
          runtime,                                                            \
          runtime.makeHandle(HermesValue::encodeTrustedNumberValue(index))));

  // array[100] = 50. This will case a reallocation.
  JSArray::setElementAt(lv.array, runtime, 100, runtime.makeHandle(50.0_hd));
  // Length must be 101.
  ASSERT_EQ(101u, lv.array->getEndIndex());
  EXPECT_INDEX_VALUE(50.0_hd, lv.array, 100);

  // array[90] = 40. This will cause a reallocation.
  JSArray::setElementAt(lv.array, runtime, 90, runtime.makeHandle(40.0_hd));
  // Length must still be 101.
  ASSERT_EQ(101u, lv.array->getEndIndex());
  EXPECT_INDEX_VALUE(40.0_hd, lv.array, 90);
  EXPECT_INDEX_VALUE(50.0_hd, lv.array, 100);

  // array[105] = 60. This will case a reallocation.
  JSArray::setElementAt(lv.array, runtime, 105, runtime.makeHandle(60.0_hd));
  ASSERT_EQ(106u, lv.array->getEndIndex());
  EXPECT_INDEX_VALUE(40.0_hd, lv.array, 90);
  EXPECT_INDEX_VALUE(50.0_hd, lv.array, 100);
  EXPECT_INDEX_VALUE(60.0_hd, lv.array, 105);

  // array[106] = 70. This will not case a reallocation.
  JSArray::setElementAt(lv.array, runtime, 106, runtime.makeHandle(70.0_hd));
  ASSERT_EQ(107u, lv.array->getEndIndex());
  EXPECT_INDEX_VALUE(40.0_hd, lv.array, 90);
  EXPECT_INDEX_VALUE(50.0_hd, lv.array, 100);
  EXPECT_INDEX_VALUE(60.0_hd, lv.array, 105);
  EXPECT_INDEX_VALUE(70.0_hd, lv.array, 106);

  // array[100] = 51. We are updating an element in-place.
  JSArray::setElementAt(lv.array, runtime, 100, runtime.makeHandle(51.0_hd));
  ASSERT_EQ(107u, lv.array->getEndIndex());
  EXPECT_INDEX_VALUE(40.0_hd, lv.array, 90);
  EXPECT_INDEX_VALUE(51.0_hd, lv.array, 100);
  EXPECT_INDEX_VALUE(60.0_hd, lv.array, 105);
  EXPECT_INDEX_VALUE(70.0_hd, lv.array, 106);

  // Shrink the array to 106 elements.
  JSArray::setStorageEndIndex(lv.array, runtime, 106);
  ASSERT_EQ(106u, lv.array->getEndIndex());
  EXPECT_INDEX_VALUE(40.0_hd, lv.array, 90);
  EXPECT_INDEX_VALUE(51.0_hd, lv.array, 100);
  EXPECT_INDEX_VALUE(60.0_hd, lv.array, 105);
  ASSERT_FALSE(*lv.array->getOwnComputedPrimitiveDescriptor(
      lv.array,
      runtime,
      runtime.makeHandle(106.0_hd),
      JSObject::IgnoreProxy::No,
      tmpSymbolStorage,
      desc));
  EXPECT_CALLRESULT_UNDEFINED(lv.array->getComputed_RJS(
      lv.array, runtime, runtime.makeHandle(106.0_hd)));

  // Increase to 107 again.
  JSArray::setStorageEndIndex(lv.array, runtime, 107);
  ASSERT_EQ(107u, lv.array->getEndIndex());
  EXPECT_INDEX_VALUE(40.0_hd, lv.array, 90);
  EXPECT_INDEX_VALUE(51.0_hd, lv.array, 100);
  EXPECT_INDEX_VALUE(60.0_hd, lv.array, 105);
  ASSERT_FALSE(*lv.array->getOwnComputedPrimitiveDescriptor(
      lv.array,
      runtime,
      runtime.makeHandle(106.0_hd),
      JSObject::IgnoreProxy::No,
      tmpSymbolStorage,
      desc));
  EXPECT_CALLRESULT_UNDEFINED(lv.array->getComputed_RJS(
      lv.array, runtime, runtime.makeHandle(106.0_hd)));

  // array[106] = 70 again.
  JSArray::setElementAt(lv.array, runtime, 106, runtime.makeHandle(70.0_hd));
  ASSERT_EQ(107u, lv.array->getEndIndex());
  EXPECT_INDEX_VALUE(40.0_hd, lv.array, 90);
  EXPECT_INDEX_VALUE(51.0_hd, lv.array, 100);
  EXPECT_INDEX_VALUE(60.0_hd, lv.array, 105);
  EXPECT_INDEX_VALUE(70.0_hd, lv.array, 106);

#undef EXPECT_INDEX_VALUE
}

TEST_F(ArrayTest, TestLength) {
  struct : Locals {
    PinnedValue<JSArray> array;
  } lv;
  LocalsRAII lraii{runtime, &lv};
  auto lengthID = Predefined::getSymbolID(Predefined::length);
  auto arrayRes = JSArray::create(runtime, 10, 10);
  ASSERT_FALSE(isException(arrayRes));
  lv.array = std::move(*arrayRes);

  // Make sure the length is 10.
  ASSERT_EQ(10u, JSArray::getLength(lv.array.get(), runtime));
  EXPECT_CALLRESULT_DOUBLE(
      10.0, JSObject::getNamed_RJS(lv.array, runtime, lengthID));

  // Change it to 5.0.
  ASSERT_TRUE(*JSObject::putNamed_RJS(
      lv.array, runtime, lengthID, runtime.makeHandle(5.0_hd)));
  ASSERT_EQ(5u, JSArray::getLength(lv.array.get(), runtime));
  EXPECT_CALLRESULT_DOUBLE(
      5.0, JSObject::getNamed_RJS(lv.array, runtime, lengthID));

  // Try setting it to an invalid value.
  {
    auto res = JSObject::putNamed_RJS(
        lv.array, runtime, lengthID, runtime.makeHandle(5.1_hd));
    ASSERT_TRUE(isException(res));
    runtime.clearThrownValue();
  }

  // Make sure it didn't change.
  ASSERT_EQ(5u, JSArray::getLength(lv.array.get(), runtime));
  EXPECT_CALLRESULT_DOUBLE(
      5.0, JSObject::getNamed_RJS(lv.array, runtime, lengthID));
}
} // namespace
