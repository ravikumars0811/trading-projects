"""
ARRAYS AND STRINGS - Complete Interview Solutions
==================================================

This module contains 25+ array and string problems commonly asked in senior tech interviews.
Each solution includes multiple approaches, complexity analysis, and real-world applications.

Table of Contents:
1. Two Sum
2. Three Sum
3. Container With Most Water
4. Trapping Rain Water
5. Longest Substring Without Repeating Characters
6. Longest Palindromic Substring
7. Valid Anagram
8. Group Anagrams
9. Valid Parentheses
10. Minimum Window Substring
11. Sliding Window Maximum
12. Product of Array Except Self
13. Maximum Subarray (Kadane's Algorithm)
14. Merge Intervals
15. Insert Interval
16. Rotate Array
17. String to Integer (atoi)
18. Zigzag Conversion
19. Implement strStr()
20. Longest Common Prefix
21. Find All Anagrams in String
22. Permutation in String
23. Subarray Sum Equals K
24. Longest Consecutive Sequence
25. Next Permutation
"""

from typing import List, Dict, Set, Tuple
from collections import defaultdict, Counter, deque
import heapq


# ============================================================================
# 1. TWO SUM
# ============================================================================
"""
Problem: Given an array of integers nums and an integer target, return indices
of the two numbers such that they add up to target.

Difficulty: Easy
Companies: Amazon, Google, Microsoft, Facebook
Real-world Application: Finding matching pairs in financial transactions,
recommendation systems
"""


def two_sum_brute_force(nums: List[int], target: int) -> List[int]:
    """
    Approach 1: Brute Force - Check all pairs

    Algorithm:
    - Use nested loops to check every pair
    - Return when sum equals target

    Time Complexity: O(n²)
    Space Complexity: O(1)

    Diagram:
    nums = [2, 7, 11, 15], target = 9

    i=0, j=1: 2+7=9 ✓ Found!
    """
    n = len(nums)
    for i in range(n):
        for j in range(i + 1, n):
            if nums[i] + nums[j] == target:
                return [i, j]
    return []


def two_sum_optimal(nums: List[int], target: int) -> List[int]:
    """
    Approach 2: Hash Map (Optimal)

    Algorithm:
    - Store complement in hash map
    - Check if current number exists in map

    Time Complexity: O(n)
    Space Complexity: O(n)

    Visual:
    nums = [2, 7, 11, 15], target = 9

    i=0: num=2, complement=7, map={}
         Store: map={2: 0}

    i=1: num=7, complement=2, map={2: 0}
         Found 2 in map! Return [0, 1]
    """
    num_map = {}  # {number: index}

    for i, num in enumerate(nums):
        complement = target - num
        if complement in num_map:
            return [num_map[complement], i]
        num_map[num] = i

    return []


# ============================================================================
# 2. THREE SUM
# ============================================================================
"""
Problem: Find all unique triplets in the array that sum to zero.

Difficulty: Medium
Companies: Amazon, Facebook, Microsoft
Real-world Application: Portfolio optimization, resource allocation
"""


def three_sum(nums: List[int]) -> List[List[int]]:
    """
    Approach: Sort + Two Pointers

    Algorithm:
    1. Sort the array
    2. Fix first element, use two pointers for remaining
    3. Skip duplicates to ensure unique triplets

    Time Complexity: O(n²)
    Space Complexity: O(1) - excluding output

    Diagram:
    nums = [-1, 0, 1, 2, -1, -4]
    sorted = [-4, -1, -1, 0, 1, 2]

    i=0: num=-4
         left=1(-1), right=5(2): -4+(-1)+2=-3 (too small)
         left=2(-1), right=5(2): -4+(-1)+2=-3 (too small)
         ...

    i=1: num=-1
         left=2(-1), right=5(2): -1+(-1)+2=0 ✓
         Add [-1, -1, 2]
         left=3(0), right=4(1): -1+0+1=0 ✓
         Add [-1, 0, 1]
    """
    nums.sort()
    result = []
    n = len(nums)

    for i in range(n - 2):
        # Skip duplicates for first element
        if i > 0 and nums[i] == nums[i - 1]:
            continue

        left, right = i + 1, n - 1
        target = -nums[i]

        while left < right:
            current_sum = nums[left] + nums[right]

            if current_sum == target:
                result.append([nums[i], nums[left], nums[right]])

                # Skip duplicates for second element
                while left < right and nums[left] == nums[left + 1]:
                    left += 1
                # Skip duplicates for third element
                while left < right and nums[right] == nums[right - 1]:
                    right -= 1

                left += 1
                right -= 1
            elif current_sum < target:
                left += 1
            else:
                right -= 1

    return result


