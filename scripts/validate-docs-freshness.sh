#!/usr/bin/env bash

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
cd "${ROOT_DIR}"

FAILURES=0
WARNINGS=0
README_DATE=""

info() {
  printf 'INFO: %s\n' "$*"
}

warn() {
  printf 'WARN: %s\n' "$*" >&2
  WARNINGS=$((WARNINGS + 1))
}

error() {
  printf 'ERROR: %s\n' "$*" >&2
  FAILURES=$((FAILURES + 1))
}

check_root_snapshot_section() {
  if [[ ! -f "README.md" ]]; then
    error "Missing README.md"
    return
  fi

  if ! grep -Fq "## Active Plugin Snapshot (LocusQ)" README.md; then
    error "README.md is missing the 'Active Plugin Snapshot (LocusQ)' section"
    return
  fi

  local line
  line="$(grep -E '^Last updated: \*\*[0-9]{4}-[0-9]{2}-[0-9]{2}\*\*$' README.md | head -n 1 || true)"
  if [[ -z "${line}" ]]; then
    error "README.md snapshot section is missing a valid 'Last updated: **YYYY-MM-DD**' line"
    return
  fi

  README_DATE="$(printf '%s\n' "${line}" | sed -E 's/^Last updated: \*\*([0-9]{4}-[0-9]{2}-[0-9]{2})\*\*$/\1/')"

  local required_markers=(
    "Phase 2.9b CI evidence closeout status"
    "Source of truth for this snapshot"
    "Submodule pointer"
    "plugins/LocusQ/status.json"
    "plugins/LocusQ/TestEvidence/build-summary.md"
    "plugins/LocusQ/TestEvidence/validation-trend.md"
  )

  local marker
  for marker in "${required_markers[@]}"; do
    if ! grep -Fq "${marker}" README.md; then
      error "README.md snapshot section is missing marker: ${marker}"
    fi
  done
}

check_changelog_sync() {
  if [[ ! -f "CHANGELOG.md" ]]; then
    error "Missing CHANGELOG.md"
    return
  fi

  if [[ -z "${README_DATE}" ]]; then
    error "Cannot verify CHANGELOG.md sync because README date was not extracted"
    return
  fi

  local expected
  expected="Root LocusQ snapshot refreshed to ${README_DATE}"
  if ! grep -Fq "${expected}" CHANGELOG.md; then
    error "CHANGELOG.md is missing sync entry: '${expected}'"
  fi
}

check_locusq_submodule_pointer() {
  local pointer
  pointer="$(git rev-parse "HEAD:plugins/LocusQ" 2>/dev/null || true)"
  if [[ -z "${pointer}" ]]; then
    error "Unable to resolve plugins/LocusQ gitlink from HEAD"
    return
  fi

  if [[ -f "README.md" ]] && ! grep -Fq "${pointer}" README.md; then
    error "README.md snapshot must include current plugins/LocusQ gitlink (${pointer})"
  fi

  local configured_branch
  configured_branch="$(git config -f .gitmodules --get submodule.plugins/LocusQ.branch || true)"
  if [[ "${configured_branch}" != "main" ]]; then
    error ".gitmodules must set submodule.plugins/LocusQ.branch to 'main' (found '${configured_branch:-unset}')"
  fi

  if [[ ! -f "plugins/LocusQ/.git" && ! -d "plugins/LocusQ/.git" ]]; then
    warn "plugins/LocusQ is not initialized locally; skipping live submodule commit checks"
    return
  fi

  local checked_out
  checked_out="$(git -C plugins/LocusQ rev-parse HEAD 2>/dev/null || true)"
  if [[ -z "${checked_out}" ]]; then
    error "plugins/LocusQ is present but does not resolve a valid git HEAD"
    return
  fi

  if [[ "${checked_out}" != "${pointer}" ]]; then
    error "Submodule checkout mismatch: gitlink=${pointer}, working tree=${checked_out}"
  fi

  if git -C plugins/LocusQ fetch --quiet origin main >/dev/null 2>&1; then
    if ! git -C plugins/LocusQ merge-base --is-ancestor "${pointer}" origin/main; then
      error "Submodule pointer ${pointer} is not reachable from plugins/LocusQ origin/main"
    fi
  else
    warn "Unable to fetch plugins/LocusQ origin/main; remote reachability check skipped"
  fi

  if command -v jq >/dev/null 2>&1; then
    local status_date
    status_date="$(jq -r '.last_modified // empty' plugins/LocusQ/status.json 2>/dev/null | cut -d'T' -f1)"
    if [[ -z "${status_date}" || "${status_date}" == "null" ]]; then
      error "plugins/LocusQ/status.json is missing a valid last_modified timestamp"
    elif [[ -n "${README_DATE}" && "${README_DATE}" != "${status_date}" ]]; then
      error "README snapshot date (${README_DATE}) does not match plugins/LocusQ/status.json date (${status_date})"
    fi
  else
    warn "jq not installed; skipping plugins/LocusQ/status.json date alignment check"
  fi
}

check_locusq_docs_freshness_if_present() {
  local script="plugins/LocusQ/scripts/validate-docs-freshness.sh"
  if [[ ! -f "${script}" ]]; then
    warn "Skipping LocusQ docs freshness check (missing ${script})"
    return
  fi

  if ! bash "${script}"; then
    error "LocusQ docs freshness script failed"
  fi
}

main() {
  info "Running documentation freshness checks from ${ROOT_DIR}"

  check_root_snapshot_section
  check_changelog_sync
  check_locusq_submodule_pointer
  check_locusq_docs_freshness_if_present

  if [[ "${FAILURES}" -gt 0 ]]; then
    printf 'FAIL: %d documentation freshness issue(s), %d warning(s).\n' "${FAILURES}" "${WARNINGS}" >&2
    exit 1
  fi

  printf 'PASS: documentation freshness checks passed with %d warning(s).\n' "${WARNINGS}"
}

main "$@"
