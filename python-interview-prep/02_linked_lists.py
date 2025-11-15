"""
LINKED LISTS - Complete Interview Solutions
============================================

This module contains 15+ linked list problems commonly asked in senior tech interviews.
Each solution includes multiple approaches, complexity analysis, and real-world applications.

Table of Contents:
1. Reverse Linked List
2. Detect Cycle in Linked List
3. Find Cycle Start
4. Merge Two Sorted Lists
5. Merge K Sorted Lists
6. Remove Nth Node From End
7. Add Two Numbers
8. Copy List with Random Pointer
9. Reorder List
10. Palindrome Linked List
11. Intersection of Two Linked Lists
12. Odd Even Linked List
13. Partition List
14. Rotate List
15. Reverse Nodes in k-Group
"""

from typing import Optional, List
import heapq


# ============================================================================
# LINKED LIST NODE DEFINITION
# ============================================================================

class ListNode:
    """Standard singly-linked list node"""
    def __init__(self, val=0, next=None):
        self.val = val
        self.next = next

    def __repr__(self):
        return f"ListNode({self.val})"


class Node:
    """Node with random pointer"""
    def __init__(self, val: int, next=None, random=None):
        self.val = val
        self.next = next
        self.random = random


# ============================================================================
# UTILITY FUNCTIONS
# ============================================================================

def create_linked_list(values: List[int]) -> Optional[ListNode]:
    """Create a linked list from a list of values"""
    if not values:
        return None

    head = ListNode(values[0])
    current = head

    for val in values[1:]:
        current.next = ListNode(val)
        current = current.next

    return head


def linked_list_to_list(head: Optional[ListNode]) -> List[int]:
    """Convert linked list to Python list for display"""
    result = []
    current = head

    while current:
        result.append(current.val)
        current = current.next

    return result


# ============================================================================
# 1. REVERSE LINKED LIST
# ============================================================================
"""
Problem: Reverse a singly linked list.

Difficulty: Easy
Companies: Amazon, Microsoft, Google, Facebook
Real-world Application: Undo functionality, navigation history
"""


def reverse_list_iterative(head: Optional[ListNode]) -> Optional[ListNode]:
    """
    Approach 1: Iterative with Three Pointers

    Algorithm:
    - Track previous, current, and next nodes
    - Reverse pointers one by one

    Time Complexity: O(n)
    Space Complexity: O(1)

    Diagram:
    Original: 1 → 2 → 3 → 4 → 5 → None

    Step 1:   None ← 1   2 → 3 → 4 → 5 → None
                   prev curr next

    Step 2:   None ← 1 ← 2   3 → 4 → 5 → None
                        prev curr next

    Step 3:   None ← 1 ← 2 ← 3   4 → 5 → None
                             prev curr next

    Final:    None ← 1 ← 2 ← 3 ← 4 ← 5
                                       head
    """
    prev = None
    current = head

    while current:
        # Save next node
        next_node = current.next

        # Reverse the pointer
        current.next = prev

        # Move pointers forward
        prev = current
        current = next_node

    return prev


def reverse_list_recursive(head: Optional[ListNode]) -> Optional[ListNode]:
    """
    Approach 2: Recursive

    Algorithm:
    - Recursively reverse the rest of the list
    - Fix the current node's pointer

    Time Complexity: O(n)
    Space Complexity: O(n) - recursion stack

    Diagram:
    reverse(1 → 2 → 3 → None)
      ↓
    reverse(2 → 3 → None) returns 3 → 2 → None
      Make 2.next.next = 2
      Make 2.next = None
      Returns 3 → 2 → 1 → None
    """
    # Base case
    if not head or not head.next:
        return head

    # Reverse the rest
    new_head = reverse_list_recursive(head.next)

    # Fix current node
    head.next.next = head
    head.next = None

    return new_head


# ============================================================================
# 2. DETECT CYCLE IN LINKED LIST
# ============================================================================
"""
Problem: Determine if a linked list has a cycle.

Difficulty: Easy
Companies: Amazon, Microsoft, Facebook
Real-world Application: Deadlock detection, circular reference detection
"""


