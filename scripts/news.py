#!/usr/bin/env python3
"""Parse, cut, and format the handwritten NEWS file.

Sections look like:

    Changes in master
    =================
    - newest unreleased item

    Changes in 1.2.1
    ================
    - shipped item

Older sections used '  * ' bullets and a long underline; both still parse.
"""

from __future__ import annotations

import argparse
import html
import re
import sys
from pathlib import Path

HEADING = re.compile(r"^Changes in (.+?)\s*$")
UNDERLINE = re.compile(r"^=+\s*$")
BULLET = re.compile(r"^(?:  )?[\*\-] (.+)$")


def underline_for(title: str) -> str:
    return "=" * len(title)


def parse(text: str) -> list[tuple[str, list[str]]]:
    """Return [(label, bullets), ...] in file order."""
    sections: list[tuple[str, list[str]]] = []
    label: str | None = None
    items: list[str] = []
    pending: str | None = None

    def flush_item() -> None:
        nonlocal pending
        if pending is not None:
            items.append(pending)
            pending = None

    def flush_section() -> None:
        nonlocal label, items
        flush_item()
        if label is not None:
            sections.append((label, items))
        label = None
        items = []

    for raw in text.splitlines():
        m = HEADING.match(raw)
        if m:
            flush_section()
            label = m.group(1).strip()
            continue
        if label is None:
            continue
        if UNDERLINE.match(raw):
            continue
        b = BULLET.match(raw)
        if b:
            flush_item()
            pending = b.group(1).strip()
            continue
        stripped = raw.strip()
        if not stripped:
            continue
        if pending is not None and (raw.startswith("    ") or raw.startswith("\t")):
            pending += " " + stripped
            continue
        # Nested '    * foo' under a parent bullet: keep as its own item.
        nested = re.match(r"^\s+[\*\-] (.+)$", raw)
        if nested:
            flush_item()
            pending = nested.group(1).strip()
    flush_section()
    return sections


def section(text: str, name: str) -> list[str]:
    want = name.strip()
    for label, items in parse(text):
        if label == want:
            return items
    raise SystemExit(f"NEWS has no 'Changes in {want}' section")


def html_items(items: list[str]) -> str:
    if not items:
        return "      <li>No NEWS entries.</li>"
    return "\n".join(f"      <li>{html.escape(i)}</li>" for i in items)


def format_section(label: str, items: list[str]) -> str:
    title = f"Changes in {label}"
    lines = [title, underline_for(title)]
    lines.extend(f"- {i}" for i in items)
    return "\n".join(lines) + "\n"


def cut_master(text: str, version: str) -> str:
    parts = parse(text)
    if not parts or parts[0][0] != "master":
        raise SystemExit("NEWS top section must be 'Changes in master' before cutting a release")
    _, items = parts[0]
    if not items:
        raise SystemExit("Changes in master is empty; nothing to cut")
    m = re.search(r"^Changes in master\s*\n(?:=+\s*\n)?", text, re.M)
    if not m:
        raise SystemExit("cannot find 'Changes in master' heading")
    rest = text[m.end() :]
    nxt = re.search(r"^Changes in ", rest, re.M)
    tail = rest[nxt.start() :] if nxt else ""
    head = format_section("master", []) + "\n" + format_section(version, items) + "\n"
    return head + tail.lstrip("\n")


def github_notes(
    text: str,
    tag: str,
    *,
    project: str,
    intro: str = "",
    tarball: str = "",
    sha256: str = "",
    git_sha: str = "",
) -> str:
    try:
        items = section(text, tag)
        label = tag
    except SystemExit:
        items = section(text, "master")
        label = "master"
    lines = [f"# {project} {tag}", ""]
    if intro:
        lines += [intro.strip(), ""]
    lines += [f"## Changes in {label}", ""]
    if items:
        lines.extend(f"- {i}" for i in items)
    else:
        lines.append("- (no NEWS entries)")
    if tarball or sha256 or git_sha:
        lines += ["", "## Source tarball"]
        if tarball:
            lines.append(f"- File: `{tarball}`")
        if sha256:
            lines.append(f"- sha256: `{sha256}`")
        if git_sha:
            lines.append(f"- Git tag: `{tag}` → `{git_sha}`")
    return "\n".join(lines).rstrip() + "\n"


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser(prog="news.py")
    sub = p.add_subparsers(dest="cmd", required=True)

    p_sec = sub.add_parser("section", help="print bullets for a section")
    p_sec.add_argument("file", type=Path)
    p_sec.add_argument("name")

    p_cut = sub.add_parser("cut", help="promote Changes in master to Changes in VERSION")
    p_cut.add_argument("file", type=Path)
    p_cut.add_argument("version")
    p_cut.add_argument("-o", "--output", type=Path)

    p_notes = sub.add_parser("github-notes", help="GitHub release body from NEWS")
    p_notes.add_argument("file", type=Path)
    p_notes.add_argument("tag")
    p_notes.add_argument("--project", default="osm-gps-map")
    p_notes.add_argument("--intro", default="")
    p_notes.add_argument("--tarball", default="")
    p_notes.add_argument("--sha256", default="")
    p_notes.add_argument("--git-sha", default="")

    args = p.parse_args(argv)
    text = args.file.read_text(encoding="utf-8")
    if args.cmd == "section":
        for item in section(text, args.name):
            print(f"- {item}")
        return 0
    if args.cmd == "cut":
        new = cut_master(text, args.version)
        dest = args.output or args.file
        dest.write_text(new, encoding="utf-8")
        return 0
    if args.cmd == "github-notes":
        sys.stdout.write(
            github_notes(
                text,
                args.tag,
                project=args.project,
                intro=args.intro,
                tarball=args.tarball,
                sha256=args.sha256,
                git_sha=args.git_sha,
            )
        )
        return 0
    raise SystemExit("unknown command")


FIXTURE = """Changes in master
=================
- pages: News from NEWS file
- ci: publish on tag push

Changes in 1.2.1
  * Replace deprecated G_TYPE_INSTANCE_GET_PRIVATE
  * Add python unit tests (Patrick Salecker)

Changes in 1.2.0
======================
  * Change max allowed zoom for various maps
"""


def _self_check() -> None:
    secs = parse(FIXTURE)
    assert secs[0][0] == "master"
    assert secs[0][1] == [
        "pages: News from NEWS file",
        "ci: publish on tag push",
    ]
    assert section(FIXTURE, "1.2.1") == [
        "Replace deprecated G_TYPE_INSTANCE_GET_PRIVATE",
        "Add python unit tests (Patrick Salecker)",
    ]
    title = "Changes in master"
    assert underline_for(title) == "================="
    assert len(underline_for("Changes in 1.2.1")) == len("Changes in 1.2.1")
    cut = cut_master(FIXTURE, "lab-4")
    assert cut.startswith(
        "Changes in master\n=================\n\nChanges in lab-4\n================\n-"
    )
    assert "- pages: News from NEWS file" in cut
    assert section(cut, "master") == []
    assert section(cut, "lab-4")[0].startswith("pages:")
    notes = github_notes(cut, "lab-4", project="ogmapala", git_sha="deadbeef")
    assert notes.startswith("# ogmapala lab-4\n")
    assert "## Changes in lab-4" in notes
    assert "- pages: News from NEWS file" in notes
    assert "deadbeef" in notes
    real = Path(__file__).resolve().parents[1] / "NEWS"
    if real.is_file():
        items = section(real.read_text(encoding="utf-8"), "1.2.1")
        assert len(items) >= 10
    print("news self-check ok")


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--self-check":
        _self_check()
    else:
        raise SystemExit(main())
