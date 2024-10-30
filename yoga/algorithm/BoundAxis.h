/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <yoga/algorithm/FlexDirection.h>
#include <yoga/algorithm/Log.h>
#include <yoga/enums/Dimension.h>
#include <yoga/enums/FlexDirection.h>
#include <yoga/node/Node.h>
#include <yoga/numeric/Comparison.h>
#include <yoga/numeric/FloatOptional.h>
#include <iostream>

namespace facebook::yoga {

inline float paddingAndBorderForAxis(
    const yoga::Node* const node,
    const FlexDirection axis,
    const Direction direction,
    const float widthSize) {
  return node->style().computeInlineStartPaddingAndBorder(
             axis, direction, widthSize) +
      node->style().computeInlineEndPaddingAndBorder(
          axis, direction, widthSize);
}

// 限制在 min 和 max 之间
inline FloatOptional boundAxisWithinMinAndMax(
    const yoga::Node* const node,
    const Direction direction,
    const FlexDirection axis,
    const FloatOptional value,
    const float axisSize,
    const float widthSize) {
  FloatOptional min;
  FloatOptional max;

  if (isColumn(axis)) {
    min = node->style().resolvedMinDimension(
        direction, Dimension::Height, axisSize, widthSize);
    max = node->style().resolvedMaxDimension(
        direction, Dimension::Height, axisSize, widthSize);
  } else if (isRow(axis)) {
    min = node->style().resolvedMinDimension(
        direction, Dimension::Width, axisSize, widthSize);
    max = node->style().resolvedMaxDimension(
        direction, Dimension::Width, axisSize, widthSize);
  }
  logWithColor(
      "min: " + std::to_string(min.unwrap()) +
          " max :" + std::to_string(max.unwrap()),
      Color::BLUE);

  if (max >= FloatOptional{0} && value > max) {
    return max;
  }

  if (min >= FloatOptional{0} && value < min) {
    return min;
  }

  return value;
}

// Like boundAxisWithinMinAndMax but also ensures that the value doesn't
// go below the padding and border amount.
inline float boundAxis(
    const yoga::Node* const node,
    const FlexDirection axis,
    const Direction direction,
    const float value,
    const float axisSize,
    const float widthSize) {
  return yoga::maxOrDefined(
      boundAxisWithinMinAndMax(
          node, direction, axis, FloatOptional{value}, axisSize, widthSize)
          .unwrap(),
      paddingAndBorderForAxis(node, axis, direction, widthSize));
}

} // namespace facebook::yoga
