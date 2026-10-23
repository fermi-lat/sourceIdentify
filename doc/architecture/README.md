---
package: sourceIdentify
summary: The gtsrcid tool, which associates LAT sources with counterpart-catalogue objects using positional coincidence, local counterpart density and Bayesian posterior probabilities, with user-defined priors, figures of merit and selections.
layer: analysis
depends_on: [catalogAccess, st_app, tip, cfitsio]
used_by: []
last_reviewed_commit: 43931a7
---

# sourceIdentify Architecture

**sourceIdentify** builds a single executable, **`gtsrcid`** (J. Knödlseder,
CESR/IRAP; version string `v2r4p0`, 2014). It reads a "source" catalogue
(normally a LAT catalogue) and a "counterpart" catalogue, finds counterpart
candidates near each source, assigns association probabilities, and writes a
FITS table with one row per (source, claimed counterpart) pair. Probabilities
combine the source error ellipse, the local density of counterparts, and a
prior that the user supplies as an expression. They are then corrected for
counterparts shared between sources and for sources with several
candidates.

The package has no library and no public headers. Everything is in
`src/gtsrcid/`.

The details of each algorithm step and every formula are in
[association-algorithm.md](association-algorithm.md).

## Purpose and scope

**Does:**
- Load two catalogues (FITS tables; in principle also VizieR TSV/CSV, but
  see [gotchas](#invariants-and-gotchas)) through `catalogAccess`, and
  identify name, position and error-ellipse columns by UCD or column name.
- For each source:
  - filter counterparts within `max(4°, 2.5 × a95)`;
  - apply user selections;
  - compute `PROB_POS`, the local density `RHO`, `PROB_CHANCE`,
    `PROB_PRIOR`, a likelihood ratio and `PROB_POST_SINGLE`.
- Correct for counterparts claimed by several sources (`PROB_POST_CAT`)
  and for multiple candidates per source (`PROB_POST`). Optionally iterate
  the prior ("catch-22").
- Evaluate a final `PROB` expression, keep at most `maxNumCpt` candidates
  with `PROB ≥ probThres`, add user-defined columns, apply final
  selections, and write the result.
- Write a detailed log, `gtsrcid.log`, including reliability and
  completeness estimates.

**Does not:**
- Fetch catalogues from the web. The code calls `catalogAccess`'s `…Web`
  methods as a fallback, but those are stubs that always fail.
- Locate catalogues by name or from an installed data directory. Every
  catalogue argument is a file path.
- Use the counterparts' own position errors in the probability. Only the
  source error ellipse enters.
- Run the multi-catalogue LAT association pipeline. That was
  `python/srcid.py`, which is not installed and is Python 2 (see
  [Public interfaces](#python)).

## Position in the system

| Direction | Package | Why |
|-----------|---------|-----|
| Depends on | `catalogAccess` | All catalogue reading (`getMaxNumRows`, `importDescription`, `import`, `getNValue`/`getSValue`, quantity names, units, UCDs, types and `Quantity` formats). No selection or save features are used. |
| Depends on | `st_app` | `StApp`/`StAppFactory` application shell and `AppParGroup` parameter access. See `src/st_app/doc/architecture/`. |
| Depends on | `tip` | Not linked directly, but `catalogAccess/catalog.h` includes `tip` headers, so `gtsrcid` compiles against them. It arrives transitively through catalogAccess's `PUBLIC` link. |
| Depends on (undeclared) | cfitsio | `Catalogue.h` and `GHealpix.h` include `fitsio.h`. All output-table work (in-memory `mem://` file, `fits_calculator`, `fits_select_rows`, column I/O) and the HEALPix density-map reader call cfitsio directly. It is linked transitively through `tip`. |
| Link only | `hoops` | In `target_link_libraries`, but no `hoops` header is included. Parameter values reach it only through `st_app`. |
| Link only | `st_facilities` | In `target_link_libraries`, but no `st_facilities` symbol is used. It also arrives transitively, through catalogAccess's `PUBLIC` link. |
| Used by | — | No package uses it. `fermitools-fhelp` has the user help, `fhelp_files/gtsrcid.txt`. |

`gtsrcid` has its own private sky-direction and HEALPix code (`GSkyDir`,
`GHealpix`) instead of using `astro` or `healpix`.

## Key abstractions

| Type | File | Role |
|------|------|------|
| `gtsrcid` (`st_app::StApp`) | `sourceIdentify.cxx` | Entry point. It opens the log, loads `Parameters`, and calls `Catalogue::build`. It never throws (see gotchas). |
| `Parameters` | `Parameters.{h,cxx}` | Copies all par-file values into members. It splits `outCatQtyNN` at `=` into a name and a formula, collects non-empty `selectNN` values, detects `CATCH-22` in `probPrior`, and builds the column prefixes `@<srcCatPrefix>_` and `@<cptCatPrefix>_`. Chatter levels: `logTerse` > 0, `logNormal` > 1, `logExplicit` > 2, `logVerbose` > 3; `logDebug` follows the `debug` parameter. |
| `Catalogue` | `Catalogue.h`; `Catalogue.cxx` (driver, loading, catalogue-level probabilities), `Catalogue_id.cxx` (per-source steps `cid_*`), `Catalogue_fits.cxx` (cfitsio table work `cfits_*`), `Catalogue_nr.cxx` (Numerical Recipes `gammln`/incomplete gamma) | Holds both input catalogues (`InCatalogue`, each wrapping a `catalogAccess::Catalog` and an `ObjectInfo[]` of name, position and 95 % ellipse), per-source `SourceInfo` with a `CCElement[]` candidate array, the in-memory FITS table `m_memFile`, and the output file `m_outFile`. |
| `GHealpix`, `GSkyDir` | `GHealpix.*`, `GSkyDir.*` | Optional counterpart-density map (`cptDensFile`): a HEALPix binary table (first column, `NSIDE`, `ORDERING` RING/NESTED, `COORDSYS` EQU/GAL) and RA/Dec ↔ l/b conversion. |
| `Log` | `Log.{h,cxx}` | `printf`-style logging with a type, a UTC timestamp and the tool version, to **`gtsrcid.log` only**. |

Error handling uses the C style throughout. Every method takes and returns a
`Status` (enum in `sourceIdentify.h`, `STATUS_OK = 0`, errors −100000…),
and the body is a `do { … } while (0)` with `continue` as "goto exit".

## Data flow and control flow

```
gtsrcid::run
  LogInit("gtsrcid.log")                       // overwrite in cwd
  Parameters::load(pars)                       // Prompt(), Save()
  Catalogue::build
    get_input_descriptor(src), (cpt)           // catalogAccess::getMaxNumRows + importDescription,
                                               //   web fallback (always fails); find columns
    [read cptDensFile → GHealpix]              // failure only warns
    cfits_create("mem://gtsrcid"), cfits_create(outCatName)   // clobber → remove() first
    get_input_catalogue(src), (cpt)            // catalogAccess::import → ObjectInfo[]
    for each source k:  cid_source
        cid_filter → cid_select → cid_refine → cid_reselect
    compute_prob_post_cat → compute_prob_post → catch22 → compute_prob
    cfits_add (claimed rows) → cfits_eval (outCatQty) → cfits_select (selectNN)
    cfits_collect → cfits_set_pars → cfits_save → dump_results
  LogClose
```

The per-source steps write their candidates into the in-memory FITS table so
that user expressions (`fom`, `probPrior`, `probMethod`, `selectNN`) can be
evaluated by cfitsio's calculator over the same columns that appear in the
output. See [association-algorithm.md](association-algorithm.md).

## Public interfaces

### Applications

| Tool | Par file | Notes |
|------|----------|-------|
| `gtsrcid` | `pfiles/gtsrcid.par` | See below. User help: `fermitools-fhelp/fhelp_files/gtsrcid.txt`. |

Parameters (`a` = asked, `h` = hidden):

| Parameter | Mode | Meaning |
|-----------|------|---------|
| `srcCatName`, `cptCatName` | a | Catalogue file paths (FITS or VizieR TSV/CSV). The defaults `../../data/3EG.fits` and `../../data/radio_white1.4GHz.tsv` are relative to `src/test/` in the source tree. |
| `srcCatPrefix`, `cptCatPrefix` | a | Output columns copied from each catalogue are named `@<prefix>_<column>`. |
| `srcCatQty`, `cptCatQty` | h | `*` = copy all columns. Otherwise a column is copied if its name is a **substring** of this string. |
| `srcPosError`, `cptPosError` | h | Error radius (degrees, 95 %) used when no error column is found. |
| `cptDensFile` | h | Optional HEALPix map of counterpart density (per deg²). If given, it replaces the local density estimate. |
| `outCatName` | a | Output FITS file. |
| `outCatQty01`–`09` | h | `NAME = expression` new output columns. |
| `probMethod` | a | Expression for the final `PROB` (default `PROB_POST`). |
| `probPrior` | a | Expression for `PROB_PRIOR` (default `0.01`), or `CATCH-22`. |
| `probThres`, `maxNumCpt` | a | Claim threshold on `PROB`, and the maximum number of claimed counterparts per source. |
| `fom` | a | Optional figure-of-merit expression. |
| `select01`–`09` | h | cfitsio row-filter expressions. |
| `chatter` (0–4), `clobber`, `debug`, `mode` | h | `debug=yes` also sets `catalogAccess::verbosity` to 3 (otherwise 0). |

The standard `st_app` hidden parameters and startup behaviour apply (see
`src/st_app/doc/architecture/`).

### Output file

A single binary table, EXTNAME `GLAST_CAT`, with one row per claimed
(source, counterpart) pair. Sources with no claimed counterpart do not
appear. Columns:

1. 23 generic columns:
   - `ID` (`CC_sssss_ccccc`: source index and candidate rank)
   - `RAJ2000`, `DEJ2000`, `POS_ERR_MAJ`, `POS_ERR_MIN`, `POS_ERR_ANG`
     (the counterpart's position and its 95 % ellipse)
   - `PROB`, `PROB_POS`, `PDF_POS`, `PROB_CHANCE`, `PDF_CHANCE`,
     `PROB_PRIOR`, `PROB_POST`, `PROB_POST_SINGLE`, `PROB_POST_CAT`,
     `LOGLR`, `ANGSEP`, `PSI`, `POSANG`, `RHO`, `MU`, `FOM`
   - `REF` (0-based row of the counterpart in the counterpart catalogue)
   - Each column has a `TBUCDn` keyword.
2. The copied source-catalogue columns, `@<srcPrefix>_…`.
3. The copied counterpart-catalogue columns, `@<cptPrefix>_…`.
4. The `outCatQty` columns.

The header records the run parameters in `P_METHOD`, `P_PRIOR`, `P_THRES`,
`MAX_CPT`, `SRC_ERR` and `CPT_ERR`. Column names start with `@`, which
astropy warns about. In expressions, wrap them as `$@3EG_F$`.

### Log file

`gtsrcid.log` in the current directory, overwritten on every run. It holds
the parameters, catalogue descriptors, per-source candidate tables (with
`chatter` ≥ 2), selection statistics, catch-22 iterations, and a summary
(claimed associations, reliability, completeness). **This is the only place
where gtsrcid reports errors.**

### C++ API

None. Nothing is installed under `include/`.

### Python

`python/srcid.py` and `python/classes/*.py` (20 source-class modules: AGN,
BZCAT, CRATES, CGRaBS, pulsars, PWN, SNR, TeV, 3EG, EGR, …) implement the
old LAT association pipeline. It runs `gtsrcid` once per class, against
catalogue files such as `obj-agn.fits` found through `$GLAST_CAT` or
`$EXTFILESSYS/catalogues`, and merges the results into the LAT catalogue.
It is **Python 2** (`print` statements, `commands`, `pyfits`), it is **not
installed** by CMake, and the catalogue files are not distributed. Treat it
as historical reference for the parameter choices (priors, FoMs and
selections per class).

### Environment variables

None in `gtsrcid` itself (no `getenv` calls). `PFILES` is used through
`st_app`/`ape`; see "Runtime environment" in ScienceTools'
`docs/architecture/README.md`.

## Build and test

- **CMake target:** the executable `gtsrcid`, built from the nine `.cxx`
  files in `src/gtsrcid/`. They contain about 11,800 lines, of which about
  900 are old CVS `$Log` headers. Much of the rest is logging and
  error-path boilerplate. The large line count of the repository comes
  mainly from `data/radio_white1.4GHz.tsv` (31,566 lines, 3.4 MB).
- **Install:**
  - `gtsrcid` goes to `bin`
  - `pfiles/gtsrcid.par` goes to `syspfiles`
  - `data/` goes to **`${FERMI_INSTALL_DATADIR}`** (`share/fermitools/data/`)
    itself, not to a package subdirectory, so the files land as
    `share/fermitools/data/3EG.fits`,
    `share/fermitools/data/radio_white1.4GHz.tsv`, and
    `share/fermitools/data/outref/test1.{fits,log}`. The `outref/`
    directory is shared with other packages' reference files. Nothing looks
    these files up at runtime.
- **Tests:** none are built or registered with CTest.
  - `src/test/test1.csh` is a manual tcsh script that runs `gtsrcid` on
    `3EG.fits` × `radio_white1.4GHz.tsv` from `src/test/`, with
    `PFILES=../../pfiles`. Its `outCatQty01` refers to `$%3EG_F$`; the
    prefix character has since changed to `@`, so that column is stale.
  - `src/test/gtsrcid` is an SCons/CMT-era wrapper (rh9_gcc32, ROOT
    5.16).
  - `src/test/gtsrcid.par` is an old (v1r2p0) par file.
  - `data/outref/test1.{fits,log}` are expected outputs from 2007
    (v1r2p0). The current output will differ.
  - That reference run uses a TSV counterpart catalogue, which currently
    fails to load (see below).
  - ScienceTools' `recipe/tests/ST-unit-test` never lists `gtsrcid` or
    any sourceIdentify test (neither the default nor the ROOT list), and
    `fermitools-data` has no `outrefFiles/sourceIdentify`. See "Testing" in
    ScienceTools' `docs/architecture/README.md`.
- **Quick manual check** (FITS × FITS; this works). Run it in a scratch
  directory, because it writes `gtsrcid.log` there. `$D` is the directory
  that holds `3EG.fits` (`data/` in the source tree, or
  `share/fermitools/data` in an installation):
  ```
  gtsrcid srcCatName=$D/3EG.fits srcCatPrefix=3EG cptCatName=$D/3EG.fits \
          cptCatPrefix=EGR outCatName=out.fits probMethod=PROB_POST \
          probPrior=0.01 probThres=0.05 maxNumCpt=4 fom="" chatter=2
  ```
- **Legacy:** `SConscript` and `src/mainpage.h`. The mainpage's
  "catalogue requirements" section is "TBW", and it `\include`s a CMT
  `requirements` file that no longer exists.

## Invariants and gotchas

**Failure reporting**
- `gtsrcid` **always exits with status 0**. `run()` turns every error into
  a `Status` code, writes it to `gtsrcid.log`, and returns normally, so the
  `st_app` exit-status contract (see `src/st_app/doc/architecture/`) is
  never triggered. This was checked at runtime with FT-2.5.1: missing input
  files give exit status 0 and no output file. Scripts must check that the
  output file exists, or read the log.
- Nothing is printed to stdout or stderr except the `st_app` banner and
  `catalogAccess` `ERROR` lines. If the current directory is not writable,
  `LogInit` fails and the tool silently does nothing.
- A `selectNN` expression that cfitsio cannot parse is only a **warning**
  in the log. That selection is skipped and the run continues unfiltered.
- A `cptDensFile` that cannot be read is also only a warning, and the local
  density estimate is used instead.

**Catalogue input**
- **VizieR TSV/CSV counterpart catalogues do not load** on 64-bit builds
  (checked at runtime with FT-2.5.1 and the shipped
  `radio_white1.4GHz.tsv`). The fault is in `catalogAccess`'s text parser;
  see `src/catalogAccess/doc/architecture/`. Use FITS catalogues.
- Columns are found by UCD or by **case-insensitive prefix match** on the
  name, and the shortest match wins (`find()` in `Catalogue.cxx`). An
  unexpected column whose name starts with `RA`, `DEC`, `L`, `B`, `NAME`,
  `ID` or `PosErr` can be picked up. The order is listed in
  [association-algorithm.md](association-algorithm.md#0-loading-and-position-errors).
- Error columns are rescaled from their confidence level to 95 % but are
  **assumed to be in degrees**. No unit conversion is done, so for example
  a `PosErr` in arcsec is read as degrees. Only source-catalogue errors
  affect the probabilities.
- `srcCatQty`/`cptCatQty` choose columns by substring: with `srcCatQty=Flux1000`,
  a column named `Flux` is also copied.
- Both catalogues are loaded in full, and the filter step scans the whole
  counterpart catalogue for every source (O(N_src × N_cpt)).
- `nsrc()`/`ncpt()` return the number of rows in the **file**, not the
  number of rows with valid positions.

**Selections**
- The same `selectNN` expressions are applied three times: before the
  probabilities exist (`cid_select`), after them (`cid_reselect`), and to
  the output (`cfits_select`). In the first pass `ANGSEP`, `PROB_*`, `RHO`
  and so on are still 0 in the in-memory table, so an expression such as
  `ANGSEP > 0.1` or `PROB_POS > 0.5` removes **every** candidate before
  any probability is computed (from reading the code; not checked at
  runtime). Expressions of the form `ANGSEP < x`, or expressions on
  catalogue columns only, are safe.

**Probabilities**
- The refine step drops candidates only below `min(probThres, 1e-20)`.
  `probThres` and `maxNumCpt` are applied in `compute_prob`, after the
  catalogue-level corrections.
- `probPrior` and `probMethod` results are clamped to [0, 1]. A prior of
  ≥ 1 forces `PROB_POST_SINGLE = 1`.
- **Possible bug (from reading the code; not checked at runtime):**
  `cid_reselect` compacts the candidate array in place but does not
  update `numSelect`. Catch-22 iterations then recompute over `numSelect`
  entries, so with both `CATCH-22` and `selectNN`, candidates removed by
  the second selection (and duplicated entries left by the compaction) can
  come back.
- `cid_assign_src_name` calls `std::remove` without `erase`. Because of
  this, blank source names are replaced by `CAT_ROW_<n>` only when the
  name is exactly one space.

**Output**
- The output file is created (and, with `clobber=yes`, first deleted with
  `remove()`) after the catalogue descriptors are read but **before** the
  catalogue data are loaded. It is completed only at `cfits_save`, so a run
  that fails in between can leave a missing or incomplete file. With
  `clobber=no` and an existing file, cfitsio's create fails (logged only).

## Extension points

- **A new error-column convention:** add a search block to
  `get_pos_error_info` (`Catalogue.cxx`) with the right `PosErrorProb`.
  Add the name convention in `get_pos_info`/`get_id_info` in the same way.
- **A new built-in function for expressions:** add its name and argument
  count to `fct_names`/`fct_nargs` in `Catalogue.h` (before `"stop"`), and
  add a branch in `cfits_eval_special_function` (`Catalogue_fits.cxx`).
- **A new output column:** add `OUTCAT_COL_*` macros in `Catalogue.h`,
  raise `OUTCAT_NUM_GENERIC`, and write it in `cfits_create`, `cfits_add`
  and `cfits_update`.
- **Probability model changes:** the per-source pieces are in
  `Catalogue_id.cxx` (`cid_prob_pos`, `cid_local_density`,
  `cid_prob_chance`, `cid_prob_post_single`). The compile-time switches
  `JEAN_BALLET_FORMULA`, `ADAPTIVE_DENSITY` and `FOM_IN_NOMINATOR` are at
  the top of that file.

## Further reading

- [association-algorithm.md](association-algorithm.md): step-by-step
  formulas.
- `fermitools-fhelp/fhelp_files/gtsrcid.txt`: user documentation of the
  parameters.
- `src/catalogAccess/doc/architecture/`: catalogue loading and its
  limitations.
- `doc/release.notes`: history to 2012 (the last tag, v2r4p0 in 2014, is
  not listed).
- Mattox et al. (1997), ApJ 481, 95, the positional likelihood. The LAT
  catalogue association papers (for example 1FGL, Abdo et al. 2010, ApJS 188,
  405) describe how `gtsrcid` was used in the Bayesian association method.
