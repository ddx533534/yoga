# 结论

yoga测试用例如下，其中child1和child2均为content-box模式
``` shell
                      root0
                    (Nan,Nan)
                    /         \
            child1              child2
(100,100,margin:10)             (Nan,Nan)  
                                    |
                                    child3
                                   (100,100)
```

```c++
void yoga_test() {
  /**
   * create root
   */
  YGNodeRef root = YGNodeNew();
  YGNodeSetTag(root, 0);
  YGNodeStyleSetDirection(root, YGDirection::YGDirectionLTR);
  YGNodeStyleSetFlexDirection(root, YGFlexDirectionRow);
  // YGNodeStyleSetBoxSizing(root, YGBoxSizing::YGBoxSizingContentBox);

  /**
   * create child1
   */
  YGNodeRef child1 = YGNodeNew();
  YGNodeSetTag(child1, 1);
  YGNodeStyleSetWidth(child1, 100.0);
  YGNodeStyleSetHeight(child1, 100.0);
  YGNodeStyleSetMargin(child1, YGEdge::YGEdgeLeft, 10.0);
  // YGNodeStyleSetFlexBasis(child1, 200.0);
  // YGNodeStyleSetBorder(child1, YGEdge::YGEdgeAll, 10.0);
//   YGNodeStyleSetPadding(child1, YGEdge::YGEdgeAll, 10.0);

  /**
   * create child2
   */
  YGNodeRef child2 = YGNodeNew();
  YGNodeSetTag(child2, 2);
  // YGNodeStyleSetWidth(child2, 110.0);
  // YGNodeStyleSetHeight(child2, 110.0);

  /**
   * create child3
   */
  YGNodeRef child3 = YGNodeNew();
  YGNodeSetTag(child3, 3);
  YGNodeStyleSetWidth(child3, 110.0);
  YGNodeStyleSetHeight(child3, 110.0);

  /**
   * manage child
   */
  YGNodeInsertChild(root, child1, 0);
  YGNodeInsertChild(root, child2, 1);
  YGNodeInsertChild(child2, child3, 0);

  // 设定boxsizing模式为 content-box
  YGNodeStyleSetBoxSizing(root, YGBoxSizing::YGBoxSizingContentBox);
  YGNodeStyleSetBoxSizing(child1, YGBoxSizing::YGBoxSizingContentBox);
  YGNodeStyleSetBoxSizing(child2, YGBoxSizing::YGBoxSizingContentBox);

  /**
   * calculatelayout
   */
  YGNodeCalculateLayout(root, YGUndefined, YGUndefined, YGDirectionLTR);

  /**
   * 递归打印节点信息
   */
  LogNodeLayoutResRecursive(root);

  /**
   * 释放节点
   */
  YGNodeFreeRecursive(root);
}
```

calculate 布局结果如下：
```shell
[ node: 0 x: 0 y: 0 width: 220 height: 110 ]

[ node: 1 x: 10 y: 0 width: 100 height: 100 ]

[ node: 2 x: 110 y: 0 width: 110 height: 110 ]

[ node: 3 x: 0 y: 0 width: 110 height: 110 ]
```

## 1.前置知识
### 1.1 [box-sizing](https://developer.mozilla.org/en-US/docs/Web/CSS/box-sizing)
存在两种模式
1. border-box，是默认的模式，这时候view的实际内容宽度等于分配的宽度减去padding和border
2. content-box，这种模式下，实际内容的宽度就等于分配的宽度，而view的整体宽度则为分配的宽度加上padding和border

```c++
  // 设定boxsizing模式为 content-box
  YGNodeStyleSetBoxSizing(root, YGBoxSizing::YGBoxSizingContentBox);
  YGNodeStyleSetBoxSizing(child1, YGBoxSizing::YGBoxSizingContentBox);
  YGNodeStyleSetBoxSizing(child2, YGBoxSizing::YGBoxSizingContentBox);
```
### 1.2 SizingMode

```c++
  // 指定可用宽高均为 100(注意此宽高非root宽高!而是当前空间的可用宽高)
  YGNodeCalculateLayout(root, 100.0, 100.0, YGDirectionLTR);
```

尺寸模式
1. MaxContent: 未指定可用空间下盒子理想的尺寸模式，通常这种情况是最小的尺寸，但又刚好能包住content
2. StretchFit: 元素会尽可能地填满可用空间，但不会超出限制。
3. FitContent: 
   
| 可用宽 | 可用高 | 宽模式     | 高模式     |
| ------ | ------ | ---------- | ---------- |
| Nan    | Nan    | MaxContent | MaxContent |
| 100.0  | 100.0  | StretchFit | StretchFit |


### 1.3 [flex-basis](https://developer.mozilla.org/en-US/docs/Web/CSS/flex-basis)
指定了 flex 元素在主轴方向上的初始大小。当一个元素同时被设置了 flex-basis (除值为 auto 外) 和 width (或者在 flex-direction: column 情况下设置了height) , flex-basis 具有更高的优先级。


### 1.4 [flex-wrap](https://developer.mozilla.org/en-US/docs/Web/CSS/flex-wrap)

