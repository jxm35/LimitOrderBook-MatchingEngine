import sys
import time
from collections import deque
from typing import Deque

import pyqtgraph as pg
from PyQt6.QtCore import QTimer
from PyQt6.QtGui import QFont
from PyQt6.QtWidgets import (QMainWindow, QWidget, QVBoxLayout, QHBoxLayout,
                             QPushButton, QLabel, QGridLayout)

sys.path.append('..')
from order_book import OrderBook
from market_simulator import MarketSimulator

LINE_WIDTH = 4
COLOUR_RED = (255, 40, 40)
COLOUR_GREEN = (10, 140, 70)
COLOUR_BLUE = (40, 40, 255)

FRAME_RATE = 60
MAX_AGE_SECONDS = 10
SCALING_LOOKBACK_SECONDS = 5
MAX_RENDERED_DEPTHS = 15


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Market Simulation")
        self.setGeometry(100, 100, 1600, 1000)

        self.max_history = MAX_AGE_SECONDS * FRAME_RATE
        self.time_data: Deque[float] = deque(maxlen=self.max_history)
        self.bid_price_data: Deque[float] = deque(maxlen=self.max_history)
        self.ask_price_data: Deque[float] = deque(maxlen=self.max_history)
        self.volume_data: Deque[float] = deque(maxlen=self.max_history)

        self.setup_ui()

        self.update_timer = QTimer()
        self.update_timer.timeout.connect(self.update_data)
        self.update_timer.start(int(1000 / FRAME_RATE))

        self.order_book = OrderBook()
        self.market_simulator = MarketSimulator(self.order_book)
        self.market_simulator.start_simulation()

        self.start_time = time.time()
        self.last_volume_check = 0

    def setup_ui(self):
        central_widget = QWidget()
        self.setCentralWidget(central_widget)
        main_layout = QGridLayout(central_widget)

        self.setup_price_chart()
        self.setup_depth_chart()
        self.setup_volume_chart()
        self.setup_controls()

        main_layout.addWidget(self.price_chart, 0, 0, 1, 1)
        main_layout.addWidget(self.depth_chart, 0, 1, 1, 1)
        main_layout.addWidget(self.volume_chart, 1, 0, 1, 1)
        main_layout.addWidget(self.controls_widget, 1, 1, 1, 1)

        main_layout.setColumnStretch(0, 3)  # Price and volume charts
        main_layout.setColumnStretch(1, 2)  # Depth chart and controls

        main_layout.setRowStretch(0, 3)
        main_layout.setRowStretch(1, 1)

    def setup_price_chart(self):
        self.price_chart = pg.PlotWidget(title="Price (pence)")
        self.price_chart.setLabel('left', 'Price', 'pence')
        self.price_chart.setLabel('bottom', 'Time Elapsed', 's')
        self.price_chart.showGrid(x=True, y=True, alpha=0.3)
        self.price_chart.setBackground('white')

        self.bid_line = self.price_chart.plot(
            pen=pg.mkPen(color=COLOUR_GREEN, width=LINE_WIDTH), name='Bid'
        )
        self.ask_line = self.price_chart.plot(
            pen=pg.mkPen(color=COLOUR_RED, width=LINE_WIDTH), name='Ask'
        )

        self.price_chart.addLegend()

    def setup_depth_chart(self):
        self.depth_chart = pg.PlotWidget(title="Order Book Depth")
        self.depth_chart.setLabel('left', 'Price Level', 'pence')
        self.depth_chart.setLabel('bottom', 'Number Of Contracts')
        self.depth_chart.showGrid(x=True, y=True, alpha=0.3)
        self.depth_chart.setBackground('white')

    def setup_volume_chart(self):
        self.volume_chart = pg.PlotWidget(title="Volume")
        self.volume_chart.setLabel('left', 'Volume')
        self.volume_chart.setLabel('bottom', 'Time Elapsed', 's')
        self.volume_chart.showGrid(x=True, y=True, alpha=0.3)
        self.volume_chart.setBackground('white')

        self.volume_bars = pg.BarGraphItem(
            x=[0], y=[0], width=0.5, height=[0],
            brush=pg.mkBrush(color=COLOUR_BLUE)
        )
        self.volume_chart.addItem(self.volume_bars)

    def setup_controls(self):
        self.controls_widget = QWidget()
        layout = QVBoxLayout(self.controls_widget)
        button_layout = QHBoxLayout()

        self.buy_button = QPushButton("Add Buy Pressure")
        self.buy_button.clicked.connect(self.on_buy_pressure)
        self.buy_button.setMinimumHeight(60)

        self.sell_button = QPushButton("Add Sell Pressure")
        self.sell_button.clicked.connect(self.on_sell_pressure)
        self.sell_button.setMinimumHeight(60)

        button_layout.addWidget(self.buy_button)
        button_layout.addWidget(self.sell_button)

        stats_layout = QGridLayout()
        self.order_count_label = QLabel("Order Count: 0")
        self.spread_label = QLabel("Spread: 0")
        self.best_bid_label = QLabel("Best Bid: 0 p")
        self.best_ask_label = QLabel("Best Ask: 0 p")
        self.bid_depth_label = QLabel("Best Bid Depth: 0")
        self.ask_depth_label = QLabel("Best Ask Depth: 0")

        label_font = QFont()
        label_font.setPointSize(10)
        for label in [self.order_count_label, self.spread_label, self.best_bid_label,
                      self.best_ask_label, self.bid_depth_label, self.ask_depth_label]:
            label.setFont(label_font)

        stats_layout.addWidget(self.order_count_label, 0, 0)
        stats_layout.addWidget(self.spread_label, 0, 1)
        stats_layout.addWidget(self.best_bid_label, 1, 0)
        stats_layout.addWidget(self.best_ask_label, 1, 1)
        stats_layout.addWidget(self.bid_depth_label, 2, 0)
        stats_layout.addWidget(self.ask_depth_label, 2, 1)

        layout.addLayout(button_layout)
        layout.addLayout(stats_layout)
        layout.addStretch()

    def update_data(self):
        current_time = time.time() - self.start_time
        best_bid = self.order_book.get_best_bid_price()
        best_ask = self.order_book.get_best_ask_price()

        if best_bid is None or best_ask is None:
            return

        self.time_data.append(current_time)
        self.bid_price_data.append(best_bid / 100.0)  # Convert to pounds
        self.ask_price_data.append(best_ask / 100.0)

        recent_volume = self.order_book.get_recent_volume(1.0)
        self.volume_data.append(recent_volume)

        if len(self.time_data) > 1:
            self.bid_line.setData(list(self.time_data), list(self.bid_price_data))
            self.ask_line.setData(list(self.time_data), list(self.ask_price_data))

            # Auto-scale Y axis around recent data
            scaling_lookback = SCALING_LOOKBACK_SECONDS * FRAME_RATE
            if len(self.bid_price_data) > scaling_lookback:
                recent_bids = list(self.bid_price_data)[-scaling_lookback:]
                recent_asks = list(self.ask_price_data)[-scaling_lookback:]
                min_price = min(min(recent_bids), min(recent_asks)) - 0.1
                max_price = max(max(recent_bids), max(recent_asks)) + 0.1
                self.price_chart.setYRange(min_price, max_price)

            self.price_chart.setXRange(max(0, current_time - 8), current_time)

        if len(self.volume_data) > 1:
            self.volume_bars.setOpts(
                x=list(self.time_data),
                y=[0] * len(self.time_data),  # Base at 0
                height=list(self.volume_data),
                width=0.05
            )
            self.volume_chart.setXRange(max(0, current_time - 8), current_time)
            max_volume = max(self.volume_data) if self.volume_data else 100
            self.volume_chart.setYRange(0, max_volume * 1.1, padding=0)

        self.update_depth_chart()
        self.update_statistics()

        if len(self.bid_price_data) > 1:
            price_y_range = self.price_chart.getViewBox().viewRange()[1]  # Get Y range from price chart
            self.depth_chart.setYRange(price_y_range[0], price_y_range[1])

    def update_depth_chart(self):
        bid_quantities = self.order_book.get_bid_quantities()
        ask_quantities = self.order_book.get_ask_quantities()

        bid_prices = sorted(bid_quantities.keys(), reverse=True)[:MAX_RENDERED_DEPTHS]
        ask_prices = sorted(ask_quantities.keys())[:MAX_RENDERED_DEPTHS]
        self.depth_chart.clear()

        if bid_prices:
            bid_y = [price / 100.0 for price in bid_prices]
            bid_widths = [bid_quantities[price] for price in bid_prices]
            bid_bars = pg.BarGraphItem(
                x=[0] * len(bid_y), y=bid_y, width=bid_widths, height=0.02,
                brush=pg.mkBrush(color=COLOUR_GREEN)
            )
            self.depth_chart.addItem(bid_bars)

        if ask_prices:
            ask_y = [price / 100.0 for price in ask_prices]
            ask_widths = [ask_quantities[price] for price in ask_prices]

            ask_bars = pg.BarGraphItem(
                x=[0] * len(ask_y), y=ask_y, width=ask_widths, height=0.02,
                brush=pg.mkBrush(color=COLOUR_RED)
            )
            self.depth_chart.addItem(ask_bars)

        all_quantities = [bid_quantities[p] for p in bid_prices] + [ask_quantities[p] for p in ask_prices]

        if all_quantities:
            max_quantity = max(all_quantities)
            self.depth_chart.setXRange(0, max_quantity * 1.1, padding=0)

    def update_statistics(self):
        order_count = self.order_book.get_order_count()
        spread = self.order_book.get_spread()
        best_bid = self.order_book.get_best_bid_price()
        best_ask = self.order_book.get_best_ask_price()
        bid_depth = self.order_book.get_best_bid_depth()
        ask_depth = self.order_book.get_best_ask_depth()

        self.order_count_label.setText(f"Order Count: {order_count}")
        if spread is not None:
            self.spread_label.setText(f"Spread: {spread / 100:.2f}")
        else:
            self.spread_label.setText("Spread: -")

        if best_bid is not None:
            self.best_bid_label.setText(f"Best Bid: {best_bid / 100:.2f}p")
        else:
            self.best_bid_label.setText("Best Bid: -")

        if best_ask is not None:
            self.best_ask_label.setText(f"Best Ask: {best_ask / 100:.2f}p")
        else:
            self.best_ask_label.setText("Best Ask: -")

        self.bid_depth_label.setText(f"Best Bid Depth: {bid_depth}")
        self.ask_depth_label.setText(f"Best Ask Depth: {ask_depth}")

    def on_buy_pressure(self):
        self.market_simulator.add_buy_pressure()

    def on_sell_pressure(self):
        self.market_simulator.add_sell_pressure()

    def closeEvent(self, event):
        self.market_simulator.stop_simulation()
        event.accept()
