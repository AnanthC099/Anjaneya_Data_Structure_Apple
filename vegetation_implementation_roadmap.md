# V6 Vegetation Implementation Roadmap
### Priority-Ordered Reading & Implementation Plan for Valley + Forest + Village Demo
#### Target: Vulkan 1.3 · RTX 4090 · 1440p @ 60fps · GPU-Driven Pipeline

---

## Current State

Terrain heightmap-displaced plane in Vulkan is the immediate execution target. Everything below builds on top of that foundation. Each phase lists **what to read**, **what to implement**, and **the exit condition** before moving to the next phase.

---

## PHASE 0 — Terrain Surface (CURRENT)
> *The ground everything grows on. You're here now.*

### Read
| Priority | Source | Chapter/Section |
|----------|--------|-----------------|
| 1 | GPU Pro 4 | Part I, Ch 2 — GPU Terrain Subdivision and Tessellation |
| 2 | GPU Gems 2 | Ch 26 — Implementing Improved Perlin Noise |
| 3 | Texturing & Modeling: A Procedural Approach | Ch 16 — Procedural Terrain (Musgrave) *(already studied)* |
| 4 | GPU Pro 1 | Part II, Ch 1 — Quadtree Displacement Mapping with Height Blending |
| 5 | GPU Zen 2 | Part I, Ch 4 — Procedural Stochastic Textures by Tiling and Blending |

### Implement
- Heightmap-displaced mesh with tessellation (Vulkan tessellation shaders)
- Multi-texture terrain blending (grass/dirt/rock) with height-based rules
- Stochastic texture tiling to kill visible repeats
- **Terrain density map output** — a texture that encodes "where can vegetation grow" (slope, altitude, moisture). This drives everything in later phases.

### Exit Condition
✅ Tessellated terrain rendered in Vulkan with blended materials and a vegetation density map exported as a texture.

---

## PHASE 1 — Grass: Basic Blades on Terrain
> *The single highest visual-impact vegetation element. A grassy valley reads as "nature" immediately.*

### Read (in this order)
| Priority | Source | Chapter/Section |
|----------|--------|-----------------|
| 1 | **GPU Gems 1** | **Ch 7 — Rendering Countless Blades of Waving Grass** |
| 2 | ShaderX2 | Section 4.1 — Animated Grass with Pixel and Vertex Shaders |
| 3 | ShaderX3 | Section 2.8 — Rendering Grass Terrains in Real-Time |
| 4 | ShaderX4 | Section 2.6 — Real-Time Rendering of Realistic-Looking Grass |
| 5 | GPU Pro 6 | Part II, Ch 2 — Rendering Grass in Real-Time with Dynamic Lighting |

### Implement
- **Grass blade mesh generation via Vulkan compute shader** — output vertex buffer of blade quads/triangles
- Sample the Phase 0 density map to control placement and density
- Per-blade vertex displacement for basic wind sway (sin-based, no wind field yet)
- Alpha-to-coverage instead of alpha test (avoid harsh edges)
- Basic Lambert + wrap lighting on blades

### Exit Condition
✅ Dense grass field covering the terrain, gently swaying, lit by a single directional sun. Millions of blades at 60fps via compute-generated geometry.

---

## PHASE 2 — Grass: LOD & GPU-Driven Rendering
> *Phase 1 grass won't scale to a full valley. This phase makes it production-viable.*

### Read
| Priority | Source | Chapter/Section |
|----------|--------|-----------------|
| 1 | **GPU Pro 7** | **Part II, Ch 4 — Grass Rendering and Simulation with LOD** |
| 2 | GPU Pro 5 | Section 2 — Screen-Space Grass |
| 3 | GPU Zen 1 | Part II, Ch 2 — Rendering Convex Occluders with Inner Conservative Rasterization |
| 4 | Real-Time Rendering 4th Ed | Section 19.10 — GPU-Driven Rendering Pipeline |

