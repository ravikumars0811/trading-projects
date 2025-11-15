"""
TREES AND BINARY SEARCH TREES - Complete Interview Solutions
=============================================================

This module contains 20+ tree problems commonly asked in senior tech interviews.
Each solution includes multiple approaches, complexity analysis, and real-world applications.

Table of Contents:
1. Binary Tree Traversals (Inorder, Preorder, Postorder, Level Order)
2. Maximum Depth of Binary Tree
3. Validate Binary Search Tree
4. Lowest Common Ancestor
5. Binary Tree Level Order Traversal
6. Zigzag Level Order Traversal
7. Construct Binary Tree from Traversals
8. Serialize and Deserialize Binary Tree
9. Invert Binary Tree
10. Symmetric Tree
11. Path Sum Problems
12. Binary Tree Maximum Path Sum
13. Count Complete Tree Nodes
14. Kth Smallest Element in BST
15. Binary Search Tree Iterator
16. Convert Sorted Array to BST
17. Flatten Binary Tree to Linked List
18. Populating Next Right Pointers
19. Recover Binary Search Tree
20. Morris Traversal
"""

from typing import Optional, List
from collections import deque, defaultdict
import sys


# ============================================================================
# TREE NODE DEFINITIONS
# ============================================================================

class TreeNode:
    """Standard binary tree node"""
    def __init__(self, val=0, left=None, right=None):
        self.val = val
        self.left = left
        self.right = right

    def __repr__(self):
        return f"TreeNode({self.val})"


class Node:
    """Tree node with next pointer"""
    def __init__(self, val: int = 0, left=None, right=None, next=None):
        self.val = val
        self.left = left
        self.right = right
        self.next = next


# ============================================================================
# UTILITY FUNCTIONS
# ============================================================================

def build_tree_from_list(values: List[Optional[int]]) -> Optional[TreeNode]:
    """Build a binary tree from level-order list representation"""
    if not values:
        return None

    root = TreeNode(values[0])
    queue = deque([root])
    i = 1

    while queue and i < len(values):
        node = queue.popleft()

        if i < len(values) and values[i] is not None:
            node.left = TreeNode(values[i])
            queue.append(node.left)
        i += 1

        if i < len(values) and values[i] is not None:
            node.right = TreeNode(values[i])
            queue.append(node.right)
        i += 1

    return root


def tree_to_list(root: Optional[TreeNode]) -> List[Optional[int]]:
    """Convert tree to level-order list"""
    if not root:
        return []

    result = []
    queue = deque([root])

    while queue:
        node = queue.popleft()
        if node:
            result.append(node.val)
            queue.append(node.left)
            queue.append(node.right)
        else:
            result.append(None)

    # Remove trailing None values
    while result and result[-1] is None:
        result.pop()

    return result


# ============================================================================
# 1. BINARY TREE TRAVERSALS
# ============================================================================
"""
Problem: Implement all tree traversal methods.

Difficulty: Easy-Medium
Companies: All major tech companies
Real-world Application: File system navigation, expression evaluation
"""


def inorder_traversal_recursive(root: Optional[TreeNode]) -> List[int]:
    """
    Inorder Traversal: Left → Root → Right (for BST gives sorted order)

    Time Complexity: O(n)
    Space Complexity: O(h) where h is height (recursion stack)

    Diagram:
         1
        / \
       2   3
      / \
     4   5

    Visit order: 4 → 2 → 5 → 1 → 3
    """
    result = []

    def inorder(node):
        if not node:
            return
        inorder(node.left)
        result.append(node.val)
        inorder(node.right)

    inorder(root)
    return result


def inorder_traversal_iterative(root: Optional[TreeNode]) -> List[int]:
    """
    Inorder Traversal: Iterative using Stack

    Algorithm:
    - Go as far left as possible, pushing nodes to stack
    - Pop and process node
    - Move to right child

    Time Complexity: O(n)
    Space Complexity: O(h)
    """
    result = []
    stack = []
    current = root

    while current or stack:
        # Go to leftmost node
        while current:
            stack.append(current)
            current = current.left

        # Process node
        current = stack.pop()
        result.append(current.val)

        # Move to right child
        current = current.right

    return result


def preorder_traversal(root: Optional[TreeNode]) -> List[int]:
    """
    Preorder Traversal: Root → Left → Right

    Diagram:
         1
        / \
       2   3
      / \
     4   5

    Visit order: 1 → 2 → 4 → 5 → 3
    """
    result = []

    def preorder(node):
        if not node:
            return
        result.append(node.val)
        preorder(node.left)
        preorder(node.right)

    preorder(root)
    return result


