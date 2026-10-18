# Dungeons of the Desert — optional sample banks

These small, mono banks complement the built-in modeled desert presets. The oud
and ney-air recordings are also built into the firmware as the SAMPLE sets OUD
(SET 3) and NEY (SET 2, looped), so the USR slots stay free for the qanun and the
tar. Uploading firmware does not install the banks. In the web editor's Samples tab, choose a bank in
the desired USR slot's dropdown, press **Prepare**, preview the notes, then **Upload**. Upload
replaces only the selected USR slot, after confirmation. Back up your existing
samples first if you want to keep them. On a SAMPLE track choose SET USR1 (Oud),
USR2 (Qanun), or USR3 (Ney Air). Set ROOT and SCALE on that track; the bank itself
is calibrated to ordinary MIDI roots and the engine applies the maqam offsets.

GRAIN can use the same user banks for sustained, spectral textures. SAMPLE plays
these recordings as one-shots; no sustain loops are provided. The CITADEL scene
uses modeled voices and works with empty user slots. To use a recording in it,
switch the desired track to SAMPLE and select its user SET, then set Bayati / D.
The modeled presets are interpretations, not acoustic emulations.

| Bank | Suggested slot | Recording |
| --- | --- | --- |
| OUD | USR1 | Three oud plucks, G2 / A2 / C3, by hammondman |
| QANUN | USR2 | Six notes from a Turkish kanun recording by barisbozkurt |
| NEY AIR | USR3 | Three short held-note tails from aliaref's ney phrase |
| TAR | Any USR slot | Four Persian tar plucks, C3 / C4 / Ab4 / Db5, from Berklee's BISA archive |

Oud, Qanun and Ney Air source recordings are CC0 1.0. [sources.json](sources.json) records author,
original page, public HQ preview URL, license link and source SHA256. We used the
public MP3 previews, not the original uncompressed recordings. Edits: mono
22,050 Hz, isolated notes, DC removal, median-pitch calibration, fades, peak
normalization, and IMA ADPCM encoding. Derived audio is also dedicated under
[CC0 1.0](https://creativecommons.org/publicdomain/zero/1.0/).

Ney Air preserves audible breath and pitch movement from a performance. Its
median is calibrated, but it is not a vibrato-free tuning reference. DUNE NEY
is the built-in, steady-pitch alternative. Wide transpositions of any sparse bank
change its timbre; these are compact creative resources, not full multisamples.

[banks.json](banks.json) records cuts, roots, calibration and measured pitch span.
Rebuild with `python3 tools/build_desert_banks.py build/desert-sources` after
downloading and decoding the verified sources (instructions in the script).
The `.hdr` / `.bin` files use the existing FM-1 user-slot format. WAV files can
also be imported manually into the editor; root names follow C4 = MIDI 60.

## Persian tar — CC BY

The **TAR** bank is adapted from the [Berklee Intersectional Soundbox Archive's
Persian Tar Sample Pack](https://remix.berklee.edu/bisa-persian-tar/), 2025.
Instrumentalist: **Erfan Hessampour**. Recording engineer: **Yangyang Hua**.
Editor: **Shuyi Xiong**. Source titles: *Forte (C3)*, *Forte (C4)*,
*Forte (Ab4)* and *Forte (Db5)*. Individual recording pages and stream hashes
are in [tar-sources.json](tar-sources.json).

Each recording page explicitly releases its audio under **CC BY**, without
specifying a version. This bank retains that attribution license; **Tar is not
CC0**. Preserve these credits, the source links and this modification notice
when redistributing the bank or its recordings. The publisher's recording pages
are the license declarations; no more specific license version is asserted here.

Changes: taken from the public AAC/HLS previews (not original WAV masters), mixed
to mono at 22,050 Hz, trimmed to approximately 1.7 seconds per note, DC removed,
pitch calibrated, normalized and faded; encoded as IMA ADPCM for the FM-1.
No sustain loops. The approximately 73 KiB bank fits one existing user slot.
The engine applies maqam offsets and cent trims after the calibrated sample root.
Recorded string beating and natural pitch movement remain; calibration measures
the median pitch 80–650 ms after the attack, not every moment of the decay.

For hardware, choose **Tar → Prepare Tar → Upload** in any USR slot, then select
**SAMPLE → SET → that USR slot**. Use a zero attack to retain the pluck. For the
browser emulator, switch on, select a track and click **Load Tar on selected
track** below the instrument. Choose USR1/2/3; replacing an occupied slot asks
first. Loading stops playback and selects SAMPLE on that track; the bank is saved
in this browser with the existing flash data. Back up a slot before replacing it
if you need its previous bank. There are still three slots, not four.

Rebuild with `python3 tools/build_tar_bank.py build/tar-sources`; see that script
for the public-stream download command. The separately generated Tar entry is
preserved when rebuilding the original CC0 banks.
