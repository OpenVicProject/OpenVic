# The Basics of Dataloading
The system for defining and loading the actual data from input files is based around a system of callbacks. The `NodeTools` utilities are our toolbox of callbacks which are used to load data from an AST in an error-safe and structured way.

## Example: `Culture.cpp`
Let's break down how `CultureManager` loads `CultureGroup`s, a good example of how the process works.
The whole deal:

```cpp
bool CultureManager::_load_culture_group(
	CountryManager const& country_manager, size_t& total_expected_cultures,
	GraphicalCultureType const* default_unit_graphical_culture_type, std::string_view culture_group_key,
	ast::NodeCPtr culture_group_node
) {
	std::string_view leader {};
	GraphicalCultureType const* unit_graphical_culture_type = default_unit_graphical_culture_type;
	bool is_overseas = true;
	Country const* union_country = nullptr;

	bool ret = expect_dictionary_keys_and_default(
		increment_callback(total_expected_cultures),
		"leader", ONE_EXACTLY, expect_identifier(assign_variable_callback(leader)),
		"unit", ZERO_OR_ONE,
			expect_graphical_culture_type_identifier(assign_variable_callback_pointer(unit_graphical_culture_type)),
		"union", ZERO_OR_ONE,
			country_manager.expect_country_identifier(assign_variable_callback_pointer(union_country)),
		"is_overseas", ZERO_OR_ONE, expect_bool(assign_variable_callback(is_overseas))
	)(culture_group_node);

	ret &= add_culture_group(culture_group_key, leader, unit_graphical_culture_type, is_overseas, union_country);
	return ret;
}
```

Now, that's a lot, so we'll break it down into more reasonable chunks.

First, let's look at the function's arguments:
- `CountryManager const& country_manager` - an object which loads and stores countries, in the same way as `CultureManager` loads and stores cultures and culture groups; here, this is used to parse the culture group's `union` country.
- `size_t& total_expected_cultures` - a reference to an unsigned integer, used to count the number of cultures in this culture group.
- `GraphicalCultureType const* default_unit_graphical_culture_type` - a pointer to a graphical culture type, representing the default `Generic` graphical culture.
- `std::string_view culture_group_key` - a string (or rather an object referencing a string, but not holding the memory itself), representing the identifier of the culture group being loaded (for example `"north_german"`).
- `ast::NodeCPtr culture_group_node` - a node representing the culture group's text defines, which is used as input to data parsing functions.

```
dict = {
	key = "string"
	list = { 0.1 200 -12 }
}
```

`ast::NodeCPtr`s, like the one at the end of the arguments above, is the fundamental type from which data is loaded. Nodes each contain either a single value, a key and another node, or a list of other nodes, with these variants together forming a tree structure that can to represent the contents of any text defines file. For example, the text defines below would be represented by a top-level node with the key `dict` and a list node as its value. That list node would contain 2 sub-nodes, the first with key `key` and value a singleton node with value `"string"`, and the second having key `list` and value a list node with 3 singleton sub-node `0.1`, `200` and `-12`.

In practice, we rarely directly use these nodes, and instead use helper functions under the namespace `NodeTools`, as we will see below.

```cpp
std::string_view leader {};
GraphicalCultureType const* unit_graphical_culture_type = default_unit_graphical_culture_type;
bool is_overseas = true;
Country const* union_country = nullptr;
```

Getting into the body of the function, we define the variables that we'll load data into; these are usually simple C++ types like uint64_t, std::vector, std::string_view, and pointers. These must be initialized with default values, even if we expect them to be set by the parsing code, as some compilers (MSVC) don't default initialize primitive variables (ints, bools, floats, pointers) which can cause crashes.

```cpp
bool ret = expect_dictionary_keys_and_default(
	increment_callback(total_expected_cultures),
	...
)(culture_group_node);
```

