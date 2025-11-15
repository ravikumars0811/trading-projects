"""
ADVANCED TOPICS - Backtracking, Heaps, Tries, Bit Manipulation
==============================================================

This module contains advanced algorithms and data structures.

Table of Contents:

BACKTRACKING:
1. Permutations
2. Combinations
3. Subsets
4. N-Queens
5. Sudoku Solver
6. Generate Parentheses
7. Letter Combinations of Phone Number
8. Palindrome Partitioning

HEAPS & PRIORITY QUEUES:
9. Kth Largest Element
10. Top K Frequent Elements
11. Merge K Sorted Lists
12. Find Median from Data Stream
13. Task Scheduler

TRIES:
14. Implement Trie
15. Word Search II
16. Design Add and Search Words

BIT MANIPULATION:
17. Single Number
18. Number of 1 Bits
19. Counting Bits
20. Sum of Two Integers
21. Reverse Bits
22. Power of Two
"""

from typing import List, Optional
from collections import Counter, deque
import heapq


# ============================================================================
# BACKTRACKING
# ============================================================================

"""
BACKTRACKING PATTERN:

1. Make a choice
2. Explore recursively
3. Undo the choice (backtrack)

Template:
def backtrack(state):
    if is_solution(state):
        add_to_results(state)
        return

    for choice in get_choices(state):
        make_choice(choice)
        backtrack(new_state)
        undo_choice(choice)
"""


# ============================================================================
# 1. PERMUTATIONS
# ============================================================================
"""
Problem: Generate all permutations of an array.

Difficulty: Medium
Companies: Amazon, Google, Microsoft
Real-world Application: Combinatorial optimization, testing
"""


def permute(nums: List[int]) -> List[List[int]]:
    """
    Approach: Backtracking

    Time Complexity: O(n! * n)
    Space Complexity: O(n)

    Diagram:
    nums = [1, 2, 3]

    Decision Tree:
                    []
           /        |        \
          1         2         3
        /  \       / \       / \
       2    3     1   3     1   2
       |    |     |   |     |   |
       3    2     3   1     2   1

    Results: [1,2,3], [1,3,2], [2,1,3], [2,3,1], [3,1,2], [3,2,1]
    """
    result = []

    def backtrack(current, remaining):
        if not remaining:
            result.append(current[:])
            return

        for i in range(len(remaining)):
            # Choose
            current.append(remaining[i])

            # Explore
            backtrack(current, remaining[:i] + remaining[i + 1:])

            # Unchoose
            current.pop()

    backtrack([], nums)
    return result


# ============================================================================
# 2. COMBINATIONS
# ============================================================================
"""
Problem: Generate all k-sized combinations from 1 to n.

Difficulty: Medium
Companies: Amazon, Google
"""


def combine(n: int, k: int) -> List[List[int]]:
    """
    Approach: Backtracking

    Time Complexity: O(C(n,k) * k)
    Space Complexity: O(k)

    Diagram for n=4, k=2:
                    []
         /      /       \      \
        1      2         3      4
      / | \    |  \       \
     2  3  4   3   4       4

    Results: [1,2], [1,3], [1,4], [2,3], [2,4], [3,4]
    """
    result = []

    def backtrack(start, current):
        if len(current) == k:
            result.append(current[:])
            return

        for i in range(start, n + 1):
            current.append(i)
            backtrack(i + 1, current)
            current.pop()

    backtrack(1, [])
    return result


# ============================================================================
# 3. SUBSETS
# ============================================================================
"""
Problem: Generate all subsets (power set).

Difficulty: Medium
Companies: Amazon, Facebook, Google
"""


def subsets(nums: List[int]) -> List[List[int]]:
    """
    Approach: Backtracking

    Time Complexity: O(2^n * n)
    Space Complexity: O(n)

    Diagram for [1,2,3]:
                      []
                /            \
              [1]             []
           /      \         /    \
         [1,2]   [1]      [2]    []
        /   \    / \     /  \    / \
    [1,2,3][1,2][1,3][1][2,3][2][3][]

    8 subsets total (2^3)
    """
    result = []

    def backtrack(start, current):
        result.append(current[:])

        for i in range(start, len(nums)):
            current.append(nums[i])
            backtrack(i + 1, current)
            current.pop()

    backtrack(0, [])
    return result


# ============================================================================
# 4. N-QUEENS
# ============================================================================
"""
Problem: Place N queens on N×N board so no two attack each other.

Difficulty: Hard
Companies: Amazon, Google, Microsoft
Real-world Application: Constraint satisfaction problems
"""