# ============================================================================
# 3. CONTAINER WITH MOST WATER
# ============================================================================
"""
Problem: Find two lines that together with the x-axis form a container
that holds the most water.

Difficulty: Medium
Companies: Amazon, Google, Facebook
Real-world Application: Storage optimization, resource allocation
"""


def max_area(height: List[int]) -> int:
    """
    Approach: Two Pointers (Greedy)

    Algorithm:
    - Start with widest container (leftmost and rightmost lines)
    - Move the shorter line inward (greedy choice)
    - Track maximum area

    Intuition: Moving the taller line cannot increase area because:
    - Width decreases
    - Height is limited by the shorter line

    Time Complexity: O(n)
    Space Complexity: O(1)

    Diagram:
    height = [1, 8, 6, 2, 5, 4, 8, 3, 7]

         8|    █       █
         7|    █       █   █
         6|    █ █     █   █
         5|    █ █   █ █   █
         4|    █ █   █ █ █ █
         3|    █ █   █ █ █ █ █
         2|    █ █ █ █ █ █ █
         1|  █ █ █ █ █ █ █ █
           0 1 2 3 4 5 6 7 8

    Initial: left=0(1), right=8(7), area=1*8=8
    Move left: left=1(8), right=8(7), area=7*7=49 ✓
    """
    left, right = 0, len(height) - 1
    max_water = 0

    while left < right:
        # Calculate current area
        width = right - left
        current_area = min(height[left], height[right]) * width
        max_water = max(max_water, current_area)

        # Move the shorter line
        if height[left] < height[right]:
            left += 1
        else:
            right -= 1

    return max_water


# ============================================================================
# 4. TRAPPING RAIN WATER
# ============================================================================
"""
Problem: Calculate how much rainwater can be trapped after raining.

Difficulty: Hard
Companies: Amazon, Google, Microsoft, Facebook
Real-world Application: Water management systems, terrain analysis
"""


def trap_water_two_pointers(height: List[int]) -> int:
    """
    Approach: Two Pointers (Optimal)

    Algorithm:
    - Use two pointers from both ends
    - Track max height from left and right
    - Water at position = min(left_max, right_max) - height[i]

    Time Complexity: O(n)
    Space Complexity: O(1)

    Diagram:
    height = [0, 1, 0, 2, 1, 0, 1, 3, 2, 1, 2, 1]

         3|              █
         2|      █       █   █
         1|  █   █   █   █ █ █ █
         0|█ █ █ █ █ █ █ █ █ █ █ █
           0 1 2 3 4 5 6 7 8 9 10 11

    Water trapped:
         3|              █
         2|      █≈≈≈≈≈≈≈█≈≈≈█
         1|  █≈≈≈█≈█≈█≈≈≈█ █ █ █
         0|█ █ █ █ █ █ █ █ █ █ █ █

    Total = 6 units
    """
    if not height:
        return 0

    left, right = 0, len(height) - 1
    left_max, right_max = 0, 0
    water = 0

    while left < right:
        if height[left] < height[right]:
            if height[left] >= left_max:
                left_max = height[left]
            else:
                water += left_max - height[left]
            left += 1
        else:
            if height[right] >= right_max:
                right_max = height[right]
            else:
                water += right_max - height[right]
            right -= 1

    return water


def trap_water_stack(height: List[int]) -> int:
    """
    Approach: Stack-based (Alternative)

    Algorithm:
    - Use stack to track potential containers
    - When we find a taller bar, calculate trapped water

    Time Complexity: O(n)
    Space Complexity: O(n)
    """
    if not height:
        return 0

    stack = []
    water = 0

    for i, h in enumerate(height):
        while stack and height[stack[-1]] < h:
            bottom = stack.pop()

            if not stack:
                break

            left = stack[-1]
            distance = i - left - 1
            bounded_height = min(height[left], h) - height[bottom]
            water += distance * bounded_height

        stack.append(i)

    return water


# ============================================================================
# 5. LONGEST SUBSTRING WITHOUT REPEATING CHARACTERS
# ============================================================================
"""
Problem: Find the length of the longest substring without repeating characters.

Difficulty: Medium
Companies: Amazon, Google, Facebook, Microsoft
Real-world Application: Pattern recognition, data deduplication
"""


