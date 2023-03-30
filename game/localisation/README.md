# Localisation

This folder contains localisations for in-game text. Each sub-folder must be named with a standard locale code, e.g. `en_GB`, to which the localisations contained within it shall apply (Godot's supported locale codes are listed [here](https://docs.godotengine.org/en/latest/tutorials/i18n/locales.html)). These folders contain `.csv` files where each line is interpreted as a key-value pair. Empty lines allow for spacing out/separating sections, and any further entries beyond the first two on a line are ignored, which can be used to add comments at the end of lines. Lines with their key and value empty are skipped, allowing free-standing comments with no preceeding localisation, but lines with only one of their key or value empty, while not loaded as a localisation, will still result in warnings about incomplete entries.

```
,, Example Localisation Comment
EXAMPLE_KEY,Example Value
ANOTHER_EXAMPLE,Another Example, This is a comment

,, Entries with empty keys/values are skipped but can still produce warnings
THIS,, produces a warning!
,As, does this!
,, This doesn't!
BUT_THIS_DOES
AND_THIS,
```
