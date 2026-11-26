# Map UI investigation: country colours and map readability

Status: research note for a possible contribution to OpenVic issue [#469](https://github.com/OpenVicProject/OpenVic/issues/469).

This document records the existing implementation and a low-risk plan before changing rendering code. It intentionally does not claim that a feature is implemented or tested.

## Scope

The parent tracker [#468 — Add Victoria 2 Compatibility](https://github.com/OpenVicProject/OpenVic/issues/468) lists several missing UI screens and map tasks. Its child [#469 — Add Map UI](https://github.com/OpenVicProject/OpenVic/issues/469) currently has the checklist item `Add Country Colors`.

The narrow goal here is to clarify what "country colours" should mean in the current architecture, identify the code path that already paints provinces, and define verification cases so an implementation can be reviewed without relying on screenshots alone.

## Repository files inspected

- `game/src/Systems/Session/Map/MapView.gd`
  - Owns map viewport interaction, camera movement/zoom, province hover/click signals, and map-to-world coordinate conversion.
- `game/src/Systems/Session/Map/MapView.tscn`
  - Wires the map view, terrain shader, projection manager, selection markers, and valid-move markers together.
- `game/src/Systems/Session/Map/MapText.gd`
  - Creates 3D province-name labels from `GameSingleton.get_province_names()`.
- `game/src/Systems/Session/Map/TerrainMap.gdshader`
  - Combines terrain, province colour textures, parchment overlay, and hover/selection highlighting.
- `extension/src/openvic-extension/singletons/GameSingleton.hpp`
  - Owns the map-mode pointer and province colour image/texture resources exposed to Godot.
- `extension/src/openvic-extension/singletons/GameSingleton.cpp`
  - Generates the province colour image, exposes map-mode accessors, and refreshes the texture when the map mode changes.
- `game/project.godot`
  - Identifies the project as Godot 4 and configures the main scene and autoloads.

## Current rendering path

The current implementation already has a colour pipeline. Avoid adding a parallel country-colouring system until the intended semantics of issue #469 are confirmed.

1. `GameSingleton` holds the active `Mapmode`.
2. The colour-image update path calls `MapmodeManager.generate_mapmode_colours(...)` to produce per-province base/stripe colour data.
3. The resulting image is uploaded to an `ImageTexture`.
4. `MapView.tscn` supplies the terrain map shader.
5. `TerrainMap.gdshader` reads province identifiers and the paired base/stripe colours, then blends these with terrain and parchment presentation.
6. The shader applies a separate highlight mix for hovered and selected provinces.

This means the likely implementation point for country colours is the existing map-mode/colour-generation path, not a second overlay in GDScript. A second overlay could disagree with map modes, stripe masks, selection highlighting, or zoom-dependent terrain blending.

## Confirmed implementation detail: political map mode already uses ownership colours

The simulation's `src/openvic-simulation/map/Mapmode.cpp` currently registers `mapmode_political` using the two-item colour helper with `ProvinceInstance::get_owner` and `ProvinceInstance::get_controller`. That is a significant finding: the political map mode already has a path intended to display owner colour as the base and controller colour as the stripe.

The same file's `MapmodeManager::generate_mapmode_colours()` iterates province instances and writes the selected map mode's base/stripe pair into the province-indexed output buffer. In the extension, `GameSingleton::_update_colour_image()` calls this generator and updates the image texture consumed by the map shader.

Consequently, a new country-colour feature should **not** simply duplicate the existing political map mode. The next debugging question is narrower:

1. Are country definition colours being parsed and assigned correctly from the source game data?
2. Does each country instance expose the expected colour at runtime?
3. Does the owner/controller pair resolve to the expected countries for the selected province?
4. Does the generated base/stripe buffer contain the expected RGBA values for that province index?
5. Does the shader display those values as expected after alpha blending with terrain and parchment?

A useful reproduction should include the country identifier, province identifier/number, owner, controller, configured country colour, expected base/stripe colour, actual screenshot, and current map mode. This lets maintainers locate whether the problem is in data loading, simulation state, colour generation, texture upload, or shader presentation.

## Important distinction: country colour vs. province colour

Victoria II-style political maps normally colour provinces according to the country that owns them. A country colour is therefore a source value; a province's displayed colour is the result of applying the active map mode and its ownership/other game-state rules.

Before coding, confirm the expected behaviour with maintainers:

- Does issue #469 mean the default political map mode should show each country's configured colour?
- Should occupied provinces use the controller's colour or the owner's colour?
- Should unowned, wasteland, sea, and invalid provinces keep their current special handling?
- Should base/stripe colours remain available for map modes that use striped overlays?
- Should colour updates occur only when changing map mode, or also when ownership/controller state changes?
- Is a dedicated country-colour map mode expected, or is the default map mode the target?

These decisions affect simulation-to-renderer semantics and should not be guessed in a visual-only patch.

## Existing province labels

`MapText.gd` currently creates `Label3D` nodes from dictionaries returned by `GameSingleton.get_province_names()`. It formats the identifier through `GUINode.format_province_name()`, positions each label using `MapView._map_to_world_coords()`, and applies optional rotation and scale values.

This is useful context for a future country-label task, but country labels are not the same as province labels. Country labels need a stable placement strategy for large and irregular territories, including countries with separated regions. Simply adding one label at each province centre would create dense overlapping text and would not be a reliable country-label implementation.

Potential future country-label requirements:

- Resolve a localised display name rather than showing a raw identifier.
- Choose an anchor point for each country that lies inside its visible territory.
- Avoid placing labels on water or over unrelated countries.
- Use zoom-aware visibility and scale limits.
- Avoid collisions between neighbouring country labels.
- Recompute or hide labels when the selected map mode makes them inappropriate.
- Keep province labels and country labels independently configurable.
- Test small countries and overseas territories, not only large contiguous countries.

These are follow-up design notes, not claims that country labels are currently implemented.

## Shader observations

The map shader already has explicit handling for:

- terrain texture selection;
- a land tint and water tint;
- paired province base and stripe colours;
- parchment overlay blending;
- hover and selected-province highlighting;
- conversion from sRGB to linear colour space.

The shader's highlight amount is calculated separately from the base province colour. This separation is desirable: a country-colour change should not silently remove hover feedback, selected-province feedback, or water handling.

Do not tune the highlight intensity as part of a country-colour change unless there is a reproducible visual defect and a separate testable rationale. Combining unrelated visual adjustments makes review and regression diagnosis harder.

## Proposed implementation sequence

### Phase A — establish expected behaviour

1. Read the current wording and comments on issue #469 and its parent tracker.
2. Ask maintainers to clarify the intended political map mode if the expected ownership/controller semantics are ambiguous.
3. Locate the simulation-side country colour definitions and the map-mode code that consumes them.
4. Confirm whether the active map mode already provides the intended colour or whether a missing map mode/colour rule is the actual gap.
5. Write down the expected output for owned, occupied, unowned, sea, and invalid provinces.

### Phase B — implement at the existing source of truth

1. Prefer the existing map-mode colour generation API.
2. Keep the existing paired base/stripe texture format intact unless the map-mode API itself is deliberately being changed.
3. Preserve the current texture dimensions and index mapping; the shader relies on the province index to look up the correct colour pair.
4. Keep water/invalid-province handling explicit.
5. Ensure the texture refreshes after any game-state change that can affect the political map.
6. Avoid per-frame allocation or creation of a new label/node for every province.
7. Keep the change narrowly scoped so a maintainer can reason about the visual difference.

### Phase C — verification

At minimum, compare before/after output for:

- two adjacent countries with distinct configured colours;
- provinces belonging to the same country;
- an occupied province, if the data and map mode support that state;
- sea and invalid province index zero;
- a province with a stripe colour;
- hovered province;
- selected province;
- a province that is both hovered and selected;
- switching away from and back to the political map mode;
- zooming between detailed terrain and parchment views.

The shader's terrain/parchment blending means a colour can appear different at different zoom levels even when the underlying colour data is correct. Validate the colour source and the final rendered output separately.

## Suggested automated/manual checks

- Run the repository's documented formatting, lint, and test commands for any changed language.
- Build the GDExtension if C++ or binding code changes.
- Launch the Godot project with the project's supported game data and inspect the map.
- Record the exact map mode, zoom level, province/country identifiers, and expected versus actual colour for visual defects.
- If a screenshot is attached to an issue or PR, include the game version/commit and the steps needed to reproduce it.
- Avoid committing generated assets or screenshots unless they are explicitly required by the contribution.

## Review checklist for a future code patch

- [ ] The patch addresses the maintainer-confirmed meaning of issue #469.
- [ ] Country colour values come from the existing simulation/map-mode source of truth.
- [ ] Province indexing and base/stripe pairing are unchanged or deliberately migrated.
- [ ] Water and invalid provinces remain correct.
- [ ] Hover and selection highlights still work.
- [ ] Map-mode changes refresh the visible colours.
- [ ] No unnecessary per-frame allocations were introduced.
- [ ] Relevant build/tests were run, and any unavailable runtime test is stated clearly.
- [ ] A before/after screenshot or precise manual reproduction steps are included when useful.

## References

- [Issue #469 — Add Map UI](https://github.com/OpenVicProject/OpenVic/issues/469)
- [Issue #468 — Add Victoria 2 Compatibility](https://github.com/OpenVicProject/OpenVic/issues/468)
- [MapView.gd](https://github.com/OpenVicProject/OpenVic/blob/master/game/src/Systems/Session/Map/MapView.gd)
- [MapView.tscn](https://github.com/OpenVicProject/OpenVic/blob/master/game/src/Systems/Session/Map/MapView.tscn)
- [MapText.gd](https://github.com/OpenVicProject/OpenVic/blob/master/game/src/Systems/Session/Map/MapText.gd)
- [TerrainMap.gdshader](https://github.com/OpenVicProject/OpenVic/blob/master/game/src/Systems/Session/Map/TerrainMap.gdshader)
- [GameSingleton.hpp](https://github.com/OpenVicProject/OpenVic/blob/master/extension/src/openvic-extension/singletons/GameSingleton.hpp)
- [GameSingleton.cpp](https://github.com/OpenVicProject/OpenVic/blob/master/extension/src/openvic-extension/singletons/GameSingleton.cpp)

## Scope and limitations

This is an architecture/research note, not an implementation of country colours. The repository was inspected remotely; no local Godot launch, screenshot capture, C++ build, or automated test run was performed for this note. The next meaningful code contribution should follow the confirmed requirements above and be tested against the project's real game data.
