# F101 R6 shaft — three-dimensional stress model

Design authority remains **head R6 / feed R6 / measurement R1**. This study does not change the shaft machining definition or release it for fabrication. It replaces an arbitrary notch multiplier with a resolved nominal-geometry elastic calculation; contact, actual threads and fatigue still require their own qualification.

## Implementation and artifacts

- [Distributed elastic solver](../engineering/src/fem.cpp) and [API](../engineering/include/fem.hpp).
- [Shaft loads, supports and refinement regions](../engineering/src/feed_fem.cpp).
- [Study driver](../engineering/src/feed_fem_analysis.cpp).
- [Numerical report](../engineering/generated/feed-fem/report.md), [regional results](../engineering/generated/feed-fem/regions.csv), [basis reactions](../engineering/generated/feed-fem/basis.csv), [acceptance checks](../engineering/generated/feed-fem/criteria.csv) and [machine-readable status](../engineering/generated/feed-fem/status.json).
- [Focused full-shaft refinement and background/phase audits](16-local-shaft-refinement.md) and [46-test verification record](verification/local-shaft-refinement.md).

The driver takes `--report`, a descending `--meshes` list of bulk:root sizes in millimetres, and `--phases`. A completed calculation normally exits zero even if its engineering checks fail; `--require-converged` instead exits two for unmet numerical gates. Invalid input, mesh, equilibrium or I/O errors exit one. The status starts with `run_complete:false`; fabrication release remains false even if all numerical checks eventually pass. Separate directories should be used to retain historical runs.

Optional `--focus-region`, `--background-root` and `--focus-growth` change only mesh allocation in the complete shaft. Every stress region is still evaluated. A mandatory focused-scope hold prevents a fixed-background series from claiming full all-root convergence. Parameters, geometry audits and actual results are recorded in the [focused study](16-local-shaft-refinement.md).

## Material and actual modeled features

The current nominal F101 B-rep is meshed directly, without changing its geometry to ease meshing. Isotropic small-strain elasticity uses $E=205$ GPa, $\nu=0.29$ and $\rho=7850$ kg/m³. Strength screens use the specified minimum yield strength of 650 MPa; this is a purchasing/inspection requirement, not a measured material certificate.

Four-node constant-strain tetrahedra include the stepped shaft, specified shoulder/groove/pocket blends, washer slot, key seats, conical ends and blind drill point. The M15 and M5 regions remain **cosmetic thread envelopes**, not actual helical threads. Final 0.05–0.10 mm edge breaks and dimensional tolerances are not resolved by this mesh. A minimum mesh-size request is not proof that a small root is adequately resolved: stress refinement remains mandatory.

## Supports do not clamp bearing-seat rotation

Six work-conjugate distributed constraints locate:

| Coordinate | Station S, mm | Selected surface band |
|---|---:|---|
| Rear radial X/Z | 26.5 | ±1.5 mm on the Ø12 seat |
| Front radial X/Z and axial Y | 77.5 | ±1.5 mm on the Ø15 seat |
| Drive spin reference about Y | 9 | ±5 mm on the Ø12 coupling seat |

The rear bearing remains axially free. No circumferential nodal clamp is imposed on either bearing. These are **ideal seat resultants**, not Hertzian ball/race contact, fit pressure or measured housing/bearing compliance.

The gravity-holding reaction is placed at the coupling of the **motor-driven cartridge**. It must not be interpreted as a real restraint at the unused shaft end of a follower or passive cartridge. Their actual holding/accelerating torque paths require role-specific treatment before complete-machine qualification; the present envelope is not a proof of every assembled role.

Selected boundary-triangle areas produce normalized nodal weights $w_i$. For patch centroid $\bar r$, loads are distributed as

$$f_i=w_i\left(F+\omega\times(r_i-\bar r)\right),$$

where the patch polar matrix determines $\omega$ to preserve both the requested resultant and moment. A unit virtual force defines mean displacement; a unit virtual couple defines rotation. The same distribution defines a constraint row. This preserves work conjugacy instead of assigning an arbitrary bearing stiffness.

### Exact numerical gauge, not added physical stiffness

Let $K$ be free-body stiffness, $C u=g$ the normalized distributed constraints, and $R$ its six analytical rigid-body modes. The model first checks that $CR$ has rank six. QR pivoting selects six scalar gauge coordinates $P$ with independent rigid motion. The factorized matrix is

