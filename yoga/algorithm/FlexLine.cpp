/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <yoga/Yoga.h>

#include <yoga/algorithm/BoundAxis.h>
#include <yoga/algorithm/FlexDirection.h>
#include <yoga/algorithm/FlexLine.h>
#include <iostream>
namespace facebook::yoga {

FlexLine calculateFlexLine(
    yoga::Node* const node,
    const Direction ownerDirection,
    const float ownerWidth,
    const float mainAxisownerSize,
    const float availableInnerWidth,
    const float availableInnerMainDim,
    Node::LayoutableChildren::Iterator& iterator,
    const size_t lineCount) {
  std::cout << node->getTag()
            << "-------------- start calculateFlexLine! ----------\n"
            << std::endl;

  std::cout << node->getTag() << "ownerWidth: " << ownerWidth
            << ",mainAxisownerSize: " << mainAxisownerSize
            << ",availableInnerWidth: " << availableInnerWidth
            << ",availableInnerMainDim: " << availableInnerMainDim << std::endl;
  std::vector<yoga::Node*> itemsInFlow;
  itemsInFlow.reserve(node->getChildCount());

  float sizeConsumed = 0.0f;
  float totalFlexGrowFactors = 0.0f;
  float totalFlexShrinkScaledFactors = 0.0f;
  size_t numberOfAutoMargins = 0;
  size_t endOfLineIndex = iterator.index();
  size_t firstElementInLineIndex = iterator.index();

  float sizeConsumedIncludingMinConstraint = 0;
  // 布局方向，尤其是在处理 margin 和 padding 等属性时需要考虑文本方向。
  const Direction direction = node->resolveDirection(ownerDirection);
  // 主轴上的flex方向,属性用于设置Flex容器中主轴的方向，从而决定Flex项目在容器中的排列方式
  const FlexDirection mainAxis =
      resolveDirection(node->style().flexDirection(), direction);
  // 是否允许换行
  const bool isNodeFlexWrap = node->style().flexWrap() != Wrap::NoWrap;
  // 主轴上的 gap
  const float gap =
      node->style().computeGapForAxis(mainAxis, availableInnerMainDim);

  // Add items to the current line until it's full or we run out of items.
  for (; iterator != node->getLayoutChildren().end();
       iterator++, endOfLineIndex = iterator.index()) {
    auto child = *iterator;
    if (child->style().display() == Display::None ||
        child->style().positionType() == PositionType::Absolute) {
      if (firstElementInLineIndex == endOfLineIndex) {
        // We haven't found the first contributing element in the line yet.
        firstElementInLineIndex++;
      }
      continue;
    }

    // 自动 margin，用于自动填充 margin，当设置 margin-left(start): auto 和
    // margin-right(end): auto 时，会自动水平居中
    if (child->style().flexStartMarginIsAuto(mainAxis, ownerDirection)) {
      numberOfAutoMargins++;
    }
    if (child->style().flexEndMarginIsAuto(mainAxis, ownerDirection)) {
      numberOfAutoMargins++;
    }

    // true
    const bool isFirstElementInLine =
        (endOfLineIndex - firstElementInLineIndex) == 0;

    child->setLineIndex(lineCount);
    const float childMarginMainAxis =
        child->style().computeMarginForAxis(mainAxis, availableInnerWidth);
    const float childLeadingGapMainAxis = isFirstElementInLine ? 0.0f : gap;
    const float flexBasisWithMinAndMaxConstraints =
        boundAxisWithinMinAndMax(
            child,
            direction,
            mainAxis,
            child->getLayout().computedFlexBasis,
            mainAxisownerSize,
            ownerWidth)
            .unwrap();

    // If this is a multi-line flow and this item pushes us over the available
    // size, we've hit the end of the current line. Break out of the loop and
    // lay out the current line.

    //  flexBasis长度加上子元素的 margin，加上子元素的前置 gap
    //  超出可用范围，并且允许换行，以及 items 不为空
    if (sizeConsumedIncludingMinConstraint + flexBasisWithMinAndMaxConstraints +
                childMarginMainAxis + childLeadingGapMainAxis >
            availableInnerMainDim &&
        isNodeFlexWrap && !itemsInFlow.empty()) {
      break;
    }

    sizeConsumedIncludingMinConstraint += flexBasisWithMinAndMaxConstraints +
        childMarginMainAxis + childLeadingGapMainAxis;
    sizeConsumed += flexBasisWithMinAndMaxConstraints + childMarginMainAxis +
        childLeadingGapMainAxis;

    std::cout << node->getTag() << "sizeConsumed: " << sizeConsumed
              << ",flexBasisWithMinAndMaxConstraints: "
              << flexBasisWithMinAndMaxConstraints
              << ",childMarginMainAxis: " << childMarginMainAxis
              << ",childLeadingGapMainAxis: " << childLeadingGapMainAxis
              << ",  child->getLayout().computedFlexBasis,"
              << (child->getLayout().computedFlexBasis.unwrap()) << std::endl;

    if (child->isNodeFlexible()) {
      totalFlexGrowFactors += child->resolveFlexGrow();

      // Unlike the grow factor, the shrink factor is scaled relative to the
      // child dimension.
      totalFlexShrinkScaledFactors += -child->resolveFlexShrink() *
          child->getLayout().computedFlexBasis.unwrap();
    }

    itemsInFlow.push_back(child);
  }

  // The total flex factor needs to be floored to 1.
  if (totalFlexGrowFactors > 0 && totalFlexGrowFactors < 1) {
    totalFlexGrowFactors = 1;
  }

  // The total flex shrink factor needs to be floored to 1.
  if (totalFlexShrinkScaledFactors > 0 && totalFlexShrinkScaledFactors < 1) {
    totalFlexShrinkScaledFactors = 1;
  }
  std::cout << node->getTag()
            << "-------------- end calculateFlexLine! ----------\n"
            << std::endl;
  return FlexLine{
      .itemsInFlow = std::move(itemsInFlow),
      .sizeConsumed = sizeConsumed,
      .endOfLineIndex = endOfLineIndex,
      .numberOfAutoMargins = numberOfAutoMargins,
      .layout = FlexLineRunningLayout{
          totalFlexGrowFactors,
          totalFlexShrinkScaledFactors,
      }};
}

} // namespace facebook::yoga
