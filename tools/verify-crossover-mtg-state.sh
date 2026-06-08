#!/usr/bin/env bash
set -euo pipefail

bottle="${1:-MTG}"
bottles_dir="${CROSSOVER_BOTTLES_DIR:-$HOME/Library/Application Support/CrossOver/Bottles}"

export CROSSOVER_BOTTLE_NAME="$bottle"
export CROSSOVER_BOTTLES_DIR="$bottles_dir"

python3 - <<'PY'
from pathlib import Path
import hashlib
import os
import re
import subprocess
import sys


BOTTLE = os.environ["CROSSOVER_BOTTLE_NAME"]
BOTTLES_DIR = Path(os.environ["CROSSOVER_BOTTLES_DIR"])
BASE = BOTTLES_DIR / BOTTLE
INSTALL = BASE / "drive_c" / "Shandalar"
START_MENU_SHORTCUT = (
    BASE
    / "drive_c"
    / "users"
    / "crossover"
    / "AppData"
    / "Roaming"
    / "Microsoft"
    / "Windows"
    / "Start Menu"
    / "Shandalar.lnk"
)
WINE = Path("/Applications/CrossOver.app/Contents/SharedSupport/CrossOver/bin/wine")

EXPECTED_HASHES = {
    "Shandalar.exe": "ebba01ad04aba5fb78841f37b6c264dfd17f1d6ca6ccfcc9851c2972b64f5f6b",
    "Program/Shandalar.exe": "ebba01ad04aba5fb78841f37b6c264dfd17f1d6ca6ccfcc9851c2972b64f5f6b",
    "Magic.exe": "93a40ce2c96aafee1d858a71ed69eb8c539aa9851796eb54b1af58f0bb97aba0",
    "Program/Magic.exe": "685669692634ec830fe228904e11b1b536bd4b20e52192863a6280c2dbff6b66",
    "FaceMaker.exe": "41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246",
    "Program/FaceMaker.exe": "41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246",
    "Shandalar.dll": "f74648745315163da15ffbe32e5bbdbc79e05aaf47c0714902c8d6898e5d00f7",
    "Program/Shandalar.dll": "f74648745315163da15ffbe32e5bbdbc79e05aaf47c0714902c8d6898e5d00f7",
    "CardArtLib.dll": "975111a7f82d4e026a8572c669a678eddea2d5ffa895dce59f6416457e510484",
    "Program/CardArtLib.dll": "975111a7f82d4e026a8572c669a678eddea2d5ffa895dce59f6416457e510484",
    "DeckDLL.dll": "5c122ea5442d209d0d74c7e75f7b1f53492b0bfcc042efce49300f3485e3fcb0",
    "Program/Deckdll.dll": "5c122ea5442d209d0d74c7e75f7b1f53492b0bfcc042efce49300f3485e3fcb0",
    "Drawcardlib.dll": "9f37f131ba4a80ba543bb9372489438ac306cd01363b58cbc5ae8b1ccfd80700",
    "Program/Drawcardlib.dll": "9f37f131ba4a80ba543bb9372489438ac306cd01363b58cbc5ae8b1ccfd80700",
    "Statwin.dll": "f1428cf548810f85df6f26b913d10dca16bc0f06a609a94c0cb0f0308347b0cf",
    "Program/Statwin.dll": "f1428cf548810f85df6f26b913d10dca16bc0f06a609a94c0cb0f0308347b0cf",
    "ManalinkEh.dll": "68f2ba31f26f99edfb0944fe3fbc577ef0a42f9f6a6d7d44cb3aaa5f9b9cadd5",
    "Program/ManalinkEh.dll": "619ce5d3f80f4ac951418e8a1b2ec803b3b9aa0128e01b827e744b80e63962fc",
    "zlib.dll": "9f8729ac49e0ccea86fe3b1a9b2c3fae9986ecd09db92853e7a588dbda85bf90",
    "Program/zlib.dll": "9f8729ac49e0ccea86fe3b1a9b2c3fae9986ecd09db92853e7a588dbda85bf90",
    "libgcc_s_dw2-1.dll": "89f6147f5ed3f271d0b88f0586e079b9ac22e76c31221e5d5013aa273cc4694b",
    "Program/libgcc_s_dw2-1.dll": "89f6147f5ed3f271d0b88f0586e079b9ac22e76c31221e5d5013aa273cc4694b",
    "Cards.dat": "abb2f631bd7897dcebde9d6c4bf61a6ea2e37e30fda42490c37b6f4d60f42e94",
    "Program/Cards.dat": "abb2f631bd7897dcebde9d6c4bf61a6ea2e37e30fda42490c37b6f4d60f42e94",
    "DBInfo.dat": "519ccecb98548c1a2e15fe8025951aafba9f116595b5775a0f2ab2bb393e48c1",
    "Program/DBInfo.dat": "519ccecb98548c1a2e15fe8025951aafba9f116595b5775a0f2ab2bb393e48c1",
    "Rarity.dat": "e0c779a73f0ed780b0c689741805a4e40f7f4949420a8d27fa73137e528ae04f",
    "Program/Rarity.dat": "e0c779a73f0ed780b0c689741805a4e40f7f4949420a8d27fa73137e528ae04f",
    "Program/Manalink.ini": "30153fd22c76b0c0751c538938af46fbf25b1b51d5b4bb2bd9a2eead1b9c2f2b",
    "Program/DuelArt/Modern.dat": "9a2d70be70b70ef27036a47550bc0d549437df0c032a4e0237a217e4731e1aee",
    "Program/DuelArt/Planeswalker.dat": "619e0b9780ec204b9fbf6f48b2eb541c9d8a6f19a73f27d4d76d25828db7d369",
    "Program/TT0530m_.ttf": "51afc07ba27699fec048dd387f6e6068177c0ee4cd95c6483eb378978fdd1cee",
    "Program/TT0127m_.ttf": "e3b5229e753851acab9450fcad1acd9f89412f7bdaebfb6fbf25fc0536ab02d2",
    "Program/TT0085m_.ttf": "e738818f4bbf3f29c68601fe5cb16cb045650e7d1854806e584204fd2686ed4c",
    "Program/TT0298m_.ttf": "a36d52dec6c6216e2dce6f0979c715e5454a0d18647bedd03096f33dbd3d707f",
    "Program/TT0299m_.ttf": "fb1ce5027aa0a0cd3817f559e63fe4d28b6e125c0c32d3635337d2acfb109519",
    "Program/TT0300m_.ttf": "83a70d460edbdc1a804764d6b17de2189765a5eb18cf598e8fa7e88058d67a79",
    "Program/CardArt/ManaSymbols.pic": "60662a25dce90dc8d4cd0b0227fe62c33b50ac95115711428d463770b8d42cbd",
    "Program/CardArt/Expansion_Symbols.pic": "01264f3dd6b9a8b5576b50bba49e951cb3fbdba1d33aee2b7ee8a9530d5e7348",
    "Program/CardArt/Watermarks.pic": "ec276a27c79a8cea55cdcb5474cbc5b96071f3744c18d7fc466ea6c503892c9c",
    "Program/CardArt/CardCounters.png": "8d26128c1932b22f25b84b96d8d01e9b2dce008cd96265f3efabdc0c5f11ecbb",
    "Program/CardArt/Modern/Triggering.png": "a8c94fc5b58540f884e799a1603f65dd61d99af9e50efee4762b4021bedc6f00",
    "Program/CardArt/Modern/CardOv_Nyx.png": "b4bbf12f1f9851e2526ba25ad9b3de147fafa6aa7b1bc4400616b65aaf25209d",
    "Program/CardArt/Planeswalker/LoyaltyBase.png": "7413ba6227b9b07a491a2730e170525ea4744d188e31e2665abc2361ebd6e79e",
    "Program/CardArt/Planeswalker/LoyaltyMinus.png": "89f01e1bda607459ea6560c0b6608a9aab409799c05cd00279fee6d0bfd82cb9",
    "Program/CardArt/Planeswalker/LoyaltyPlus.png": "ad4b8971dd43955ccfd3daf9020b3a6f60c0a8fe9f21b73847c07a81b12af3ef",
    "Program/CardArt/Planeswalker/LoyaltyZero.png": "8faf7ec5225538bcb97b539a1614282007ea484317411806a311f1c2d800ccef",
}