def solve_n_queens(n: int) -> List[List[str]]:
    """
    Approach: Backtracking with pruning

    Time Complexity: O(n!)
    Space Complexity: O(n²)

    Diagram for n=4:
    Solution 1:     Solution 2:
    . Q . .         . . Q .
    . . . Q         Q . . .
    Q . . .         . . . Q
    . . Q .         . Q . .

    Constraints:
    - One queen per row
    - One queen per column
    - One queen per diagonal
    """
    result = []
    board = [['.'] * n for _ in range(n)]

    # Track columns and diagonals
    cols = set()
    diag1 = set()  # row - col
    diag2 = set()  # row + col

    def backtrack(row):
        if row == n:
            result.append([''.join(row) for row in board])
            return

        for col in range(n):
            if col in cols or (row - col) in diag1 or (row + col) in diag2:
                continue

            # Place queen
            board[row][col] = 'Q'
            cols.add(col)
            diag1.add(row - col)
            diag2.add(row + col)

            backtrack(row + 1)

            # Remove queen
            board[row][col] = '.'
            cols.remove(col)
            diag1.remove(row - col)
            diag2.remove(row + col)

    backtrack(0)
    return result


# ============================================================================
# 5. GENERATE PARENTHESES
# ============================================================================
"""
Problem: Generate all valid combinations of n pairs of parentheses.

Difficulty: Medium
Companies: Amazon, Google, Facebook
"""


def generate_parenthesis(n: int) -> List[str]:
    """
    Approach: Backtracking with constraints

    Time Complexity: O(4^n / √n) - Catalan number
    Space Complexity: O(n)

    Diagram for n=3:
                    ""
                    (
               (           )
            (     )     (
          (  )   ( )   (
         () (   () (   ()
        ()( ()(  ()(  (()

    Valid sequences: ((())), (()()), (())(), ()(()), ()()()
    """
    result = []

    def backtrack(current, open_count, close_count):
        if len(current) == 2 * n:
            result.append(current)
            return

        # Can always add opening if we have quota
        if open_count < n:
            backtrack(current + '(', open_count + 1, close_count)

        # Can add closing only if more opens than closes
        if close_count < open_count:
            backtrack(current + ')', open_count, close_count + 1)

    backtrack('', 0, 0)
    return result


# ============================================================================
# HEAPS & PRIORITY QUEUES
# ============================================================================

"""
HEAP OPERATIONS:
- heapq.heappush(heap, item): O(log n)
- heapq.heappop(heap): O(log n)
- heapq.heapify(list): O(n)
- heap[0]: O(1) - peek min

Python heapq is a MIN heap by default
For MAX heap: negate values or use custom comparator
"""


# ============================================================================
# 6. KTH LARGEST ELEMENT
# ============================================================================
"""
Problem: Find the kth largest element in an unsorted array.

Difficulty: Medium
Companies: Amazon, Facebook, Microsoft
Real-world Application: Ranking systems, top-k queries
"""


def find_kth_largest(nums: List[int], k: int) -> int:
    """
    Approach 1: Min Heap of size k

    Time Complexity: O(n log k)
    Space Complexity: O(k)

    Diagram for nums=[3,2,1,5,6,4], k=2:

    Process elements, maintain heap of size k:
    [3]
    [2, 3]
    [2, 3] (remove 1)
    [3, 5]
    [5, 6]
    [5, 6] (remove 4)

    Answer: min of heap = 5 (2nd largest)
    """
    heap = []

    for num in nums:
        heapq.heappush(heap, num)
        if len(heap) > k:
            heapq.heappop(heap)

    return heap[0]


def find_kth_largest_quickselect(nums: List[int], k: int) -> int:
    """
    Approach 2: QuickSelect

    Time Complexity: O(n) average, O(n²) worst
    Space Complexity: O(1)
    """
    k = len(nums) - k  # Convert to finding kth smallest

    def quickselect(left, right):
        pivot = nums[right]
        i = left

        for j in range(left, right):
            if nums[j] <= pivot:
                nums[i], nums[j] = nums[j], nums[i]
                i += 1

        nums[i], nums[right] = nums[right], nums[i]

        if i == k:
            return nums[i]
        elif i < k:
            return quickselect(i + 1, right)
        else:
            return quickselect(left, i - 1)

    return quickselect(0, len(nums) - 1)


# ============================================================================
# 7. TOP K FREQUENT ELEMENTS
# ============================================================================
"""
Problem: Find k most frequent elements.

Difficulty: Medium
Companies: Amazon, Facebook, Google
"""


def top_k_frequent(nums: List[int], k: int) -> List[int]:
    """
    Approach: Heap

    Time Complexity: O(n log k)
    Space Complexity: O(n)

    Diagram for nums=[1,1,1,2,2,3], k=2:

    Frequency count: {1: 3, 2: 2, 3: 1}

    Min heap (by frequency):
    [(3, 1)]
    [(2, 2), (3, 1)]

    Result: [1, 2]
    """
    count = Counter(nums)

    # Use heap to find k most frequent
    return [num for num, freq in count.most_common(k)]