def has_cycle(head: Optional[ListNode]) -> bool:
    """
    Approach: Floyd's Cycle Detection (Tortoise and Hare)

    Algorithm:
    - Use two pointers: slow (1 step) and fast (2 steps)
    - If they meet, there's a cycle
    - If fast reaches end, no cycle

    Time Complexity: O(n)
    Space Complexity: O(1)

    Diagram:
    No cycle:
    1 → 2 → 3 → 4 → None
    s   f
        s       f
            s           (fast reaches end)

    With cycle:
    1 → 2 → 3 → 4
        ↑       ↓
        6 ← 5 ←/

    s=1, f=2
    s=2, f=4
    s=3, f=6
    s=4, f=3
    s=5, f=5  ← They meet! Cycle detected
    """
    if not head or not head.next:
        return False

    slow = head
    fast = head.next

    while slow != fast:
        if not fast or not fast.next:
            return False
        slow = slow.next
        fast = fast.next.next

    return True


# ============================================================================
# 3. FIND CYCLE START
# ============================================================================
"""
Problem: If cycle exists, find the node where the cycle begins.

Difficulty: Medium
Companies: Amazon, Google, Microsoft
Real-world Application: Memory leak detection, graph analysis
"""


def detect_cycle(head: Optional[ListNode]) -> Optional[ListNode]:
    """
    Approach: Floyd's Algorithm Extended

    Algorithm:
    1. Detect cycle using slow/fast pointers
    2. If cycle exists, reset one pointer to head
    3. Move both one step at a time - they'll meet at cycle start

    Mathematical Proof:
    - Let distance to cycle start = x
    - Let distance from cycle start to meeting point = y
    - Let remaining cycle distance = z

    When they meet:
    - Slow traveled: x + y
    - Fast traveled: x + y + z + y = x + 2y + z

    Since fast = 2 * slow:
    x + 2y + z = 2(x + y)
    x + 2y + z = 2x + 2y
    z = x

    So moving from head and meeting point at same speed
    will meet at cycle start!

    Time Complexity: O(n)
    Space Complexity: O(1)

    Diagram:
    1 → 2 → 3 → 4 → 5
            ↑       ↓
            8 ← 7 ← 6

    Phase 1: Detect cycle
    - slow and fast meet at node 7

    Phase 2: Find start
    - ptr1 = head (node 1)
    - ptr2 = meeting point (node 7)
    - Move both 1 step at a time
    - They meet at node 3 (cycle start)
    """
    if not head:
        return None

    # Phase 1: Detect cycle
    slow = fast = head

    while fast and fast.next:
        slow = slow.next
        fast = fast.next.next

        if slow == fast:
            # Cycle detected
            break
    else:
        # No cycle
        return None

    # Phase 2: Find cycle start
    slow = head

    while slow != fast:
        slow = slow.next
        fast = fast.next

    return slow


# ============================================================================
# 4. MERGE TWO SORTED LISTS
# ============================================================================
"""
Problem: Merge two sorted linked lists into one sorted list.

Difficulty: Easy
Companies: Amazon, Microsoft, Google
Real-world Application: Data merging, sorted stream processing
"""


def merge_two_lists(l1: Optional[ListNode], l2: Optional[ListNode]) -> Optional[ListNode]:
    """
    Approach: Iterative Merge

    Algorithm:
    - Use dummy node to simplify edge cases
    - Compare values and attach smaller one
    - Attach remaining list at the end

    Time Complexity: O(n + m)
    Space Complexity: O(1)

    Diagram:
    l1: 1 → 3 → 5
    l2: 2 → 4 → 6

    Step 1: dummy → 1
    Step 2: dummy → 1 → 2
    Step 3: dummy → 1 → 2 → 3
    Step 4: dummy → 1 → 2 → 3 → 4
    Step 5: dummy → 1 → 2 → 3 → 4 → 5
    Step 6: dummy → 1 → 2 → 3 → 4 → 5 → 6
    """
    dummy = ListNode(0)
    current = dummy

    while l1 and l2:
        if l1.val <= l2.val:
            current.next = l1
            l1 = l1.next
        else:
            current.next = l2
            l2 = l2.next
        current = current.next

    # Attach remaining nodes
    current.next = l1 if l1 else l2

    return dummy.next


