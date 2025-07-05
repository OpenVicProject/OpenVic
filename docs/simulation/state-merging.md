# State merging
In the game, it may happen that two different countries own provinces that are part of a same state. Suppose a situation where a country acquires the remainder of a state it already owned. What happens to the buildings?

## Ports
The port rules are straightforward. The highest-level port is retained, and the rest are deleted.

## Factories
Factory logic is quite complicated. If there are two factories of the same type in the states to be merged, the following rules are applied in order:

- If the factory in the state that was already owned is subsidized, it takes priority, regardless of level.
- The factory with the highest level takes precedence.
- The factory whom the country already owned is chosen, if it comes to a stalemate.

The subsidization and hire priorities seem to be retained in factories that the country already owned, and discarded in factories that are acquired.
