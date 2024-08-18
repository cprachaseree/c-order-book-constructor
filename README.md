# c-order-book-constructor

A C sparse static array implementation for order book construction,
inspired by Quant Cup 1's winning implementation.

input market events/messages are parsed from LOBSTER format [specs](https://lobsterdata.com/info/DataStructure.php)

Limit order insertion should be O(1). Deletions should usually be O(1), but depending on sparse-ness of the prices deletion of best bid or best ask may take O(1) over min and max prices (stll constant because min and max is defined and not dependent on input message amount).