### Implement
- **3-tier grass LOD**: geometry blades (near) → billboard crosses (mid) → terrain texture blend (far)
- GPU-driven indirect draw: compute shader culls grass patches against frustum + occlusion, writes `VkDrawIndirectCommand`
- Hierarchical-Z occlusion culling for grass patches
- Smooth LOD crossfade (dither or alpha blend between tiers)

### Exit Condition
✅ Full valley of grass rendering at 60fps with camera flying from close-up blade detail to distant overview. No popping, no framerate drops.

---

## PHASE 3 — Tree Geometry & Basic Rendering
> *Trees define the forest. Start with the geometry pipeline before worrying about shading.*

### Read
| Priority | Source | Chapter/Section |
|----------|--------|-----------------|
| 1 | **GPU Gems 3** | **Ch 4 — Next-Generation SpeedTree Rendering** |
| 2 | GPU Gems 2 | Ch 1 — Toward Photorealism in Virtual Botany |
| 3 | ShaderX6 | Section 2.3 — Interactive Rendering of Trees |
| 4 | Graphics Gems III | Ch 2.4 — Realistic Modeling of Plant Life |
| 5 | Graphics Gems IV | Ch 2.5 — Procedural Generation of Trees and Branches |

### Implement
- **Tree mesh pipeline**: trunk (cylinder mesh with noise displacement), branches (recursive subdivision or L-system), leaf cards (camera-facing quads with alpha leaf textures)
- At least 3-4 tree species (banyan, mango, neem, coconut palm — for Indian village authenticity)
- Billboard impostor generation: render each tree from 8-16 angles into an impostor atlas
- **3-tier tree LOD**: full mesh (near) → simplified mesh (mid) → billboard impostor (far)
- Instanced rendering with per-instance transforms from a placement buffer

### Exit Condition
✅ A grove of 50-100 trees rendered with LOD transitions, instanced. No leaf shading polish yet — just geometry and basic diffuse.

---

## PHASE 4 — Foliage Shading: Translucency & Leaf Lighting
> *This is what makes vegetation look alive instead of like plastic.*

### Read
| Priority | Source | Chapter/Section |
|----------|--------|-----------------|
| 1 | **GPU Gems 3** | **Ch 16 — Vegetation Procedural Animation and Shading in Crysis** |
| 2 | **GPU Gems 1** | **Ch 16 — Real-Time Approximations to Subsurface Scattering** |
| 3 | ShaderX7 | Section 2.4 — Vegetation Procedural Animation and Shading |
| 4 | Real-Time Rendering 4th Ed | Section 13.7 — Foliage Rendering |
| 5 | Real-Time Rendering 4th Ed | Section 14.7.2 — Vegetation in Landscape Rendering |

### Implement
- **Leaf translucency shader**: back-face lighting using view-dependent wrap term + thickness map. When sun is behind a leaf, light bleeds through with green-gold tint.
- Two-sided leaf lighting (flip normal for back faces, different BRDF per side)
- Terrain color bleeding into trunk/branch base (Crysis technique — vegetation picks up ground albedo at base)
- Ambient occlusion baked per-tree or screen-space
- Specular on waxy leaf surfaces (mango, banyan leaves have visible spec)

### Exit Condition
✅ Trees look "alive" — leaves glow when backlit by sun, trunks darken near ground, waxy specular visible on close-up leaves.

---

## PHASE 5 — Wind System
> *Static vegetation looks dead. Wind is what sells the scene.*

### Read
| Priority | Source | Chapter/Section |
|----------|--------|-----------------|
| 1 | **GPU Gems 3** | **Ch 6 — GPU-Generated Procedural Wind Animations for Trees** |
| 2 | **GPU Gems 3** | **Ch 16 — (wind sections) Crysis vegetation animation** |
| 3 | ShaderX5 | Section 2.5 — Animating Vegetation Using GPU Programs |
| 4 | ShaderX7 | Section 2.4 — (wind response curves section) |

### Implement
- **Global wind field texture**: 2D scrolling noise texture sampled in vertex shader, updated via compute
- **Hierarchical wind response**:
  - Trunk: slow, large-amplitude sway
  - Branches: medium frequency, phase offset per branch
  - Leaves: high frequency flutter, semi-random per leaf card
