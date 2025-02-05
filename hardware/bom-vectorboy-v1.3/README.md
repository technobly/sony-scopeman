# Vectorboy v1.3 PCB & BOM

Everything you need to order parts for your Sony Scopeman / Vectorboy!

## PCB

:warning: **NOTE: Be sure to order the PCB thinner than standard, 0.040" (1.0mm) should be good.**  If you end up ordering as 0.062" (1.6mm), [you will have to make a bunch of modifications, but it can be done.](thick-pcb-mods.md)

### JLCPCB

All parts including PCB's for 2 complete boards was $159.82 ($80 each).

I got a brand new Sony Watchman FD-10A from eBay for $30 shipped.

I made some minor tweaks to the silkscreen and ordered PCBs from JLCPCB.

- Added "JLCJLCJLCJLC" to the back side of the PCB for part number placement
- Moved "PURPLE / BLACK" text for readability
- Changed main text to say "Vectorboy v1.0" for fun and motivation
- I ordered 20 PCBs in White soldermask with Black silkscreen with a stencil for $34.38 shipped.

Order PCB's by using the included [gerbers-vectorboy-v1.3.zip](../gerber-vectorboy-v1.3/) and uploading those with all of the necessary specs [JLCPCB](https://jlcpcb.com)

Centroid/Position data [is here](../gerber-vectorboy-v1.3/pos.csv) for ordering assembled boards.  (Note: I have not used this file myself, just trying to keep it updated from the original)

## BOM

### LCSC

Most of the parts are included in [LCSC-Bom-Vectorboy-v1.3.csv](LCSC-Bom-Vectorboy-v1.3.csv)

However, some of those parts were out of stock and I had to get them at Digikey or eBay.

Here is the exact order that was exported

- [LCSC-Exported-Vectorboy-v1.3.csv](LCSC-Exported-Vectorboy-v1.3.csv) - $33.14

To import this:

- Upload the CSV to [LCSC's BOM Tool](https://lcsc.com/bom)
- Set the columns for "LCSC Part Number", "Customer Part Number" (these are the ref des.), "Quantity"
- This will add enough parts for 2 complete boards (adjust as necessary)

> NOTE: if you plan to build these by hand, you might want to import the [LCSC-Exported-Condensed-Designators-Vectorboy-v1.3.csv](LCSC-Exported-Condensed-Designators-Vectorboy-v1.3.csv) BOM which has Condensed Designators in decending order (e.g. "C37,30,28-25,19,14,11,8,2") that will fit on the bags of parts you'll receive.  DO NOT use this one if you are planning on having JLCPCB assemble your boards for you.

### Digikey

A few parts needed to be ordered from Digikey

- [Here's the shared cart](https://www.digikey.com/short/332bzwh8) - $44.50

### eBay

Grumble grumble, part shortage... get these at eBay

- [TPS63001DRCR](https://www.ebay.com/itm/265393777178) - $12.98
- [TPS63700DRCR](https://www.ebay.com/itm/223277053580) - $18.37

## Batteries

Don't forget the batteries!

- [803040](https://www.amazon.com/1000mAh-Battery-Rechargeable-Lithium-Connector/dp/B09FFGP6N5) - $16.44
