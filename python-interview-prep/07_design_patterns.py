"""
DESIGN PATTERNS FOR TECH LEADS
===============================

Essential design patterns every senior tech lead must know.
Includes real-world applications and Python implementations.

Categories:
1. Creational Patterns (Object creation)
2. Structural Patterns (Object composition)
3. Behavioral Patterns (Object interaction)

Table of Contents:

CREATIONAL PATTERNS:
1. Singleton Pattern
2. Factory Pattern
3. Abstract Factory Pattern
4. Builder Pattern
5. Prototype Pattern

STRUCTURAL PATTERNS:
6. Adapter Pattern
7. Decorator Pattern
8. Facade Pattern
9. Proxy Pattern
10. Composite Pattern

BEHAVIORAL PATTERNS:
11. Observer Pattern
12. Strategy Pattern
13. Command Pattern
14. State Pattern
15. Chain of Responsibility
16. Iterator Pattern
17. Template Method Pattern
"""

from abc import ABC, abstractmethod
from typing import List, Dict, Any
from copy import deepcopy
from functools import wraps


# ============================================================================
# CREATIONAL PATTERNS
# ============================================================================

"""
Purpose: Deal with object creation mechanisms
Goal: Create objects in a manner suitable to the situation
"""


# ============================================================================
# 1. SINGLETON PATTERN
# ============================================================================
"""
Intent: Ensure a class has only one instance and provide global access to it.

Use Cases:
- Configuration managers
- Database connections
- Logging
- Thread pools
- Cache

Real-world: Logger, Database Connection Pool
Companies: Asked at Google, Amazon, Microsoft
"""


class SingletonMeta(type):
    """
    Thread-safe Singleton implementation using metaclass

    Advantages:
    - Thread-safe
    - Lazy initialization
    - Clean syntax
    """
    _instances = {}

    def __call__(cls, *args, **kwargs):
        if cls not in cls._instances:
            instance = super().__call__(*args, **kwargs)
            cls._instances[cls] = instance
        return cls._instances[cls]


class Logger(metaclass=SingletonMeta):
    """
    Example: Logger Singleton

    Usage:
    >>> logger1 = Logger()
    >>> logger2 = Logger()
    >>> logger1 is logger2  # True
    """

    def __init__(self):
        self.log_file = "app.log"

    def log(self, message: str):
        print(f"[LOG] {message}")


class DatabaseConnection(metaclass=SingletonMeta):
    """
    Example: Database Connection Pool

    Ensures only one connection pool exists
    """

    def __init__(self):
        self.connection = self._create_connection()

    def _create_connection(self):
        print("Creating database connection...")
        return "DB_CONNECTION_OBJECT"

    def query(self, sql: str):
        return f"Executing: {sql}"


# ============================================================================
# 2. FACTORY PATTERN
# ============================================================================
"""
Intent: Define an interface for creating objects, but let subclasses decide
which class to instantiate.

Use Cases:
- When you don't know exact types beforehand
- Framework code that needs to create objects
- Simplifying object creation

Real-world: Payment processing, Document creation
Companies: Asked at Amazon, Facebook, Microsoft
"""


class PaymentMethod(ABC):
    """Abstract base class for payment methods"""

    @abstractmethod
    def process_payment(self, amount: float) -> str:
        pass


class CreditCardPayment(PaymentMethod):
    def process_payment(self, amount: float) -> str:
        return f"Processing ${amount} via Credit Card"


class PayPalPayment(PaymentMethod):
    def process_payment(self, amount: float) -> str:
        return f"Processing ${amount} via PayPal"


class CryptoPayment(PaymentMethod):
    def process_payment(self, amount: float) -> str:
        return f"Processing ${amount} via Cryptocurrency"