def merge_two_lists_recursive(l1: Optional[ListNode], l2: Optional[ListNode]) -> Optional[ListNode]:
    """
    Approach: Recursive

    Time Complexity: O(n + m)
    Space Complexity: O(n + m) - recursion stack
    """
    if not l1:
        return l2
    if not l2:
        return l1

    if l1.val <= l2.val:
        l1.next = merge_two_lists_recursive(l1.next, l2)
        return l1
    else:
        l2.next = merge_two_lists_recursive(l1, l2.next)
        return l2


# ============================================================================
# 5. MERGE K SORTED LISTS
# ============================================================================
"""
Problem: Merge k sorted linked lists into one sorted list.

Difficulty: Hard
Companies: Amazon, Google, Facebook, Microsoft
Real-world Application: External sorting, distributed data processing
"""


def merge_k_lists_heap(lists: List[Optional[ListNode]]) -> Optional[ListNode]:
    """
    Approach 1: Min Heap

    Algorithm:
    - Add first node from each list to min heap
    - Extract minimum, add to result
    - Add next node from same list to heap

    Time Complexity: O(N log k) where N = total nodes, k = lists
    Space Complexity: O(k) for heap

    Diagram:
    lists = [
      1 → 4 → 5,
      1 → 3 → 4,
      2 → 6
    ]

    Heap: [(1,list0), (1,list1), (2,list2)]

    Extract 1 from list0, add 4
    Heap: [(1,list1), (2,list2), (4,list0)]

    Extract 1 from list1, add 3
    Heap: [(2,list2), (3,list1), (4,list0)]

    Continue until heap empty...
    """
    if not lists:
        return None

    # Min heap: (value, list_index, node)
    heap = []

    # Add first node from each list
    for i, node in enumerate(lists):
        if node:
            heapq.heappush(heap, (node.val, i, node))

    dummy = ListNode(0)
    current = dummy

    while heap:
        val, list_idx, node = heapq.heappop(heap)

        # Add to result
        current.next = node
        current = current.next

        # Add next node from same list
        if node.next:
            heapq.heappush(heap, (node.next.val, list_idx, node.next))

    return dummy.next


def merge_k_lists_divide_conquer(lists: List[Optional[ListNode]]) -> Optional[ListNode]:
    """
    Approach 2: Divide and Conquer

    Algorithm:
    - Pair up lists and merge each pair
    - Repeat until one list remains

    Time Complexity: O(N log k)
    Space Complexity: O(1) - if we don't count recursion

    Diagram:
    Round 1: [1→4→5, 1→3→4, 2→6, 3→7]
             Merge pairs: [1→1→3→4→4→5, 2→3→6→7]

    Round 2: [1→1→3→4→4→5, 2→3→6→7]
             Merge: [1→1→2→3→3→4→4→5→6→7]
    """
    if not lists:
        return None
    if len(lists) == 1:
        return lists[0]

    def merge_lists(lists: List[Optional[ListNode]]) -> Optional[ListNode]:
        if len(lists) == 0:
            return None
        if len(lists) == 1:
            return lists[0]

        mid = len(lists) // 2
        left = merge_lists(lists[:mid])
        right = merge_lists(lists[mid:])

        return merge_two_lists(left, right)

    return merge_lists(lists)


# ============================================================================
# 6. REMOVE NTH NODE FROM END
# ============================================================================
"""
Problem: Remove the nth node from the end of the list.

Difficulty: Medium
Companies: Amazon, Microsoft, Facebook
Real-world Application: Buffer management, sliding window operations
"""


