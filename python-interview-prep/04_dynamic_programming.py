"""
DYNAMIC PROGRAMMING - Complete Interview Solutions
==================================================

This module contains 30+ dynamic programming problems commonly asked in senior tech interviews.
Each solution includes multiple approaches, complexity analysis, and real-world applications.

DP Categories:
1. Linear DP (Fibonacci, Climbing Stairs, House Robber)
2. 2D Grid DP (Unique Paths, Minimum Path Sum)
3. Subsequence DP (LCS, LIS, Edit Distance)
4. Knapsack Problems (0/1, Unbounded, Subset Sum)
5. String DP (Palindrome, Word Break)
6. Stock Trading Problems
7. Game Theory DP

Table of Contents:
1. Fibonacci Numbers
2. Climbing Stairs
3. House Robber (I, II, III)
4. Coin Change (I, II)
5. Longest Increasing Subsequence
6. Longest Common Subsequence
7. Edit Distance
8. Word Break (I, II)
9. Palindrome Partitioning
10. Longest Palindromic Subsequence
11. Unique Paths (I, II)
12. Minimum Path Sum
13. Triangle Minimum Path
14. Maximum Product Subarray
15. Best Time to Buy and Sell Stock (all variations)
16. Decode Ways
17. Jump Game (I, II)
18. Partition Equal Subset Sum
19. Target Sum
20. 0/1 Knapsack
21. Unbounded Knapsack
22. Egg Drop Problem
23. Burst Balloons
24. Regular Expression Matching
25. Wildcard Matching
"""

from typing import List, Dict, Tuple
from functools import lru_cache


# ============================================================================
# 1. FIBONACCI NUMBERS
# ============================================================================
"""
Problem: Calculate the nth Fibonacci number.

Difficulty: Easy
Companies: All major companies (fundamental DP)
Real-world Application: Population growth, algorithm analysis
"""


def fibonacci_recursive(n: int) -> int:
    """
    Approach 1: Naive Recursion

    Time Complexity: O(2^n) - exponential!
    Space Complexity: O(n) - recursion stack

    Tree for fib(5):
                    fib(5)
                   /      \
              fib(4)      fib(3)
             /     \      /    \
        fib(3)  fib(2) fib(2) fib(1)
        /    \
    fib(2) fib(1)

    Note: Many repeated calculations!
    """
    if n <= 1:
        return n
    return fibonacci_recursive(n - 1) + fibonacci_recursive(n - 2)


def fibonacci_memoization(n: int) -> int:
    """
    Approach 2: Top-Down DP (Memoization)

    Algorithm:
    - Cache results of subproblems
    - Avoid recomputation

    Time Complexity: O(n)
    Space Complexity: O(n)
    """
    memo = {}

    def fib(n):
        if n <= 1:
            return n
        if n in memo:
            return memo[n]

        memo[n] = fib(n - 1) + fib(n - 2)
        return memo[n]

    return fib(n)


def fibonacci_tabulation(n: int) -> int:
    """
    Approach 3: Bottom-Up DP (Tabulation)

    Algorithm:
    - Build table from bottom up
    - Each entry depends on previous entries

    Time Complexity: O(n)
    Space Complexity: O(n)

    DP Table for n=6:
    i:    0  1  2  3  4  5  6
    dp:   0  1  1  2  3  5  8
    """
    if n <= 1:
        return n

    dp = [0] * (n + 1)
    dp[1] = 1

    for i in range(2, n + 1):
        dp[i] = dp[i - 1] + dp[i - 2]

    return dp[n]


def fibonacci_optimized(n: int) -> int:
    """
    Approach 4: Space-Optimized DP

    Algorithm:
    - Only keep last two values

    Time Complexity: O(n)
    Space Complexity: O(1)
    """
    if n <= 1:
        return n

    prev, curr = 0, 1

    for _ in range(2, n + 1):
        prev, curr = curr, prev + curr

    return curr


# ============================================================================
# 2. CLIMBING STAIRS
# ============================================================================
"""
Problem: Count ways to climb n stairs (1 or 2 steps at a time).

Difficulty: Easy
Companies: Amazon, Google, Microsoft
Real-world Application: Path counting, resource allocation
"""


