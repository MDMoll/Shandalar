#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: tools/verify-install-tree.sh [install-root]

Verifies that an installed Shandalar tree contains the current patched runtime
files and adjacent Program/ assets needed by the active repo layout. This is a
static file/hash/patch-byte check; it does not launch the game or prove
gameplay.

If install-root is omitted, the current directory is checked.
EOF
}

if [ "${1:-}" = "--help" ] || [ "${1:-}" = "-h" ]; then
  usage
  exit 0
fi

install_root="${1:-.}"

fail() {
  printf 'FAIL: %s\n' "$*" >&2
  exit 1
}

pass() {
  printf 'ok: %s\n' "$*"
}

expect_file() {
  local path="$1"
  [ -e "$path" ] || fail "missing $path"
}

expect_hash() {
  local path="$1"
  local expected="$2"
  expect_file "$path"
  local actual
  actual="$(shasum -a 256 "$path" | awk '{print $1}')"
  [ "$actual" = "$expected" ] || fail "$path sha256 $actual != $expected"
}

expect_same_hash() {
  local lhs="$1"
  local rhs="$2"
  expect_file "$lhs"
  expect_file "$rhs"
  local lhs_hash rhs_hash
  lhs_hash="$(shasum -a 256 "$lhs" | awk '{print $1}')"
  rhs_hash="$(shasum -a 256 "$rhs" | awk '{print $1}')"
  [ "$lhs_hash" = "$rhs_hash" ] || fail "$lhs sha256 $lhs_hash != $rhs sha256 $rhs_hash"
}

expect_hex_prefix() {
  local path="$1"
  local offset="$2"
  local length="$3"
  local expected="$4"
  expect_file "$path"
  local actual
  actual="$(xxd -p -l "$length" -s "$offset" "$path" | tr -d '\n')"
  [ "$actual" = "$expected" ] || fail "$path hex at $offset $actual != $expected"
}

expect_text_match() {
  local path="$1"
  local pattern="$2"
  expect_file "$path"
  grep -Eq "$pattern" "$path" || fail "$path does not match $pattern"
}

expect_no_text_match() {
  local path="$1"
  local pattern="$2"
  expect_file "$path"
  ! grep -Eq "$pattern" "$path" || fail "$path unexpectedly matches $pattern"
}

expect_deck_card_total() {
  local path="$1"
  local expected="$2"
  expect_file "$path"
  local actual
  actual="$(LC_ALL=C awk 'BEGIN{sum=0} /^\.[0-9]/{sum+=$2} END{print sum}' "$path")"
  [ "$actual" = "$expected" ] || fail "$path has $actual deck cards, expected $expected"
}

expect_plain_summoned_wizard_deck_pair() {
  local id="$1"
  local expected_cards="$2"
  expect_same_hash "decks/$id.dck" "Program/decks/$id.dck"
  expect_no_text_match "decks/$id.dck" '^\.[vV]'
  expect_no_text_match "Program/decks/$id.dck" '^\.[vV]'
  expect_deck_card_total "decks/$id.dck" "$expected_cards"
  expect_deck_card_total "Program/decks/$id.dck" "$expected_cards"
}

[ -d "$install_root" ] || fail "install root is not a directory: $install_root"
cd "$install_root"
pass "install root exists: $(pwd)"