Next, we get into the real meat using `NodeTools`. The function `expect_dictionary_keys_and_default` returns a `node_callback_t`, another function which takes in an `ast::NodeCPtr`, in this case `culture_group_node`, and returns a boolean representing if it was successful or not. The function is expecting the node to be a list full of key-node pairs (a dictionary), and so will parse it as such and provide the resulting data for further parsing. In particular, the `keys` part of the function's name means it accepts specific expected keys and callback for handling them, unlike `expect_dictionary` which just passes on every key-node pair regardless of the key. The `default` part of the function's name means it accepts a default callback, used for any key not explicitly given in the function's other arguments; without a default callback, any unexpected key would cause an error to be logged and the function to return false (although all other keys would still be parsed as usual). The default callback here is `increment_callback`, which just adds one to the variable given to it, in this case `total_expected_cultures`, and returns true; for culture groups, all keys other than a few reserved ones are the names of cultures themselves, so incrementing `total_expected_cultures` for each of them means we are counting the number of cultures in this culture group.

```cpp
"leader", ONE_EXACTLY, expect_identifier(assign_variable_callback(leader)),
"unit", ZERO_OR_ONE,
	expect_graphical_culture_type_identifier(assign_variable_callback_pointer(unit_graphical_culture_type)),
"union", ZERO_OR_ONE,
	country_manager.expect_country_identifier(assign_variable_callback_pointer(union_country)),
"is_overseas", ZERO_OR_ONE, expect_bool(assign_variable_callback(is_overseas))
```

Inside `expect_dictionary_keys_and_default`, we have a list of arguments specifying the expected contenets of the dictionary; they come in groups of three, and there can be as many of these as you want, so long as the first arguments are unique.
- That unique first argument is a string dictating a key that is expected in the dictionary.
- Second is the expected count, how many times the key should appear; this can be `ZERO_OR_ONE`, `ZERO_OR_MORE`, `ONE_EXACTLY`, or `ONE_OR_MORE`, which are pretty self-explanatory; if a key appears fewer or more times than expected, an error will logged and the function will return false, with the additional instances in the "more than" case not being parsed. For `"leader"` we specify `ONE_EXACTLY`, as there must be a single leader key in each culture group definition, and for `"unit"`, the graphical culture type, we specify `ZERO_OR_ONE`, as there can't be more than one but there doesn't have to be any, if it doesn't appear the default graphical culture type will be used.
- Finally we create a `node_callback_t`, a function taking in an `ast::NodeCPtr` and returning a success-indicating bool. When a key is found, its callback will be called with the key's value node as its argument. Here, the callbacks are each made up of two components - an expect function, which creates a `node_callback_t` that converts an `ast::NodeCPtr` into a certain type, and a callback which the expect function calls when it is successful, with its parsed type as the argument. The expect functions are `expect_identifier`, which parses an unquoted sequences of characters into a `std::string_view`, `expect_bool`, which parses `yes` or `no` into a `bool`, and `expect_graphical_culture_type_identifier` and `expect_country_identifier`, which are specialisations of `expect_identifier` that further convert its result into a `GraphicalCultureType const&` and a `Country const&` respectively. The callbacks are `assign_variable_callback`, which simply sets it's argument variable to the parsed value of the same type, and `assign_variable_callback_pointer`, which sets it's argument variable to a pointer to the parsed value, useful for converting `const&` (const reference) values into `const*` (const pointer) values, as `const&`s can't be reassigned so they don't work as variables that aren't initialized immediately.

All together, these triplets of arguments tell `expect_dictionary_keys_and_default` what keys to look for, how many of them to expect, and what to do once it finds them. In this case, that means parsing a string, graphical culture type, country and bool and storing them in intermediate variables.

```cpp
ret &= add_culture_group(culture_group_key, leader, unit_graphical_culture_type, is_overseas, union_country);
```

This final call stores the data we've just parsed in the actual gamestate by passing the parsed variables to `add_culture_group`, a function that validates their values, in this case that `culture_group_key` isn't empty or a duplicate, that `leader` isn't empty, and that `unit_graphical_culture_type` isn't empty; it returns true if the validation is successful and the culture group is added to their registry, otherwise it returns false. This call will differ for each different type of data you load, but generally is pretty simple and follows the same pattern throughout the codebase.

For more examples the other loading functions in `Culture.cpp` are a great way to see how the system works in a scaled but not too complicated manner.