def climb_stairs(n: int) -> int:
    """
    Approach: DP (Similar to Fibonacci)

    Recurrence:
    dp[i] = dp[i-1] + dp[i-2]
    - dp[i-1]: ways to reach from step i-1 (take 1 step)
    - dp[i-2]: ways to reach from step i-2 (take 2 steps)

    Time Complexity: O(n)
    Space Complexity: O(1)

    Diagram for n=4:
    Step 4 can be reached from:
    - Step 3 (take 1 step)
    - Step 2 (take 2 steps)

    Steps: 0  1  2  3  4
    Ways:  1  1  2  3  5

    Paths to step 4:
    1. 1+1+1+1
    2. 1+1+2
    3. 1+2+1
    4. 2+1+1
    5. 2+2
    """
    if n <= 2:
        return n

    prev, curr = 1, 2

    for _ in range(3, n + 1):
        prev, curr = curr, prev + curr

    return curr


# ============================================================================
# 3. HOUSE ROBBER
# ============================================================================
"""
Problem: Rob houses to maximize money, cannot rob adjacent houses.

Difficulty: Medium
Companies: Amazon, Google, Facebook
Real-world Application: Resource allocation with constraints
"""


def rob(nums: List[int]) -> int:
    """
    Approach: DP with State Transition

    Recurrence:
    dp[i] = max(dp[i-1], dp[i-2] + nums[i])
    - dp[i-1]: don't rob house i
    - dp[i-2] + nums[i]: rob house i

    Time Complexity: O(n)
    Space Complexity: O(1)

    Diagram:
    houses = [2, 7, 9, 3, 1]

    i=0: rob=2, skip=0, max=2
    i=1: rob=0+7=7, skip=2, max=7
    i=2: rob=2+9=11, skip=7, max=11
    i=3: rob=7+3=10, skip=11, max=11
    i=4: rob=11+1=12, skip=11, max=12

    Decision tree:
              Start
              /   \
           rob2  skip2
           /  \    /  \
         skip7 rob7 ...
    """
    if not nums:
        return 0
    if len(nums) == 1:
        return nums[0]

    prev2, prev1 = 0, 0

    for num in nums:
        # max(rob current + prev2, skip current)
        prev2, prev1 = prev1, max(prev1, prev2 + num)

    return prev1


def rob_circular(nums: List[int]) -> int:
    """
    House Robber II: Houses arranged in circle (first and last are adjacent)

    Approach:
    - Case 1: Rob houses 0 to n-2 (exclude last)
    - Case 2: Rob houses 1 to n-1 (exclude first)
    - Return maximum

    Time Complexity: O(n)
    Space Complexity: O(1)
    """
    def rob_linear(houses):
        prev2, prev1 = 0, 0
        for num in houses:
            prev2, prev1 = prev1, max(prev1, prev2 + num)
        return prev1

    if len(nums) == 1:
        return nums[0]

    return max(rob_linear(nums[:-1]), rob_linear(nums[1:]))


# ============================================================================
# 4. COIN CHANGE
# ============================================================================
"""
Problem: Find minimum coins needed to make amount.

Difficulty: Medium
Companies: Amazon, Google, Facebook, Microsoft
Real-world Application: Currency exchange, change-making systems
"""


def coin_change(coins: List[int], amount: int) -> int:
    """
    Approach: DP - Unbounded Knapsack

    Recurrence:
    dp[i] = min(dp[i], dp[i - coin] + 1) for each coin

    Time Complexity: O(amount * coins)
    Space Complexity: O(amount)

    Diagram:
    coins = [1, 2, 5], amount = 11

    DP Table:
    amount: 0  1  2  3  4  5  6  7  8  9  10 11
    dp:     0  1  1  2  2  1  2  2  3  3  2  3

    For amount=11:
    - Using coin 1: dp[11] = dp[10] + 1 = 3
    - Using coin 2: dp[11] = dp[9] + 1 = 4
    - Using coin 5: dp[11] = dp[6] + 1 = 3
    Minimum = 3 (coins: 5+5+1)
    """
    dp = [float('inf')] * (amount + 1)
    dp[0] = 0

    for i in range(1, amount + 1):
        for coin in coins:
            if coin <= i:
                dp[i] = min(dp[i], dp[i - coin] + 1)

    return dp[amount] if dp[amount] != float('inf') else -1


