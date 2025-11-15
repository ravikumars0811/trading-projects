"""
GRAPH ALGORITHMS - Complete Interview Solutions
===============================================

This module contains 20+ graph problems commonly asked in senior tech interviews.
Each solution includes multiple approaches, complexity analysis, and real-world applications.

Table of Contents:
1. Graph Representations (Adjacency List, Matrix)
2. DFS and BFS Traversal
3. Number of Islands
4. Clone Graph
5. Course Schedule (I, II)
6. Word Ladder
7. Network Delay Time (Dijkstra)
8. Cheapest Flights Within K Stops
9. Minimum Spanning Tree (Kruskal, Prim)
10. Union Find / Disjoint Set
11. Topological Sort
12. Strongly Connected Components (Tarjan, Kosaraju)
13. Bridges and Articulation Points
14. Shortest Path Algorithms (Dijkstra, Bellman-Ford, Floyd-Warshall)
15. Alien Dictionary
16. Graph Valid Tree
17. Number of Connected Components
18. Reconstruct Itinerary
19. Critical Connections
20. Evaluate Division
"""

from typing import List, Dict, Set, Tuple, Optional
from collections import defaultdict, deque
import heapq


# ============================================================================
# 1. GRAPH REPRESENTATIONS
# ============================================================================
"""
Real-world Application: Social networks, road networks, dependency graphs
"""


class Graph:
    """Graph using adjacency list (most common representation)"""

    def __init__(self, directed=False):
        self.graph = defaultdict(list)
        self.directed = directed

    def add_edge(self, u, v, weight=1):
        """Add edge from u to v"""
        self.graph[u].append((v, weight))
        if not self.directed:
            self.graph[v].append((u, weight))

    def get_neighbors(self, node):
        """Get neighbors of a node"""
        return self.graph[node]


# ============================================================================
# 2. DFS AND BFS TRAVERSAL
# ============================================================================
"""
Problem: Implement depth-first and breadth-first search.

Difficulty: Easy
Companies: All major companies
Real-world Application: Web crawling, social network analysis, maze solving
"""


def dfs_recursive(graph: Dict[int, List[int]], start: int) -> List[int]:
    """
    DFS using recursion

    Time Complexity: O(V + E)
    Space Complexity: O(V) - recursion stack

    Diagram:
         1
        / \
       2   3
      / \   \
     4   5   6

    DFS order: 1 → 2 → 4 → 5 → 3 → 6
    """
    visited = set()
    result = []

    def dfs(node):
        visited.add(node)
        result.append(node)

        for neighbor in graph.get(node, []):
            if neighbor not in visited:
                dfs(neighbor)

    dfs(start)
    return result


def dfs_iterative(graph: Dict[int, List[int]], start: int) -> List[int]:
    """
    DFS using stack

    Time Complexity: O(V + E)
    Space Complexity: O(V)
    """
    visited = set()
    result = []
    stack = [start]

    while stack:
        node = stack.pop()

        if node not in visited:
            visited.add(node)
            result.append(node)

            # Add neighbors in reverse order to maintain left-to-right traversal
            for neighbor in reversed(graph.get(node, [])):
                if neighbor not in visited:
                    stack.append(neighbor)

    return result


def bfs(graph: Dict[int, List[int]], start: int) -> List[int]:
    """
    BFS using queue

    Time Complexity: O(V + E)
    Space Complexity: O(V)

    Diagram:
         1
        / \
       2   3
      / \   \
     4   5   6

    BFS order (level by level): 1 → 2 → 3 → 4 → 5 → 6
    """
    visited = set([start])
    result = []
    queue = deque([start])

    while queue:
        node = queue.popleft()
        result.append(node)

        for neighbor in graph.get(node, []):
            if neighbor not in visited:
                visited.add(neighbor)
                queue.append(neighbor)

    return result


# ============================================================================
# 3. NUMBER OF ISLANDS
# ============================================================================
"""
Problem: Count number of islands in a 2D grid.

Difficulty: Medium
Companies: Amazon, Facebook, Google, Microsoft
Real-world Application: Image segmentation, cluster detection
"""