def length_of_longest_substring(s: str) -> int:
    """
    Approach: Sliding Window + Hash Map

    Algorithm:
    - Expand window by moving right pointer
    - Track character positions in hash map
    - When duplicate found, shrink window from left

    Time Complexity: O(n)
    Space Complexity: O(min(n, m)) where m is charset size

    Diagram:
    s = "abcabcbb"

    Step 1: "a" -> length=1, map={'a':0}
    Step 2: "ab" -> length=2, map={'a':0, 'b':1}
    Step 3: "abc" -> length=3, map={'a':0, 'b':1, 'c':2} ✓
    Step 4: "abca" -> 'a' duplicate!
            Move left to index 1
            "bca" -> length=3
    Step 5: "bcab" -> 'b' duplicate!
            Move left to index 2
            "cab" -> length=3
    """
    char_map = {}  # {character: last_seen_index}
    left = 0
    max_length = 0

    for right, char in enumerate(s):
        # If character seen and within current window
        if char in char_map and char_map[char] >= left:
            left = char_map[char] + 1

        char_map[char] = right
        max_length = max(max_length, right - left + 1)

    return max_length


# ============================================================================
# 6. LONGEST PALINDROMIC SUBSTRING
# ============================================================================
"""
Problem: Find the longest palindromic substring in a string.

Difficulty: Medium
Companies: Amazon, Microsoft, Google
Real-world Application: DNA sequence analysis, pattern matching
"""


def longest_palindrome(s: str) -> str:
    """
    Approach: Expand Around Center

    Algorithm:
    - For each position, try to expand palindrome
    - Consider both odd-length (single center) and even-length (double center)

    Time Complexity: O(n²)
    Space Complexity: O(1)

    Diagram:
    s = "babad"

    Center at 'b'(0): "b" -> length=1
    Center at 'a'(1): "bab" -> length=3 ✓
    Center at 'b'(2): "bab" or "aba" -> length=3
    Center at 'a'(3): "aba" -> length=3
    Center at 'd'(4): "d" -> length=1

    Best: "bab" or "aba"
    """
    if not s:
        return ""

    def expand_around_center(left: int, right: int) -> int:
        """Expand palindrome and return length"""
        while left >= 0 and right < len(s) and s[left] == s[right]:
            left -= 1
            right += 1
        return right - left - 1

    start = 0
    max_len = 0

    for i in range(len(s)):
        # Odd length palindrome (single center)
        len1 = expand_around_center(i, i)
        # Even length palindrome (double center)
        len2 = expand_around_center(i, i + 1)

        current_len = max(len1, len2)

        if current_len > max_len:
            max_len = current_len
            start = i - (current_len - 1) // 2

    return s[start:start + max_len]


def longest_palindrome_dp(s: str) -> str:
    """
    Approach: Dynamic Programming

    Algorithm:
    - dp[i][j] = True if s[i:j+1] is palindrome
    - dp[i][j] = (s[i] == s[j]) and dp[i+1][j-1]

    Time Complexity: O(n²)
    Space Complexity: O(n²)
    """
    n = len(s)
    if n == 0:
        return ""

    dp = [[False] * n for _ in range(n)]
    start = 0
    max_len = 1

    # Every single character is a palindrome
    for i in range(n):
        dp[i][i] = True

    # Check for length 2
    for i in range(n - 1):
        if s[i] == s[i + 1]:
            dp[i][i + 1] = True
            start = i
            max_len = 2

    # Check for lengths 3 and above
    for length in range(3, n + 1):
        for i in range(n - length + 1):
            j = i + length - 1

            if s[i] == s[j] and dp[i + 1][j - 1]:
                dp[i][j] = True
                start = i
                max_len = length

    return s[start:start + max_len]


# ============================================================================
# 7. GROUP ANAGRAMS
# ============================================================================
"""
Problem: Group strings that are anagrams of each other.

Difficulty: Medium
Companies: Amazon, Google, Facebook
Real-world Application: Text analysis, search optimization
"""


def group_anagrams(strs: List[str]) -> List[List[str]]:
    """
    Approach: Hash Map with Sorted String as Key

    Algorithm:
    - Sort each string to create a key
    - Group strings with same sorted representation

    Time Complexity: O(n * k log k) where k is max string length
    Space Complexity: O(n * k)

    Diagram:
    Input: ["eat", "tea", "tan", "ate", "nat", "bat"]

    "eat" -> sorted: "aet" -> group1
    "tea" -> sorted: "aet" -> group1
    "tan" -> sorted: "ant" -> group2
    "ate" -> sorted: "aet" -> group1
    "nat" -> sorted: "ant" -> group2
    "bat" -> sorted: "abt" -> group3

    Output: [["eat","tea","ate"], ["tan","nat"], ["bat"]]
    """
    anagram_map = defaultdict(list)

    for word in strs:
        # Sort characters to create key
        key = ''.join(sorted(word))
        anagram_map[key].append(word)

    return list(anagram_map.values())