expect_hash Shandalar.exe 17f7af843fd2fd5424e7d36d547f4315d20fdfa840fb5050a96ab9a727a181f6
expect_hash Program/Shandalar.exe 17f7af843fd2fd5424e7d36d547f4315d20fdfa840fb5050a96ab9a727a181f6
expect_hash FaceMaker.exe 41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246
expect_hash Program/FaceMaker.exe 41f062874f94d732cc4feb40b568728b8462879fd3ec2bc55810f118e9c5f246
expect_hash FaceMaker-nores.exe 43331d22d05787979af0d29cea1775fd3bcebf8acdb3c3be34524e9ca7762f4b
expect_hash Program/FaceMaker-nores.exe 43331d22d05787979af0d29cea1775fd3bcebf8acdb3c3be34524e9ca7762f4b
expect_hash Magic.exe 93a40ce2c96aafee1d858a71ed69eb8c539aa9851796eb54b1af58f0bb97aba0
expect_hash Program/Magic.exe 685669692634ec830fe228904e11b1b536bd4b20e52192863a6280c2dbff6b66
expect_hash Shandalar.dll f74648745315163da15ffbe32e5bbdbc79e05aaf47c0714902c8d6898e5d00f7
expect_hash Program/Shandalar.dll f74648745315163da15ffbe32e5bbdbc79e05aaf47c0714902c8d6898e5d00f7
expect_hash CardArtLib.dll 975111a7f82d4e026a8572c669a678eddea2d5ffa895dce59f6416457e510484
expect_hash Program/CardArtLib.dll 975111a7f82d4e026a8572c669a678eddea2d5ffa895dce59f6416457e510484
expect_hash DeckDLL.dll 5c122ea5442d209d0d74c7e75f7b1f53492b0bfcc042efce49300f3485e3fcb0
expect_hash Program/Deckdll.dll 5c122ea5442d209d0d74c7e75f7b1f53492b0bfcc042efce49300f3485e3fcb0
expect_hash Drawcardlib.dll 79096fd15ef22ed50f84aee681e48c6c3e678690c48e71f5430a03beee5cb7d1
expect_hash Program/Drawcardlib.dll 79096fd15ef22ed50f84aee681e48c6c3e678690c48e71f5430a03beee5cb7d1
expect_hash Statwin.dll f1428cf548810f85df6f26b913d10dca16bc0f06a609a94c0cb0f0308347b0cf
expect_hash Program/Statwin.dll f1428cf548810f85df6f26b913d10dca16bc0f06a609a94c0cb0f0308347b0cf
expect_hash ManalinkEh.dll 68f2ba31f26f99edfb0944fe3fbc577ef0a42f9f6a6d7d44cb3aaa5f9b9cadd5
expect_hash Program/ManalinkEh.dll 619ce5d3f80f4ac951418e8a1b2ec803b3b9aa0128e01b827e744b80e63962fc
expect_hash zlib.dll 9f8729ac49e0ccea86fe3b1a9b2c3fae9986ecd09db92853e7a588dbda85bf90
expect_hash Program/zlib.dll 9f8729ac49e0ccea86fe3b1a9b2c3fae9986ecd09db92853e7a588dbda85bf90
expect_hash libgcc_s_dw2-1.dll 89f6147f5ed3f271d0b88f0586e079b9ac22e76c31221e5d5013aa273cc4694b
expect_hash Program/libgcc_s_dw2-1.dll 89f6147f5ed3f271d0b88f0586e079b9ac22e76c31221e5d5013aa273cc4694b
expect_hash Cards.dat abb2f631bd7897dcebde9d6c4bf61a6ea2e37e30fda42490c37b6f4d60f42e94
expect_hash Program/Cards.dat abb2f631bd7897dcebde9d6c4bf61a6ea2e37e30fda42490c37b6f4d60f42e94
expect_hash DBInfo.dat 519ccecb98548c1a2e15fe8025951aafba9f116595b5775a0f2ab2bb393e48c1
expect_hash Program/DBInfo.dat 519ccecb98548c1a2e15fe8025951aafba9f116595b5775a0f2ab2bb393e48c1
expect_hash Rarity.dat e0c779a73f0ed780b0c689741805a4e40f7f4949420a8d27fa73137e528ae04f
expect_hash Program/Rarity.dat e0c779a73f0ed780b0c689741805a4e40f7f4949420a8d27fa73137e528ae04f
expect_plain_summoned_wizard_deck_pair 0016 60
expect_plain_summoned_wizard_deck_pair 0283 60
expect_plain_summoned_wizard_deck_pair 0150 60
expect_plain_summoned_wizard_deck_pair 0076 56
expect_plain_summoned_wizard_deck_pair 0102 60
expect_hash Program/Manalink.ini 30153fd22c76b0c0751c538938af46fbf25b1b51d5b4bb2bd9a2eead1b9c2f2b
expect_hash Program/DuelArt/Modern.dat 9a2d70be70b70ef27036a47550bc0d549437df0c032a4e0237a217e4731e1aee
expect_hash Program/DuelArt/Planeswalker.dat 619e0b9780ec204b9fbf6f48b2eb541c9d8a6f19a73f27d4d76d25828db7d369
expect_hash Program/TT0530m_.ttf 51afc07ba27699fec048dd387f6e6068177c0ee4cd95c6483eb378978fdd1cee
expect_hash Program/TT0127m_.ttf e3b5229e753851acab9450fcad1acd9f89412f7bdaebfb6fbf25fc0536ab02d2
expect_hash Program/TT0085m_.ttf e738818f4bbf3f29c68601fe5cb16cb045650e7d1854806e584204fd2686ed4c
expect_hash Program/TT0298m_.ttf a36d52dec6c6216e2dce6f0979c715e5454a0d18647bedd03096f33dbd3d707f
expect_hash Program/TT0299m_.ttf fb1ce5027aa0a0cd3817f559e63fe4d28b6e125c0c32d3635337d2acfb109519
expect_hash Program/TT0300m_.ttf 83a70d460edbdc1a804764d6b17de2189765a5eb18cf598e8fa7e88058d67a79
expect_hash Program/CardArt/ManaSymbols.pic 60662a25dce90dc8d4cd0b0227fe62c33b50ac95115711428d463770b8d42cbd
expect_hash Program/CardArt/Expansion_Symbols.pic 01264f3dd6b9a8b5576b50bba49e951cb3fbdba1d33aee2b7ee8a9530d5e7348
expect_hash Program/CardArt/Watermarks.pic ec276a27c79a8cea55cdcb5474cbc5b96071f3744c18d7fc466ea6c503892c9c
expect_hash Program/CardArt/CardCounters.png 8d26128c1932b22f25b84b96d8d01e9b2dce008cd96265f3efabdc0c5f11ecbb
expect_hash Program/CardArt/Modern/Triggering.png a8c94fc5b58540f884e799a1603f65dd61d99af9e50efee4762b4021bedc6f00
expect_hash Program/CardArt/Modern/CardOv_Nyx.png b4bbf12f1f9851e2526ba25ad9b3de147fafa6aa7b1bc4400616b65aaf25209d
expect_hash Program/CardArt/Planeswalker/LoyaltyBase.png 7413ba6227b9b07a491a2730e170525ea4744d188e31e2665abc2361ebd6e79e
expect_hash Program/CardArt/Planeswalker/LoyaltyMinus.png 89f01e1bda607459ea6560c0b6608a9aab409799c05cd00279fee6d0bfd82cb9
expect_hash Program/CardArt/Planeswalker/LoyaltyPlus.png ad4b8971dd43955ccfd3daf9020b3a6f60c0a8fe9f21b73847c07a81b12af3ef
expect_hash Program/CardArt/Planeswalker/LoyaltyZero.png 8faf7ec5225538bcb97b539a1614282007ea484317411806a311f1c2d800ccef
pass "install runtime hashes match current patched manifest"