def postorder_traversal(root: Optional[TreeNode]) -> List[int]:
    """
    Postorder Traversal: Left → Right → Root

    Diagram:
         1
        / \
       2   3
      / \
     4   5

    Visit order: 4 → 5 → 2 → 3 → 1
    """
    result = []

    def postorder(node):
        if not node:
            return
        postorder(node.left)
        postorder(node.right)
        result.append(node.val)

    postorder(root)
    return result


def level_order_traversal(root: Optional[TreeNode]) -> List[List[int]]:
    """
    Level Order Traversal: Breadth-First Search

    Algorithm:
    - Use queue to process nodes level by level
    - Track level size to group nodes

    Time Complexity: O(n)
    Space Complexity: O(w) where w is max width

    Diagram:
         1
        / \
       2   3
      / \   \
     4   5   6

    Output: [[1], [2,3], [4,5,6]]
    """
    if not root:
        return []

    result = []
    queue = deque([root])

    while queue:
        level_size = len(queue)
        level = []

        for _ in range(level_size):
            node = queue.popleft()
            level.append(node.val)

            if node.left:
                queue.append(node.left)
            if node.right:
                queue.append(node.right)

        result.append(level)

    return result


# ============================================================================
# 2. MAXIMUM DEPTH OF BINARY TREE
# ============================================================================
"""
Problem: Find the maximum depth (height) of a binary tree.

Difficulty: Easy
Companies: Amazon, Microsoft, Google
Real-world Application: File system depth, organization hierarchy analysis
"""


def max_depth_recursive(root: Optional[TreeNode]) -> int:
    """
    Approach 1: Recursive DFS

    Algorithm:
    - Depth = 1 + max(left_depth, right_depth)

    Time Complexity: O(n)
    Space Complexity: O(h)

    Diagram:
         3
        / \
       9  20
          / \
         15  7

    Depth calculation:
    - Node 15: depth = 1
    - Node 7:  depth = 1
    - Node 20: depth = 1 + max(1, 1) = 2
    - Node 9:  depth = 1
    - Node 3:  depth = 1 + max(1, 2) = 3
    """
    if not root:
        return 0

    return 1 + max(max_depth_recursive(root.left), max_depth_recursive(root.right))


def max_depth_iterative(root: Optional[TreeNode]) -> int:
    """
    Approach 2: Iterative BFS

    Time Complexity: O(n)
    Space Complexity: O(w)
    """
    if not root:
        return 0

    queue = deque([root])
    depth = 0

    while queue:
        depth += 1
        level_size = len(queue)

        for _ in range(level_size):
            node = queue.popleft()
            if node.left:
                queue.append(node.left)
            if node.right:
                queue.append(node.right)

    return depth


# ============================================================================
# 3. VALIDATE BINARY SEARCH TREE
# ============================================================================
"""
Problem: Determine if a binary tree is a valid BST.

Difficulty: Medium
Companies: Amazon, Facebook, Microsoft, Google
Real-world Application: Data integrity validation, index verification
"""


def is_valid_bst(root: Optional[TreeNode]) -> bool:
    """
    Approach 1: Recursive with Range Validation

    Algorithm:
    - Each node must be within valid range [min, max]
    - Left subtree: max becomes current value
    - Right subtree: min becomes current value

    Time Complexity: O(n)
    Space Complexity: O(h)

    Diagram:
    Valid BST:
         5
        / \
       3   8
      / \ / \
     1  4 6  9

    Node 5: range (-∞, +∞) ✓
    Node 3: range (-∞, 5) ✓
    Node 8: range (5, +∞) ✓
    Node 1: range (-∞, 3) ✓
    Node 4: range (3, 5) ✓
    Node 6: range (5, 8) ✓
    Node 9: range (8, +∞) ✓

    Invalid BST:
         5
        / \
       3   8
      / \ / \
     1  6 6  9  ← 6 should be < 5 but is in left subtree
    """
    def validate(node, min_val, max_val):
        if not node:
            return True

        if node.val <= min_val or node.val >= max_val:
            return False

        return (validate(node.left, min_val, node.val) and
                validate(node.right, node.val, max_val))

    return validate(root, float('-inf'), float('inf'))


def is_valid_bst_inorder(root: Optional[TreeNode]) -> bool:
    """
    Approach 2: Inorder Traversal (should be sorted)

    Algorithm:
    - Perform inorder traversal
    - Check if values are strictly increasing

    Time Complexity: O(n)
    Space Complexity: O(n)
    """
    prev = [float('-inf')]

    def inorder(node):
        if not node:
            return True

        if not inorder(node.left):
            return False

        if node.val <= prev[0]:
            return False
        prev[0] = node.val

        return inorder(node.right)

    return inorder(root)