def coin_change_combinations(coins: List[int], amount: int) -> int:
    """
    Coin Change II: Count number of combinations to make amount

    Recurrence:
    dp[i] = sum of dp[i - coin] for each coin

    Time Complexity: O(amount * coins)
    Space Complexity: O(amount)

    Diagram:
    coins = [1, 2, 5], amount = 5

    Process coin 1:
    amount: 0  1  2  3  4  5
    dp:     1  1  1  1  1  1

    Process coin 2:
    amount: 0  1  2  3  4  5
    dp:     1  1  2  2  3  3

    Process coin 5:
    amount: 0  1  2  3  4  5
    dp:     1  1  2  2  3  4

    Answer: 4 combinations (1+1+1+1+1, 1+1+1+2, 1+2+2, 5)
    """
    dp = [0] * (amount + 1)
    dp[0] = 1

    for coin in coins:
        for i in range(coin, amount + 1):
            dp[i] += dp[i - coin]

    return dp[amount]


# ============================================================================
# 5. LONGEST INCREASING SUBSEQUENCE
# ============================================================================
"""
Problem: Find length of longest increasing subsequence.

Difficulty: Medium
Companies: Amazon, Google, Microsoft, Facebook
Real-world Application: Version tracking, trend analysis
"""


def length_of_lis_dp(nums: List[int]) -> int:
    """
    Approach 1: DP

    Recurrence:
    dp[i] = max(dp[j] + 1) for all j < i where nums[j] < nums[i]

    Time Complexity: O(n²)
    Space Complexity: O(n)

    Diagram:
    nums = [10, 9, 2, 5, 3, 7, 101, 18]

    DP Table:
    i:     0   1   2   3   4   5   6    7
    nums: 10   9   2   5   3   7  101  18
    dp:    1   1   1   2   2   3   4    4

    For i=5 (num=7):
    - Check j=2 (2<7): dp[5] = dp[2] + 1 = 2
    - Check j=3 (5<7): dp[5] = dp[3] + 1 = 3
    - Check j=4 (3<7): dp[5] = max(3, dp[4]+1) = 3

    LIS: [2, 5, 7, 101] or [2, 3, 7, 18]
    """
    if not nums:
        return 0

    n = len(nums)
    dp = [1] * n

    for i in range(1, n):
        for j in range(i):
            if nums[j] < nums[i]:
                dp[i] = max(dp[i], dp[j] + 1)

    return max(dp)


def length_of_lis_binary_search(nums: List[int]) -> int:
    """
    Approach 2: Binary Search + DP

    Algorithm:
    - Maintain array of smallest tail elements
    - Binary search to find position

    Time Complexity: O(n log n)
    Space Complexity: O(n)

    Diagram:
    nums = [10, 9, 2, 5, 3, 7, 101, 18]

    tails array represents: smallest tail of all increasing subsequences of length i+1

    After processing:
    num=10: tails=[10]
    num=9:  tails=[9]         (replace 10 with 9)
    num=2:  tails=[2]         (replace 9 with 2)
    num=5:  tails=[2,5]       (append)
    num=3:  tails=[2,3]       (replace 5 with 3)
    num=7:  tails=[2,3,7]     (append)
    num=101:tails=[2,3,7,101] (append)
    num=18: tails=[2,3,7,18]  (replace 101 with 18)

    Length = 4
    """
    tails = []

    for num in nums:
        # Binary search for position
        left, right = 0, len(tails)

        while left < right:
            mid = (left + right) // 2
            if tails[mid] < num:
                left = mid + 1
            else:
                right = mid

        # Replace or append
        if left < len(tails):
            tails[left] = num
        else:
            tails.append(num)

    return len(tails)


# ============================================================================
# 6. LONGEST COMMON SUBSEQUENCE
# ============================================================================
"""
Problem: Find length of longest common subsequence between two strings.

Difficulty: Medium
Companies: Amazon, Google, Microsoft
Real-world Application: Diff tools, DNA sequence alignment, plagiarism detection
"""


