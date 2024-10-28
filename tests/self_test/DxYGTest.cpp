#include <yoga/Yoga.h>
#include <yoga/node/Node.h>
#include <cmath>
#include <iostream>

/**
 * box_size
 * 存在两种模式，一种为border-box，是默认的模式，这时候view的实际内容宽度等于分配的宽度减去padding和border
 *                    另一种为
 * content-box，这种模式下，实际内容的宽度就等于分配的宽度，而view的整体宽度则为分配的宽度加上padding和border
 */
void box_sizing_test() {
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
  // YGNodeStyleSetPadding(child1, YGEdge::YGEdgeAll, 10.0);

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

int main(int argc, const char** argv) {
  box_sizing_test();
  return 0;
}