def remove_nth_from_end(head: Optional[ListNode], n: int) -> Optional[ListNode]:
    """
    Approach: Two Pointers (One Pass)

    Algorithm:
    - Use two pointers n nodes apart
    - When fast reaches end, slow is at node before target
    - Remove target node

    Time Complexity: O(L) where L is list length
    Space Complexity: O(1)

    Diagram:
    Remove 2nd from end in: 1 → 2 → 3 → 4 → 5

    Step 1: Move fast n+1 steps ahead
    dummy → 1 → 2 → 3 → 4 → 5 → None
    slow              fast

    Step 2: Move both until fast reaches end
    dummy → 1 → 2 → 3 → 4 → 5 → None
                   slow         fast

    Step 3: Remove node
    dummy → 1 → 2 → 3 → 5 → None
                   slow
    """
    dummy = ListNode(0, head)
    slow = fast = dummy

    # Move fast n+1 steps ahead
    for _ in range(n + 1):
        if fast:
            fast = fast.next

    # Move both until fast reaches end
    while fast:
        slow = slow.next
        fast = fast.next

    # Remove the nth node
    slow.next = slow.next.next

    return dummy.next


# ============================================================================
# 7. ADD TWO NUMBERS
# ============================================================================
"""
Problem: Add two numbers represented by linked lists (digits in reverse order).

Difficulty: Medium
Companies: Amazon, Microsoft, Google
Real-world Application: Big integer arithmetic, calculator implementation
"""


def add_two_numbers(l1: Optional[ListNode], l2: Optional[ListNode]) -> Optional[ListNode]:
    """
    Approach: Elementary Math

    Algorithm:
    - Add digits from both lists
    - Track carry for next position
    - Handle different lengths

    Time Complexity: O(max(m, n))
    Space Complexity: O(max(m, n))

    Diagram:
    l1: 2 → 4 → 3  (represents 342)
    l2: 5 → 6 → 4  (represents 465)

    Step 1: 2 + 5 = 7, carry = 0
    Result: 7

    Step 2: 4 + 6 = 10, carry = 1
    Result: 7 → 0

    Step 3: 3 + 4 + 1 = 8, carry = 0
    Result: 7 → 0 → 8  (represents 807)
    """
    dummy = ListNode(0)
    current = dummy
    carry = 0

    while l1 or l2 or carry:
        # Get values
        val1 = l1.val if l1 else 0
        val2 = l2.val if l2 else 0

        # Calculate sum
        total = val1 + val2 + carry
        carry = total // 10
        digit = total % 10

        # Create new node
        current.next = ListNode(digit)
        current = current.next

        # Move to next nodes
        l1 = l1.next if l1 else None
        l2 = l2.next if l2 else None

    return dummy.next


# ============================================================================
# 8. COPY LIST WITH RANDOM POINTER
# ============================================================================
"""
Problem: Deep copy a linked list where each node has a random pointer.

Difficulty: Medium
Companies: Amazon, Microsoft, Facebook
Real-world Application: Graph cloning, state replication
"""


def copy_random_list(head: Optional[Node]) -> Optional[Node]:
    """
    Approach 1: Hash Map

    Algorithm:
    - First pass: Create all nodes and store in map
    - Second pass: Set next and random pointers

    Time Complexity: O(n)
    Space Complexity: O(n)

    Diagram:
    Original:
    1 → 2 → 3
    ↓   ↓   ↓
    3   1   None

    Pass 1: Create nodes
    old_to_new = {
        1: 1',
        2: 2',
        3: 3'
    }

    Pass 2: Set pointers
    1' → 2' → 3'
    ↓    ↓    ↓
    3'   1'   None
    """
    if not head:
        return None

    # Create mapping from old to new nodes
    old_to_new = {}

    # First pass: create all nodes
    current = head
    while current:
        old_to_new[current] = Node(current.val)
        current = current.next

    # Second pass: set pointers
    current = head
    while current:
        if current.next:
            old_to_new[current].next = old_to_new[current.next]
        if current.random:
            old_to_new[current].random = old_to_new[current.random]
        current = current.next

    return old_to_new[head]