# ============================================================================
# 8. FIND MEDIAN FROM DATA STREAM
# ============================================================================
"""
Problem: Design a data structure to find median from a stream.

Difficulty: Hard
Companies: Amazon, Google, Facebook
Real-world Application: Real-time statistics, monitoring
"""


class MedianFinder:
    """
    Approach: Two Heaps

    Algorithm:
    - Max heap for lower half
    - Min heap for upper half
    - Keep heaps balanced

    Operations:
    - addNum: O(log n)
    - findMedian: O(1)

    Diagram:
    Stream: 1, 2, 3, 4, 5

    After 1:     After 2:     After 3:
    max: [1]     max: [1]     max: [2, 1]
    min: []      min: [2]     min: [3]
    median: 1    median: 1.5  median: 2
    """

    def __init__(self):
        self.small = []  # Max heap (negated)
        self.large = []  # Min heap

    def addNum(self, num: int) -> None:
        # Add to max heap (small)
        heapq.heappush(self.small, -num)

        # Balance: ensure all in small <= all in large
        if self.small and self.large and (-self.small[0] > self.large[0]):
            val = -heapq.heappop(self.small)
            heapq.heappush(self.large, val)

        # Balance sizes
        if len(self.small) > len(self.large) + 1:
            val = -heapq.heappop(self.small)
            heapq.heappush(self.large, val)
        if len(self.large) > len(self.small):
            val = heapq.heappop(self.large)
            heapq.heappush(self.small, -val)

    def findMedian(self) -> float:
        if len(self.small) > len(self.large):
            return -self.small[0]
        return (-self.small[0] + self.large[0]) / 2.0


# ============================================================================
# TRIES
# ============================================================================

"""
TRIE STRUCTURE:

         root
        / | \
       a  b  c
      /   |   \
     p    a    a
     |    |    |
     p    t    t
    (end)(end)(end)

Words: app, bat, cat
"""


# ============================================================================
# 9. IMPLEMENT TRIE
# ============================================================================
"""
Problem: Implement a trie with insert, search, and startsWith operations.

Difficulty: Medium
Companies: Amazon, Google, Microsoft
Real-world Application: Autocomplete, spell checker
"""


class TrieNode:
    def __init__(self):
        self.children = {}
        self.is_end = False


class Trie:
    """
    Operations:
    - insert: O(m) where m is word length
    - search: O(m)
    - startsWith: O(m)

    Space Complexity: O(ALPHABET_SIZE * N * M)
    """

    def __init__(self):
        self.root = TrieNode()

    def insert(self, word: str) -> None:
        """Insert a word into the trie"""
        node = self.root

        for char in word:
            if char not in node.children:
                node.children[char] = TrieNode()
            node = node.children[char]

        node.is_end = True

    def search(self, word: str) -> bool:
        """Returns True if word is in trie"""
        node = self.root

        for char in word:
            if char not in node.children:
                return False
            node = node.children[char]

        return node.is_end

    def startsWith(self, prefix: str) -> bool:
        """Returns True if there's a word with given prefix"""
        node = self.root

        for char in prefix:
            if char not in node.children:
                return False
            node = node.children[char]

        return True


# ============================================================================
# BIT MANIPULATION
# ============================================================================

"""
COMMON BIT OPERATIONS:

1. Check if ith bit is set: (n & (1 << i)) != 0
2. Set ith bit: n | (1 << i)
3. Clear ith bit: n & ~(1 << i)
4. Toggle ith bit: n ^ (1 << i)
5. Clear lowest set bit: n & (n - 1)
6. Get lowest set bit: n & (-n)
7. Check if power of 2: n & (n - 1) == 0
"""


# ============================================================================
# 10. SINGLE NUMBER
# ============================================================================
"""
Problem: Find the element that appears once while others appear twice.

Difficulty: Easy
Companies: Amazon, Google
Real-world Application: Error detection, data deduplication
"""


def single_number(nums: List[int]) -> int:
    """
    Approach: XOR

    Property: a ^ a = 0, a ^ 0 = a

    Time Complexity: O(n)
    Space Complexity: O(1)

    Diagram:
    nums = [4, 1, 2, 1, 2]

    4 ^ 1 = 5   (100 ^ 001 = 101)
    5 ^ 2 = 7   (101 ^ 010 = 111)
    7 ^ 1 = 6   (111 ^ 001 = 110)
    6 ^ 2 = 4   (110 ^ 010 = 100)

    Answer: 4
    """
    result = 0
    for num in nums:
        result ^= num
    return result


