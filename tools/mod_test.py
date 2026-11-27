#!/usr/bin/env python3
"""mod_test.py -- `make mod-test`: the asset round trip into a modded ROM, checked statically.

  1. extract rom.z64 and pack without edits; `make mod ASSETS=packed` must give the retail SHA-1;
  2. append bytes to one str member, pack, `make mod ASSETS=packed`; the ROM's checksum must verify, and extracting it
     must give the edited member (larger, so every later member moved) and every other member unchanged.

Work happens in build/mod-test/. Skips (exit 0) when rom.z64 is missing.
"""

import hashlib
import json
import os
import subprocess
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import assets  # noqa: E402

ROOT = os.path.dirname(HERE)
WORK = os.path.join(ROOT, "build", "mod-test")
APPEND = bytes(range(37))
MEMBER = 0


def run(*cmd):
    subprocess.check_call(cmd, cwd=ROOT, stdout=subprocess.DEVNULL)


def make_mod(ext, name):
    out = os.path.join("build", "mod-test", name)
    run("make", "mod", "ASSETS=packed", "PACKED=" + os.path.join(ext, "packed"), "MOD=" + out)
    return os.path.join(ROOT, out, "superbowling.z64")


def fail(msg):
    print("MOD-TEST FAIL: " + msg, file=sys.stderr)
    return 1


def main():
    rom_path = os.path.join(ROOT, "rom.z64")
    if not os.path.isfile(rom_path):
        print("MOD-TEST SKIP: rom.z64 is missing")
        return 0
    os.makedirs(WORK, exist_ok=True)
    ext = os.path.join(WORK, "ext")
    run(sys.executable, "tools/assets.py", "extract", "rom.z64", "-d", ext)
    run(sys.executable, "tools/assets.py", "pack", "-d", ext)
    rom = open(make_mod(ext, "same"), "rb").read()
    if hashlib.sha1(rom).hexdigest() != assets.ROM_SHA1:
        return fail("unedited pack does not link to the retail ROM")
    print("(a) unedited pack: retail SHA-1")

    man = json.load(open(os.path.join(ext, "manifest.json")))
    member = man["str"]["members"][MEMBER]
    if member["file"].endswith(".png"):
        return fail("member %d is an image" % MEMBER)
    path = os.path.join(ext, member["file"])
    old = open(path, "rb").read()
    with open(path, "wb") as f:
        f.write(old + APPEND)
    run(sys.executable, "tools/assets.py", "pack", "-d", ext)
    modrom = make_mod(ext, "edited")
    if subprocess.call([sys.executable, "tools/n64crc.py", modrom], cwd=ROOT, stdout=subprocess.DEVNULL):
        return fail("checksum does not verify")
    back = os.path.join(WORK, "back")
    run(sys.executable, "tools/assets.py", "extract", modrom, "-d", back, "--modded")
    new = open(os.path.join(back, member["file"]), "rb").read()
    if new != old + APPEND:
        return fail("member %d did not come back edited" % MEMBER)
    bman = json.load(open(os.path.join(back, "manifest.json")))
    fat = {e["name"]: (e["rom"], e["end"]) for e in bman["files"]}
    oldfat = {e["name"]: (e["rom"], e["end"]) for e in man["files"]}
    if fat["STR     SYS"][1] - fat["STR     SYS"][0] <= oldfat["STR     SYS"][1] - oldfat["STR     SYS"][0]:
        return fail("the str segment did not grow")
    for name, (start, end) in fat.items():
        if start % 2:
            return fail("%s starts at odd ROM address 0x%X" % (name, start))
    nm = len(man["str"]["members"])
    for n in range(nm):
        f = man["str"]["members"][n]["file"]
        if n != MEMBER and open(os.path.join(back, f), "rb").read() != open(os.path.join(ext, f), "rb").read():
            return fail("str member %d changed" % n)
    for e in man["files"]:
        if e["file"] and e["name"] != assets.IDX_NAME and open(os.path.join(back, e["file"]), "rb").read() != open(os.path.join(ext, e["file"]), "rb").read():
            return fail("%s changed" % e["name"])
    print("(b) edited member %d: ROM re-extracts with it, %d other members and %d other files unchanged (IDX changed), checksum OK"
          % (MEMBER, nm - 1, len(man["files"]) - 2))
    print("MOD-TEST OK")
    return 0


if __name__ == "__main__":
    sys.exit(main())