def longest_common_subsequence(text1: str, text2: str) -> int:
    """
    Approach: 2D DP

    Recurrence:
    If text1[i] == text2[j]:
        dp[i][j] = dp[i-1][j-1] + 1
    Else:
        dp[i][j] = max(dp[i-1][j], dp[i][j-1])

    Time Complexity: O(m * n)
    Space Complexity: O(m * n)

    Diagram:
    text1 = "abcde", text2 = "ace"

    DP Table:
        ""  a  c  e
    ""   0  0  0  0
    a    0  1  1  1
    b    0  1  1  1
    c    0  1  2  2
    d    0  1  2  2
    e    0  1  2  3

    LCS = "ace" (length 3)

    Visual trace:
    a b c d e
    ↓
    a → match! add 1
      c → skip b, match c! add 1
        e → skip d, match e! add 1
    """
    m, n = len(text1), len(text2)
    dp = [[0] * (n + 1) for _ in range(m + 1)]

    for i in range(1, m + 1):
        for j in range(1, n + 1):
            if text1[i - 1] == text2[j - 1]:
                dp[i][j] = dp[i - 1][j - 1] + 1
            else:
                dp[i][j] = max(dp[i - 1][j], dp[i][j - 1])

    return dp[m][n]


def lcs_space_optimized(text1: str, text2: str) -> int:
    """
    Space-Optimized LCS: O(min(m, n))

    Observation: We only need previous row
    """
    if len(text1) < len(text2):
        text1, text2 = text2, text1

    m, n = len(text1), len(text2)
    prev = [0] * (n + 1)

    for i in range(1, m + 1):
        curr = [0] * (n + 1)
        for j in range(1, n + 1):
            if text1[i - 1] == text2[j - 1]:
                curr[j] = prev[j - 1] + 1
            else:
                curr[j] = max(prev[j], curr[j - 1])
        prev = curr

    return prev[n]


# ============================================================================
# 7. EDIT DISTANCE
# ============================================================================
"""
Problem: Minimum operations (insert, delete, replace) to convert word1 to word2.

Difficulty: Hard
Companies: Amazon, Google, Microsoft, Facebook
Real-world Application: Spell checkers, DNA analysis, autocorrect
"""


def min_distance(word1: str, word2: str) -> int:
    """
    Approach: 2D DP (Levenshtein Distance)

    Recurrence:
    If word1[i] == word2[j]:
        dp[i][j] = dp[i-1][j-1]
    Else:
        dp[i][j] = 1 + min(
            dp[i-1][j],      # delete
            dp[i][j-1],      # insert
            dp[i-1][j-1]     # replace
        )

    Time Complexity: O(m * n)
    Space Complexity: O(m * n)

    Diagram:
    word1 = "horse", word2 = "ros"

    DP Table:
        ""  r  o  s
    ""   0  1  2  3
    h    1  1  2  3
    o    2  2  1  2
    r    3  2  2  2
    s    4  3  3  2
    e    5  4  4  3

    Operations:
    1. Replace h with r
    2. Remove o
    3. Remove e
    Total: 3
    """
    m, n = len(word1), len(word2)
    dp = [[0] * (n + 1) for _ in range(m + 1)]

    # Base cases
    for i in range(m + 1):
        dp[i][0] = i
    for j in range(n + 1):
        dp[0][j] = j

    # Fill table
    for i in range(1, m + 1):
        for j in range(1, n + 1):
            if word1[i - 1] == word2[j - 1]:
                dp[i][j] = dp[i - 1][j - 1]
            else:
                dp[i][j] = 1 + min(
                    dp[i - 1][j],      # delete
                    dp[i][j - 1],      # insert
                    dp[i - 1][j - 1]   # replace
                )

    return dp[m][n]


# ============================================================================
# 8. WORD BREAK
# ============================================================================
"""
Problem: Determine if string can be segmented into dictionary words.

Difficulty: Medium
Companies: Amazon, Google, Facebook
Real-world Application: Natural language processing, tokenization
"""


