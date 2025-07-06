"""
Python bindings for OrderEntry Client library
"""
from __future__ import annotations
import typing
__all__ = ['CancelOrderMessage', 'ExecutionReportMessage', 'MessageHeader', 'ModifyOrderMessage', 'NewOrderMessage', 'OEMessageType', 'OESide', 'OrderAckMessage', 'OrderEntryClient', 'OrderRejectMessage', 'OrderType', 'TimeInForce', 'create_cancel_order', 'create_new_order', 'format_oe_timestamp', 'get_oe_timestamp_ns', 'oe_message_type_to_string']
class CancelOrderMessage:
    def to_debug_string(self) -> str:
        ...
    @property
    def client_order_id(self) -> int:
        ...
    @property
    def header(self) -> MessageHeader:
        ...
    @property
    def original_client_order_id(self) -> int:
        ...
    @property
    def symbol(self) -> str:
        ...
class ExecutionReportMessage:
    def to_debug_string(self) -> str:
        ...
    @property
    def client_order_id(self) -> int:
        ...
    @property
    def exchange_order_id(self) -> int:
        ...
    @property
    def execution_id(self) -> int:
        ...
    @property
    def execution_price(self) -> int:
        ...
    @property
    def execution_quantity(self) -> int:
        ...
    @property
    def header(self) -> MessageHeader:
        ...
    @property
    def leaves_quantity(self) -> int:
        ...
    @property
    def side(self) -> OESide:
        ...
class MessageHeader:
    def to_debug_string(self) -> str:
        ...
    @property
    def instrument_id(self) -> int:
        ...
    @property
    def message_length(self) -> int:
        ...
    @property
    def message_type(self) -> int:
        ...
    @property
    def sequence_number(self) -> int:
        ...
    @property
    def timestamp_ns(self) -> int:
        ...
class ModifyOrderMessage:
    def to_debug_string(self) -> str:
        ...
    @property
    def client_order_id(self) -> int:
        ...
    @property
    def header(self) -> MessageHeader:
        ...
    @property
    def new_price(self) -> int:
        ...
    @property
    def new_quantity(self) -> int:
        ...
    @property
    def original_client_order_id(self) -> int:
        ...
    @property
    def symbol(self) -> str:
        ...
class NewOrderMessage:
    def to_debug_string(self) -> str:
        ...
    @property
    def client_order_id(self) -> int:
        ...
    @property
    def header(self) -> MessageHeader:
        ...
    @property
    def order_type(self) -> OrderType:
        ...
    @property
    def price(self) -> int:
        ...
    @property
    def quantity(self) -> int:
        ...
    @property
    def side(self) -> OESide:
        ...
    @property
    def symbol(self) -> str:
        ...
    @property
    def time_in_force(self) -> TimeInForce:
        ...
class OEMessageType:
    """
    Members:
    
      NEW_ORDER
    
      CANCEL_ORDER
    
      MODIFY_ORDER
    
      ORDER_ACK
    
      ORDER_REJECT
    
      EXECUTION_REPORT
    """
    CANCEL_ORDER: typing.ClassVar[OEMessageType]  # value = <OEMessageType.CANCEL_ORDER: 2>
    EXECUTION_REPORT: typing.ClassVar[OEMessageType]  # value = <OEMessageType.EXECUTION_REPORT: 6>
    MODIFY_ORDER: typing.ClassVar[OEMessageType]  # value = <OEMessageType.MODIFY_ORDER: 3>
    NEW_ORDER: typing.ClassVar[OEMessageType]  # value = <OEMessageType.NEW_ORDER: 1>
    ORDER_ACK: typing.ClassVar[OEMessageType]  # value = <OEMessageType.ORDER_ACK: 4>
    ORDER_REJECT: typing.ClassVar[OEMessageType]  # value = <OEMessageType.ORDER_REJECT: 5>
    __members__: typing.ClassVar[dict[str, OEMessageType]]  # value = {'NEW_ORDER': <OEMessageType.NEW_ORDER: 1>, 'CANCEL_ORDER': <OEMessageType.CANCEL_ORDER: 2>, 'MODIFY_ORDER': <OEMessageType.MODIFY_ORDER: 3>, 'ORDER_ACK': <OEMessageType.ORDER_ACK: 4>, 'ORDER_REJECT': <OEMessageType.ORDER_REJECT: 5>, 'EXECUTION_REPORT': <OEMessageType.EXECUTION_REPORT: 6>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: int) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: int) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class OESide:
    """
    Members:
    
      BUY
    
      SELL
    """
    BUY: typing.ClassVar[OESide]  # value = <OESide.BUY: 1>
    SELL: typing.ClassVar[OESide]  # value = <OESide.SELL: 2>
    __members__: typing.ClassVar[dict[str, OESide]]  # value = {'BUY': <OESide.BUY: 1>, 'SELL': <OESide.SELL: 2>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: int) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: int) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class OrderAckMessage:
    def to_debug_string(self) -> str:
        ...
    @property
    def client_order_id(self) -> int:
        ...
    @property
    def exchange_order_id(self) -> int:
        ...
    @property
    def header(self) -> MessageHeader:
        ...
    @property
    def symbol(self) -> str:
        ...