# ============================================================================
# 4. LOWEST COMMON ANCESTOR
# ============================================================================
"""
Problem: Find the lowest common ancestor of two nodes in a binary tree.

Difficulty: Medium
Companies: Amazon, Facebook, Microsoft, Google
Real-world Application: Version control, organizational hierarchy
"""


def lowest_common_ancestor(root: TreeNode, p: TreeNode, q: TreeNode) -> TreeNode:
    """
    Approach: Recursive DFS

    Algorithm:
    - If root is p or q, return root
    - Recursively find in left and right subtrees
    - If both found in different subtrees, current node is LCA
    - If found in one subtree only, that subtree contains LCA

    Time Complexity: O(n)
    Space Complexity: O(h)

    Diagram:
           3
          / \
         5   1
        / \ / \
       6  2 0  8
         / \
        7   4

    LCA(5, 1) = 3
    LCA(5, 4) = 5
    LCA(6, 2) = 5
    LCA(7, 4) = 2

    Example for LCA(7, 4):
    - At node 3: left finds something, right doesn't
    - At node 5: left finds something, right doesn't
    - At node 2: left finds 7, right finds 4 → return 2
    """
    # Base case
    if not root or root == p or root == q:
        return root

    # Search in left and right subtrees
    left = lowest_common_ancestor(root.left, p, q)
    right = lowest_common_ancestor(root.right, p, q)

    # If both found, current node is LCA
    if left and right:
        return root

    # Return whichever is not None
    return left if left else right


def lowest_common_ancestor_bst(root: TreeNode, p: TreeNode, q: TreeNode) -> TreeNode:
    """
    Optimized for BST: Use BST property

    Algorithm:
    - If both p and q are smaller than root, LCA is in left
    - If both are larger, LCA is in right
    - Otherwise, current node is LCA

    Time Complexity: O(h)
    Space Complexity: O(1)
    """
    while root:
        if p.val < root.val and q.val < root.val:
            root = root.left
        elif p.val > root.val and q.val > root.val:
            root = root.right
        else:
            return root

    return None


# ============================================================================
# 5. SERIALIZE AND DESERIALIZE BINARY TREE
# ============================================================================
"""
Problem: Design an algorithm to serialize and deserialize a binary tree.

Difficulty: Hard
Companies: Amazon, Facebook, Google, Microsoft
Real-world Application: Data persistence, network transmission, caching
"""


class Codec:
    """
    Approach: Preorder Traversal with Markers

    Algorithm:
    - Serialize: Preorder traversal, use '#' for null nodes
    - Deserialize: Reconstruct using preorder sequence

    Time Complexity: O(n) for both
    Space Complexity: O(n)

    Diagram:
         1
        / \
       2   3
          / \
         4   5

    Serialized: "1,2,#,#,3,4,#,#,5,#,#"

    Preorder: 1 → 2 → # → # → 3 → 4 → # → # → 5 → # → #
    """

    def serialize(self, root: Optional[TreeNode]) -> str:
        """Encode tree to a string"""
        def preorder(node):
            if not node:
                result.append('#')
                return
            result.append(str(node.val))
            preorder(node.left)
            preorder(node.right)

        result = []
        preorder(root)
        return ','.join(result)

    def deserialize(self, data: str) -> Optional[TreeNode]:
        """Decode string to tree"""
        def build():
            val = next(values)
            if val == '#':
                return None

            node = TreeNode(int(val))
            node.left = build()
            node.right = build()
            return node

        values = iter(data.split(','))
        return build()


# ============================================================================
# 6. BINARY TREE MAXIMUM PATH SUM
# ============================================================================
"""
Problem: Find the maximum path sum in a binary tree.
Path can start and end at any node.

Difficulty: Hard
Companies: Amazon, Facebook, Google
Real-world Application: Network optimization, resource allocation
"""


def max_path_sum(root: Optional[TreeNode]) -> int:
    """
    Approach: Recursive DFS with Global Maximum

    Algorithm:
    - For each node, calculate max path going through it
    - Path through node = node.val + left_max + right_max
    - Return to parent: node.val + max(left_max, right_max, 0)

    Time Complexity: O(n)
    Space Complexity: O(h)

    Diagram:
        -10
        / \
       9  20
          / \
         15  7

    Calculations:
    - Node 15: max_path = 15, return 15
    - Node 7:  max_path = 7, return 7
    - Node 20: max_path = 20 + 15 + 7 = 42 ✓, return 20 + 15 = 35
    - Node 9:  max_path = 9, return 9
    - Node -10: max_path = -10 + 9 + 35 = 34, return max(35, 9) - 10 = 25

    Result: 42 (path: 15 → 20 → 7)
    """
    max_sum = [float('-inf')]

    def max_gain(node):
        if not node:
            return 0

        # Get max path sum from left and right (ignore negative)
        left_gain = max(max_gain(node.left), 0)
        right_gain = max(max_gain(node.right), 0)

        # Path through current node
        current_path_sum = node.val + left_gain + right_gain

        # Update global maximum
        max_sum[0] = max(max_sum[0], current_path_sum)

        # Return max path sum including current node
        return node.val + max(left_gain, right_gain)

    max_gain(root)
    return max_sum[0]