- Wind gusts: occasional directional bursts that ripple across the field (modify wind texture amplitude in a wave pattern)
- **Grass wind integration**: same wind field texture drives grass blade displacement (unified system)
- Touch bending: player/object proximity bends nearby grass and small branches

### Exit Condition
✅ Full scene (grass + trees) responds to a unified wind field. Gusts visibly ripple across the valley. Grass and tree canopy move coherently.

---

## PHASE 6 — Alpha Handling & Antialiasing for Vegetation
> *Vegetation is the #1 source of aliasing artifacts in real-time rendering. This phase fixes it.*

### Read
| Priority | Source | Chapter/Section |
|----------|--------|-----------------|
| 1 | **Real-Time Rendering 4th Ed** | **Section 6.6 — Alpha Testing and Transparency** |
| 2 | GPU Pro 2 | Part II, Ch 2 — Practical Morphological Antialiasing (MLAA) |
| 3 | Hashed Alpha Testing paper | Wyman & McGuire, 2017 |
| 4 | Ray Tracing Gems I | Ch 9 — Multi-Hit Ray Tracing in DXR |
| 5 | Ray Tracing Gems I | Ch 20 — Texture LOD Strategies for Real-Time Ray Tracing |

### Implement
- **Alpha-to-coverage** on all vegetation (MSAA resolve handles edges)
- Pre-filtered alpha mipmaps (avoid disappearing leaves at distance)
- Hashed alpha testing for temporal stability (no shimmer on vegetation edges between frames)
- Screen-space AA pass (TAA or SMAA) tuned for vegetation
- If using RT shadows for vegetation: multi-hit any-hit shader for correct leaf shadow density

### Exit Condition
✅ Vegetation edges are clean at 1440p. No shimmer during camera motion. Distant trees don't sparkle or lose leaf density.

---

## PHASE 7 — Vegetation Placement & Ecosystem Rules
> *Hand-placing trees doesn't scale. Procedural placement with biome rules creates a believable landscape.*

### Read
| Priority | Source | Chapter/Section |
|----------|--------|-----------------|
| 1 | **Texturing & Modeling: A Procedural Approach** | **Ch 20 — Modeling Plant Ecosystems** |
| 2 | GPU Pro 3 | Part II, Ch 4 — Procedural Content Generation on the GPU |
| 3 | GPU Pro 4 | Part II, Ch 3 — Real-Time Deformable Terrain |

### Implement
- **Poisson disk sampling on GPU** for tree placement — guarantees minimum spacing
- Biome rules driven by terrain data:
  - Altitude → treeline cutoff
  - Slope → no trees on cliffs, grass only
  - Moisture (river proximity) → denser vegetation, different species
- Village clearing zones: mask areas where buildings/paths go, suppress vegetation
- Edge blending: forest→clearing transitions use decreasing density + more shrubs/bushes
- Compute shader outputs a **vegetation instance buffer** (position, species, scale, rotation) consumed by the instanced draw pipeline

### Exit Condition
✅ Entire valley/forest/village landscape has procedurally placed vegetation that looks hand-authored. Dense forest, open clearings, riverside clusters, bare cliff faces — all automatic.

---

## PHASE 8 — Performance: Full GPU-Driven Vegetation Pipeline
> *Tying everything together into a production pipeline that hits 60fps with the full scene.*

### Read
| Priority | Source | Chapter/Section |
|----------|--------|-----------------|
| 1 | GPU Pro 5 | Section 6.4 — Tiled Forward Shading |
| 2 | GPU Pro 6 | Part II, Ch 6 — Real-Time Lighting via Light Linked List |
| 3 | GPU Pro 7 | Part V, Ch 2 — Real-Time BC6H Compression on GPU |
| 4 | GPU Zen 1 | Part II, Ch 4 — Real-Time Layered Materials Compositing |