$$A=K+\alpha P^T P,\qquad \alpha=s\max\operatorname{diag}(K).$$

Unlike $C^TC$, this diagonal augmentation does not create dense cliques between all nodes of a bearing patch. With exactly six independent coordinates, define $B=CR$ and solve

$$B^T\lambda=R^Tf,\qquad w=A^{-1}(f-C^T\lambda),\qquad q=B^{-1}(g-Cw),\qquad u=w+Rq.$$

Rigid equilibrium implies $\alpha(PR)^TPw=0$, hence $Pw=0$ because $PR$ is nonsingular. Thus $Kw=f-C^T\lambda$; the rigid lift changes no strain and makes $Cu=g$. This exact specialization avoids a false dependence rejection from a mixed-scale saddle matrix on a strongly refined mesh. It does not weaken the rank or residual checks.

For seven–twelve coordinates, define $y=A^{-1}f$, $V=A^{-1}C^T$, $S=CV$. The retained small system is

$$\begin{bmatrix}\alpha S&-B\\-B^T&0\end{bmatrix}
\begin{bmatrix}\lambda\\z\end{bmatrix}
=\begin{bmatrix}\alpha(Cy-g)\\-R^Tf\end{bmatrix},
\qquad u=y-V\lambda+Rz/\alpha.$$

Physical reactions are $-C^T\lambda$. Rigid-body equilibrium makes the temporary gauge forces vanish; the final equations are $Ku=f-C^T\lambda$ and $Cu=g$. The implementation symmetrizes $S$ to remove solve-roundoff asymmetry. Neither $\alpha$ nor the gauge pins is a material or bearing property.

The distributed solver uses CHOLMOD supernodal Cholesky through Eigen, with METIS nested-dissection ordering to limit three-dimensional factor fill. The engineering build requires SuiteSparse/CHOLMOD with METIS support. The verified installation is SuiteSparse7.14.0, CHOLMOD5.3.5 and METIS5.1.0. Ordering and factorization implementation change neither stiffness nor constraints; the older prescribed-nodal-support solver remains unchanged. Successful Cholesky factorization requires a positive pivot at every column.

Each solution independently checks equilibrium, constraint satisfaction and force/moment balance below the specified $10^{-7}$ relative limits. Tests vary $s$ over0.1,1,10 and impose simultaneous nonzero translations/rotations at all six coordinates,as well as seven/twelve-coordinate axial strain. A solver residual checks the discrete equations; it does not establish mesh accuracy.

## Eight physical load bases

| Basis | Unit load |
|---:|---|
| 0/1 | Pulley X/Z force, resultant at S47.5 |
| 2/3 | Roller X/Z force, resultant at actual wire contact S108.75 |
| 4 | +1 Nm at coupling, −1 Nm at roller |
| 5 | +1 Nm at coupling, −1 Nm at pulley |
| 6/7 | 1 m/s² body acceleration in material X/Z |

Loads act on finite seat bands, not an invented key-contact pressure distribution. The roller force is applied over its seat with the moment correction needed to put the resultant at the actual wire lane. Independent statics checks require rear/front reactions of −0.588235294/−0.411764706 N per unit pulley load and +0.612745098/−1.612745098 N per unit roller load.

Balanced transmission bases must have negligible spin-reference reaction. The machined shaft's centre of mass is eccentric, so gravity genuinely requires a drive-holding moment. Its calculated reaction must equal minus the actual mesh body-load moment about Y; it must not be forced to zero. This holding torque is additional to the prescribed transmission-torque limits.

## All radial signs, torque paths and rotation phases

The stationary load box uses pulley X force ±480 N and roller X/Z forces independently ±60 N. Material-frame loads are rotated through shaft phase with $R_y(-\phi)$, including gravity. All eight sign corners are evaluated.

These are specified load envelopes, not measured or mechanically enforced force limits. Radial resultants at the shaft axis and interface torque are separate equivalent loads; a real tangential wire force must also satisfy its moment relation. At a nominal 20 mm roller radius, 5 Nm corresponds to 250 N tangential force if contact can transmit it. The pressure/friction/jam model must establish the actual attainable combinations before claiming fault coverage; the present ±60 N roller box is not itself a proof of a 5 Nm jam-force limit.

Let $T_r$ and $T_p$ be coupling-to-roller and coupling-to-pulley torque transfers. Each interface is bounded by

