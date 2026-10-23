# gtsrcid association algorithm

This file describes what `gtsrcid` actually computes, step by step, with the
function that implements each step. It complements the overview in
[README.md](README.md). All formulas were read from the code at
`last_reviewed_commit` (`src/gtsrcid/Catalogue.cxx`, `Catalogue_id.cxx`).
The Doxygen comments in those files are older than some of the code, and
where they disagree this file follows the code.

Notation: *k* indexes LAT ("source catalogue") sources, *i* indexes
counterpart candidates, *r* is the angular separation in degrees.

## 0. Loading and position errors

`get_input_descriptor` / `get_input_catalogue` / `set_info` (`Catalogue.cxx`).

Both catalogues are loaded completely into memory with
`catalogAccess::Catalog::import`. For each object the code stores a name,
an RA/Dec position, and a 95 % error ellipse (semi-major *a*, semi-minor *b*,
position angle *φ*), found by **column name**:

| What | Search order (first match wins) |
|------|---------------------------------|
| Name | first column with UCD containing `ID_MAIN`; else a column whose name starts with `NAME`, `ID`, `NICKNAME` or `SOURCE_NAME` |
| Position | UCDs `POS_EQ_RA_MAIN`/`POS_EQ_DEC_MAIN`; columns `RAdeg`/`DEdeg`; `_RAJ2000`/`_DEJ2000`; `RAJ2000`/`DEJ2000`; `RA`/`DEC`; UCDs `POS_GAL_LON`/`POS_GAL_LAT`; `_GLON`/`_GLAT`; `GLON`/`GLAT`; `L`/`B`. Galactic positions are converted to equatorial. |
| Error | `Conf_95_SemiMajor/SemiMinor/PosAng` (ellipse, 95 %); `Conf_68_…` (68 %); `POS_ERR_MAJ/MIN/ANG` (gtsrcid's own output, 95 %); `e_RAdeg`/`e_DEdeg` or `e_RAJ2000`/`e_DEJ2000` (1σ per axis, made into an RA/Dec-aligned ellipse); `theta95` (radius, 95 %); `PosErr68`, `PosErr90`, `PosErr95`, `PosErr99` (radius); `PosErr` (radius, 1σ) |

Name matching (`find()`) is case-insensitive and **prefix** based, and the
shortest matching name wins. So `RA` matches `RA`, `RAJ2000`, `RA_PNT`, …,
and `L` matches any column whose name starts with `L` (it is only reached if
no RA/Dec pair was found).

Error values are rescaled from their confidence level to 2-D 95 %
(`e_pos_scale = e_norm_X / e_norm_95`, based on χ² with 2 degrees of
freedom). They are **not** converted for units: the value is taken to be in
degrees. If no error column was found, or the value cannot be read, the
`srcPosError`/`cptPosError` parameter (in degrees) is used. Errors smaller
than `ERPOSABS = 1e-4 × 2.486 ≈ 2.5e-4°` are raised to that floor.

## 1. Filter (per source) — `cid_filter`

- Filter radius `R = max(4°, 2.5 · a_k)`. The comment calls 2.5·a "5 sigma".
- Keep every counterpart with a valid position inside the RA/Dec box
  `Dec ± R`, `RA ± R / cos Dec` (RA wrap-around is handled; near the poles
  the RA range becomes the full circle).
- This is a linear scan over the whole counterpart catalogue for every
  source, so the cost is O(N_src × N_cpt).
- The density ring used in step 3 is `[0, R]`, and the error-ellipse solid
  angle is `π a b`.

## 2. Pre-selection — `cid_select`

The candidates are written to an in-memory FITS table (`mem://gtsrcid`, the
same columns as the output). Then each non-empty `selectNN` expression is
applied with `fits_select_rows`, and the surviving `ID`s are read back.
At this point the probability and geometry columns (`ANGSEP`, `PROB_*`, …)
are still **0**. See the gotcha in [README.md](README.md#invariants-and-gotchas).

## 3. Refine (per source) — `cid_refine`

In this order:

1. **FoM** (`cid_fom`). If `fom` is non-empty, it is evaluated as a column
   expression and stored in `FOM`. Otherwise `FOM = 0`.
2. **Position** (`cid_prob_pos`). This step computes the separation *r*,
   the position angle θ of the candidate as seen from the source, and the
   effective 95 % radius of the **source** ellipse in that direction:

   Ψ² = [cos²(θ−φ)/a² + sin²(θ−φ)/b²]⁻¹

   PROB_POS = exp(−d r²/Ψ²),  PDF_POS = d · exp(−d r²/Ψ²) / (π a b),
   where d = −ln 0.05 = 2.9957 (`dnorm`).

   The candidate's own position error does **not** enter. It is only copied
   to `POS_ERR_*` in the output. (The Doxygen comment quotes 5.991 and
   2.996. The code uses 2.9957 in both places.)
3. **Counterpart density ρ** (`RHO`, sources per deg²):
   - If no `cptDensFile` is given (`cid_local_density`): ρ is the number
     of candidates (after step 2) with `0 ≤ r ≤ R`, divided by the solid
     angle of that disc. At least one count is assumed, so an empty field
     gives an upper limit. If a FoM is given, each candidate's ρ counts only
     the candidates whose FoM is ≥ its own.
   - If a `cptDensFile` is given and could be read (`cid_map_density`): ρ is
     the value of the HEALPix map pixel at the candidate's position.
     `cid_global_density` (all-sky mean) exists but is commented out.
4. **Chance coincidence** (`cid_prob_chance`): MU = π r² ρ,
   PROB_CHANCE = 1 − e^(−MU), PDF_CHANCE = ρ. The last formula comes from
   the compile-time switch `JEAN_BALLET_FORMULA = 1`; the alternative would
   be ρ·e^(−MU).
5. **Prior** (`cid_prob_prior`): `PROB_PRIOR` is the `probPrior` expression
   evaluated per row (for example a constant `0.01`, or a formula in
   catalogue columns), clamped to [0, 1]. In catch-22 mode it is the current
   global prior (initially 0.1).
6. **Single-source posterior** (`cid_prob_post_single`):

   LOGLR = ln PDF_POS − ln PDF_CHANCE (0 if either PDF is 0, and then the
   posterior is 0),
   η = P_prior / (1 − P_prior),
   PROB_POST_SINGLE = 1 / (1 + 1/(LR·η)), computed in log space with
   overflow guards.

   A "divergent LR" flag is set when `d/Ψ² − πρ < 1`. It is only counted in
   the log. `FOM_IN_NOMINATOR = 0`, so the FoM does **not** enter the
   likelihood ratio, only ρ (through the FoM-ranked density) and any
   formula that refers to the `FOM` column.
7. The candidates are sorted by PROB_POST_SINGLE. Those below
   `min(probThres, 1e-20)` are dropped. In practice only near-zero
   probabilities are cut here; `probThres` is applied in step 6.

## 4. Re-selection — `cid_reselect`

The `selectNN` expressions are applied again, now that `ANGSEP`, `PROB_*`
and so on are filled in.

## 5. Catalogue-level posterior — `compute_prob_post_cat`

This step corrects for one counterpart being claimed by several LAT sources.
A sparse matrix (one column per counterpart *i*, rows = sources *k* that
kept it) gives:

P_cat(k,i) ∝ P_single(k,i) · Π_{k'≠k} (1 − P_single(k',i)),

normalised over *k* together with the "no LAT source" term
Π_{k'} (1 − P_single(k',i)). Stored as `PROB_POST_CAT`.