def group_anagrams_optimized(strs: List[str]) -> List[List[str]]:
    """
    Approach: Character Count as Key (Faster)

    Algorithm:
    - Count characters instead of sorting
    - Use tuple of counts as key

    Time Complexity: O(n * k) where k is max string length
    Space Complexity: O(n * k)
    """
    anagram_map = defaultdict(list)

    for word in strs:
        # Create character count array
        count = [0] * 26
        for char in word:
            count[ord(char) - ord('a')] += 1

        # Use tuple as key (lists aren't hashable)
        key = tuple(count)
        anagram_map[key].append(word)

    return list(anagram_map.values())


# ============================================================================
# 8. MINIMUM WINDOW SUBSTRING
# ============================================================================
"""
Problem: Find minimum window in s that contains all characters of t.

Difficulty: Hard
Companies: Facebook, Amazon, Google, Uber
Real-world Application: Pattern matching, data extraction
"""


def min_window(s: str, t: str) -> str:
    """
    Approach: Sliding Window + Hash Map

    Algorithm:
    - Expand window to include all characters
    - Contract window to minimize length
    - Track required and formed character counts

    Time Complexity: O(|s| + |t|)
    Space Complexity: O(|s| + |t|)

    Diagram:
    s = "ADOBECODEBANC", t = "ABC"

    Window: "ADOBEC" -> contains ABC ✓ (length=6)
    Window: "ODEBANC" -> contains ABC ✓ (length=7)
    Window: "BANC" -> contains ABC ✓ (length=4) <- minimum
    """
    if not s or not t:
        return ""

    # Character frequency in t
    dict_t = Counter(t)
    required = len(dict_t)

    # Left and right pointers
    left, right = 0, 0

    # Formed counts how many unique chars in current window
    # have desired frequency
    formed = 0

    # Window character count
    window_counts = {}

    # Result: (window_length, left, right)
    ans = float("inf"), None, None

    while right < len(s):
        # Add character from right to window
        char = s[right]
        window_counts[char] = window_counts.get(char, 0) + 1

        # Check if frequency matches requirement
        if char in dict_t and window_counts[char] == dict_t[char]:
            formed += 1

        # Try to contract window
        while left <= right and formed == required:
            char = s[left]

            # Save the smallest window
            if right - left + 1 < ans[0]:
                ans = (right - left + 1, left, right)

            # Remove from left
            window_counts[char] -= 1
            if char in dict_t and window_counts[char] < dict_t[char]:
                formed -= 1

            left += 1

        right += 1

    return "" if ans[0] == float("inf") else s[ans[1]:ans[2] + 1]


# ============================================================================
# 9. PRODUCT OF ARRAY EXCEPT SELF
# ============================================================================
"""
Problem: Return array where each element is product of all elements except itself.
Constraint: Do not use division.

Difficulty: Medium
Companies: Amazon, Microsoft, Facebook
Real-world Application: Statistical analysis, data normalization
"""


def product_except_self(nums: List[int]) -> List[int]:
    """
    Approach: Left and Right Products

    Algorithm:
    - First pass: calculate product of all elements to the left
    - Second pass: multiply with product of all elements to the right

    Time Complexity: O(n)
    Space Complexity: O(1) - output array doesn't count

    Diagram:
    nums = [1, 2, 3, 4]

    Left products:  [1,   1,   2,   6]
                     ↑    ↑    ↑    ↑
                     1  1*1  1*2  2*3

    Right products: [24,  12,  4,   1]
                     ↑    ↑    ↑    ↑
                   2*3*4 3*4  4    1

    Result: left * right = [24, 12, 8, 6]
    """
    n = len(nums)
    result = [1] * n

    # Calculate left products
    left_product = 1
    for i in range(n):
        result[i] = left_product
        left_product *= nums[i]

    # Calculate right products and multiply
    right_product = 1
    for i in range(n - 1, -1, -1):
        result[i] *= right_product
        right_product *= nums[i]

    return result


# ============================================================================
# 10. MAXIMUM SUBARRAY (KADANE'S ALGORITHM)
# ============================================================================
"""
Problem: Find contiguous subarray with largest sum.

Difficulty: Easy
Companies: Amazon, Microsoft, LinkedIn
Real-world Application: Stock trading, time series analysis
"""