EXPECTED_DLL_CHARACTERISTICS = {
    "Drawcardlib.dll": 0x0000,
    "Program/Drawcardlib.dll": 0x0000,
}

SHANDALAR_LAND_CIP_CAVE = bytes.fromhex(
    "83fe017557833d00d28b0001744ef60534a99300027545813d78d19400db000000"
    "75393935381c8e0075313935b8f47b007529391dc42f8e0075218d149b89d0c1"
    "e00429d08d148752e842ed31ffa90100000074065fe90f0000005a8d149b8b0d"
    "381c8e00e9d38833ffc705ace97b0000000000ff3578d19400ff3564b7d106"
    "ff35b8f47b00ff35c42f8e00814f08000100106a006affba0100000029f252"
    "53566a7e5356e896c832ff83c4208b0424a3c42f8e008b442404a3b8f47b00"
    "8b442408a364b7d1068b44240ca378d19400816708fffeffefc705ace97b00"
    "0000000068000300005356e873972fff83c40c8f05c42f8e008f05b8f47b00"
    "8f0564b7d1068f0578d19400e96d8a33ff"
)

SHANDALAR_PLAYER_TARGET_CAVE = bytes.fromhex(
    "898d3cfdffff837f08017515817f1400100000750c31c0a36cd49400e9fe052bff"
    "b8f8424c00ffd0a16cd49400e9d6052bff0000000000000000000000000000"
)

