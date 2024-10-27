# 结论

yoga测试用例如下，其中child1和child2均为content-box模式
```
                      root0
                    (Nan,Nan)
                    /         \
            child1              child2
(100,100,padding:10,border:10) (110,110)  
                                    |
                                    child3
                                   (Nan,Nan)
```
calculate 测试信息如下：
```shell
node tag: 0,availableWidth: nan,availableHeight: nan,widthSizingMode: MaxContent,heightSizingMode: MaxContent,ownerWidth: nan,ownerHeight: nan
node tag: 1,availableWidth: 140,availableHeight: 140,widthSizingMode: StretchFit,heightSizingMode: StretchFit,ownerWidth: nan,ownerHeight: nan
node tag: 2,availableWidth: 110,availableHeight: 110,widthSizingMode: StretchFit,heightSizingMode: StretchFit,ownerWidth: nan,ownerHeight: nan
# 此处存在疑问，为什么node3的宽高模式就是StretchFit?
node tag: 3,availableWidth: 110,availableHeight: 110,widthSizingMode: StretchFit,heightSizingMode: FitContent,ownerWidth: 110,ownerHeight: 110
node tag: 3,availableWidth: 110,availableHeight: 0,widthSizingMode: StretchFit,heightSizingMode: StretchFit,ownerWidth: 110,ownerHeight: 110
node tag: 3,availableWidth: 110,availableHeight: 0,widthSizingMode: StretchFit,heightSizingMode: StretchFit,ownerWidth: 110,ownerHeight: 110
```

## 1.前置知识
### 1.1 BoxSizing
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


### 1.3 flex-basis
指定了 flex 元素在主轴方向上的初始大小。当一个元素同时被设置了 flex-basis (除值为 auto 外) 和 width (或者在 flex-direction: column 情况下设置了height) , flex-basis 具有更高的优先级。


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

3. 步骤3：计算