# ============================================================================
# 11. NUMBER OF 1 BITS
# ============================================================================
"""
Problem: Count the number of 1 bits in an integer.

Difficulty: Easy
Companies: All major companies
"""


def hamming_weight(n: int) -> int:
    """
    Approach: Clear lowest set bit repeatedly

    Time Complexity: O(k) where k is number of 1 bits
    Space Complexity: O(1)

    Diagram for n=11 (1011):
    1011 & 1010 = 1010  (cleared 1 bit, count=1)
    1010 & 1001 = 1000  (cleared 1 bit, count=2)
    1000 & 0111 = 0000  (cleared 1 bit, count=3)

    Answer: 3
    """
    count = 0
    while n:
        n &= n - 1  # Clear lowest set bit
        count += 1
    return count


# ============================================================================
# 12. COUNTING BITS
# ============================================================================
"""
Problem: For 0 ≤ i ≤ n, count number of 1's in binary representation.

Difficulty: Easy
Companies: Amazon, Google
"""


def count_bits(n: int) -> List[int]:
    """
    Approach: DP with bit manipulation

    Observation: bits(i) = bits(i >> 1) + (i & 1)

    Time Complexity: O(n)
    Space Complexity: O(1) excluding output

    Diagram:
    i:    0  1  2  3  4  5  6  7  8
    bin:  0  1 10 11 100 101 110 111 1000
    bits: 0  1  1  2  1  2  2  3  1

    bits(6) = bits(3) + 0 = 2 + 0 = 2
    """
    result = [0] * (n + 1)

    for i in range(1, n + 1):
        result[i] = result[i >> 1] + (i & 1)

    return result


# ============================================================================
# 13. REVERSE BITS
# ============================================================================
"""
Problem: Reverse bits of a 32-bit unsigned integer.

Difficulty: Easy
Companies: Amazon, Apple
"""


def reverse_bits(n: int) -> int:
    """
    Approach: Process bit by bit

    Time Complexity: O(32) = O(1)
    Space Complexity: O(1)

    Diagram:
    Input:  00000010100101000001111010011100
    Output: 00111001011110000010100101000000
    """
    result = 0

    for i in range(32):
        # Get least significant bit
        bit = (n >> i) & 1
        # Place it in reversed position
        result |= (bit << (31 - i))

    return result


# ============================================================================
# TESTING AND EXAMPLES
# ============================================================================

def run_examples():
    """Run example test cases for all problems"""

    print("=" * 70)
    print("ADVANCED TOPICS - EXAMPLE OUTPUTS")
    print("=" * 70)

    # Permutations
    print("\n1. PERMUTATIONS")
    print("-" * 70)
    nums = [1, 2, 3]
    print(f"Input: {nums}")
    print(f"Permutations: {permute(nums)}")

    # N-Queens
    print("\n2. N-QUEENS (n=4)")
    print("-" * 70)
    solutions = solve_n_queens(4)
    print(f"Number of solutions: {len(solutions)}")
    print("First solution:")
    for row in solutions[0]:
        print(row)

    # Generate Parentheses
    print("\n3. GENERATE PARENTHESES")
    print("-" * 70)
    n = 3
    print(f"n = {n}")
    print(f"Valid combinations: {generate_parenthesis(n)}")

    # Kth Largest
    print("\n4. KTH LARGEST ELEMENT")
    print("-" * 70)
    nums = [3, 2, 1, 5, 6, 4]
    k = 2
    print(f"Array: {nums}, k={k}")
    print(f"Kth largest: {find_kth_largest(nums, k)}")

    # Median Finder
    print("\n5. MEDIAN FINDER")
    print("-" * 70)
    mf = MedianFinder()
    stream = [1, 2, 3, 4, 5]
    print(f"Stream: {stream}")
    for num in stream:
        mf.addNum(num)
        print(f"After adding {num}, median: {mf.findMedian()}")

    # Trie
    print("\n6. TRIE OPERATIONS")
    print("-" * 70)
    trie = Trie()
    words = ["apple", "app", "apricot"]
    print(f"Inserting: {words}")
    for word in words:
        trie.insert(word)
    print(f"Search 'app': {trie.search('app')}")
    print(f"Search 'appl': {trie.search('appl')}")
    print(f"StartsWith 'app': {trie.startsWith('app')}")

    # Bit Manipulation
    print("\n7. BIT MANIPULATION")
    print("-" * 70)
    nums = [4, 1, 2, 1, 2]
    print(f"Single Number in {nums}: {single_number(nums)}")
    n = 11
    print(f"Number of 1 bits in {n}: {hamming_weight(n)}")
    n = 5
    print(f"Count bits 0 to {n}: {count_bits(n)}")

    print("\n" + "=" * 70)


if __name__ == "__main__":
    run_examples()