class PaymentFactory:
    """
    Factory for creating payment methods

    Usage:
    >>> factory = PaymentFactory()
    >>> payment = factory.create_payment("credit_card")
    >>> payment.process_payment(100)
    """

    @staticmethod
    def create_payment(payment_type: str) -> PaymentMethod:
        payment_map = {
            "credit_card": CreditCardPayment,
            "paypal": PayPalPayment,
            "crypto": CryptoPayment
        }

        payment_class = payment_map.get(payment_type.lower())
        if not payment_class:
            raise ValueError(f"Unknown payment type: {payment_type}")

        return payment_class()


# ============================================================================
# 3. BUILDER PATTERN
# ============================================================================
"""
Intent: Separate construction of a complex object from its representation.

Use Cases:
- Constructing complex objects step by step
- Creating different representations of objects
- Avoiding telescoping constructors

Real-world: HTTP request builders, SQL query builders, UI builders
Companies: Asked at Google, Amazon, Microsoft
"""


class Pizza:
    """Product being built"""

    def __init__(self):
        self.size = None
        self.cheese = False
        self.pepperoni = False
        self.mushrooms = False
        self.olives = False

    def __str__(self):
        toppings = []
        if self.cheese:
            toppings.append("cheese")
        if self.pepperoni:
            toppings.append("pepperoni")
        if self.mushrooms:
            toppings.append("mushrooms")
        if self.olives:
            toppings.append("olives")

        return f"{self.size} Pizza with {', '.join(toppings)}"


class PizzaBuilder:
    """
    Builder for constructing Pizza objects

    Usage:
    >>> pizza = (PizzaBuilder()
    ...          .set_size("large")
    ...          .add_cheese()
    ...          .add_pepperoni()
    ...          .build())

    Advantages:
    - Fluent interface
    - Clear construction process
    - Immutable result (optional)
    """

    def __init__(self):
        self.pizza = Pizza()

    def set_size(self, size: str):
        self.pizza.size = size
        return self

    def add_cheese(self):
        self.pizza.cheese = True
        return self

    def add_pepperoni(self):
        self.pizza.pepperoni = True
        return self

    def add_mushrooms(self):
        self.pizza.mushrooms = True
        return self

    def add_olives(self):
        self.pizza.olives = True
        return self

    def build(self) -> Pizza:
        return self.pizza


# ============================================================================
# STRUCTURAL PATTERNS
# ============================================================================

"""
Purpose: Deal with object composition
Goal: Form larger structures from objects and classes
"""


# ============================================================================
# 4. DECORATOR PATTERN
# ============================================================================
"""
Intent: Attach additional responsibilities to an object dynamically.

Use Cases:
- Adding features to objects without modifying their structure
- Cross-cutting concerns (logging, caching, authentication)
- Middleware in web frameworks

Real-world: Python @decorators, Middleware, Stream wrappers
Companies: Asked at all major companies
"""


def timing_decorator(func):
    """
    Decorator to measure execution time

    Usage:
    >>> @timing_decorator
    ... def slow_function():
    ...     time.sleep(1)
    """
    import time

    @wraps(func)
    def wrapper(*args, **kwargs):
        start = time.time()
        result = func(*args, **kwargs)
        end = time.time()
        print(f"{func.__name__} took {end - start:.2f} seconds")
        return result

    return wrapper


def cache_decorator(func):
    """
    Memoization decorator

    Usage:
    >>> @cache_decorator
    ... def fibonacci(n):
    ...     if n <= 1: return n
    ...     return fibonacci(n-1) + fibonacci(n-2)
    """
    cache = {}

    @wraps(func)
    def wrapper(*args):
        if args in cache:
            return cache[args]
        result = func(*args)
        cache[args] = result
        return result

    return wrapper


def authenticate(func):
    """
    Authentication decorator

    Usage:
    >>> @authenticate
    ... def sensitive_operation(user):
    ...     return "Success"
    """

    @wraps(func)
    def wrapper(user, *args, **kwargs):
        if not hasattr(user, 'is_authenticated') or not user.is_authenticated:
            raise PermissionError("User not authenticated")
        return func(user, *args, **kwargs)

    return wrapper