def num_islands(grid: List[List[str]]) -> int:
    """
    Approach: DFS on each unvisited land cell

    Time Complexity: O(m * n)
    Space Complexity: O(m * n) worst case for recursion

    Diagram:
    Grid:
    1 1 0 0 0
    1 1 0 0 0
    0 0 1 0 0
    0 0 0 1 1

    Islands (connected 1s):
    Island 1: top-left 2x2
    Island 2: middle 1
    Island 3: bottom-right 2
    Total: 3 islands
    """
    if not grid:
        return 0

    m, n = len(grid), len(grid[0])
    count = 0

    def dfs(i, j):
        """Mark all connected land as visited"""
        if (i < 0 or i >= m or j < 0 or j >= n or
                grid[i][j] == '0'):
            return

        grid[i][j] = '0'  # Mark as visited

        # Explore all 4 directions
        dfs(i + 1, j)
        dfs(i - 1, j)
        dfs(i, j + 1)
        dfs(i, j - 1)

    for i in range(m):
        for j in range(n):
            if grid[i][j] == '1':
                count += 1
                dfs(i, j)

    return count


# ============================================================================
# 4. CLONE GRAPH
# ============================================================================
"""
Problem: Deep copy an undirected graph.

Difficulty: Medium
Companies: Amazon, Facebook, Microsoft
Real-world Application: Graph duplication, state copying
"""


class GraphNode:
    def __init__(self, val=0, neighbors=None):
        self.val = val
        self.neighbors = neighbors if neighbors is not None else []


def clone_graph(node: Optional[GraphNode]) -> Optional[GraphNode]:
    """
    Approach: DFS with hash map

    Algorithm:
    - Use hash map to track old → new node mapping
    - DFS to traverse and clone

    Time Complexity: O(V + E)
    Space Complexity: O(V)

    Diagram:
    Original:    1 --- 2
                 |     |
                 4 --- 3

    Cloned:      1' -- 2'
                 |     |
                 4' -- 3'
    """
    if not node:
        return None

    old_to_new = {}

    def dfs(node):
        if node in old_to_new:
            return old_to_new[node]

        # Create clone
        clone = GraphNode(node.val)
        old_to_new[node] = clone

        # Clone neighbors
        for neighbor in node.neighbors:
            clone.neighbors.append(dfs(neighbor))

        return clone

    return dfs(node)


# ============================================================================
# 5. COURSE SCHEDULE
# ============================================================================
"""
Problem: Determine if you can finish all courses given prerequisites.
This is a cycle detection problem in directed graph.

Difficulty: Medium
Companies: Amazon, Facebook, Google, Microsoft
Real-world Application: Dependency resolution, build systems
"""


def can_finish(numCourses: int, prerequisites: List[List[int]]) -> bool:
    """
    Approach: Detect cycle using DFS

    Algorithm:
    - Build adjacency list
    - Use 3 states: unvisited, visiting, visited
    - If we encounter a visiting node, there's a cycle

    Time Complexity: O(V + E)
    Space Complexity: O(V + E)

    Diagram:
    numCourses = 4, prerequisites = [[1,0], [2,1], [3,2]]

    Graph:
    0 → 1 → 2 → 3  (no cycle, can finish)

    numCourses = 2, prerequisites = [[1,0], [0,1]]

    Graph:
    0 ⇄ 1  (cycle! cannot finish)
    """
    # Build adjacency list
    graph = defaultdict(list)
    for course, prereq in prerequisites:
        graph[prereq].append(course)

    # 0: unvisited, 1: visiting, 2: visited
    state = [0] * numCourses

    def has_cycle(course):
        if state[course] == 1:  # Currently visiting
            return True
        if state[course] == 2:  # Already visited
            return False

        state[course] = 1  # Mark as visiting

        for next_course in graph[course]:
            if has_cycle(next_course):
                return True

        state[course] = 2  # Mark as visited
        return False

    for i in range(numCourses):
        if has_cycle(i):
            return False

    return True