shandalar_land_cip_cave_hex=83fe017557833d00d28b0001744ef60534a99300027545813d78d19400db00000075393935381c8e0075313935b8f47b007529391dc42f8e0075218d149b89d0c1e00429d08d148752e842ed31ffa90100000074065fe90f0000005a8d149b8b0d381c8e00e9d38833ffc705ace97b0000000000ff3578d19400ff3564b7d106ff35b8f47b00ff35c42f8e00814f08000100106a006affba0100000029f25253566a7e5356e896c832ff83c4208b0424a3c42f8e008b442404a3b8f47b008b442408a364b7d1068b44240ca378d19400816708fffeffefc705ace97b000000000068000300005356e873972fff83c40c8f05c42f8e008f05b8f47b008f0564b7d1068f0578d19400e96d8a33ff
shandalar_player_target_cave_hex=898d3cfdffff837f08017515817f1400100000750c31c0a36cd49400e9fe052bffb8f8424c00ffd0a16cd49400e9d6052bff0000000000000000000000000000

expect_hex_prefix Shandalar.exe 0x1785b0 11 6a0057508b4d1051ff7504
expect_hex_prefix Program/Shandalar.exe 0x1785b0 11 6a0057508b4d1051ff7504
expect_hex_prefix Shandalar.exe 0xcda2e 5 9090909090
expect_hex_prefix Program/Shandalar.exe 0xcda2e 5 9090909090
expect_hex_prefix Shandalar.exe 0xcdccd 16 ff05f09d5800c2140090909090909090
expect_hex_prefix Program/Shandalar.exe 0xcdccd 16 ff05f09d5800c2140090909090909090
expect_hex_prefix Shandalar.exe 0xcdd3f 5 9090909090
expect_hex_prefix Program/Shandalar.exe 0xcdd3f 5 9090909090
expect_hex_prefix Shandalar.exe 0x16c320 6 b804000000c3
expect_hex_prefix Program/Shandalar.exe 0x16c320 6 b804000000c3
expect_hex_prefix Statwin.dll 0x2a10 6 b807000000c3
expect_hex_prefix Program/Statwin.dll 0x2a10 6 b807000000c3
expect_hex_prefix FaceMaker.exe 0x5f40 11 6a0057508b4d1051ff7504
expect_hex_prefix Program/FaceMaker.exe 0x5f40 11 6a0057508b4d1051ff7504
expect_hex_prefix Magic.exe 0x3c303 13 e9c0d801009090909090909090
expect_hex_prefix Program/Magic.exe 0x3c303 13 e9c0d801009090909090909090
expect_hex_prefix Magic.exe 0x59bc8 13 f74608040000000f8512000000
expect_hex_prefix Program/Magic.exe 0x59bc8 13 f74608040000000f8512000000
expect_hex_prefix Magic.exe 0x5db1f 10 c7055c72780000000000
expect_hex_prefix Program/Magic.exe 0x5db1f 10 c7055c72780000000000
expect_hex_prefix Magic.exe 0x694b7 16 6a0068ac5c71008b451050e8c9710000
expect_hex_prefix Program/Magic.exe 0x694b7 16 6a0068ac5c71008b451050e8c9710000
expect_hex_prefix Magic.exe 0x694eb 16 6a0068ac5c71008b451050e895710000
expect_hex_prefix Program/Magic.exe 0x694eb 16 6a0068ac5c71008b451050e895710000
expect_hex_prefix Shandalar.dll 0x94d34 9 e9c776cc0090909090
expect_hex_prefix Program/Shandalar.dll 0x94d34 9 e9c776cc0090909090
expect_hex_prefix Shandalar.dll 0x1174800 269 "$shandalar_land_cip_cave_hex"
expect_hex_prefix Program/Shandalar.dll 0x1174800 269 "$shandalar_land_cip_cave_hex"
expect_hex_prefix Shandalar.dll 0xcb16 18 e905fad40090909090909090909090909090
expect_hex_prefix Program/Shandalar.dll 0xcb16 18 e905fad40090909090909090909090909090
expect_hex_prefix Shandalar.dll 0x1174920 64 "$shandalar_player_target_cave_hex"
expect_hex_prefix Program/Shandalar.dll 0x1174920 64 "$shandalar_player_target_cave_hex"
expect_hex_prefix ManalinkEh.dll 0x3bb035 16 f60590f14e00040f84ae000000e90100
expect_hex_prefix Program/ManalinkEh.dll 0x381a25 16 f60590f14e00040f84ae000000e90100
expect_hex_prefix ManalinkEh.dll 0x1a8 4 00020000
expect_hex_prefix Program/ManalinkEh.dll 0x1a8 4 00020000
expect_hex_prefix ManalinkEh.dll 0x44cb23 5 e9089b0400
expect_hex_prefix Program/ManalinkEh.dll 0x40f115 5 e916450400
expect_hex_prefix ManalinkEh.dll 0x495a30 38 f7c30000000f7410f60590f14e0004750731d2e9d961fbfff6c3040f844265fbffe9d264fbff
expect_hex_prefix Program/ManalinkEh.dll 0x452c30 38 f7c30000000f7410f60590f14e0004750731d2e9d9b7fbfff6c3040f8434bbfbffe9c4bafbff
expect_hex_prefix ManalinkEh.dll 0x40d0e1 11 e97a950800909090909090
expect_hex_prefix Program/ManalinkEh.dll 0x3d2da1 11 e9ba080800909090909090
expect_hex_prefix ManalinkEh.dll 0x495a60 24 89c385c07e0881fb0e0100007e05bb0e010000e9746af7ff
expect_hex_prefix Program/ManalinkEh.dll 0x452c60 24 89c385c07e0881fb0e0100007e05bb0e010000e934f7f7ff
expect_hex_prefix ManalinkEh.dll 0x40db84 5 e9078b0800
expect_hex_prefix Program/ManalinkEh.dll 0x3d3844 5 e947fe0700
expect_hex_prefix ManalinkEh.dll 0x495a90 59 8d7db431c08b0c85c0f34e00890c874083f81075f0ba010000002b5508c1e20531c08b8c82a0f44e00898c82c0f34e004083f80875ece9eb74f7ff
expect_hex_prefix Program/ManalinkEh.dll 0x452c90 59 8d7db431c08b0c85c0f34e00890c874083f81075f0ba010000002b5508c1e20531c08b8c82a0f44e00898c82c0f34e004083f80875ece9ab01f8ff
expect_hex_prefix ManalinkEh.dll 0x429acf 10 e92ccc06009090909090
expect_hex_prefix Program/ManalinkEh.dll 0x3ec7cf 10 e92c6f06009090909090
expect_hex_prefix ManalinkEh.dll 0x3fe7a0 75 c744240c00000000b80100000029d88944240889742404891c24e881cd060085c00f84bfffffff89742404891c24e89dec0100c7442404010000008b4074890424e88aea0300e99bffffff
expect_hex_prefix Program/ManalinkEh.dll 0x3c4930 75 c744240c00000000b80100000029d88944240889742404891c24e8c16a060085c00f84bfffffff89742404891c24e80dd60100c7442404010000008b4074890424e80aaf0300e99bffffff
expect_hex_prefix ManalinkEh.dll 0x3f63e0 117 c744240c00000000b80100000029d88944240889742404891c24e84151070085c00f84bfffffff89742404891c24e85d7002008b4074890424e8c2970200e9a3ffffff9090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090
expect_hex_prefix Program/ManalinkEh.dll 0x3bc630 117 c744240c00000000b80100000029d88944240889742404891c24e8c1ed060085c00f84bfffffff89742404891c24e80d5902008b4074890424e8e27c0200e9a3ffffff9090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090909090
expect_hex_prefix ManalinkEh.dll 0x469583 17 e948d10200909090909090909090909090
expect_hex_prefix ManalinkEh.dll 0x495ad0 44 898d18fdffff817d1c00100000750d31d28915e42f7a00e9a82efdffe85f1f00fe8b15e42f7a00e9982efdff
expect_hex_prefix Program/ManalinkEh.dll 0x429453 17 e978a20200909090909090909090909090
expect_hex_prefix Program/ManalinkEh.dll 0x452cd0 44 898d18fdffff817d1c00100000750d31d28915e42f7a00e9785dfdffe85f4f04fe8b15e42f7a00e9685dfdff
expect_hex_prefix ManalinkEh.dll 0x3f63bd 5 e8ceed0300
expect_hex_prefix ManalinkEh.dll 0x3fe77d 5 e80e6a0300
expect_hex_prefix ManalinkEh.dll 0x495b00 247 83fb017542833d74857200017439813d782d7a00db000000752d391d1c7e73007525391d7cc16200751d3935209a730075156a015653e8d5c4f9ff83c40c85c07405e90f000000c705f8e9600000000000e98333f9ffc705f8e9600000000000ff35782d7a00ff35a4025902ff357cc16200ff35209a73005653e8f16cf8ff83c40889c7814f08000100106affba0100000029da526a7e5653e8122af9ff83c4148b0424a3209a73008b442404a37cc162008b442408a3a40259028b44240ca3782d7a00816708fffeffef68000300005653e8d9e7f9fd83c40c8f05209a73008f057cc162008f05a40259028f05782d7a00e94734f9ff
expect_hex_prefix Program/ManalinkEh.dll 0x3bc60d 5 e81ead0300
expect_hex_prefix Program/ManalinkEh.dll 0x3c490d 5 e81e2a0300
expect_hex_prefix Program/ManalinkEh.dll 0x452d00 247 83fb017542833d74857200017439813d782d7a00db000000752d391d1c7e73007525391d7cc16200751d3935209a730075156a015653e8251afaff83c40c85c07405e90f000000c705f8e9600000000000e98390f9ffc705f8e9600000000000ff35782d7a00ff3524055402ff357cc16200ff35209a73005653e8f1e7f8ff83c40889c7814f08000100106affba0100000029da526a7e5653e89289f9ff83c4148b0424a3209a73008b442404a37cc162008b442408a3240554028b44240ca3782d7a00816708fffeffef68000300005653e8d917fefd83c40c8f05209a73008f057cc162008f05240554028f05782d7a00e94791f9ff
pass "install patch bytes match current patched runtime"

expect_text_match config.txt '^AiDecisionTime:270$'
expect_text_match Program/config.txt '^AiDecisionTime:270$'
expect_text_match Shandalar.ini '^AiDecisionTime[[:space:]]*=[[:space:]]*270$'
expect_text_match Program/Shandalar.ini '^AiDecisionTime[[:space:]]*=[[:space:]]*270$'
pass "install config values match current runtime settings"

printf 'Install-tree verification passed.\n'
