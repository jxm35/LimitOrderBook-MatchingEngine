"""
Python bindings for MDFeed MulticastReceiver library
"""
from __future__ import annotations
import datetime
import typing
__all__ = ['BookClearMessage', 'HeartbeatMessage', 'MDMessageType', 'MDSide', 'MessageHeader', 'MulticastReceiver', 'PriceLevelDeleteMessage', 'PriceLevelUpdateMessage', 'ReceiverConfig', 'ReceiverStats', 'SnapshotBeginMessage', 'SnapshotEndMessage', 'SnapshotEntryMessage', 'TradeMessage', 'UpdateAction', 'cast_md_message', 'format_md_timestamp', 'get_md_timestamp_ns', 'md_message_type_to_string']
class BookClearMessage:
    def to_debug_string(self) -> str:
        ...
    @property
    def header(self) -> MessageHeader:
        ...
    @property
    def reason_code(self) -> int:
        ...
class HeartbeatMessage:
    def to_debug_string(self) -> str:
        ...
    @property
    def header(self) -> MessageHeader:
        ...
class MDMessageType:
    """
    Members:
    
      HEARTBEAT
    
      PRICE_LEVEL_UPDATE
    
      PRICE_LEVEL_DELETE
    
      TRADE
    
      SNAPSHOT_BEGIN
    
      SNAPSHOT_ENTRY
    
      SNAPSHOT_END
    
      BOOK_CLEAR
    """
    BOOK_CLEAR: typing.ClassVar[MDMessageType]  # value = <MDMessageType.BOOK_CLEAR: 8>
    HEARTBEAT: typing.ClassVar[MDMessageType]  # value = <MDMessageType.HEARTBEAT: 1>
    PRICE_LEVEL_DELETE: typing.ClassVar[MDMessageType]  # value = <MDMessageType.PRICE_LEVEL_DELETE: 3>
    PRICE_LEVEL_UPDATE: typing.ClassVar[MDMessageType]  # value = <MDMessageType.PRICE_LEVEL_UPDATE: 2>
    SNAPSHOT_BEGIN: typing.ClassVar[MDMessageType]  # value = <MDMessageType.SNAPSHOT_BEGIN: 5>
    SNAPSHOT_END: typing.ClassVar[MDMessageType]  # value = <MDMessageType.SNAPSHOT_END: 7>
    SNAPSHOT_ENTRY: typing.ClassVar[MDMessageType]  # value = <MDMessageType.SNAPSHOT_ENTRY: 6>
    TRADE: typing.ClassVar[MDMessageType]  # value = <MDMessageType.TRADE: 4>
    __members__: typing.ClassVar[dict[str, MDMessageType]]  # value = {'HEARTBEAT': <MDMessageType.HEARTBEAT: 1>, 'PRICE_LEVEL_UPDATE': <MDMessageType.PRICE_LEVEL_UPDATE: 2>, 'PRICE_LEVEL_DELETE': <MDMessageType.PRICE_LEVEL_DELETE: 3>, 'TRADE': <MDMessageType.TRADE: 4>, 'SNAPSHOT_BEGIN': <MDMessageType.SNAPSHOT_BEGIN: 5>, 'SNAPSHOT_ENTRY': <MDMessageType.SNAPSHOT_ENTRY: 6>, 'SNAPSHOT_END': <MDMessageType.SNAPSHOT_END: 7>, 'BOOK_CLEAR': <MDMessageType.BOOK_CLEAR: 8>}
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
class MDSide:
    """
    Members:
    
      BUY
    
      SELL
    """
    BUY: typing.ClassVar[MDSide]  # value = <MDSide.BUY: 1>
    SELL: typing.ClassVar[MDSide]  # value = <MDSide.SELL: 2>
    __members__: typing.ClassVar[dict[str, MDSide]]  # value = {'BUY': <MDSide.BUY: 1>, 'SELL': <MDSide.SELL: 2>}
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
class MulticastReceiver:
    @typing.overload
    def __init__(self) -> None:
        ...
    @typing.overload
    def __init__(self, arg0: ReceiverConfig) -> None:
        ...
    def get_stats(self) -> ReceiverStats:
        """
        Get receiver statistics
        """
    def initialize(self, config: ReceiverConfig) -> bool:
        """
        Initialize receiver with config
        """
    def is_running(self) -> bool:
        """
        Check if receiver is running
        """
    def reset_stats(self) -> None:
        """
        Reset receiver statistics
        """
    def set_message_handler(self, handler: typing.Callable[[MessageHeader, capsule, int], None]) -> None:
        """
        Set custom message handler function
        """
    def start(self) -> bool:
        """
        Start the receiver
        """
    def stop(self) -> None:
        """
        Stop the receiver
        """