def copy_random_list_interleaving(head: Optional[Node]) -> Optional[Node]:
    """
    Approach 2: Interleaving (O(1) space)

    Algorithm:
    - Create copy nodes interleaved with original
    - Set random pointers
    - Separate the lists

    Time Complexity: O(n)
    Space Complexity: O(1)

    Diagram:
    Original: 1 → 2 → 3

    After interleaving:
    1 → 1' → 2 → 2' → 3 → 3'

    Set random pointers:
    if node.random exists:
        node.next.random = node.random.next

    Separate lists:
    1 → 2 → 3
    1' → 2' → 3'
    """
    if not head:
        return None

    # Step 1: Create interleaved list
    current = head
    while current:
        new_node = Node(current.val, current.next)
        current.next = new_node
        current = new_node.next

    # Step 2: Set random pointers
    current = head
    while current:
        if current.random:
            current.next.random = current.random.next
        current = current.next.next

    # Step 3: Separate lists
    old_head = head
    new_head = head.next
    current_old = old_head
    current_new = new_head

    while current_old:
        current_old.next = current_old.next.next
        current_new.next = current_new.next.next if current_new.next else None
        current_old = current_old.next
        current_new = current_new.next

    return new_head


# ============================================================================
# 9. REORDER LIST
# ============================================================================
"""
Problem: Reorder list: L0 → Ln → L1 → Ln-1 → L2 → Ln-2 → ...

Difficulty: Medium
Companies: Amazon, Microsoft, Facebook
Real-world Application: Data restructuring, playlist shuffling
"""


def reorder_list(head: Optional[ListNode]) -> None:
    """
    Approach: Find Middle + Reverse + Merge

    Algorithm:
    1. Find middle using slow/fast pointers
    2. Reverse second half
    3. Merge two halves alternately

    Time Complexity: O(n)
    Space Complexity: O(1)

    Diagram:
    Original: 1 → 2 → 3 → 4 → 5

    Step 1: Find middle
    First half:  1 → 2 → 3
    Second half: 4 → 5

    Step 2: Reverse second half
    First half:  1 → 2 → 3
    Second half: 5 → 4

    Step 3: Merge
    Result: 1 → 5 → 2 → 4 → 3
    """
    if not head or not head.next:
        return

    # Find middle
    slow = fast = head
    while fast.next and fast.next.next:
        slow = slow.next
        fast = fast.next.next

    # Reverse second half
    second = slow.next
    slow.next = None
    prev = None

    while second:
        next_node = second.next
        second.next = prev
        prev = second
        second = next_node

    second = prev

    # Merge two halves
    first = head
    while second:
        temp1 = first.next
        temp2 = second.next

        first.next = second
        second.next = temp1

        first = temp1
        second = temp2


# ============================================================================
# 10. PALINDROME LINKED LIST
# ============================================================================
"""
Problem: Check if a linked list is a palindrome.

Difficulty: Easy
Companies: Amazon, Microsoft, Facebook
Real-world Application: Data validation, pattern recognition
"""


def is_palindrome(head: Optional[ListNode]) -> bool:
    """
    Approach: Find Middle + Reverse Second Half + Compare

    Algorithm:
    1. Find middle using slow/fast pointers
    2. Reverse second half
    3. Compare first and second halves
    4. Optionally restore list

    Time Complexity: O(n)
    Space Complexity: O(1)

    Diagram:
    List: 1 → 2 → 3 → 2 → 1

    Step 1: Find middle
    First:  1 → 2 → 3
    Second: 2 → 1

    Step 2: Reverse second
    First:  1 → 2 → 3
    Second: 1 → 2

    Step 3: Compare
    1 == 1 ✓
    2 == 2 ✓
    Result: True (palindrome)
    """
    if not head or not head.next:
        return True

    # Find middle
    slow = fast = head
    while fast and fast.next:
        slow = slow.next
        fast = fast.next.next

    # Reverse second half
    prev = None
    while slow:
        next_node = slow.next
        slow.next = prev
        prev = slow
        slow = next_node

    # Compare halves
    left, right = head, prev
    while right:  # Only need to check second half
        if left.val != right.val:
            return False
        left = left.next
        right = right.next

    return True


