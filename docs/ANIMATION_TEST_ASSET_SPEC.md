# EchoForest S3 Animation Test Asset Specification

## 1. Goal

Provide a small, license-safe, clearly visible animation that proves the real S3 ST7789 screen can initialize, decode frames, play at a controlled speed, and coexist with sensor upload and backend polling.

The animation is a hardware smoke-test asset, not the final art direction.

## 2. Required visual content

Use a simple forest-growth sequence that is easy to recognize on a 240 x 320 screen:

```text
empty soil -> seed -> sprout -> small plant -> young tree -> healthy tree
```

The sequence should visibly change between frames and should not rely on subtle gradients.

Allowed sources:

- project-generated procedural art;
- AI-generated art created specifically for EchoForest;
- original team-created art;
- CC0/public-domain art with provenance recorded.

Do not copy arbitrary copyrighted animation or game assets.

## 3. Technical target

| Item | Required value |
|---|---|
| Display | ST7789, 240 x 320 |
| Color | RGB565 compatible |
| Orientation | Match verified S3 panel orientation |
| Frame count | 8-16 frames |
| Frame delay | Configurable, default 100 ms |
| Total visible loop | Approximately 0.8-2.0 seconds |
| Playback | Local on S3; no network frame streaming |
| Memory | Must fit the final 2 MB-flash build |

The existing reference package already demonstrates full-screen RLE `{count, color}` frames. That format may be reused if the final build size remains acceptable.

## 4. Required source and generated artifacts

The integration task should keep both:

1. Human-editable source frames, preferably PNG.
2. Generated firmware representation, such as RLE C arrays or a binary asset partition.
3. A reproducible conversion script.
4. A size report showing source size, generated size, and final firmware size impact.

Do not manually paste opaque frame data without a reproducible generator.

## 5. Playback behavior

Required APIs or equivalent behavior:

```c
animation_init();
animation_play_once(animation_id);
animation_set_frame_delay_ms(...);
animation_stop();
```

Playback requirements:

- LCD must be initialized first.
- Invalid frame index or asset corruption must fail safely.
- Animation must not run inside an unbounded tight loop.
- Sensor sampling, heartbeats, environment uploads, and backend polling must continue during playback.
- The test must be stoppable or time-bounded.
- A fallback static screen must remain available.

## 6. Acceptance test

The animation test passes when:

1. S3 boots and shows a solid-color screen test.
2. The complete forest-growth sequence is visibly recognizable.
3. Playback speed is human-visible and configurable.
4. The sequence loops or plays once without crash, watchdog reset, tearing severe enough to obscure the scene, or corrupted colors.
5. During playback, S3 continues to upload environment data and receive backend state.
6. `idf.py size` confirms the final build fits configured flash with reasonable margin.
7. The source frames, conversion script, generated output, and asset provenance are committed.

## 7. State mapping after smoke test

Once the smoke asset works, backend states may select local visuals:

| Backend state | Local visual intent |
|---|---|
| `idle` | quiet waiting forest |
| `growing` | growth animation |
| `light_warning` | dim scene or lamp cue |
| `noise_warning` | shaking/noise cue |
| `comfort_warning` | temperature/humidity warning cue |
| `break` | resting forest |
| `completed` | completion celebration |

Only `growing` needs a true multi-frame animation for the first onsite test. Other states may use clear static screens until the full art set is ready.