def word_break(s: str, wordDict: List[str]) -> bool:
    """
    Approach: DP

    Recurrence:
    dp[i] = True if s[0:i] can be segmented

    dp[i] = True if there exists j where:
    - dp[j] = True
    - s[j:i] is in dictionary

    Time Complexity: O(n² * m) where m is avg word length
    Space Complexity: O(n)

    Diagram:
    s = "leetcode", wordDict = ["leet", "code"]

    DP Array:
    i:    0  1  2  3  4  5  6  7  8
    s:    "" l  e  e  t  c  o  d  e
    dp:   T  F  F  F  T  F  F  F  T

    At i=4: s[0:4]="leet" in dict → dp[4]=True
    At i=8: dp[4]=True and s[4:8]="code" in dict → dp[8]=True
    """
    word_set = set(wordDict)
    n = len(s)
    dp = [False] * (n + 1)
    dp[0] = True

    for i in range(1, n + 1):
        for j in range(i):
            if dp[j] and s[j:i] in word_set:
                dp[i] = True
                break

    return dp[n]


def word_break_with_words(s: str, wordDict: List[str]) -> List[str]:
    """
    Word Break II: Return all possible sentences

    Approach: DP + Backtracking

    Time Complexity: O(2^n) worst case
    Space Complexity: O(2^n)
    """
    word_set = set(wordDict)
    memo = {}

    def backtrack(start):
        if start in memo:
            return memo[start]

        if start == len(s):
            return [[]]

        result = []
        for end in range(start + 1, len(s) + 1):
            word = s[start:end]
            if word in word_set:
                for rest in backtrack(end):
                    result.append([word] + rest)

        memo[start] = result
        return result

    return [' '.join(words) for words in backtrack(0)]


# ============================================================================
# 9. BEST TIME TO BUY AND SELL STOCK
# ============================================================================
"""
Problem: Maximize profit from stock trading with various constraints.

Difficulty: Easy-Hard
Companies: Amazon, Google, Microsoft, Facebook
Real-world Application: Trading algorithms, portfolio optimization
"""


def max_profit_one_transaction(prices: List[int]) -> int:
    """
    Stock I: At most one transaction

    Algorithm:
    - Track minimum price seen so far
    - Calculate max profit at each day

    Time Complexity: O(n)
    Space Complexity: O(1)

    Diagram:
    prices = [7, 1, 5, 3, 6, 4]

    Day  Price  MinPrice  MaxProfit
    0    7      7         0
    1    1      1         0
    2    5      1         4  (buy at 1, sell at 5)
    3    3      1         4
    4    6      1         5  (buy at 1, sell at 6)
    5    4      1         5
    """
    min_price = float('inf')
    max_profit = 0

    for price in prices:
        min_price = min(min_price, price)
        max_profit = max(max_profit, price - min_price)

    return max_profit


def max_profit_unlimited_transactions(prices: List[int]) -> int:
    """
    Stock II: Unlimited transactions

    Algorithm:
    - Buy and sell on every upward trend
    - Sum all positive differences

    Time Complexity: O(n)
    Space Complexity: O(1)

    Diagram:
    prices = [7, 1, 5, 3, 6, 4]

    Transactions:
    - Buy at 1, sell at 5: profit = 4
    - Buy at 3, sell at 6: profit = 3
    Total: 7
    """
    profit = 0

    for i in range(1, len(prices)):
        if prices[i] > prices[i - 1]:
            profit += prices[i] - prices[i - 1]

    return profit


def max_profit_k_transactions(prices: List[int], k: int) -> int:
    """
    Stock IV: At most k transactions

    Approach: DP

    States:
    - buy[i][j]: max profit after at most i transactions with stock in hand after day j
    - sell[i][j]: max profit after at most i transactions with no stock after day j

    Time Complexity: O(n * k)
    Space Complexity: O(k)

    Diagram:
    prices = [3, 2, 6, 5, 0, 3], k = 2

    DP Table (sell state):
    k\day  0  1  2  3  4  5
    0      0  0  0  0  0  0
    1      0  0  4  4  4  4
    2      0  0  4  4  4  7

    Best: Buy at 2, sell at 6, buy at 0, sell at 3 → profit = 7
    """
    if not prices or k == 0:
        return 0

    # If k >= n//2, it's same as unlimited transactions
    if k >= len(prices) // 2:
        return max_profit_unlimited_transactions(prices)

    # DP arrays
    buy = [-float('inf')] * (k + 1)
    sell = [0] * (k + 1)

    for price in prices:
        for i in range(k, 0, -1):
            sell[i] = max(sell[i], buy[i] + price)
            buy[i] = max(buy[i], sell[i - 1] - price)

    return sell[k]