# ============================================================================
# 7. KTH SMALLEST ELEMENT IN BST
# ============================================================================
"""
Problem: Find the kth smallest element in a BST.

Difficulty: Medium
Companies: Amazon, Google, Microsoft
Real-world Application: Ranking systems, top-k queries
"""


def kth_smallest(root: Optional[TreeNode], k: int) -> int:
    """
    Approach 1: Inorder Traversal (Iterative)

    Algorithm:
    - Inorder traversal gives sorted order for BST
    - Return kth element

    Time Complexity: O(h + k) where h is height
    Space Complexity: O(h)

    Diagram:
         5
        / \
       3   6
      / \
     2   4
    /
   1

    Inorder: 1, 2, 3, 4, 5, 6
    k=3 → return 3
    """
    stack = []
    current = root
    count = 0

    while current or stack:
        while current:
            stack.append(current)
            current = current.left

        current = stack.pop()
        count += 1

        if count == k:
            return current.val

        current = current.right

    return -1


def kth_smallest_optimized(root: Optional[TreeNode], k: int) -> int:
    """
    Approach 2: Augmented BST (for repeated queries)

    If we need to find kth element multiple times:
    - Store count of nodes in each subtree
    - Navigate based on counts

    Time Complexity: O(h) per query after O(n) preprocessing
    Space Complexity: O(n)
    """
    def count_nodes(node):
        if not node:
            return 0
        return 1 + count_nodes(node.left) + count_nodes(node.right)

    left_count = count_nodes(root.left)

    if k <= left_count:
        return kth_smallest_optimized(root.left, k)
    elif k == left_count + 1:
        return root.val
    else:
        return kth_smallest_optimized(root.right, k - left_count - 1)


# ============================================================================
# 8. CONSTRUCT BINARY TREE FROM TRAVERSALS
# ============================================================================
"""
Problem: Construct binary tree from inorder and preorder/postorder traversals.

Difficulty: Medium
Companies: Amazon, Microsoft, Facebook
Real-world Application: Tree reconstruction, data recovery
"""


def build_tree_preorder_inorder(preorder: List[int], inorder: List[int]) -> Optional[TreeNode]:
    """
    Construct tree from preorder and inorder traversals

    Algorithm:
    - First element of preorder is root
    - Find root in inorder to split left/right subtrees
    - Recursively build left and right subtrees

    Time Complexity: O(n)
    Space Complexity: O(n)

    Diagram:
    preorder = [3, 9, 20, 15, 7]
    inorder  = [9, 3, 15, 20, 7]

    Step 1: Root = 3 (first in preorder)
            Split inorder: [9] | 3 | [15, 20, 7]

    Step 2: Left subtree from [9] and [9]
            Right subtree from [20, 15, 7] and [15, 20, 7]

    Result:
         3
        / \
       9  20
          / \
         15  7
    """
    if not preorder or not inorder:
        return None

    # Build index map for inorder
    inorder_map = {val: i for i, val in enumerate(inorder)}

    def build(pre_start, pre_end, in_start, in_end):
        if pre_start > pre_end:
            return None

        # Root is first element in preorder
        root_val = preorder[pre_start]
        root = TreeNode(root_val)

        # Find root in inorder
        root_index = inorder_map[root_val]
        left_size = root_index - in_start

        # Build left and right subtrees
        root.left = build(pre_start + 1, pre_start + left_size,
                         in_start, root_index - 1)
        root.right = build(pre_start + left_size + 1, pre_end,
                          root_index + 1, in_end)

        return root

    return build(0, len(preorder) - 1, 0, len(inorder) - 1)


# ============================================================================
# 9. FLATTEN BINARY TREE TO LINKED LIST
# ============================================================================
"""
Problem: Flatten a binary tree to a linked list in-place (preorder).

Difficulty: Medium
Companies: Amazon, Microsoft, Facebook
Real-world Application: Tree linearization, memory optimization
"""