class PriceLevelDeleteMessage:
    def to_debug_string(self) -> str:
        ...
    @property
    def header(self) -> MessageHeader:
        ...
    @property
    def price(self) -> int:
        ...
    @property
    def side(self) -> MDSide:
        ...
class PriceLevelUpdateMessage:
    def to_debug_string(self) -> str:
        ...
    @property
    def action(self) -> UpdateAction:
        ...
    @property
    def header(self) -> MessageHeader:
        ...
    @property
    def price(self) -> int:
        ...
    @property
    def quantity(self) -> int:
        ...
    @property
    def side(self) -> MDSide:
        ...
class ReceiverConfig:
    enable_logging: bool
    interface_ip: str
    log_file_path: str
    log_to_console: bool
    multicast_ip: str
    multicast_port: int
    receive_buffer_size: int
    stats_interval: datetime.timedelta
    validate_sequence_numbers: bool
    def __init__(self) -> None:
        ...
class ReceiverStats:
    @property
    def invalid_messages(self) -> int:
        ...
    @property
    def sequence_gaps(self) -> int:
        ...
    @property
    def start_time(self) -> datetime.timedelta:
        ...
    @property
    def total_bytes_received(self) -> int:
        ...
    @property
    def total_messages_received(self) -> int:
        ...
class SnapshotBeginMessage:
    def to_debug_string(self) -> str:
        ...
    @property
    def header(self) -> MessageHeader:
        ...
    @property
    def total_entries(self) -> int:
        ...
class SnapshotEndMessage:
    def to_debug_string(self) -> str:
        ...
    @property
    def checksum(self) -> int:
        ...
    @property
    def header(self) -> MessageHeader:
        ...
class SnapshotEntryMessage:
    def to_debug_string(self) -> str:
        ...
    @property
    def header(self) -> MessageHeader:
        ...
    @property
    def price(self) -> int:
        ...
    @property
    def quantity(self) -> int:
        ...
    @property
    def side(self) -> MDSide:
        ...
class TradeMessage:
    def to_debug_string(self) -> str:
        ...
    @property
    def aggressor_side(self) -> MDSide:
        ...
    @property
    def header(self) -> MessageHeader:
        ...
    @property
    def price(self) -> int:
        ...
    @property
    def quantity(self) -> int:
        ...
    @property
    def trade_id(self) -> int:
        ...
class UpdateAction:
    """
    Members:
    
      NEW
    
      CHANGE
    
      DELETE
    """
    CHANGE: typing.ClassVar[UpdateAction]  # value = <UpdateAction.CHANGE: 2>
    DELETE: typing.ClassVar[UpdateAction]  # value = <UpdateAction.DELETE: 3>
    NEW: typing.ClassVar[UpdateAction]  # value = <UpdateAction.NEW: 1>
    __members__: typing.ClassVar[dict[str, UpdateAction]]  # value = {'NEW': <UpdateAction.NEW: 1>, 'CHANGE': <UpdateAction.CHANGE: 2>, 'DELETE': <UpdateAction.DELETE: 3>}
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
def cast_md_message(header: MessageHeader, data: bytes) -> typing.Any:
    """
    Cast raw message data to appropriate market data message type
    """
def format_md_timestamp(timestamp_ns: int) -> str:
    """
    Format timestamp to readable string
    """
def get_md_timestamp_ns() -> int:
    """
    Get current timestamp in nanoseconds
    """
def md_message_type_to_string(type: MDMessageType) -> str:
    """
    Convert market data message type enum to string
    """