### Implement
- **GPU-driven indirect rendering for ALL vegetation**: single compute dispatch culls + sorts + writes indirect commands for grass, trees, shrubs
- Async compute: wind field update and vegetation culling on async compute queue while rasterization runs on graphics queue
- Clustered/tiled shading for multiple lights in village area (torches, fires) affecting nearby foliage
- Texture streaming and virtual texturing for vegetation atlas (bark, leaves, grass)
- BC7/BC6H compressed vegetation textures generated offline
- **Performance budget**: measure per-category (grass, trees, wind, shadows) and ensure total vegetation cost < 8ms at 1440p

### Exit Condition
✅ Full Valley + Forest + Village scene with all vegetation systems active, 1440p, sustained 60fps on RTX 4090. GPU timeline shows vegetation fits within budget.

---

## PHASE 9 — Ray-Traced Vegetation (Polish)
> *RTX 4090 flex. RT shadows and GI on vegetation for final quality.*

### Read
| Priority | Source | Chapter/Section |
|----------|--------|-----------------|
| 1 | Ray Tracing Gems I | Ch 9 — Multi-Hit Ray Tracing in DXR |
| 2 | Ray Tracing Gems I | Ch 20 — Texture LOD for RT |
| 3 | Ray Tracing Gems II | Ch 38 — Correct Soft Shadows |
| 4 | Ray Tracing Gems II | Ch 14 — The Reference Path Tracer |
| 5 | PBRT v3 | Ch 11/14 — Volume Scattering *(already studied)* |

### Implement
- RT shadows through foliage: any-hit shader with alpha test, stochastic transparency for canopy density
- 1-bounce diffuse GI for light bouncing off sunlit grass onto tree trunks and village walls
- RT ambient occlusion in forest interior (under canopy darkening)
- Volumetric light shafts through canopy gaps (combine with your volume rendering knowledge)

### Exit Condition
✅ Forest interior has correct dappled light. Canopy casts soft, dense shadows. Light shafts pierce through gaps. Village walls pick up green bounce light from surrounding vegetation.

---

## Quick Reference: Phase → Chapter Mapping

```
PHASE 0 (Terrain)     → GPU Pro 4 I.2, GPU Gems 2 Ch26, Musgrave Ch16, GPU Pro 1 II.1, GPU Zen 2 I.4
PHASE 1 (Basic Grass)  → GPU Gems 1 Ch7, ShaderX2 4.1, ShaderX3 2.8, ShaderX4 2.6, GPU Pro 6 II.2
PHASE 2 (Grass LOD)    → GPU Pro 7 II.4, GPU Pro 5 S2, GPU Zen 1 II.2, RTR4 19.10
PHASE 3 (Tree Geo)     → GPU Gems 3 Ch4, GPU Gems 2 Ch1, ShaderX6 2.3, GG3 2.4, GG4 2.5
PHASE 4 (Leaf Shading) → GPU Gems 3 Ch16, GPU Gems 1 Ch16, ShaderX7 2.4, RTR4 13.7/14.7.2
PHASE 5 (Wind)         → GPU Gems 3 Ch6, GPU Gems 3 Ch16, ShaderX5 2.5, ShaderX7 2.4
PHASE 6 (Alpha/AA)     → RTR4 6.6, GPU Pro 2 II.2, Hashed Alpha, RTG1 Ch9/20
PHASE 7 (Placement)    → Musgrave Ch20, GPU Pro 3 II.4, GPU Pro 4 II.3
PHASE 8 (Perf)         → GPU Pro 5 6.4, GPU Pro 6 II.6, GPU Pro 7 V.2, GPU Zen 1 II.4
PHASE 9 (RT Polish)    → RTG1 Ch9/20, RTG2 Ch14/38, PBRT Ch11/14
```

---

## The Rule

**Every session working on vegetation: have Vulkan code open in your editor.** Read the chapter, then implement the technique. Do not move to the next phase until the exit condition is met with running, rendered output on your RTX 4090.

Total chapters to work through: ~45
Estimated implementation time per phase: 1-3 weeks each
Full vegetation pipeline: ~3-5 months if executing consistently

---

*Document generated for V6 Valley + Forest + Village Demo Pipeline*
*Last updated: March 2026*