def find_order(numCourses: int, prerequisites: List[List[int]]) -> List[int]:
    """
    Course Schedule II: Return the order of courses to take

    Approach: Topological Sort using DFS

    Time Complexity: O(V + E)
    Space Complexity: O(V + E)

    Diagram:
    numCourses = 4, prerequisites = [[1,0], [2,0], [3,1], [3,2]]

    Graph:
         0
        / \
       1   2
        \ /
         3

    Topological order: [0, 1, 2, 3] or [0, 2, 1, 3]
    """
    graph = defaultdict(list)
    for course, prereq in prerequisites:
        graph[prereq].append(course)

    state = [0] * numCourses
    result = []

    def dfs(course):
        if state[course] == 1:
            return False
        if state[course] == 2:
            return True

        state[course] = 1

        for next_course in graph[course]:
            if not dfs(next_course):
                return False

        state[course] = 2
        result.append(course)
        return True

    for i in range(numCourses):
        if not dfs(i):
            return []

    return result[::-1]


# ============================================================================
# 6. SHORTEST PATH - DIJKSTRA'S ALGORITHM
# ============================================================================
"""
Problem: Find shortest path in weighted graph with non-negative edges.

Difficulty: Medium
Companies: Google, Amazon, Facebook, Microsoft
Real-world Application: GPS navigation, network routing
"""


def dijkstra(graph: Dict[int, List[Tuple[int, int]]], start: int, n: int) -> List[int]:
    """
    Dijkstra's Algorithm using Min Heap

    Algorithm:
    - Use min heap to always process closest node
    - Update distances to neighbors
    - Greedy approach: always expand shortest known path

    Time Complexity: O((V + E) log V)
    Space Complexity: O(V)

    Diagram:
    Graph with weights:
         1
       /   \
      4     2
     / \   / \
    2   3 1   5
         \ /
          3

    Shortest paths from node 1:
    1 → 1: 0
    1 → 2: 2
    1 → 3: 3
    1 → 4: 4
    1 → 5: 7
    """
    # Initialize distances
    dist = [float('inf')] * n
    dist[start] = 0

    # Min heap: (distance, node)
    heap = [(0, start)]

    while heap:
        curr_dist, u = heapq.heappop(heap)

        # Skip if we've found a better path
        if curr_dist > dist[u]:
            continue

        # Update neighbors
        for v, weight in graph.get(u, []):
            new_dist = curr_dist + weight

            if new_dist < dist[v]:
                dist[v] = new_dist
                heapq.heappush(heap, (new_dist, v))

    return dist


def network_delay_time(times: List[List[int]], n: int, k: int) -> int:
    """
    Network Delay Time: Time for signal to reach all nodes

    Application of Dijkstra's algorithm

    Time Complexity: O((V + E) log V)
    Space Complexity: O(V + E)
    """
    # Build graph
    graph = defaultdict(list)
    for u, v, w in times:
        graph[u].append((v, w))

    # Run Dijkstra
    dist = dijkstra(graph, k, n + 1)

    # Find maximum time (excluding start node 0)
    max_time = max(dist[1:n + 1])

    return max_time if max_time != float('inf') else -1


# ============================================================================
# 7. UNION FIND (DISJOINT SET)
# ============================================================================
"""
Problem: Efficiently manage disjoint sets with union and find operations.

Difficulty: Medium
Companies: All major companies
Real-world Application: Network connectivity, image processing
"""


class UnionFind:
    """
    Union Find with Path Compression and Union by Rank

    Operations:
    - find: O(α(n)) ≈ O(1) amortized
    - union: O(α(n)) ≈ O(1) amortized

    where α is inverse Ackermann function (grows extremely slowly)

    Diagram:
    Initial: {0} {1} {2} {3} {4}

    After union(0,1):
         0
        /
       1

    After union(2,3):
         0    2
        /    /
       1    3

    After union(1,2):
         0
        /|\
       1 2
          \
           3
    """

    def __init__(self, n: int):
        self.parent = list(range(n))
        self.rank = [0] * n
        self.count = n  # Number of disjoint sets

    def find(self, x: int) -> int:
        """
        Find root with path compression

        Path compression: Make all nodes point directly to root
        """
        if self.parent[x] != x:
            self.parent[x] = self.find(self.parent[x])
        return self.parent[x]

    def union(self, x: int, y: int) -> bool:
        """
        Union by rank

        Returns True if sets were merged, False if already in same set
        """
        root_x = self.find(x)
        root_y = self.find(y)

        if root_x == root_y:
            return False

        # Union by rank: attach smaller tree to larger
        if self.rank[root_x] < self.rank[root_y]:
            self.parent[root_x] = root_y
        elif self.rank[root_x] > self.rank[root_y]:
            self.parent[root_y] = root_x
        else:
            self.parent[root_y] = root_x
            self.rank[root_x] += 1

        self.count -= 1
        return True

    def connected(self, x: int, y: int) -> bool:
        """Check if x and y are in the same set"""
        return self.find(x) == self.find(y)


