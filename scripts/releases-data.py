#!/usr/bin/env python3
"""Write website/_data/releases.json from the GitHub Releases API.

Download list is Releases only. Tag-only tags are omitted on purpose.
News bullets come from the release body (`## Changes in …`), not from
re-parsing NEWS in CI.
"""

from __future__ import annotations

import argparse
import json
import os
import re
import sys
import urllib.error
import urllib.request
from pathlib import Path

CHANGES_HEAD = re.compile(r"^#{1,2} Changes in\b")
BULLET = re.compile(r"^[\*\-] (.+)$")


def _api(url: str, token: str | None) -> object:
    headers = {
        "Accept": "application/vnd.github+json",
        "User-Agent": "osm-gps-map-pages",
    }
    if token:
        headers["Authorization"] = f"Bearer {token}"
    req = urllib.request.Request(url, headers=headers)
    with urllib.request.urlopen(req, timeout=30) as resp:
        return json.load(resp)


def tar_asset(rel: dict) -> str | None:
    for asset in rel.get("assets") or []:
        name = asset.get("name") or ""
        url = asset.get("browser_download_url") or ""
        if name.endswith(".tar.gz") and url:
            return str(url)
    return None


def archive_url(repo: str, tag: str, asset: str | None) -> str:
    if asset:
        return asset
    return f"https://github.com/{repo}/archive/refs/tags/{tag}.tar.gz"


def changes_from_body(body: str) -> list[str]:
    items: list[str] = []
    in_changes = False
    for line in (body or "").splitlines():
        if CHANGES_HEAD.match(line.strip()):
            in_changes = True
            continue
        if in_changes and line.startswith("## "):
            break
        if not in_changes:
            continue
        b = BULLET.match(line.strip())
        if b:
            items.append(b.group(1).strip())
    return items


def fetch_releases(repo: str, token: str | None) -> list[dict]:
    out: list[dict] = []
    page = 1
    while True:
        data = _api(
            f"https://api.github.com/repos/{repo}/releases?per_page=100&page={page}",
            token,
        )
        if not isinstance(data, list) or not data:
            break
        for item in data:
            if item.get("draft"):
                continue
            tag = item.get("tag_name")
            if not tag:
                continue
            body = item.get("body") or ""
            out.append(
                {
                    "tag": str(tag),
                    "date": (item.get("published_at") or "")[:10],
                    "prerelease": bool(item.get("prerelease")),
                    "html_url": str(
                        item.get("html_url")
                        or f"https://github.com/{repo}/releases/tag/{tag}"
                    ),
                    "archive_url": archive_url(repo, str(tag), tar_asset(item)),
                    "changes": changes_from_body(body),
                }
            )
        if len(data) < 100:
            break
        page += 1
    return out


def payload(repo: str, rels: list[dict]) -> dict:
    published = [r for r in rels if not r["prerelease"]]
    latest = published[0] if published else (rels[0] if rels else None)
    return {
        "repo": repo,
        "clone_url": f"https://github.com/{repo}.git",
        "master_tar": f"https://github.com/{repo}/archive/refs/heads/master.tar.gz",
        "master_zip": f"https://github.com/{repo}/archive/refs/heads/master.zip",
        "latest": latest,
        "archives": rels,
    }


def main(argv: list[str] | None = None) -> int:
    p = argparse.ArgumentParser()
    p.add_argument("--repo", required=True)
    p.add_argument("--out", type=Path, required=True)
    p.add_argument("--token", default=os.environ.get("GITHUB_TOKEN", ""))
    args = p.parse_args(argv)
    try:
        rels = fetch_releases(args.repo, args.token or None)
    except urllib.error.HTTPError as exc:
        raise SystemExit(f"releases API failed: {exc}") from exc
    args.out.parent.mkdir(parents=True, exist_ok=True)
    args.out.write_text(
        json.dumps(payload(args.repo, rels), indent=2) + "\n",
        encoding="utf-8",
    )
    return 0


SAMPLE_BODY = """# osm-gps-map lab-1

## Changes in lab-1

- jekyll: boilerplate landing page
- ci: Actions deploys Pages

## Source tarball

- File: `osm-gps-map-lab-1.tar.gz`
"""

SAMPLE_STAR_BODY = """# osm-gps-map 1.2.1

## Changes in 1.2.1
  * Replace deprecated G_TYPE_INSTANCE_GET_PRIVATE
  * Add python unit tests (Patrick Salecker)
"""


def _self_check() -> None:
    assert changes_from_body(SAMPLE_BODY) == [
        "jekyll: boilerplate landing page",
        "ci: Actions deploys Pages",
    ]
    assert changes_from_body(SAMPLE_STAR_BODY) == [
        "Replace deprecated G_TYPE_INSTANCE_GET_PRIVATE",
        "Add python unit tests (Patrick Salecker)",
    ]
    assert changes_from_body("no heading\n- nope") == []
    data = payload(
        "nzjrs/osm-gps-map",
        [
            {
                "tag": "lab-1",
                "date": "2026-09-10",
                "prerelease": False,
                "html_url": "https://github.com/nzjrs/osm-gps-map/releases/tag/lab-1",
                "archive_url": archive_url("nzjrs/osm-gps-map", "lab-1", None),
                "changes": ["jekyll: boilerplate landing page"],
            }
        ],
    )
    assert data["latest"]["tag"] == "lab-1"
    assert len(data["archives"]) == 1
    empty = payload("nzjrs/osm-gps-map", [])
    assert empty["latest"] is None
    assert empty["archives"] == []
    print("releases-data self-check ok")


if __name__ == "__main__":
    if len(sys.argv) > 1 and sys.argv[1] == "--self-check":
        _self_check()
    else:
        raise SystemExit(main())