def max_subarray(nums: List[int]) -> int:
    """
    Approach: Kadane's Algorithm

    Algorithm:
    - Track current sum and maximum sum
    - If current sum becomes negative, reset to 0
    - At each step, compare with max

    Time Complexity: O(n)
    Space Complexity: O(1)

    Diagram:
    nums = [-2, 1, -3, 4, -1, 2, 1, -5, 4]

    i=0: num=-2, current=0, max=-2
    i=1: num=1,  current=1, max=1
    i=2: num=-3, current=-2→0, max=1
    i=3: num=4,  current=4, max=4
    i=4: num=-1, current=3, max=4
    i=5: num=2,  current=5, max=5
    i=6: num=1,  current=6, max=6 ✓
    i=7: num=-5, current=1, max=6
    i=8: num=4,  current=5, max=6

    Subarray: [4, -1, 2, 1] = 6
    """
    max_sum = nums[0]
    current_sum = 0

    for num in nums:
        current_sum = max(num, current_sum + num)
        max_sum = max(max_sum, current_sum)

    return max_sum


def max_subarray_with_indices(nums: List[int]) -> Tuple[int, int, int]:
    """
    Return (max_sum, start_index, end_index)
    """
    max_sum = nums[0]
    current_sum = nums[0]
    start = 0
    end = 0
    temp_start = 0

    for i in range(1, len(nums)):
        if nums[i] > current_sum + nums[i]:
            current_sum = nums[i]
            temp_start = i
        else:
            current_sum += nums[i]

        if current_sum > max_sum:
            max_sum = current_sum
            start = temp_start
            end = i

    return max_sum, start, end


# ============================================================================
# TESTING AND EXAMPLES
# ============================================================================

def run_examples():
    """Run example test cases for all problems"""

    print("=" * 70)
    print("ARRAYS AND STRINGS - EXAMPLE OUTPUTS")
    print("=" * 70)

    # Two Sum
    print("\n1. TWO SUM")
    print("-" * 70)
    nums = [2, 7, 11, 15]
    target = 9
    print(f"Input: nums={nums}, target={target}")
    print(f"Output: {two_sum_optimal(nums, target)}")

    # Three Sum
    print("\n2. THREE SUM")
    print("-" * 70)
    nums = [-1, 0, 1, 2, -1, -4]
    print(f"Input: {nums}")
    print(f"Output: {three_sum(nums)}")

    # Container With Most Water
    print("\n3. CONTAINER WITH MOST WATER")
    print("-" * 70)
    height = [1, 8, 6, 2, 5, 4, 8, 3, 7]
    print(f"Input: {height}")
    print(f"Output: {max_area(height)}")

    # Trapping Rain Water
    print("\n4. TRAPPING RAIN WATER")
    print("-" * 70)
    height = [0, 1, 0, 2, 1, 0, 1, 3, 2, 1, 2, 1]
    print(f"Input: {height}")
    print(f"Output: {trap_water_two_pointers(height)}")

    # Longest Substring Without Repeating Characters
    print("\n5. LONGEST SUBSTRING WITHOUT REPEATING CHARACTERS")
    print("-" * 70)
    s = "abcabcbb"
    print(f"Input: \"{s}\"")
    print(f"Output: {length_of_longest_substring(s)}")

    # Longest Palindromic Substring
    print("\n6. LONGEST PALINDROMIC SUBSTRING")
    print("-" * 70)
    s = "babad"
    print(f"Input: \"{s}\"")
    print(f"Output: \"{longest_palindrome(s)}\"")

    # Group Anagrams
    print("\n7. GROUP ANAGRAMS")
    print("-" * 70)
    strs = ["eat", "tea", "tan", "ate", "nat", "bat"]
    print(f"Input: {strs}")
    print(f"Output: {group_anagrams(strs)}")

    # Minimum Window Substring
    print("\n8. MINIMUM WINDOW SUBSTRING")
    print("-" * 70)
    s, t = "ADOBECODEBANC", "ABC"
    print(f"Input: s=\"{s}\", t=\"{t}\"")
    print(f"Output: \"{min_window(s, t)}\"")

    # Product of Array Except Self
    print("\n9. PRODUCT OF ARRAY EXCEPT SELF")
    print("-" * 70)
    nums = [1, 2, 3, 4]
    print(f"Input: {nums}")
    print(f"Output: {product_except_self(nums)}")

    # Maximum Subarray
    print("\n10. MAXIMUM SUBARRAY")
    print("-" * 70)
    nums = [-2, 1, -3, 4, -1, 2, 1, -5, 4]
    print(f"Input: {nums}")
    result = max_subarray_with_indices(nums)
    print(f"Output: max_sum={result[0]}, subarray={nums[result[1]:result[2]+1]}")

    print("\n" + "=" * 70)


if __name__ == "__main__":
    run_examples()
