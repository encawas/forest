# EchoForest Real Hardware Handoff Index

Use this page as the entry point for the next single goal-mode Codex run.

## Read in this order

1. `tasks/TASK_REAL_HARDWARE_INTEGRATION.md`
   - Single bounded harness task.
   - Defines the final real S3/C5 field-debuggable result, constraints, verification and acceptance criteria.

2. `docs/HARDWARE_SOURCE_OF_TRUTH.md`
   - Authoritative latest S3 and C5 hardware facts.
   - Latest received S3 source package takes precedence over older inferred pin maps.
   - Current repository API and wire enums remain authoritative.

3. `docs/FIELD_HARDWARE_CHECKLIST.md`
   - Physical wiring, power, network, sensor calibration and first onsite smoke sequence.
   - Intended for the person physically checking or rewiring simple connections.

4. `docs/ANIMATION_TEST_ASSET_SPEC.md`
   - Requirements for a license-safe 240 x 320 forest-growth smoke animation.
   - Defines frame count, RGB565/RLE compatibility, conversion reproducibility, playback behavior and size checks.

5. `docs/C5_PACKAGE_CAPABILITY_SUPPLEMENT.md`
   - Verified C5 base-package LCD, touch, Wi-Fi and TTS capabilities.

6. `hardware_reference/README.md`
   - How to reconstruct and inspect the accepted S3 reference project.

## Precedence

When information conflicts:

1. Latest accepted real hardware package/source.
2. `docs/HARDWARE_SOURCE_OF_TRUTH.md`.
3. Current backend code and `docs/API_CONTRACT.md` for endpoints, payload names and wire values.
4. Older planning documents and inferred wiring notes.

Do not reintroduce the older `/api/session/*` contract, older forest-state names, or snake_case `device_id` into active code.

## Next action

Pull the latest default branch and run the task in:

```text
tasks/TASK_REAL_HARDWARE_INTEGRATION.md
```

This is one goal-mode task. It may internally plan and checkpoint, but it should continue through all repository-completable implementation, testing, ESP-IDF build integration, animation conversion support, and field runbook preparation.

Physical outcomes must remain explicitly labeled `NOT YET FIELD-TESTED` until observed on the real boards.