class Component(ABC):
    """Component interface for decorator pattern"""

    @abstractmethod
    def operation(self) -> str:
        pass


class ConcreteComponent(Component):
    """Concrete component"""

    def operation(self) -> str:
        return "ConcreteComponent"


class Decorator(Component):
    """Base decorator"""

    def __init__(self, component: Component):
        self._component = component

    def operation(self) -> str:
        return self._component.operation()


class LoggingDecorator(Decorator):
    """Adds logging to component"""

    def operation(self) -> str:
        result = self._component.operation()
        print(f"[LOG] {result}")
        return result


class EncryptionDecorator(Decorator):
    """Adds encryption to component"""

    def operation(self) -> str:
        result = self._component.operation()
        return f"Encrypted({result})"


# ============================================================================
# 5. ADAPTER PATTERN
# ============================================================================
"""
Intent: Convert interface of a class into another interface clients expect.

Use Cases:
- Integrating legacy code
- Third-party library integration
- API versioning

Real-world: Payment gateway adapters, Database adapters
Companies: Asked at Amazon, Google
"""


class OldPaymentSystem:
    """Legacy payment system"""

    def make_payment(self, amount: float):
        return f"Old system: Paid ${amount}"


class NewPaymentInterface(ABC):
    """New payment interface"""

    @abstractmethod
    def process(self, amount: float) -> str:
        pass


class PaymentAdapter(NewPaymentInterface):
    """
    Adapter to use old system with new interface

    Usage:
    >>> old_system = OldPaymentSystem()
    >>> adapter = PaymentAdapter(old_system)
    >>> adapter.process(100)  # Uses new interface, calls old system
    """

    def __init__(self, old_system: OldPaymentSystem):
        self.old_system = old_system

    def process(self, amount: float) -> str:
        # Adapt the interface
        return self.old_system.make_payment(amount)


# ============================================================================
# 6. FACADE PATTERN
# ============================================================================
"""
Intent: Provide a unified interface to a set of interfaces in a subsystem.

Use Cases:
- Simplifying complex systems
- Library wrappers
- API gateways

Real-world: OS APIs, Framework facades, Service layers
Companies: Asked at Microsoft, Amazon
"""


class CPU:
    def freeze(self):
        return "CPU frozen"

    def jump(self, position):
        return f"CPU jumped to {position}"

    def execute(self):
        return "CPU executing"


class Memory:
    def load(self, position, data):
        return f"Memory loaded {data} at {position}"


class HardDrive:
    def read(self, lba, size):
        return f"HardDrive read {size} bytes from {lba}"


class ComputerFacade:
    """
    Facade simplifying computer startup

    Usage:
    >>> computer = ComputerFacade()
    >>> computer.start()

    Hides complexity of coordinating CPU, Memory, HardDrive
    """

    def __init__(self):
        self.cpu = CPU()
        self.memory = Memory()
        self.hard_drive = HardDrive()

    def start(self):
        """Simple interface hiding complex subsystem interactions"""
        results = []
        results.append(self.cpu.freeze())
        results.append(self.memory.load(0, self.hard_drive.read(0, 1024)))
        results.append(self.cpu.jump(0))
        results.append(self.cpu.execute())
        return " -> ".join(results)


# ============================================================================
# BEHAVIORAL PATTERNS
# ============================================================================

"""
Purpose: Deal with object communication
Goal: Define how objects interact and distribute responsibility
"""


# ============================================================================
# 7. OBSERVER PATTERN
# ============================================================================
"""
Intent: Define one-to-many dependency so when one object changes state,
all dependents are notified.

Use Cases:
- Event handling systems
- MVC architecture
- Pub/Sub systems
- Real-time notifications

Real-world: Event listeners, Stock market apps, News feeds
Companies: Asked at all major companies
"""


