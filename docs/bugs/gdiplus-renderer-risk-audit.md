# GDI+ Renderer Risk Audit

Date: 2026-06-08

## Summary

The recurring CrossOver freeze reports no longer look card-specific. The current
evidence points at legacy renderer/helper DLLs entering Wine/GDI+ callback,
codec, or worker-thread paths while Shandalar is showing cards, Spell Chain, or
announcement panels.

The active repo and local `MTG` bottle now have only two GDI+-importing active
renderer DLL families:

| Runtime helper | Role | Current mitigation |
| --- | --- | --- |
| `Drawcardlib.dll` / `Program/Drawcardlib.dll` | Draws full/small card frames, mana symbols, counters, overlays, and calls `CardArtLib` for large art. | Suppresses GDI+ background thread and external codecs, calls notification hook/unhook explicitly, checks hook status, keeps PE `DllCharacteristics` `0x0000`. |
| `CardArtLib.dll` / `Program/CardArtLib.dll` | Loads and draws `CardArtManalink/*.jpg` large art. | Starts GDI+ lazily outside `DllMain`, suppresses GDI+ background thread and external codecs, checks hook status, validates image load status before caching, keeps PE `DllCharacteristics` `0x0000`. |

Use `tools/audit-gdiplus-renderers.sh .` as the first static check before and
after future renderer changes.

## Current Evidence

The repeated freeze signature was a worker-thread page fault or invalid execution
target with `EIP == EDX == 0xfff50de4`. This remained after several card-rule and
sound/video patches, while frozen-process inspection repeatedly showed
renderer/card-art modules loaded.

The strongest renderer-specific observations were:

| Scenario | Relevant observation | Follow-up |
| --- | --- | --- |
| Thorn Thallid Spell Chain freeze | `Drawcardlib.dll`, `CardArtLib.dll`, GDI+, and `CardArtManalink/Thorn Thallid.jpg` were live; `MagSnd.dll` and `magvid.dll` were absent. | Rebuilt Drawcardlib with explicit GDI+ notification hook/unhook handling and legacy-safe PE flags. |
| Brilliant Halo Spell Chain freeze | `CardArtManalink/Brilliant Halo.jpg`, `CardArtLib.dll`, GDI+, and Windows codec modules were live. | Rebuilt CardArtLib so GDI+ starts lazily, suppresses external codecs, and validates image loads. |
| Follow-up audit | Drawcardlib still had `SuppressExternalCodecs = 0` and ignored the notification hook status. | Rebuilt Drawcardlib with `SuppressExternalCodecs = 1` and hook-status checking. |

## Remaining Risks

| Risk | Why it matters | Suggested next move |
| --- | --- | --- |
| Drawcardlib still has many direct `Gdip*` calls. | Most callsites do not check `GpStatus`, so a failed graphics/image operation can cascade into null or invalid GDI+ objects. | Add small checked wrapper helpers for creation, clone, lock, draw, and graphics-context calls; convert high-frequency paths first. |
| Drawcardlib and CardArtLib have independent GDI+ lifecycles. | Cross-DLL cleanup order is already fragile; Drawcardlib explicitly destroys CardArtLib images before shutting its own GDI+ session down. | Keep shutdown order documented and audited; consider a shared renderer lifecycle source module if more GDI+ code changes are needed. |
| Runtime proof is still static/loader-heavy. | Passing hashes, PE flags, and bounded launch smoke does not prove Spell Chain gameplay stability. | Retest visible Spell Chain scenarios after each renderer rebuild: ordinary spell, aura on creature, activated ability, ETB target, and start-of-duel card display. |
| Full GDI+ removal would be a larger renderer rewrite. | GDI+ currently handles alpha compositing, scaling, image attributes, and large-art JPEG decoding. | Do not jump straight to replacement unless checked wrappers plus lifecycle hardening still fail. A staged replacement would start with CardArtLib large-art drawing. |

## Modification Rules

Future renderer changes should preserve these invariants:

| Invariant | Check |
| --- | --- |
| GDI+ must not start in `DllMain`. | `tools/audit-gdiplus-renderers.sh .` |
| GDI+ background thread must be suppressed and notification hook/unhook used. | `tools/audit-gdiplus-renderers.sh .` |
| External GDI+ codecs must be suppressed unless a visible test proves they are required. | `tools/audit-gdiplus-renderers.sh .` |
| Renderer DLLs must keep PE `DllCharacteristics` `00000000`. | `tools/verify-install-tree.sh .` and `tools/verify-crossover-mtg-state.sh` |
| Root, Program, and local bottle renderer hashes must match after deployment. | `shasum -a 256 Drawcardlib.dll Program/Drawcardlib.dll CardArtLib.dll Program/CardArtLib.dll` plus bottle paths. |

## Larger Fix Options

| Option | Work | Risk | Notes |
| --- | --- | --- | --- |
| Audit gate plus targeted lifecycle hardening | Low | Low | Current path. Keeps the game behavior closest to the existing renderer. |
| Checked GDI+ wrapper helpers in Drawcardlib | Medium | Medium | Best next engineering step if another renderer freeze appears. Improves failure handling without changing rendering architecture. |
| Shared renderer lifecycle source module | Medium | Medium | Reduces policy drift between C and C++ renderers, but still leaves each DLL with its own linked copy unless a new runtime DLL is introduced. |
| Replace CardArtLib GDI+ JPEG path first | Medium-high | Medium-high | Narrowest GDI+ removal slice because CardArtLib mostly loads and scales large JPG art. Needs image decoding and stretch/alpha fallback decisions. |
| Remove GDI+ from Drawcardlib | High | High | Requires replacing frame scaling, alpha blending, image attributes, and generated GDI+ bitmap logic. Save this for repeated failures after wrappers. |
