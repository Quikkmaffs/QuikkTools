# QuikkTools Blueprint Bridge Plan

This document is the implementation reference for a future chat where we start building the feature. It records the current design decisions so the implementation can begin from a stable baseline instead of re-deriving the tool shape from scratch.

## Summary

`QuikkTools Blueprint Bridge` will be an editor-only diagnostics/export feature inside `QuikkToolsEditor` that turns Unreal Blueprint clipboard text and selected asset context into structured, LLM-friendly exports.

The system will have two layers:

- A reusable parse/render foundation, vendored into the `QuikkTools` repo from an MIT-licensed upstream such as `UEBlueprint`.
- A `QuikkTools`-owned bridge layer that slices, summarizes, warns, and exports data for Codex-first debugging workflows.

The Unreal-side schema stays provider-neutral, but the first consumer we optimize for is Codex through a separate lightweight local Codex companion plugin.

## Design Decisions Already Locked

- Home for the Unreal feature: `Plugins/QuikkTools`
- Unreal module scope: `QuikkToolsEditor` only
- Export output: Markdown + JSON
- User workflow in v1: explicit slices, not giant whole-graph exports
- Codex strategy: Codex-first companion, neutral core schema
- Reuse strategy: vendor a pinned snapshot of `UEBlueprint` into the `QuikkTools` repo instead of rebuilding the parser/viewer from zero
- Asset/package parsing: `uasset-reader-js` is a possible later add-on, not the v1 core path

## Why This Direction

The most important insight from studying blueprintUE and similar tools is that the raw Unreal clipboard text is already rich enough to reconstruct the graph. The useful architecture is:

1. Raw clipboard text
2. Lossless parser
3. Normalized internal graph model
4. Slice and warning analysis
5. LLM-friendly export

That means the bridge should not try to summarize the raw pasted text directly. It should first normalize the text into an internal graph representation, then generate smaller and more intentional exports from that graph.

## Product Naming

### Unreal Feature Name

- `QuikkTools Blueprint Bridge`

### Menu Group

- `QuikkTools -> Diagnostics -> Blueprint Bridge`

### Initial Actions

- `Export Selected Asset Summary`
- `Export Selected Graph Slice`
- `Trace Selected Variable`
- `Export Selected Data Snapshot`
- `Open Latest Bridge Export`

### Console Commands

- `QuikkTools.ExportSelectedAssetSummary`
- `QuikkTools.ExportSelectedGraphSlice`
- `QuikkTools.TraceSelectedVariable`
- `QuikkTools.ExportSelectedDataSnapshot`

### Export Kinds

- `asset_summary`
- `graph_slice`
- `variable_trace`
- `data_snapshot`

### Codex Companion Plugin

- plugin name: `quikk-blueprint-bridge`
- display name: `Blueprint Bridge for Codex`

### Codex Companion Skill Names

- `inspect-blueprint-export`
- `find-blueprint-blocker`
- `summarize-data-ui-mismatch`

## Repo Structure

### Unreal / QuikkTools

- `Plugins/QuikkTools/Source/QuikkToolsEditor/`
  The main Unreal integration, menu entries, export commands, and bridge logic.
- `Plugins/QuikkTools/Resources/Web/UEBlueprint/` or `Plugins/QuikkTools/ThirdParty/UEBlueprint/`
  Vendored upstream parser/viewer snapshot.
- `Plugins/QuikkTools/THIRD_PARTY_NOTICES.md`
  Upstream URLs, pinned commit/tag, license text, and any local modifications.

### Export Output

Exports should go to:

- `Exports/BlueprintBridge/<AssetName>__<ExportKind>__<YYYYMMDD-HHMMSS>/`

Each bundle should contain:

- `report.md`
- `bridge.json`
- `manifest.json`

### Codex Companion

Planned as a local Codex plugin outside this repo when implementation starts:

- `~/plugins/quikk-blueprint-bridge/`

## Architecture

### Layer 1: Parse / View Foundation

The vendored `UEBlueprint` layer is used as a starting point for:

- parsing Unreal clipboard text
- reconstructing nodes, pins, links, and layout
- keeping the representation close to the source text
- optionally showing a visual debug view later

This layer is treated as third-party foundation code, not the place where project-specific behavior lives.

### Layer 2: QuikkTools Bridge IR

`QuikkToolsEditor` will translate the parsed graph into a bridge-owned normalized model.

The normalized model should keep:

- asset identity and type
- node class/type
- node title / member name
- pin identity and direction
- pin category / type
- links between pins
- literal values
- array operations
- loop nodes and loop-bound sources
- variable get/set usage
- widget references
- selected data asset fields

The normalized model should intentionally drop or de-emphasize:

- visual layout noise when not needed
- editor-only cosmetic metadata
- duplicate formatting details
- comment box geometry unless useful for slicing context

### Layer 3: Slice / Diagnose / Export

The bridge layer adds:

- targeted slicing
- mismatch warnings
- Markdown reporting
- JSON serialization
- export bundle creation

This is the real value of the feature for Codex and future LLM workflows.

## v1 Behavior

### `Export Selected Asset Summary`

Exports:

- asset path and asset class
- exposed/default variables
- widget references
- graph entry points
- detected loops
- likely warnings

Use when the user wants a high-level picture before drilling deeper.

### `Export Selected Graph Slice`

Input:

- current selected Blueprint nodes