$$|T_r|\le T,\quad |T_p|\le T,\quad |T_r+T_p|\le T.$$

The six vertices are $(T,0),(0,T),(-T,T),(-T,0),(0,-T),(T,-T)$. This includes opposing torque paths. $T=1$ Nm is the working transmission envelope and $T=5$ Nm the separate fault envelope. Convexity of displacement norm and von Mises seminorm makes box/polygon vertices sufficient for each fixed phase in this linear model.

In the regional CSV, `torque_vertex` is the zero-based index in that six-vertex order. For `sign_index`, bit 0 selects pulley X, bit 1 roller X and bit 2 roller Z: a set bit means positive and a clear bit negative. Thus index 5 means +480/−60/+60 N in stationary pulley-X/roller-X/roller-Z coordinates. `phase_deg` is the material rotation $\phi$ about +Y, with stationary loads transformed by $R_y(-\phi)$. These indices select a load case, not a physical assembly instance.

Only displacement vectors and full stress tensors are superposed. Tensor order is xx, yy, zz, xy, yz, xz; shear entries are tensor stresses. Von Mises magnitudes themselves are never linearly superposed.

For a field $a+b\cos\phi+c\sin\phi$, the nearest of $N$ uniform samples is at most $\pi/N$ away. The unsampled norm increment is bounded by

$$\Delta\le2\sin\!\left(\frac{\pi}{2N}\right)
\sqrt{\|b\|^2+\|c\|^2}.$$

The same inequality applies to the von Mises seminorm. In particular, define the linear transformation

$$L\sigma=\left((\sigma_{xx}-\sigma_{yy})/\sqrt2,(\sigma_{yy}-\sigma_{zz})/\sqrt2,
(\sigma_{zz}-\sigma_{xx})/\sqrt2,\sqrt3\sigma_{xy},\sqrt3\sigma_{yz},\sqrt3\sigma_{xz}\right).$$

Then $\sigma_{VM}=\|L\sigma\|_2$. Apply the triangle inequality and Cauchy–Schwarz to $Lb(\cos\phi-\cos\psi)+Lc(\sin\phi-\sin\psi)$; the two trigonometric differences have Euclidean norm $2|\sin((\phi-\psi)/2)|$. This proves the bound for all finite stress tensors,including hydrostatic null components. Possible higher harmonics in the scalar magnitude do not invalidate this norm argument.

This encloses unsampled phases **for the particular discrete linear solution**, not mesh error, geometric tolerances, contact changes or fatigue.

## Refinement, visualization and acceptance

Ten overlapping stress regions retain the whole shaft and individual groove, shoulder, slot/key and blind-bore neighborhoods. Centroid classification does not remove any element from the whole-shaft peak. Every region keeps its sampled raw maximum **and** its continuous-phase bound. Both have separate 5% last-refinement checks. No percentile or exclusion of a sharp feature replaces the raw peak.

Other gates require at least three strictly descending meshes, working displacement below 0.05 mm with less than 3% last-refinement change, FE/B-rep volume agreement within 0.5%, working stress below 650/3 MPa, and fault stress below 650/1.5 MPa. A low stress magnitude cannot override an unconverged peak. The direct-solver budget remains 150000 nodes.

Numbered VTU artifacts contain the **single actual sampled load case** governing whole-shaft raw stress in all-root mode,or the selected region in focused mode,as identified by `field_governing_region`. They contain the complete shaft,undeformed coordinates in mm, displacement in mm and cell von Mises stress in MPa. Their displacement need not be the maximum-displacement load case. The regional CSV coordinates/signs/torque vertex likewise identify a sampled maximum; the angular upper bound can be governed by another element. An envelope is not presented as one simultaneous physical field.

Inspect the original cell stress array when checking extrema. A viewer's cell-to-point averaging or smoothed contours can reduce visible peaks and must not replace the reported raw-cell maximum. Any exaggerated deformation scale is a display aid, not the actual displacement magnitude.

## Remaining physical work

Resolve actual thread geometry and remaining edge breaks, matched-fit prestress, KM2/M5 preload, key/collar contact, bearing/housing/module compliance and rotating multiaxial fatigue. Combine those results with tolerance and physical inspection data before using the study to release parts. The existing beam's 3× stress multiplier remains a sensitivity screen, not a verified concentration factor or a replacement for these results. Hardware outputs remain disabled.