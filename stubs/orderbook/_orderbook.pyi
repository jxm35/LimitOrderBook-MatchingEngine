"""
Python bindings for C++ OrderBook library
"""
from __future__ import annotations
import typing
__all__ = ['Order', 'OrderBook', 'OrderBookSpread', 'OrderCore', 'Security', 'create_order']
class Order:
    def __init__(self, order_core: OrderCore, price: int, quantity: int, is_buy: bool) -> None:
        """
        Create a new Order
        """
    def current_quantity(self) -> int:
        """
        Get current quantity
        """
    def initial_quantity(self) -> int:
        """
        Get initial quantity
        """
    def is_buy(self) -> bool:
        """
        Check if this is a buy order
        """
    def order_id(self) -> int:
        """
        Get order ID
        """
    def price(self) -> int:
        """
        Get order price
        """
    def security_id(self) -> int:
        """
        Get security ID
        """
    def username(self) -> str:
        """
        Get username
        """
class OrderBook:
    def __init__(self, security: Security) -> None:
        """
        Create a new OrderBook
        """
    def add_order(self, order: Order) -> None:
        """
        Add a limit order
        """
    def amend_order(self, order_id: int, new_order: Order) -> None:
        """
        Amend an existing order
        """
    def contains_order(self, order_id: int) -> bool:
        """
        Check if order exists
        """
    def count(self) -> int:
        """
        Get total number of orders
        """
    def get_ask_quantities(self) -> dict[int, int]:
        """
        Get ask quantities by price level
        """
    def get_best_ask_price(self) -> typing.Any:
        """
        Get best ask price (None if no asks)
        """
    def get_best_bid_price(self) -> typing.Any:
        """
        Get best bid price (None if no bids)
        """
    def get_bid_quantities(self) -> dict[int, int]:
        """
        Get bid quantities by price level
        """
    def get_orders_matched(self) -> int:
        """
        Get total quantity of orders matched
        """
    def get_spread(self) -> OrderBookSpread:
        """
        Get bid-ask spread
        """
    def place_market_buy_order(self, quantity: int) -> None:
        """
        Place a market buy order
        """
    def place_market_sell_order(self, quantity: int) -> None:
        """
        Place a market sell order
        """
    def remove_order(self, order_id: int) -> None:
        """
        Remove an order
        """
class OrderBookSpread:
    def spread(self) -> typing.Any:
        """
        Get the spread (None if no spread available)
        """
class OrderCore:
    @typing.overload
    def __init__(self, username: str, security_id: int) -> None:
        """
        Create OrderCore with auto-generated ID
        """
    @typing.overload
    def __init__(self, order_id: int, username: str, security_id: int) -> None:
        """
        Create OrderCore with specific ID
        """
    def order_id(self) -> int:
        """
        Get order ID
        """
    def security_id(self) -> int:
        """
        Get security ID
        """
    def username(self) -> str:
        """
        Get username
        """
class Security:
    def __init__(self, name: str, ticker: str, security_id: int) -> None:
        """
        Create a new Security
        """
    def get_security_id(self) -> int:
        """
        Get the security ID
        """
def create_order(username: str, security_id: int, price: int, quantity: int, is_buy: bool) -> Order:
    """
    Create an order with auto-generated ID
    """
