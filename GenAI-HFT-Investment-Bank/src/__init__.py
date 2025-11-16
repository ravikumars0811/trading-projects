"""
GenAI HFT & Investment Banking
A comprehensive generative AI system for high-frequency trading and investment banking
"""

__version__ = "1.0.0"
__author__ = "GenAI Finance Team"

from . import models
from . import hft
from . import investment_banking
from . import data
from . import utils

__all__ = [
    "models",
    "hft",
    "investment_banking",
    "data",
    "utils",
]