EXPECTED_HEX_PREFIXES = {
    "Shandalar.exe": [
        (0xCDA2E, bytes.fromhex("9090909090")),
        (0xCDCCD, bytes.fromhex("ff05f09d5800c2140090909090909090")),
        (0xCDD3F, bytes.fromhex("9090909090")),
        (0x16C320, bytes.fromhex("b804000000c3")),
        (0x178010, bytes.fromhex("31c0c3909090")),
    ],
    "Program/Shandalar.exe": [
        (0xCDA2E, bytes.fromhex("9090909090")),
        (0xCDCCD, bytes.fromhex("ff05f09d5800c2140090909090909090")),
        (0xCDD3F, bytes.fromhex("9090909090")),
        (0x16C320, bytes.fromhex("b804000000c3")),
        (0x178010, bytes.fromhex("31c0c3909090")),
    ],
    "Statwin.dll": [
        (0x2A10, bytes.fromhex("b807000000c3")),
    ],
    "Program/Statwin.dll": [
        (0x2A10, bytes.fromhex("b807000000c3")),
    ],
    "Shandalar.dll": [
        (0x94D34, bytes.fromhex("e9c776cc0090909090")),
        (0x1174800, SHANDALAR_LAND_CIP_CAVE),
        (0xCB16, bytes.fromhex("e905fad40090909090909090909090909090")),
        (0x1174920, SHANDALAR_PLAYER_TARGET_CAVE),
    ],
    "Program/Shandalar.dll": [
        (0x94D34, bytes.fromhex("e9c776cc0090909090")),
        (0x1174800, SHANDALAR_LAND_CIP_CAVE),
        (0xCB16, bytes.fromhex("e905fad40090909090909090909090909090")),
        (0x1174920, SHANDALAR_PLAYER_TARGET_CAVE),
    ],
    "Magic.exe": [
        (0x5DB1F, bytes.fromhex("c7055c72780000000000")),
        (0x694B7, bytes.fromhex("6a0068ac5c71008b451050e8c9710000")),
        (0x694EB, bytes.fromhex("6a0068ac5c71008b451050e895710000")),
    ],
    "Program/Magic.exe": [
        (0x5DB1F, bytes.fromhex("c7055c72780000000000")),
        (0x694B7, bytes.fromhex("6a0068ac5c71008b451050e8c9710000")),
        (0x694EB, bytes.fromhex("6a0068ac5c71008b451050e895710000")),
    ],
    "ManalinkEh.dll": [
        (0x3BB035, bytes.fromhex("f60590f14e00040f84ae000000e90100")),
        (0x1A8, bytes.fromhex("00020000")),
        (0x44CB23, bytes.fromhex("e9089b0400")),
        (0x495A30, bytes.fromhex("f7c30000000f7410f60590f14e0004750731d2e9d961fbfff6c3040f844265fbffe9d264fbff")),
        (0x40D0E1, bytes.fromhex("e97a950800909090909090")),
        (0x495A60, bytes.fromhex("89c385c07e0881fb0e0100007e05bb0e010000e9746af7ff")),
        (0x40DB84, bytes.fromhex("e9078b0800")),
        (0x495A90, bytes.fromhex("8d7db431c08b0c85c0f34e00890c874083f81075f0ba010000002b5508c1e20531c08b8c82a0f44e00898c82c0f34e004083f80875ece9eb74f7ff")),
        (0x429ACF, bytes.fromhex("e92ccc06009090909090")),
        (0x3FE7A0, bytes.fromhex("c744240c00000000b80100000029d88944240889742404891c24e881cd060085c00f84bfffffff89742404891c24e89dec0100c7442404010000008b4074890424e88aea0300e99bffffff")),
        (0x3F63E0, bytes.fromhex("c744240c00000000b80100000029d88944240889742404891c24e84151070085c00f84bfffffff89742404891c24e85d7002008b4074890424e8c2970200e9a3ffffff9090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090")),
        (0x469583, bytes.fromhex("e948d10200909090909090909090909090")),
        (0x495AD0, bytes.fromhex("898d18fdffff817d1c00100000750d31d28915e42f7a00e9a82efdffe85f1f00fe8b15e42f7a00e9982efdff")),
        (0x3F63BD, bytes.fromhex("e8ceed0300")),
        (0x3FE77D, bytes.fromhex("e80e6a0300")),
        (0x495B00, bytes.fromhex("83fb017542833d74857200017439813d782d7a00db000000752d391d1c7e73007525391d7cc16200751d3935209a730075156a015653e8d5c4f9ff83c40c85c07405e90f000000c705f8e9600000000000e98333f9ffc705f8e9600000000000ff35782d7a00ff35a4025902ff357cc16200ff35209a73005653e8f16cf8ff83c40889c7814f08000100106affba0100000029da526a7e5653e8122af9ff83c4148b0424a3209a73008b442404a37cc162008b442408a3a40259028b44240ca3782d7a00816708fffeffef68000300005653e8d9e7f9fd83c40c8f05209a73008f057cc162008f05a40259028f05782d7a00e94734f9ff")),
    ],
    "Program/ManalinkEh.dll": [
        (0x381A25, bytes.fromhex("f60590f14e00040f84ae000000e90100")),
        (0x1A8, bytes.fromhex("00020000")),
        (0x40F115, bytes.fromhex("e916450400")),
        (0x452C30, bytes.fromhex("f7c30000000f7410f60590f14e0004750731d2e9d9b7fbfff6c3040f8434bbfbffe9c4bafbff")),
        (0x3D2DA1, bytes.fromhex("e9ba080800909090909090")),
        (0x452C60, bytes.fromhex("89c385c07e0881fb0e0100007e05bb0e010000e934f7f7ff")),
        (0x3D3844, bytes.fromhex("e947fe0700")),
        (0x452C90, bytes.fromhex("8d7db431c08b0c85c0f34e00890c874083f81075f0ba010000002b5508c1e20531c08b8c82a0f44e00898c82c0f34e004083f80875ece9ab01f8ff")),
        (0x3EC7CF, bytes.fromhex("e92c6f06009090909090")),
        (0x3C4930, bytes.fromhex("c744240c00000000b80100000029d88944240889742404891c24e8c16a060085c00f84bfffffff89742404891c24e80dd60100c7442404010000008b4074890424e80aaf0300e99bffffff")),
        (0x3BC630, bytes.fromhex("c744240c00000000b80100000029d88944240889742404891c24e8c1ed060085c00f84bfffffff89742404891c24e80d5902008b4074890424e8e27c0200e9a3ffffff9090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090")),
        (0x429453, bytes.fromhex("e978a20200909090909090909090909090")),
        (0x452CD0, bytes.fromhex("898d18fdffff817d1c00100000750d31d28915e42f7a00e9785dfdffe85f4f04fe8b15e42f7a00e9685dfdff")),
        (0x3BC60D, bytes.fromhex("e81ead0300")),
        (0x3C490D, bytes.fromhex("e81e2a0300")),
        (0x452D00, bytes.fromhex("83fb017542833d74857200017439813d782d7a00db000000752d391d1c7e73007525391d7cc16200751d3935209a730075156a015653e8251afaff83c40c85c07405e90f000000c705f8e9600000000000e98390f9ffc705f8e9600000000000ff35782d7a00ff3524055402ff357cc16200ff35209a73005653e8f1e7f8ff83c40889c7814f08000100106affba0100000029da526a7e5653e89289f9ff83c4148b0424a3209a73008b442404a37cc162008b442408a3240554028b44240ca3782d7a00816708fffeffef68000300005653e8d917fefd83c40c8f05209a73008f057cc162008f05240554028f05782d7a00e94791f9ff")),
    ],
}

