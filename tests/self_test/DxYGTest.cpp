#include <yoga/Yoga.h>
#include <cmath>
#include <iostream>

/**
 * box_size
 * 存在两种模式，一种为border-box，是默认的模式，这时候view的实际内容宽度等于分配的宽度减去padding和border
 *                    另一种为
 * content-box，这种模式下，实际内容的宽度就等于分配的宽度，而view的整体宽度则为分配的宽度加上padding和border
 */
void box_sizing_test() {
  YGNodeRef root = YGNodeNew();
  YGNodeSetTag(root, 0);

  YGNodeRef child1 = YGNodeNew();
  YGNodeSetTag(child1, 1);
  YGNodeStyleSetWidth(child1, 100.0);
  YGNodeStyleSetHeight(child1, 100.0);
  // 给child1 施加一个 10.0的 border 和 padding
  YGNodeStyleSetBorder(child1, YGEdge::YGEdgeAll, 10.0);
  YGNodeStyleSetPadding(child1, YGEdge::YGEdgeAll, 10.0);

  YGNodeRef child2 = YGNodeNew();
  YGNodeSetTag(child2, 2);
  YGNodeStyleSetWidth(child2, 110.0);
  YGNodeStyleSetHeight(child2, 110.0);

  YGNodeRef child3 = YGNodeNew();
  YGNodeSetTag(child3, 3);
  YGNodeInsertChild(root, child1, 0);
  YGNodeInsertChild(root, child2, 1);
  YGNodeInsertChild(child2, child3, 0);

  // 方向从左到右
  YGNodeStyleSetDirection(root, YGDirection::YGDirectionLTR);

  // 纵向排列
  YGNodeStyleSetFlexDirection(root, YGFlexDirectionColumn);

  // 设定boxsizing模式为 content-box
  // YGNodeStyleSetBoxSizing(root, YGBoxSizing::YGBoxSizingContentBox);

  // 设定boxsizing模式为 content-box
  YGNodeStyleSetBoxSizing(root, YGBoxSizing::YGBoxSizingContentBox);
  YGNodeStyleSetBoxSizing(child1, YGBoxSizing::YGBoxSizingContentBox);
  YGNodeStyleSetBoxSizing(child2, YGBoxSizing::YGBoxSizingContentBox);

  // 指定宽高均为 YGUndefined
  YGNodeCalculateLayout(root, YGUndefined, YGUndefined, YGDirectionLTR);

  // 指定可用宽高均为 100(注意此宽高非root宽高!而是当前空间的可用宽高)
  // YGNodeCalculateLayout(root, 100.0, 100.0, YGDirectionLTR);

  std::cout << "box_sizing 是 content-box，该情况下宽高分别为:" << std::endl;
  std::cout << "root width: " << YGNodeLayoutGetWidth(root) << std::endl;
  std::cout << "root height: " << YGNodeLayoutGetHeight(root) << std::endl;

  YGNodeFree(root);
  YGNodeFree(child1);
  YGNodeFree(child2);
  YGNodeFree(child3);
}

int main(int argc, const char** argv) {
  box_sizing_test();
  return 0;
}