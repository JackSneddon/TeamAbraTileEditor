if distributing, add the contents of "addToWorkingDir" to the directory of the executable.
please don't use the included tilesheets in your own project, they're just for reference
an example tilesheet is included
no guarantee this will work with a custom tilesheet of yours,
if you do use a custom one it'll either need to be formatted the same as the included ones or you'll need to change the constants in the code and recompile

Controls:
R - apply rule tile
ESCAPE - clear grid
Space - save island
Enter - load island, .csv file must be in working dir, enter island name in the console
0-9 - load palette with that name from /gfx/

How to use:
left click a tile from your palette on the left to start drawing with it
water tiles (top left tile in palette) are written as 0, so not drawn when loaded in game