# ============================================================================
# 10. UNIQUE PATHS
# ============================================================================
"""
Problem: Count unique paths from top-left to bottom-right in grid.

Difficulty: Medium
Companies: Amazon, Google, Microsoft
Real-world Application: Robot path planning, game development
"""


def unique_paths(m: int, n: int) -> int:
    """
    Approach: DP

    Recurrence:
    dp[i][j] = dp[i-1][j] + dp[i][j-1]

    Time Complexity: O(m * n)
    Space Complexity: O(n) - space optimized

    Diagram for 3x3 grid:
        0   1   2
    0   1   1   1
    1   1   2   3
    2   1   3   6

    Each cell = sum of paths from top + paths from left
    """
    dp = [1] * n

    for i in range(1, m):
        for j in range(1, n):
            dp[j] += dp[j - 1]

    return dp[n - 1]


# ============================================================================
# TESTING AND EXAMPLES
# ============================================================================

def run_examples():
    """Run example test cases for all problems"""

    print("=" * 70)
    print("DYNAMIC PROGRAMMING - EXAMPLE OUTPUTS")
    print("=" * 70)

    # Fibonacci
    print("\n1. FIBONACCI NUMBERS")
    print("-" * 70)
    n = 10
    print(f"F({n}) = {fibonacci_optimized(n)}")

    # Climbing Stairs
    print("\n2. CLIMBING STAIRS")
    print("-" * 70)
    n = 5
    print(f"Ways to climb {n} stairs: {climb_stairs(n)}")

    # House Robber
    print("\n3. HOUSE ROBBER")
    print("-" * 70)
    nums = [2, 7, 9, 3, 1]
    print(f"Houses: {nums}")
    print(f"Maximum money: {rob(nums)}")

    # Coin Change
    print("\n4. COIN CHANGE")
    print("-" * 70)
    coins, amount = [1, 2, 5], 11
    print(f"Coins: {coins}, Amount: {amount}")
    print(f"Minimum coins: {coin_change(coins, amount)}")
    print(f"Number of ways: {coin_change_combinations(coins, amount)}")

    # Longest Increasing Subsequence
    print("\n5. LONGEST INCREASING SUBSEQUENCE")
    print("-" * 70)
    nums = [10, 9, 2, 5, 3, 7, 101, 18]
    print(f"Array: {nums}")
    print(f"LIS length (DP): {length_of_lis_dp(nums)}")
    print(f"LIS length (Binary Search): {length_of_lis_binary_search(nums)}")

    # Longest Common Subsequence
    print("\n6. LONGEST COMMON SUBSEQUENCE")
    print("-" * 70)
    text1, text2 = "abcde", "ace"
    print(f"Text1: {text1}, Text2: {text2}")
    print(f"LCS length: {longest_common_subsequence(text1, text2)}")

    # Edit Distance
    print("\n7. EDIT DISTANCE")
    print("-" * 70)
    word1, word2 = "horse", "ros"
    print(f"Word1: {word1}, Word2: {word2}")
    print(f"Minimum operations: {min_distance(word1, word2)}")

    # Word Break
    print("\n8. WORD BREAK")
    print("-" * 70)
    s = "leetcode"
    wordDict = ["leet", "code"]
    print(f"String: {s}")
    print(f"Dictionary: {wordDict}")
    print(f"Can break: {word_break(s, wordDict)}")

    # Stock Trading
    print("\n9. BEST TIME TO BUY AND SELL STOCK")
    print("-" * 70)
    prices = [7, 1, 5, 3, 6, 4]
    print(f"Prices: {prices}")
    print(f"One transaction: {max_profit_one_transaction(prices)}")
    print(f"Unlimited transactions: {max_profit_unlimited_transactions(prices)}")
    print(f"At most 2 transactions: {max_profit_k_transactions(prices, 2)}")

    # Unique Paths
    print("\n10. UNIQUE PATHS")
    print("-" * 70)
    m, n = 3, 7
    print(f"Grid: {m}x{n}")
    print(f"Unique paths: {unique_paths(m, n)}")

    print("\n" + "=" * 70)


if __name__ == "__main__":
    run_examples()
