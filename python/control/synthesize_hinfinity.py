#!/usr/bin/env python3
"""Offline continuous-time H-infinity state-feedback synthesis.

Requires: Python 3.10+, numpy, cvxpy and an SDP-capable solver (CLARABEL or SCS).
Plant: x_dot = A x + B u + E w; z = C x + D u.
Finds u=Kx minimizing gamma for the continuous-time bounded-real LMI.
This is an offline design utility; it is not part of the embedded runtime.
"""
from __future__ import annotations
import argparse
import json
from pathlib import Path
import numpy as np

def synthesize(a, b, e, c, d, solver=None):
    import cvxpy as cp
    n, m = b.shape
    nw = e.shape[1]
    nz = c.shape[0]
    p = cp.Variable((n, n), symmetric=True)
    y = cp.Variable((m, n))
    gamma = cp.Variable(nonneg=True)
    ap = a @ p - b @ y
    cp_term = c @ p + d @ y
    # Continuous-time strict bounded-real LMI (strictness via epsilon I).
    lmi = cp.bmat([
        [ap + ap.T, e, cp_term.T],
        [e.T, -gamma * np.eye(nw), np.zeros((nw, nz))],
        [cp_term, np.zeros((nz, nw)), -gamma * np.eye(nz)],
    ])
    constraints = [p >> 1e-7 * np.eye(n), lmi << -1e-7 * np.eye(n + nw + nz)]
    problem = cp.Problem(cp.Minimize(gamma), constraints)
    chosen = solver or ("CLARABEL" if "CLARABEL" in cp.installed_solvers() else "SCS")
    problem.solve(solver=chosen, verbose=False)
    if problem.status not in ("optimal", "optimal_inaccurate") or p.value is None:
        raise RuntimeError(f"H-infinity synthesis failed: {problem.status}")
    k = y.value @ np.linalg.inv(p.value)
    return k, float(gamma.value), problem.status, chosen

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--output", default="hinfinity_gain.json")
    args = parser.parse_args()
    # Example double-integrator line-error model; replace with identified robot
    # plant matrices before using the result on hardware.
    a = np.array([[0.0, 1.0], [0.0, 0.0]])
    b = np.array([[0.0], [1.0]])
    e = np.array([[0.0], [1.0]])
    c = np.eye(2)
    d = np.zeros((2, 1))
    k, gamma, status, solver = synthesize(a, b, e, c, d)
    result = {
        "schema": "oblivion.hinf.state_feedback.v1",
        "state_order": ["line_error", "line_error_rate"],
        "feedback_convention": "u = -Kx",
        "gain": k.reshape(-1).tolist(),
        "gamma": gamma,
        "solver": solver,
        "status": status,
        "plant": {"A": a.tolist(), "B": b.tolist(), "E": e.tolist(),
                  "C": c.tolist(), "D": d.tolist()},
        "warning": "Example plant only. Identify and validate the actual robot plant before deployment."
    }
    Path(args.output).write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    print(f"Wrote {args.output}; gamma={gamma:.6g}; solver={solver}")

if __name__ == "__main__":
    main()