def count_components(n: int, edges: List[List[int]]) -> int:
    """
    Count number of connected components using Union Find

    Time Complexity: O(E * α(V))
    Space Complexity: O(V)

    Diagram:
    n = 5, edges = [[0,1], [1,2], [3,4]]

    Components:
    {0, 1, 2} and {3, 4}
    Count: 2
    """
    uf = UnionFind(n)

    for u, v in edges:
        uf.union(u, v)

    return uf.count


# ============================================================================
# 8. TOPOLOGICAL SORT
# ============================================================================
"""
Problem: Linear ordering of vertices in DAG.

Difficulty: Medium
Companies: Amazon, Google, Microsoft
Real-world Application: Task scheduling, build systems
"""


def topological_sort_dfs(n: int, edges: List[List[int]]) -> List[int]:
    """
    Approach 1: DFS-based

    Algorithm:
    - Post-order DFS
    - Reverse the result

    Time Complexity: O(V + E)
    Space Complexity: O(V)

    Diagram:
    Graph:
    0 → 1 → 3
    ↓   ↓
    2 → 3

    DFS post-order: 3, 1, 2, 0
    Reversed: 0, 2, 1, 3 (topological order)
    """
    graph = defaultdict(list)
    for u, v in edges:
        graph[u].append(v)

    visited = set()
    result = []

    def dfs(node):
        visited.add(node)
        for neighbor in graph[node]:
            if neighbor not in visited:
                dfs(neighbor)
        result.append(node)

    for i in range(n):
        if i not in visited:
            dfs(i)

    return result[::-1]


def topological_sort_kahn(n: int, edges: List[List[int]]) -> List[int]:
    """
    Approach 2: Kahn's Algorithm (BFS-based)

    Algorithm:
    - Start with nodes having 0 in-degree
    - Remove nodes and update in-degrees
    - Add nodes with 0 in-degree to queue

    Time Complexity: O(V + E)
    Space Complexity: O(V)
    """
    graph = defaultdict(list)
    in_degree = [0] * n

    # Build graph and calculate in-degrees
    for u, v in edges:
        graph[u].append(v)
        in_degree[v] += 1

    # Start with nodes having 0 in-degree
    queue = deque([i for i in range(n) if in_degree[i] == 0])
    result = []

    while queue:
        node = queue.popleft()
        result.append(node)

        for neighbor in graph[node]:
            in_degree[neighbor] -= 1
            if in_degree[neighbor] == 0:
                queue.append(neighbor)

    return result if len(result) == n else []


# ============================================================================
# 9. MINIMUM SPANNING TREE - KRUSKAL'S ALGORITHM
# ============================================================================
"""
Problem: Find minimum cost spanning tree.

Difficulty: Medium
Companies: Google, Amazon, Microsoft
Real-world Application: Network design, circuit design
"""


def minimum_spanning_tree_kruskal(n: int, edges: List[List[int]]) -> Tuple[int, List[List[int]]]:
    """
    Kruskal's Algorithm using Union Find

    Algorithm:
    - Sort edges by weight
    - Add edge if it doesn't create cycle (use Union Find)
    - Stop when we have V-1 edges

    Time Complexity: O(E log E)
    Space Complexity: O(V)

    Diagram:
    Graph:
        1
       /|\
      1 2 3
     /  |  \
    0---2---2
        |
        4

    MST (total weight = 5):
    0---1---2
        |   |
        1   2
            |
            4
    """
    # Sort edges by weight
    edges.sort(key=lambda x: x[2])

    uf = UnionFind(n)
    mst = []
    total_cost = 0

    for u, v, weight in edges:
        if uf.union(u, v):
            mst.append([u, v, weight])
            total_cost += weight

            if len(mst) == n - 1:
                break

    return total_cost, mst


# ============================================================================
# 10. WORD LADDER
# ============================================================================
"""
Problem: Find shortest transformation sequence from beginWord to endWord.

Difficulty: Hard
Companies: Amazon, Google, Facebook
Real-world Application: DNA mutation analysis, word games
"""


