# QuikkTools

Reusable runtime and editor utilities for Unreal Engine projects.

`QuikkTools` is intended as a shared plugin for practical project helpers that do not belong inside a single game module. The plugin is currently split into a runtime module for packaged-build-safe Blueprint helpers and an editor module for editor-only tools.

## Modules

### `QuikkToolsRuntime`

Runtime-safe Blueprint helpers that can be used in packaged builds.

Current library:

- `UQuikkFileAccessLibrary`

Current nodes:

- `Resolve File Path`

This node is meant for path validation and file-access diagnostics before debugging higher-level systems such as media playback, parsing, or asset loading.

Typical use case:

1. Enter a base folder path directly on the node.
2. Provide only a file name such as `video.mp4`.
3. Inspect the returned path/access results.

Useful base folder examples:

```text
Android OBB: /storage/emulated/0/Android/obb/com.genericapp.vr/
Android App Files: /storage/emulated/0/Android/data/com.genericapp.vr/files/
Windows: C:/Projects/MyApp/Content/Movies/
```

Returned diagnostics include:

- `Full Path`
- `Exists`
- `Accessible`
- `File Size`
- `Can Open Read`
- `Can Read Bytes`
- `Diagnostic`

The helper also writes a log line containing:

```text
File access test:
```

### `QuikkToolsEditor`

Editor-only utilities.

Current library:

- `UQuikkProjectLogLibrary`

Current features:

- `Export Current Project Log` Blueprint/editor utility
- `Tools -> Export Current Project Log` menu entry
- Main Level Editor toolbar dropdown: `QuikkTools`
- `QuikkTools -> Export Current Project Log` toolbar menu entry
- `QuikkTools.ExportCurrentProjectLog` console command

This exports the current project log into the host project's `Exports` folder using the naming pattern:

```text
<ProjectName>__<BuildTargetName>__<BuildConfiguration>__<YYYYMMDD-HHMMSS>.log
```

## Design Principles

- Keep reusable runtime helpers in `QuikkToolsRuntime`.
- Keep editor-only tools in `QuikkToolsEditor`.
- Avoid project-specific names and assumptions inside the plugin.
- Prefer helpers that are portable across Android, Windows, and future projects.
- Treat the plugin as a toolbox, but avoid turning it into a junk drawer.

## Current Scope

What is already included:

- Generic file/path access helpers for Blueprint
- Editor project log export utilities

What may be added later:

- Additional Android storage/path helpers
- Small packaged-build diagnostic helpers
- More editor-side project maintenance tools

## Integration Notes

- The plugin descriptor is `QuikkTools.uplugin`.
- Runtime code can depend on `QuikkToolsRuntime`.
- Editor code can depend on `QuikkToolsEditor`.

If you are migrating older project-specific helpers into this plugin, keep temporary wrappers marked as deprecated until Blueprints or code references have been updated.
