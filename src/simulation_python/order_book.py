import random
import time
from collections import defaultdict
from dataclasses import dataclass
from typing import Dict, List, Optional


@dataclass
class Order:
    order_id: int
    price: int
    quantity: int
    is_buy: bool
    timestamp: float


@dataclass
class Trade:
    price: int
    quantity: int
    timestamp: float


class OrderBook:
    def __init__(self):
        self.orders: Dict[int, Order] = {}
        self.bid_levels: Dict[int, List[Order]] = defaultdict(list)
        self.ask_levels: Dict[int, List[Order]] = defaultdict(list)
        self.next_order_id = 1
        self.trades: List[Trade] = []
        self.orders_matched = 0

        self._initialize_book()

    def _initialize_book(self):
        base_price = 50000
        self._add_order_direct(base_price - 200, 1000, True)  # 498.00
        self._add_order_direct(base_price + 200, 1000, False)  # 502.00

        for i in range(1, 6):
            bid_price = base_price - 200 - (i * 100)
            ask_price = base_price + 200 + (i * 100)
            quantity = random.randint(100, 500)

            self._add_order_direct(bid_price, quantity, True)
            self._add_order_direct(ask_price, quantity, False)

    def _add_order_direct(self, price: int, quantity: int, is_buy: bool) -> int:
        order = Order(
            order_id=self.next_order_id,
            price=price,
            quantity=quantity,
            is_buy=is_buy,
            timestamp=time.time()
        )
        self.next_order_id += 1

        self.orders[order.order_id] = order
        if is_buy:
            self.bid_levels[price].append(order)
        else:
            self.ask_levels[price].append(order)

        return order.order_id

    def add_order(self, price: int, quantity: int, is_buy: bool) -> int:
        order_id = self._add_order_direct(price, quantity, is_buy)
        self.match()
        return order_id

    def remove_order(self, order_id: int) -> bool:
        if order_id not in self.orders:
            return False

        order = self.orders[order_id]
        levels = self.bid_levels if order.is_buy else self.ask_levels

        if order.price in levels and order in levels[order.price]:
            levels[order.price].remove(order)
            if not levels[order.price]:
                del levels[order.price]

        del self.orders[order_id]
        return True

    def match(self):
        while True:
            best_bid_price = self.get_best_bid_price()
            best_ask_price = self.get_best_ask_price()

            if (best_bid_price is None or best_ask_price is None or
                    best_bid_price < best_ask_price):
                break

            bid_orders = self.bid_levels[best_bid_price]
            ask_orders = self.ask_levels[best_ask_price]

            if not bid_orders or not ask_orders:
                break

            bid_order = bid_orders[0]
            ask_order = ask_orders[0]

            trade_price = best_ask_price
            trade_quantity = min(bid_order.quantity, ask_order.quantity)

            trade = Trade(trade_price, trade_quantity, time.time())
            self.trades.append(trade)
            self.orders_matched += trade_quantity

            bid_order.quantity -= trade_quantity
            ask_order.quantity -= trade_quantity

            if bid_order.quantity == 0:
                self.remove_order(bid_order.order_id)
            if ask_order.quantity == 0:
                self.remove_order(ask_order.order_id)

    def place_market_buy_order(self, quantity: int):
        remaining = quantity
        while remaining > 0 and self.ask_levels:
            best_ask = self.get_best_ask_price()
            if best_ask is None:
                break

            self.add_order(best_ask, remaining, True)
            break

    def place_market_sell_order(self, quantity: int):
        remaining = quantity
        while remaining > 0 and self.bid_levels:
            best_bid = self.get_best_bid_price()
            if best_bid is None:
                break

            self.add_order(best_bid, remaining, False)
            break

    def get_best_bid_price(self) -> Optional[int]:
        if not self.bid_levels:
            return None
        return max(self.bid_levels.keys())

    def get_best_ask_price(self) -> Optional[int]:
        if not self.ask_levels:
            return None
        return min(self.ask_levels.keys())

    def get_spread(self) -> Optional[int]:
        best_bid = self.get_best_bid_price()
        best_ask = self.get_best_ask_price()

        if best_bid is None or best_ask is None:
            return None

        return best_ask - best_bid

    def get_bid_quantities(self) -> Dict[int, int]:
        quantities = {}
        for price, orders in self.bid_levels.items():
            quantities[price] = sum(order.quantity for order in orders)
        return quantities

    def get_ask_quantities(self) -> Dict[int, int]:
        quantities = {}
        for price, orders in self.ask_levels.items():
            quantities[price] = sum(order.quantity for order in orders)
        return quantities

    def get_order_count(self) -> int:
        return len(self.orders)

    def get_best_bid_depth(self) -> int:
        best_bid = self.get_best_bid_price()
        if best_bid is None:
            return 0
        return sum(order.quantity for order in self.bid_levels[best_bid])

    def get_best_ask_depth(self) -> int:
        best_ask = self.get_best_ask_price()
        if best_ask is None:
            return 0
        return sum(order.quantity for order in self.ask_levels[best_ask])

    def get_recent_volume(self, seconds: float = 1.0) -> int:
        current_time = time.time()
        recent_trades = [
            trade for trade in self.trades
            if current_time - trade.timestamp <= seconds
        ]
        return sum(trade.quantity for trade in recent_trades)