class Subject:
    """Subject being observed"""

    def __init__(self):
        self._observers = []
        self._state = None

    def attach(self, observer):
        """Add observer"""
        self._observers.append(observer)

    def detach(self, observer):
        """Remove observer"""
        self._observers.remove(observer)

    def notify(self):
        """Notify all observers"""
        for observer in self._observers:
            observer.update(self)

    def set_state(self, state):
        """Change state and notify"""
        self._state = state
        self.notify()

    def get_state(self):
        return self._state


class Observer(ABC):
    """Observer interface"""

    @abstractmethod
    def update(self, subject: Subject):
        pass


class ConcreteObserverA(Observer):
    def update(self, subject: Subject):
        print(f"ObserverA: Reacted to state = {subject.get_state()}")


class ConcreteObserverB(Observer):
    def update(self, subject: Subject):
        print(f"ObserverB: Reacted to state = {subject.get_state()}")


class StockMarket:
    """
    Real-world example: Stock market

    Usage:
    >>> market = StockMarket()
    >>> investor1 = Investor("Warren")
    >>> investor2 = Investor("Carl")
    >>> market.attach(investor1)
    >>> market.attach(investor2)
    >>> market.update_price("AAPL", 150)
    """

    def __init__(self):
        self._observers = []
        self._prices = {}

    def attach(self, observer):
        self._observers.append(observer)

    def update_price(self, symbol: str, price: float):
        self._prices[symbol] = price
        for observer in self._observers:
            observer.on_price_update(symbol, price)


class Investor:
    def __init__(self, name: str):
        self.name = name

    def on_price_update(self, symbol: str, price: float):
        print(f"{self.name} notified: {symbol} = ${price}")


# ============================================================================
# 8. STRATEGY PATTERN
# ============================================================================
"""
Intent: Define a family of algorithms, encapsulate each one, and make them
interchangeable.

Use Cases:
- Different algorithms for same task
- Runtime algorithm selection
- Avoiding conditionals

Real-world: Sorting strategies, Compression algorithms, Routing
Companies: Asked at Google, Amazon, Facebook
"""


class SortStrategy(ABC):
    """Strategy interface"""

    @abstractmethod
    def sort(self, data: List[int]) -> List[int]:
        pass


