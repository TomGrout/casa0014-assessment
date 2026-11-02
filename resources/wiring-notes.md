Shared Ground (-) Rails: One jumper runs from the Arduino GND pin to the small breadboard's negative rail and bridges down to both outer Blue (-) rails on the large 60-column breadboard (leave the VCC wire unplugged).

Player 1 Buttons (Top Half, Rows g–i): Spaced out at Cols 8–10 (Yellow), Cols 22–24 (Blue), Cols 36–38 (Red), and Cols 50–52 (Green). Each button's outer-left leg (Row j) connects via a short loop to the Top Blue (-) Rail, while its inner-right leg (Row f) routes down the center trench to Arduino pins D6, D7, D8, and D9.

Player 2 Buttons (Bottom Half, Rows b–d): Mounted directly opposite Player 1 in the exact same column pairs. Each button's outer-left leg (Row a) connects via a short loop to the Bottom Blue (-) Rail, while its inner-right leg (Row e) routes down the center trench to Arduino pins A1, A2, A3, and A4 (leaving A0 empty for randomSeed).

Central Elevated LEDs (Small Breadboard): The 4 LEDs (Yellow, Blue, Red, Green) sit with their metal legs left full-length so the bulbs stand above the jumper bundle. Their long legs (+ Anode) wire to D5, D4, D3, and D2, and their short legs (- Cathode) connect through the four 330ohm resistors to the GND rail.