## 6. Unique posterior per source — `compute_prob_post`

This step corrects for one source having several candidates:

P_post(k,i) ∝ P_cat(k,i) · Π_{i'≠i} (1 − P_cat(k,i')),

normalised over *i* together with the "no counterpart" term
Π_{i'} (1 − P_cat(k,i')). Stored as `PROB_POST`. The mean of (1 − norm)
is logged as the "fraction of non-unique sources".

## 7. Catch-22 prior iteration — `catch22`

This step runs only if `probPrior` contains `CATCH-22` or `CATCH22`
(case-insensitive). Starting from a prior of 0.1, each iteration does the
following:
- new prior = Σ_{k,i} PROB_POST / N_cpt (the number of loaded counterpart
  objects), clamped to [1e-20, 1];
- stop if the relative change is < 1 %, the absolute change is < 1e-30, or
  the prior hit a bound;
- otherwise recompute PROB_PRIOR and PROB_POST_SINGLE for the first
  `numSelect` candidates of every source, re-sort, keep those
  > 1e-20, and redo steps 5 and 6.

There are at most 10 iterations.

## 8. Final probability and claim — `compute_prob` / `cid_prob`

- `PROB` = the `probMethod` expression (default `PROB_POST`), evaluated
  per row and clamped to [0, 1].
- Candidates are sorted by PROB. A source **claims** the leading candidates
  with `PROB ≥ probThres`, at most `maxNumCpt` of them.
- The log reports the following, where Σ PROB is taken over the refined
  candidates:
  - **reliability** = Σ PROB(claimed) / N_claimed
  - **completeness** = Σ PROB(claimed) / Σ PROB(all refined)

## 9. Output

- Claimed rows are appended to the output table (`cfits_add`).
- The `outCatQtyNN` columns are evaluated (`cfits_eval`).
- The `selectNN` expressions are applied **a third time**, to the output
  table (`cfits_select`).
- The run parameters are written as keywords (`cfits_set_pars`) and the
  file is closed (`cfits_save`).

## Expressions

`probMethod`, `probPrior`, `fom`, `outCatQtyNN` (right-hand side) and
`selectNN` are all **cfitsio calculator / row-filter expressions** on the
in-memory or output table. They can refer to:
- any of the 23 generic columns (`ANGSEP`, `PSI`, `PROB_POS`, `RHO`, …);
- copied catalogue columns, by their output names `@<prefix>_<name>`;
- earlier `outCatQty` columns.

Column names that contain `@`, `-`, `+` or `.` must be wrapped in `$…$`,
e.g. `$@3EG_F$ / $@WB14_S1.4$`. Before the expression reaches cfitsio,
gtsrcid evaluates these special functions itself into temporary columns
(`cfits_eval_special_expression`):

| Function | Value |
|----------|-------|
| `gammln(x)` | ln Γ(x) (Numerical Recipes) |
| `erf(x)`, `erfc(x)` | via the incomplete gamma function |
| `nsrc()`, `nlat()` | number of rows in the source catalogue **file** |
| `ncpt()` | number of rows in the counterpart catalogue file |