# ============================================================================
# 11. REVERSE NODES IN K-GROUP
# ============================================================================
"""
Problem: Reverse nodes in groups of k.

Difficulty: Hard
Companies: Amazon, Microsoft, Google
Real-world Application: Batch processing, data chunking
"""


def reverse_k_group(head: Optional[ListNode], k: int) -> Optional[ListNode]:
    """
    Approach: Iterative Group Reversal

    Algorithm:
    1. Count if k nodes available
    2. Reverse k nodes
    3. Connect with previous and next groups
    4. Repeat

    Time Complexity: O(n)
    Space Complexity: O(1)

    Diagram:
    List: 1 → 2 → 3 → 4 → 5, k=2

    Step 1: Reverse first 2
    2 → 1 → 3 → 4 → 5

    Step 2: Reverse next 2
    2 → 1 → 4 → 3 → 5

    Step 3: Only 1 node left (< k), keep as is
    Result: 2 → 1 → 4 → 3 → 5
    """
    def reverse_linked_list(head, k):
        """Reverse first k nodes and return new head"""
        prev = None
        current = head

        for _ in range(k):
            next_node = current.next
            current.next = prev
            prev = current
            current = next_node

        return prev

    # Count nodes
    count = 0
    node = head
    while node:
        count += 1
        node = node.next

    dummy = ListNode(0, head)
    prev_group_end = dummy

    while count >= k:
        group_start = prev_group_end.next
        group_end = group_start

        # Find end of current group
        for _ in range(k - 1):
            group_end = group_end.next

        next_group_start = group_end.next

        # Reverse current group
        new_group_start = reverse_linked_list(group_start, k)

        # Connect with previous and next groups
        prev_group_end.next = new_group_start
        group_start.next = next_group_start

        # Move to next group
        prev_group_end = group_start
        count -= k

    return dummy.next


# ============================================================================
# TESTING AND EXAMPLES
# ============================================================================

def run_examples():
    """Run example test cases for all problems"""

    print("=" * 70)
    print("LINKED LISTS - EXAMPLE OUTPUTS")
    print("=" * 70)

    # Reverse Linked List
    print("\n1. REVERSE LINKED LIST")
    print("-" * 70)
    head = create_linked_list([1, 2, 3, 4, 5])
    print(f"Input: {linked_list_to_list(head)}")
    reversed_head = reverse_list_iterative(head)
    print(f"Output: {linked_list_to_list(reversed_head)}")

    # Merge Two Sorted Lists
    print("\n2. MERGE TWO SORTED LISTS")
    print("-" * 70)
    l1 = create_linked_list([1, 3, 5])
    l2 = create_linked_list([2, 4, 6])
    print(f"List 1: {linked_list_to_list(l1)}")
    print(f"List 2: {linked_list_to_list(l2)}")
    merged = merge_two_lists(l1, l2)
    print(f"Output: {linked_list_to_list(merged)}")

    # Remove Nth From End
    print("\n3. REMOVE NTH NODE FROM END")
    print("-" * 70)
    head = create_linked_list([1, 2, 3, 4, 5])
    n = 2
    print(f"Input: {linked_list_to_list(head)}, n={n}")
    result = remove_nth_from_end(head, n)
    print(f"Output: {linked_list_to_list(result)}")

    # Add Two Numbers
    print("\n4. ADD TWO NUMBERS")
    print("-" * 70)
    l1 = create_linked_list([2, 4, 3])
    l2 = create_linked_list([5, 6, 4])
    print(f"Number 1: {linked_list_to_list(l1)} (represents 342)")
    print(f"Number 2: {linked_list_to_list(l2)} (represents 465)")
    result = add_two_numbers(l1, l2)
    print(f"Output: {linked_list_to_list(result)} (represents 807)")

    # Palindrome Linked List
    print("\n5. PALINDROME LINKED LIST")
    print("-" * 70)
    head = create_linked_list([1, 2, 3, 2, 1])
    print(f"Input: {linked_list_to_list(head)}")
    print(f"Is Palindrome: {is_palindrome(head)}")

    print("\n" + "=" * 70)


if __name__ == "__main__":
    run_examples()