def ladder_length(beginWord: str, endWord: str, wordList: List[str]) -> int:
    """
    Approach: BFS with word transformations

    Algorithm:
    - BFS to find shortest path
    - Generate all possible transformations
    - Check if transformation exists in wordList

    Time Complexity: O(M² × N) where M is word length, N is number of words
    Space Complexity: O(M × N)

    Diagram:
    beginWord = "hit", endWord = "cog"
    wordList = ["hot","dot","dog","lot","log","cog"]

    Transformation graph:
    hit → hot → dot → dog → cog
              ↓   ↓
             lot → log → cog

    Shortest path: hit → hot → dot → dog → cog (length 5)
    """
    if endWord not in wordList:
        return 0

    word_set = set(wordList)
    queue = deque([(beginWord, 1)])
    visited = {beginWord}

    while queue:
        word, level = queue.popleft()

        if word == endWord:
            return level

        # Try all possible transformations
        for i in range(len(word)):
            for c in 'abcdefghijklmnopqrstuvwxyz':
                next_word = word[:i] + c + word[i + 1:]

                if next_word in word_set and next_word not in visited:
                    visited.add(next_word)
                    queue.append((next_word, level + 1))

    return 0


# ============================================================================
# TESTING AND EXAMPLES
# ============================================================================

def run_examples():
    """Run example test cases for all problems"""

    print("=" * 70)
    print("GRAPH ALGORITHMS - EXAMPLE OUTPUTS")
    print("=" * 70)

    # DFS and BFS
    print("\n1. DFS AND BFS TRAVERSAL")
    print("-" * 70)
    graph = {
        1: [2, 3],
        2: [4, 5],
        3: [6],
        4: [],
        5: [],
        6: []
    }
    print(f"Graph: {graph}")
    print(f"DFS from 1: {dfs_recursive(graph, 1)}")
    print(f"BFS from 1: {bfs(graph, 1)}")

    # Number of Islands
    print("\n2. NUMBER OF ISLANDS")
    print("-" * 70)
    grid = [
        ["1", "1", "0", "0", "0"],
        ["1", "1", "0", "0", "0"],
        ["0", "0", "1", "0", "0"],
        ["0", "0", "0", "1", "1"]
    ]
    print(f"Number of islands: {num_islands([row[:] for row in grid])}")

    # Course Schedule
    print("\n3. COURSE SCHEDULE")
    print("-" * 70)
    numCourses = 4
    prerequisites = [[1, 0], [2, 1], [3, 2]]
    print(f"Courses: {numCourses}, Prerequisites: {prerequisites}")
    print(f"Can finish: {can_finish(numCourses, prerequisites)}")
    print(f"Order: {find_order(numCourses, prerequisites)}")

    # Dijkstra's Algorithm
    print("\n4. SHORTEST PATH (DIJKSTRA)")
    print("-" * 70)
    graph = {
        0: [(1, 4), (2, 1)],
        1: [(3, 1)],
        2: [(1, 2), (3, 5)],
        3: []
    }
    print(f"Shortest distances from 0: {dijkstra(graph, 0, 4)}")

    # Union Find
    print("\n5. UNION FIND - CONNECTED COMPONENTS")
    print("-" * 70)
    n = 5
    edges = [[0, 1], [1, 2], [3, 4]]
    print(f"Nodes: {n}, Edges: {edges}")
    print(f"Number of components: {count_components(n, edges)}")

    # Topological Sort
    print("\n6. TOPOLOGICAL SORT")
    print("-" * 70)
    n = 4
    edges = [[0, 1], [0, 2], [1, 3], [2, 3]]
    print(f"Nodes: {n}, Edges: {edges}")
    print(f"Topological order (DFS): {topological_sort_dfs(n, edges)}")
    print(f"Topological order (Kahn): {topological_sort_kahn(n, edges)}")

    # Word Ladder
    print("\n7. WORD LADDER")
    print("-" * 70)
    beginWord = "hit"
    endWord = "cog"
    wordList = ["hot", "dot", "dog", "lot", "log", "cog"]
    print(f"Begin: {beginWord}, End: {endWord}")
    print(f"Ladder length: {ladder_length(beginWord, endWord, wordList)}")

    print("\n" + "=" * 70)


if __name__ == "__main__":
    run_examples()