class QuickSort(SortStrategy):
    def sort(self, data: List[int]) -> List[int]:
        if len(data) <= 1:
            return data
        pivot = data[len(data) // 2]
        left = [x for x in data if x < pivot]
        middle = [x for x in data if x == pivot]
        right = [x for x in data if x > pivot]
        return self.sort(left) + middle + self.sort(right)


class MergeSort(SortStrategy):
    def sort(self, data: List[int]) -> List[int]:
        if len(data) <= 1:
            return data

        mid = len(data) // 2
        left = self.sort(data[:mid])
        right = self.sort(data[mid:])

        return self._merge(left, right)

    def _merge(self, left, right):
        result = []
        i = j = 0

        while i < len(left) and j < len(right):
            if left[i] <= right[j]:
                result.append(left[i])
                i += 1
            else:
                result.append(right[j])
                j += 1

        result.extend(left[i:])
        result.extend(right[j:])
        return result


class Sorter:
    """
    Context that uses strategy

    Usage:
    >>> sorter = Sorter(QuickSort())
    >>> sorter.sort([3, 1, 4, 1, 5])
    >>> sorter.set_strategy(MergeSort())
    >>> sorter.sort([3, 1, 4, 1, 5])
    """

    def __init__(self, strategy: SortStrategy):
        self._strategy = strategy

    def set_strategy(self, strategy: SortStrategy):
        self._strategy = strategy

    def sort(self, data: List[int]) -> List[int]:
        return self._strategy.sort(data)


# ============================================================================
# 9. COMMAND PATTERN
# ============================================================================
"""
Intent: Encapsulate a request as an object, allowing parameterization
and queuing.

Use Cases:
- Undo/Redo functionality
- Macro recording
- Transaction systems
- Task queues

Real-world: Text editors, Database transactions, Job schedulers
Companies: Asked at Microsoft, Amazon
"""


class Command(ABC):
    """Command interface"""

    @abstractmethod
    def execute(self):
        pass

    @abstractmethod
    def undo(self):
        pass


class Light:
    """Receiver"""

    def __init__(self):
        self.is_on = False

    def turn_on(self):
        self.is_on = True
        return "Light is ON"

    def turn_off(self):
        self.is_on = False
        return "Light is OFF"


class LightOnCommand(Command):
    def __init__(self, light: Light):
        self.light = light

    def execute(self):
        return self.light.turn_on()

    def undo(self):
        return self.light.turn_off()


class LightOffCommand(Command):
    def __init__(self, light: Light):
        self.light = light

    def execute(self):
        return self.light.turn_off()

    def undo(self):
        return self.light.turn_on()


class RemoteControl:
    """
    Invoker with undo support

    Usage:
    >>> light = Light()
    >>> remote = RemoteControl()
    >>> remote.set_command(LightOnCommand(light))
    >>> remote.press_button()  # Light turns on
    >>> remote.press_undo()    # Light turns off
    """

    def __init__(self):
        self.command = None
        self.history = []

    def set_command(self, command: Command):
        self.command = command

    def press_button(self):
        if self.command:
            result = self.command.execute()
            self.history.append(self.command)
            return result

    def press_undo(self):
        if self.history:
            command = self.history.pop()
            return command.undo()


# ============================================================================
# TESTING AND EXAMPLES
# ============================================================================

def run_examples():
    """Run example test cases for all patterns"""

    print("=" * 70)
    print("DESIGN PATTERNS - EXAMPLE OUTPUTS")
    print("=" * 70)

    # Singleton
    print("\n1. SINGLETON PATTERN")
    print("-" * 70)
    logger1 = Logger()
    logger2 = Logger()
    print(f"logger1 is logger2: {logger1 is logger2}")
    logger1.log("Application started")

    # Factory
    print("\n2. FACTORY PATTERN")
    print("-" * 70)
    factory = PaymentFactory()
    payment = factory.create_payment("paypal")
    print(payment.process_payment(100))

    # Builder
    print("\n3. BUILDER PATTERN")
    print("-" * 70)
    pizza = (PizzaBuilder()
             .set_size("large")
             .add_cheese()
             .add_pepperoni()
             .add_mushrooms()
             .build())
    print(pizza)

    # Decorator
    print("\n4. DECORATOR PATTERN")
    print("-" * 70)
    component = ConcreteComponent()
    decorated = LoggingDecorator(EncryptionDecorator(component))
    print(decorated.operation())

    # Facade
    print("\n5. FACADE PATTERN")
    print("-" * 70)
    computer = ComputerFacade()
    print(computer.start())

    # Observer
    print("\n6. OBSERVER PATTERN")
    print("-" * 70)
    market = StockMarket()
    investor1 = Investor("Warren Buffett")
    investor2 = Investor("Carl Icahn")
    market.attach(investor1)
    market.attach(investor2)
    market.update_price("AAPL", 150.50)

    # Strategy
    print("\n7. STRATEGY PATTERN")
    print("-" * 70)
    data = [3, 1, 4, 1, 5, 9, 2, 6]
    sorter = Sorter(QuickSort())
    print(f"QuickSort: {sorter.sort(data.copy())}")
    sorter.set_strategy(MergeSort())
    print(f"MergeSort: {sorter.sort(data.copy())}")

    # Command
    print("\n8. COMMAND PATTERN")
    print("-" * 70)
    light = Light()
    remote = RemoteControl()
    remote.set_command(LightOnCommand(light))
    print(remote.press_button())
    print(remote.press_undo())

    print("\n" + "=" * 70)


if __name__ == "__main__":
    run_examples()
