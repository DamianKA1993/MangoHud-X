# Changelog

All notable changes to **MangoHud-X** will be documented in this file.

## [1.0.0] - 2026-09-07

### Added
* **Custom Layout Engine**: Introduced support for structured `[window]` and `[layout]` blocks in the configuration file, allowing custom rows, columns, and layout definitions.
* **Brand Color Auto-Detection**: Added dynamic color matching (`color = #brand`) based on the active GPU vendor (NVIDIA green, AMD red, Intel blue).
* **Granular Typography & Opacity**: Added support for custom font scaling (`font = big`, `font = small`) and per-column opacity controls.
* **Window Styling Options**: Added controls for window rounding (`round`), background color  (`background`).

### Fixed
* **Anchor Offsets**: Corrected coordinate calculation bugs for window positioning relative to the `bottom` and `right` screen edges.