EXPECTED_SUMMONED_WIZARD_DECKS = {
    "0016": 60,
    "0283": 60,
    "0150": 60,
    "0076": 56,
    "0102": 60,
}

FACE_SUPPORT = [
    "FaceData.txt",
    "FaceButtons.txt",
    "FaceArt",
    "Program/FaceData.txt",
    "Program/FaceButtons.txt",
    "Program/FaceArt",
]


def fail(message: str) -> None:
    print(f"FAIL: {message}", file=sys.stderr)
    sys.exit(1)


def ok(message: str) -> None:
    print(f"ok: {message}")


def read_text(path: Path) -> str:
    if not path.is_file():
        fail(f"missing {path}")
    return path.read_text(encoding="utf-8", errors="replace")


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as handle:
        for chunk in iter(lambda: handle.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def expect_file(path: Path) -> None:
    if not path.exists():
        fail(f"missing {path}")


def expect_hash(rel_path: str, expected: str) -> None:
    path = INSTALL / rel_path
    expect_file(path)
    actual = sha256_file(path)
    if actual != expected:
        fail(f"{rel_path} sha256 {actual} != {expected}")


def expect_same_hash(lhs_rel_path: str, rhs_rel_path: str) -> None:
    lhs = INSTALL / lhs_rel_path
    rhs = INSTALL / rhs_rel_path
    expect_file(lhs)
    expect_file(rhs)
    lhs_hash = sha256_file(lhs)
    rhs_hash = sha256_file(rhs)
    if lhs_hash != rhs_hash:
        fail(f"{lhs_rel_path} sha256 {lhs_hash} != {rhs_rel_path} sha256 {rhs_hash}")


def expect_hex_prefix(rel_path: str, offset: int, expected: bytes) -> None:
    path = INSTALL / rel_path
    expect_file(path)
    with path.open("rb") as handle:
        handle.seek(offset)
        actual = handle.read(len(expected))
    if actual != expected:
        fail(
            f"{rel_path} hex at 0x{offset:x} {actual.hex()} != {expected.hex()}"
        )


def pe_dll_characteristics(path: Path) -> int:
    data = path.read_bytes()
    if len(data) < 0x40:
        fail(f"{path} is too small to contain a PE header")
    e_lfanew = int.from_bytes(data[0x3C:0x40], "little")
    offset = e_lfanew + 4 + 20 + 0x46
    if len(data) < offset + 2:
        fail(f"{path} is too small to contain PE DllCharacteristics")
    return int.from_bytes(data[offset : offset + 2], "little")


def expect_pe_dll_characteristics(rel_path: str, expected: int) -> None:
    path = INSTALL / rel_path
    expect_file(path)
    actual = pe_dll_characteristics(path)
    if actual != expected:
        fail(f"{rel_path} PE DllCharacteristics 0x{actual:04x} != 0x{expected:04x}")


def expect_no_variant_markers(rel_path: str) -> None:
    text = read_text(INSTALL / rel_path)
    for line_number, line in enumerate(text.splitlines(), 1):
        if re.match(r"^\.[vV]", line):
            fail(f"{rel_path}:{line_number} has summoned-wizard unsafe variant marker {line!r}")


def expect_deck_card_total(rel_path: str, expected_cards: int) -> None:
    text = read_text(INSTALL / rel_path)
    actual = 0
    for line in text.splitlines():
        match = re.match(r"^\.[0-9]+\s+(-?[0-9]+)\b", line)
        if match:
            actual += int(match.group(1))
    if actual != expected_cards:
        fail(f"{rel_path} has {actual} deck cards, expected {expected_cards}")


def expect_summoned_wizard_deck_pair(deck_id: str, expected_cards: int) -> None:
    root_deck = f"decks/{deck_id}.dck"
    program_deck = f"Program/decks/{deck_id}.dck"
    expect_same_hash(root_deck, program_deck)
    expect_no_variant_markers(root_deck)
    expect_no_variant_markers(program_deck)
    expect_deck_card_total(root_deck, expected_cards)
    expect_deck_card_total(program_deck, expected_cards)


def expect_save_decks_at_minimum(min_deck_size: int = 40) -> None:
    saves = sorted(
        path
        for path in INSTALL.glob("MAGIC*.SVE")
        if re.match(r"^MAGIC[0-9a-d]\.SVE$", path.name, re.IGNORECASE)
    )
    if not saves:
        fail(f"no MAGIC*.SVE saves found under {INSTALL}")

    for save in saves:
        data = save.read_bytes()
        offset = 0x1420
        counts = {1: 0, 2: 0, 3: 0}
        while offset + 4 <= len(data):
            low = data[offset]
            high = data[offset + 1]
            if low == 0xFF and high == 0xFF:
                break
            deck_mask = data[offset + 2]
            for deck, mask in ((1, 0x01), (2, 0x02), (3, 0x04)):
                if deck_mask & mask:
                    counts[deck] += 1
            offset += 4
        else:
            fail(f"{save.name} deck table terminator not found")

        low_decks = [
            f"deck{deck}={count}"
            for deck, count in sorted(counts.items())
            if count and count < min_deck_size
        ]
        if counts[1] == 0:
            low_decks.insert(0, "deck1=0")
        if low_decks:
            fail(f"{save.name} has under-minimum campaign deck count(s): {', '.join(low_decks)}")


def expect_shortcut_target(path: Path, target: str) -> None:
    expect_file(path)
    data = path.read_bytes()
    ascii_target = target.encode("ascii")
    utf16_target = target.encode("utf-16le")
    if ascii_target not in data and utf16_target not in data:
        fail(f"{path} does not point at {target}")


def section_has_value(text: str, section: str, raw_value: str) -> bool:
    in_section = False
    section_header = f"[{section}]"
    for line in text.splitlines():
        stripped = line.strip()
        if stripped == section_header or stripped.startswith(f"{section_header} "):
            in_section = True
            continue
        if in_section and stripped.startswith("["):
            return False
        if in_section and stripped == raw_value:
            return True
    return False


expect_file(BASE)
expect_file(INSTALL)
ok(f"CrossOver bottle install exists: {INSTALL}")

if WINE.is_file():
    version = subprocess.check_output([str(WINE), "--version"], text=True)
    if "Product Name: CrossOver" not in version or "Product Version: 26.1.0.39808" not in version:
        fail("unexpected CrossOver wine version output")
    ok("CrossOver wine helper reports 26.1.0.39808")
else:
    fail(f"missing CrossOver wine helper: {WINE}")

for rel_path, expected in EXPECTED_HASHES.items():
    expect_hash(rel_path, expected)
ok(f"patched bottle runtime hashes match docs ({len(EXPECTED_HASHES)} checked)")

for rel_path, expected in EXPECTED_DLL_CHARACTERISTICS.items():
    expect_pe_dll_characteristics(rel_path, expected)
ok("patched bottle Drawcardlib PE startup flags are legacy-safe")

for rel_path, checks in EXPECTED_HEX_PREFIXES.items():
    for offset, expected in checks:
        expect_hex_prefix(rel_path, offset, expected)
ok("patched bottle representative byte checks match docs")

for rel_path in FACE_SUPPORT:
    expect_file(INSTALL / rel_path)
ok("FaceMaker support files are present in root and Program paths")

for deck_id, expected_cards in EXPECTED_SUMMONED_WIZARD_DECKS.items():
    expect_summoned_wizard_deck_pair(deck_id, expected_cards)
ok("patched bottle summoned-wizard deck handoff files match docs")

expect_save_decks_at_minimum()
ok("MTG save decks have no populated deck below the 40-card minimum")

expect_shortcut_target(START_MENU_SHORTCUT, r"C:\Shandalar\Shandalar.exe")
ok("MTG Start Menu shortcut targets root Shandalar.exe")

for rel_path in ["Shandalar.ini", "Program/Shandalar.ini"]:
    text = read_text(INSTALL / rel_path)
    if not re.search(r"(?m)^Window\s*=\s*2$", text):
        fail(f"{rel_path} does not contain Window = 2")
    if not re.search(r"(?m)^AiDecisionTime\s*=\s*270$", text):
        fail(f"{rel_path} does not contain AiDecisionTime = 270")
ok("bottle Shandalar ini files use Window = 2")

for rel_path in ["config.txt", "Program/config.txt"]:
    text = read_text(INSTALL / rel_path)
    if not re.search(r"(?m)^AiDecisionTime:270$", text):
        fail(f"{rel_path} does not contain AiDecisionTime:270")
ok("bottle config files use AiDecisionTime:270")

user_reg = read_text(BASE / "user.reg")
if not section_has_value(
    user_reg,
    "Software\\\\MicroProse\\\\Magic: The Gathering\\\\DuelOptions",
    '"ShowCoinFlips"="0"',
):
    fail("DuelOptions ShowCoinFlips=0 not found")
ok("MTG DuelOptions disables coin-flip animations")

for exe in ["FaceMaker.exe", "Magic.exe", "Shandalar.exe"]:
    if not section_has_value(user_reg, f"Software\\\\Wine\\\\AppDefaults\\\\{exe}", '"Version"="win7"'):
        fail(f"{exe} app-default Version=win7 not found")
    if not section_has_value(user_reg, f"Software\\\\Wine\\\\AppDefaults\\\\{exe}\\\\Explorer", '"Desktop"="Shandalar1440"'):
        fail(f"{exe} app-default Desktop=Shandalar1440 not found")

if not section_has_value(
    user_reg,
    "Software\\\\Wine\\\\Explorer\\\\Desktops",
    '"Shandalar1440"="1440x1080"',
):
    fail("Shandalar1440=1440x1080 desktop setting not found")
ok("MTG app-default win7 and Shandalar1440 desktop settings are present")

system_reg = read_text(BASE / "system.reg")
for needle in [
    '"ProductName"="Microsoft Windows 7"',
    '"CurrentVersion"="6.1"',
    '"PagingFiles"=str(7):"C:\\\\pagefile.sys 512 1024\\0"',
]:
    if needle not in system_reg:
        fail(f"system.reg missing {needle}")
ok("MTG system registry has Windows 7 identity and paging file setting")

print("CrossOver MTG state checks passed.")
PY