def flatten(root: Optional[TreeNode]) -> None:
    """
    Approach: Reverse Postorder (Right → Left → Root)

    Algorithm:
    - Process right subtree first
    - Then left subtree
    - Connect current node to previously processed

    Time Complexity: O(n)
    Space Complexity: O(h)

    Diagram:
         1
        / \
       2   5
      / \   \
     3   4   6

    Flattened:
    1 → 2 → 3 → 4 → 5 → 6

    Process order (reverse postorder):
    6 → 5 → 4 → 3 → 2 → 1
    """
    prev = [None]

    def flatten_tree(node):
        if not node:
            return

        # Process right, then left, then current
        flatten_tree(node.right)
        flatten_tree(node.left)

        # Connect current node
        node.right = prev[0]
        node.left = None
        prev[0] = node

    flatten_tree(root)


# ============================================================================
# 10. MORRIS TRAVERSAL
# ============================================================================
"""
Problem: Inorder traversal with O(1) space (no stack/recursion).

Difficulty: Hard
Companies: Google, Facebook
Real-world Application: Memory-constrained systems, embedded systems
"""


def morris_inorder_traversal(root: Optional[TreeNode]) -> List[int]:
    """
    Approach: Morris Traversal using Threaded Binary Tree

    Algorithm:
    - Create temporary links (threads) to successor nodes
    - Use these threads to navigate back
    - Remove threads after use

    Time Complexity: O(n)
    Space Complexity: O(1) - excluding output

    Diagram:
         1
        / \
       2   3
      / \
     4   5

    Step 1: Create thread from 5 to 1
         1
        / \
       2   3
      / \
     4   5 ⤶

    Step 2: Visit nodes using threads
    Step 3: Remove threads
    """
    result = []
    current = root

    while current:
        if not current.left:
            # No left child, visit and go right
            result.append(current.val)
            current = current.right
        else:
            # Find inorder predecessor
            predecessor = current.left
            while predecessor.right and predecessor.right != current:
                predecessor = predecessor.right

            if not predecessor.right:
                # Create thread
                predecessor.right = current
                current = current.left
            else:
                # Remove thread, visit, go right
                predecessor.right = None
                result.append(current.val)
                current = current.right

    return result


# ============================================================================
# TESTING AND EXAMPLES
# ============================================================================

def run_examples():
    """Run example test cases for all problems"""

    print("=" * 70)
    print("TREES AND BST - EXAMPLE OUTPUTS")
    print("=" * 70)

    # Maximum Depth
    print("\n1. MAXIMUM DEPTH OF BINARY TREE")
    print("-" * 70)
    root = build_tree_from_list([3, 9, 20, None, None, 15, 7])
    print(f"Tree: {tree_to_list(root)}")
    print(f"Maximum Depth: {max_depth_recursive(root)}")

    # Validate BST
    print("\n2. VALIDATE BINARY SEARCH TREE")
    print("-" * 70)
    root = build_tree_from_list([5, 3, 8, 1, 4, 6, 9])
    print(f"Tree: {tree_to_list(root)}")
    print(f"Is Valid BST: {is_valid_bst(root)}")

    # Level Order Traversal
    print("\n3. LEVEL ORDER TRAVERSAL")
    print("-" * 70)
    root = build_tree_from_list([1, 2, 3, 4, 5, None, 6])
    print(f"Tree: {tree_to_list(root)}")
    print(f"Level Order: {level_order_traversal(root)}")

    # Serialize and Deserialize
    print("\n4. SERIALIZE AND DESERIALIZE BINARY TREE")
    print("-" * 70)
    root = build_tree_from_list([1, 2, 3, None, None, 4, 5])
    codec = Codec()
    serialized = codec.serialize(root)
    print(f"Original Tree: {tree_to_list(root)}")
    print(f"Serialized: {serialized}")
    deserialized = codec.deserialize(serialized)
    print(f"Deserialized: {tree_to_list(deserialized)}")

    # Maximum Path Sum
    print("\n5. BINARY TREE MAXIMUM PATH SUM")
    print("-" * 70)
    root = build_tree_from_list([-10, 9, 20, None, None, 15, 7])
    print(f"Tree: {tree_to_list(root)}")
    print(f"Maximum Path Sum: {max_path_sum(root)}")

    # Kth Smallest in BST
    print("\n6. KTH SMALLEST ELEMENT IN BST")
    print("-" * 70)
    root = build_tree_from_list([5, 3, 6, 2, 4, None, None, 1])
    k = 3
    print(f"Tree: {tree_to_list(root)}")
    print(f"Kth Smallest (k={k}): {kth_smallest(root, k)}")

    print("\n" + "=" * 70)


if __name__ == "__main__":
    run_examples()
