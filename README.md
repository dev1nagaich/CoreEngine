# CoreEngine

A low-latency limit order matching engine written in C++17. Maintains bid/ask
price levels in AVL-balanced binary search trees for O(log n) insertion and
O(1) matching against the best price, and supports limit, stop, and
stop-limit tickets.

## Structure

- `Ledger` — the order book itself: bid/ask trees, stop trees, and the
  matching logic (`Ledger.hpp` / `Ledger.cpp`).
- `PriceLevel` — a single price level in the book, holding a FIFO queue of
  tickets at that price (`PriceLevel.hpp` / `PriceLevel.cpp`).
- `Ticket` — a single resting order (`Ticket.hpp` / `Ticket.cpp`).
- `TicketFeed` — replays newline-delimited ticket commands from a file into
  a `Ledger`.
- `TicketGenerator` — generates random tickets for load testing.

## Building

```sh
cmake -S . -B build
cmake --build build
```

This builds the `CoreEngine` executable and the `CoreEngineTests` GoogleTest
suite (fetched automatically via CMake `FetchContent`).

## Running

```sh
./build/CoreEngine
```

Reads `initialOrders.txt` to seed the book, then replays `Orders.txt` while
timing the run.

## Testing

```sh
ctest --test-dir build
```

## Ticket file format

Each line is one command:

```
L <ticketId> <isBuySide 0|1> <quantity> <price>   add limit ticket
C <ticketId>                                      cancel limit ticket
M <ticketId> <newQuantity> <newPrice>              modify limit ticket
```