class OrderEntryClient:
    def __init__(self) -> None:
        ...
    def connect(self, server_address: str, port: int) -> bool:
        """
        Connect to order entry server
        """
    def disconnect(self) -> None:
        """
        Disconnect from server
        """
    def is_connected(self) -> bool:
        """
        Check if client is connected
        """
    def send_cancel_order(self, client_order_id: int, original_order_id: int, symbol: str) -> bool:
        """
        Send a cancel order
        """
    def send_new_order(self, client_order_id: int, symbol: str, side: OESide, price: int, quantity: int) -> bool:
        """
        Send a new order
        """
class OrderRejectMessage:
    def to_debug_string(self) -> str:
        ...
    @property
    def client_order_id(self) -> int:
        ...
    @property
    def header(self) -> MessageHeader:
        ...
    @property
    def reject_reason(self) -> int:
        ...
    @property
    def reject_text(self) -> str:
        ...
    @property
    def symbol(self) -> str:
        ...
class OrderType:
    """
    Members:
    
      MARKET
    
      LIMIT
    """
    LIMIT: typing.ClassVar[OrderType]  # value = <OrderType.LIMIT: 2>
    MARKET: typing.ClassVar[OrderType]  # value = <OrderType.MARKET: 1>
    __members__: typing.ClassVar[dict[str, OrderType]]  # value = {'MARKET': <OrderType.MARKET: 1>, 'LIMIT': <OrderType.LIMIT: 2>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: int) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: int) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
class TimeInForce:
    """
    Members:
    
      DAY
    
      IOC
    """
    DAY: typing.ClassVar[TimeInForce]  # value = <TimeInForce.DAY: 1>
    IOC: typing.ClassVar[TimeInForce]  # value = <TimeInForce.IOC: 2>
    __members__: typing.ClassVar[dict[str, TimeInForce]]  # value = {'DAY': <TimeInForce.DAY: 1>, 'IOC': <TimeInForce.IOC: 2>}
    def __eq__(self, other: typing.Any) -> bool:
        ...
    def __getstate__(self) -> int:
        ...
    def __hash__(self) -> int:
        ...
    def __index__(self) -> int:
        ...
    def __init__(self, value: int) -> None:
        ...
    def __int__(self) -> int:
        ...
    def __ne__(self, other: typing.Any) -> bool:
        ...
    def __repr__(self) -> str:
        ...
    def __setstate__(self, state: int) -> None:
        ...
    def __str__(self) -> str:
        ...
    @property
    def name(self) -> str:
        ...
    @property
    def value(self) -> int:
        ...
def create_cancel_order(client_order_id: int, original_order_id: int, symbol: str) -> CancelOrderMessage:
    """
    Create a cancel order message
    """
def create_new_order(client_order_id: int, symbol: str, side: OESide, price: int, quantity: int) -> NewOrderMessage:
    """
    Create a new order message
    """
def format_oe_timestamp(timestamp_ns: int) -> str:
    """
    Format timestamp to readable string
    """
def get_oe_timestamp_ns() -> int:
    """
    Get current timestamp in nanoseconds
    """
def oe_message_type_to_string(type: OEMessageType) -> str:
    """
    Convert order entry message type enum to string
    """