用于设置 flex 项目是强制排在一行还是可以换行到多行。如果允许换行，则设置行的堆叠方向。

1. nowrap: 默认值，弹性项目以单行形式排列，这可能会导致弹性容器溢出。

2. wrap: 弹性项目会分成多行。

3. wrap-reverse:行为与 相同wrap，但是起点和终点是颠倒的。

### 1.5 [justify-content](https://developer.mozilla.org/zh-CN/docs/Web/CSS/justify-content)

用于定义在主轴（主轴可以是水平或垂直方向，取决于 flex-direction 的值）上如何对齐和分布Flex容器中的项目。


## 2.Calculate

## 2.1 calculateLayoutImpl

1. 步骤1：算法前的准备工作，计算整个算法所需要的其他关键值，方便后续计算。

```c++
// 弹性布局主轴
  const FlexDirection mainAxis =
      resolveDirection(node->style().flexDirection(), direction);
  // 弹性布局副轴
  const FlexDirection crossAxis = resolveCrossDirection(mainAxis, direction);
  // 主轴是否是行
  const bool isMainAxisRow = isRow(mainAxis);
  
  const bool isNodeFlexWrap = node->style().flexWrap() != Wrap::NoWrap;
  // 主轴上的宿主长度
  const float mainAxisOwnerSize = isMainAxisRow ? ownerWidth : ownerHeight;
  // 副轴上的宿主长度
  const float crossAxisOwnerSize = isMainAxisRow ? ownerHeight : ownerWidth;

  // 主轴上的padding + border
  const float paddingAndBorderAxisMain =
      paddingAndBorderForAxis(node, mainAxis, direction, ownerWidth);
  // 副轴上的padding + border
  const float paddingAndBorderAxisCross =
      paddingAndBorderForAxis(node, crossAxis, direction, ownerWidth);
  // 副轴上开头的 padding和border
  const float leadingPaddingAndBorderCross =
      node->style().computeFlexStartPaddingAndBorder(
          crossAxis, direction, ownerWidth);
  // 主轴sizing模式
  SizingMode sizingModeMainDim =
      isMainAxisRow ? widthSizingMode : heightSizingMode;
  // 副轴sizing模式
  SizingMode sizingModeCrossDim =
      isMainAxisRow ? heightSizingMode : widthSizingMode;
  
  // row上的padding + border  
  const float paddingAndBorderAxisRow =
      isMainAxisRow ? paddingAndBorderAxisMain : paddingAndBorderAxisCross;
  // column上的padding + border  
  const float paddingAndBorderAxisColumn =
      isMainAxisRow ? paddingAndBorderAxisCross : paddingAndBorderAxisMain;
```

2. 步骤2：计算主轴和副轴上的可用空间
此处一般指排除padding和border后的空间,主要是针对具有确定长度的轴，做一个最大值最小值限制，免得超过范围。

```c++
float availableInnerWidth = calculateAvailableInnerDimension(
      node,
      direction,
      Dimension::Width,
      availableWidth - marginAxisRow,
      paddingAndBorderAxisRow,
      ownerWidth,
      ownerWidth);

//...
availableInnerDim = yoga::maxOrDefined(
        yoga::minOrDefined(availableInnerDim, maxInnerDim), minInnerDim);
```

3. 步骤3：计算主轴上的全部空间

```c++
float totalMainDim = 0;
  totalMainDim += computeFlexBasisForChildren(
      node,
      availableInnerWidth,
      availableInnerHeight,
      widthSizingMode,
      heightSizingMode,
      direction,
      mainAxis,
      performLayout,
      layoutMarkerData,
      depth,
      generationCount);

  // 叠加 gap 长度
  if (childCount > 1) {
    totalMainDim +=
        node->style().computeGapForAxis(mainAxis, availableInnerMainDim) *
        static_cast<float>(childCount - 1);
  }

  // 主轴上计算的总长度是否大于可用空间
  const bool mainAxisOverflows =
      (sizingModeMainDim != SizingMode::MaxContent) &&
      totalMainDim > availableInnerMainDim;

  if (isNodeFlexWrap && mainAxisOverflows &&
      sizingModeMainDim == SizingMode::FitContent) {
    sizingModeMainDim = SizingMode::StretchFit;
  }
```

递归计算的核心方法！空间长度等于以下项的累加：
    - 计算指定flexBasis的子元素长度，这个长度优先等于flex-basis 设置的长度，其次等于宽度
    - 计算 gap

4. 将 flex 项目转换为 flex 行列 

```c++
    auto flexLine = calculateFlexLine(
        node,
        ownerDirection,
        ownerWidth,
        mainAxisOwnerSize,
        availableInnerWidth,
        availableInnerMainDim,
        startOfLineIterator,
        lineCount);
```

计算 flex 布局项目主轴的行数

5. 解析主轴上的flex 长度
计算需要分配的剩余可用空间。如果不知道主轴大小，则根据行数计算它，因此没有更多的空间可以分配。


6. 主轴长度调整，副轴长度计算。
到这一步，所有 flex 项目主轴上宽度已经确认了，只需要按照顺序设置下 layout 即可