Expansion rules:

- include selected nodes
- include one hop upstream and downstream across exec and data pins
- always include loop-bound contributors
- always include variable and literal nodes feeding index/count math

Hard cap:

- stop expanding after 150 nodes
- emit a truncation warning instead of failing

### `Trace Selected Variable`

Exports:

- all get/set references for the chosen variable
- nearest control-flow context
- nearby array length / index / assignment logic

Use when debugging a suspect variable like `DisplayedWidgets` or `Experiences`.

### `Export Selected Data Snapshot`

Exports:

- data asset fields
- array lengths
- per-element counts where appropriate
- referenced asset paths

Use when comparing source data to UI or graph capacity.

## Warning Heuristics For v1

The first version should explicitly call out:

- array length mismatches
- widget slot count smaller than backing data count
- loop bounds driven by constants
- loop bounds driven by a smaller helper array
- missing widget/property references
- truncated graph slices

The warning system should be deterministic and generated before any LLM sees the output.

## Export Format

### `manifest.json`

Purpose:

- identify schema version
- identify export kind
- identify source asset
- list generated files
- include generation timestamp and tool version

Top-level keys:

- `schemaVersion`
- `tool`
- `exportKind`
- `asset`
- `engineVersion`
- `generatedAt`
- `files`
- `warnings`

### `bridge.json`

Purpose:

- machine-readable normalized graph/data representation

Top-level keys:

- `schemaVersion`
- `producer`
- `exportKind`
- `asset`
- `selection`
- `variables`
- `loops`
- `warnings`
- `nodes`
- `edges`
- `relatedAssets`

### `report.md`

Purpose:

- compact human/LLM-readable explanation layer

Recommended section order:

- asset summary
- relevant variables
- loops and bounds
- data-flow summary
- warnings
- optional raw node appendix

## Engine Version Handling

The bridge should preserve exact engine version metadata when known from the editor context.

The parser layer should also support format-era detection from the text itself when possible, similar to blueprintUE-style handling of serialization differences. The version should be treated as:

- explicit metadata when we know it from Unreal
- a parser compatibility hint
- not the only source of truth for clipboard-text interpretation

## Third-Party Reuse Rules

### Vendoring Strategy

- vendor a pinned snapshot of `UEBlueprint` into the `QuikkTools` repo
- do not rely on runtime downloads
- do not keep the dependency only in a separate unmanaged location
- prefer a vendored snapshot or subtree-style workflow over a Git submodule

### Documentation Requirements

The vendored code must ship with:

- upstream repository URL
- pinned commit or tag
- original license
- any local modification notes

### Ownership Split

- vendored `UEBlueprint`: parser/viewer foundation
- `QuikkToolsEditor`: Unreal integration and bridge logic
- Codex companion plugin: consumer guidance and workflow prompts

## Codex Companion Plugin Shape

The Codex plugin is intentionally lightweight in v1.

It should provide:

- a plugin manifest
- skills that explain how to inspect a bridge export
- starter prompts for debugging
- instructions to read `report.md` first and `bridge.json` second

It should not require:

- custom MCP servers
- runtime app connectors
- direct Unreal integration

The bridge remains file-based in v1.

## Suggested Implementation Phases

### Phase 1: Foundation

- vendor `UEBlueprint`
- document upstream and licensing
- prove we can parse large clipboard text locally
- prove we can access a normalized graph structure from the vendored layer

### Phase 2: QuikkTools Integration

- add Blueprint Bridge menu entries to `QuikkToolsEditor`
- add export command plumbing
- write bundle creation code in `Exports/BlueprintBridge/`

### Phase 3: Bridge IR

- map parsed nodes and pins into bridge-owned JSON
- create `manifest.json`, `bridge.json`, and `report.md`
- preserve engine version and asset identity metadata

### Phase 4: Diagnostics

- implement explicit-slice export behavior
- implement warning heuristics
- confirm that the carousel-style 5-vs-10 mismatch becomes obvious in the output

### Phase 5: Codex Companion

- scaffold local Codex plugin
- add bridge-aware skills/prompts
- test the export bundle in a fresh Codex chat

## Validation And Tests

Minimum validation expectations:

- parser handles real copied Blueprint text from this project
- graph slice export respects node cap and warns when truncated
- variable trace captures all get/set usages for a selected variable
- data snapshot reports array lengths accurately
- export filenames and bundle layout are deterministic
- Markdown report is understandable without looking at raw Blueprint text

Project-specific acceptance case:

- export a graph slice from `BP_UMG_Carousel`
- export a data snapshot from `DA_Experiences_Default`
- confirm the report clearly shows the 5 display slots vs 10 data entries mismatch

## Non-Goals For v1

- editing Blueprint graphs from the bridge
- round-tripping a modified LLM export back into Unreal
- deep `.uasset` binary inspection as the primary path
- automatic symptom inference from logs without a user-selected scope
- remote/cloud transport between Unreal and Codex

## Assumptions To Keep Unless We Revisit Them

- v1 is editor-only
- v1 is Windows-first
- v1 uses explicit user-driven scope selection
- exports are file-based
- the bridge schema is neutral, but the first workflow docs target Codex
- `UEBlueprint` is reused as a foundation, not as the final product identity

## Implementation Start Note

When implementation begins in a fresh chat, start from this document and treat it as the current source of truth unless a newer note explicitly replaces it.
