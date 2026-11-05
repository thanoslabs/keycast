#!/usr/bin/env python3
"""Validate a local Cubism Editor export for the Club Penguin profile."""
import argparse
import json
from pathlib import Path

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("manifest", type=Path)
    args = parser.parse_args()
    manifest = args.manifest.resolve(strict=True)
    data = json.loads(manifest.read_text(encoding="utf-8"))
    refs = data["FileReferences"]
    assets = [refs["Moc"], *refs["Textures"]]
    if "Physics" in refs:
        assets.append(refs["Physics"])
    for group in refs.get("Motions", {}).values():
        assets.extend(entry["File"] for entry in group)
    assets.extend(entry["File"] for entry in refs.get("Expressions", []))
    root = manifest.parent
    for name in assets:
        asset = (root / name).resolve(strict=True)
        if not asset.is_relative_to(root) or not asset.is_file():
            raise ValueError(f"Unsafe asset path: {name}")
    if not str(refs["Moc"]).endswith(".moc3"):
        raise ValueError("Cubism Editor .moc3 export required")
    print(f"Validated {len(assets)} Cubism assets: {manifest}")

if __name__ == "__main__":
    main()
