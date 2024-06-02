# msk2bmpGUI
a GUI version of the msk2bmp tool made by temaperacl

This project ended up being a lot more ambitious than 
I originally planned, but it's turning out quite nicely.

A special "Thank You!" goes out to several people in the 
community who helped in the developement of this Image Editing tool:
BakerStaunch
DandyMcGee
MonotoneZombie
...and a ton of other people who 
   I will eventually list when I 
   remember them all :P
   Thank you everyone!

Ok so here's how "Q's (version) Fallout Image Editor" works:

The program currently opens with a very simple "Load files..." 
button and the full default Fallout palette including the 
upper cycling colors.

### *Load File*
Loading a compatible image (jpg, png, bmp) or Fallout FRM file
will open a second window with a preview of this image.
The red squares indicate how the image will be tiled if it's
supposed to be an overworld map.
- Currently only a single frame of an FRM will be displayed,
  Plans are to expand this to display all frames, and to allow
  each frame to be edited individually.
- FRM's that use color cycling currently don't display correctly,
  Plans are to add color cycling to this image display too.

Once the image is loaded there are several buttons above the image
with conversion and editing options.
- These buttons are currently fairly generic and mostly apply to
  non-FRM images, 
 - Plans are to make them context specific with dropdowns for the
  alternative color matching profiles (to include dithering options).

### *Preview Tiles*
The "Preview Tiles" buttons are intended to be used to make Fallout's
overworld map tiles from the image provided, by splitting it up into
the correct sized tiles (according to the red squares overlapping the
loaded image screen), and applying post processing algorithms (palettizing, 
color matching, and dithering).
The tiled image opens in a new panel with the "Save as Map Tiles..."
button at the top.

Clicking "Save as Map Tiles..." will export the individual tiles
directly to the Fallout 2 install directory set in the "Default
Fallout Path" configuration, under the "data/art/interface" folder,
with the appropriately numbered name for the tile, .
If the "Default Fallout Path" is not set yet, a warning will pop
up asking the user to set it.
- This implementation is currently very simple, and will automatically
  overwrite any map tile images that may already be there.
  Plans are to add warning checks if files already exist, as well as 
  directory checks (might crash if "data/art/interface" doesn't exist).
  
### *Preview as Image*
"Preview as Image" buttons will display the image in its entirety
with the selected color matching and dithering options applied to it.

### *Save as Image*
The "Save as Image..." button at the top will export the full image 
with all the post processing (palettizing/color matching/dithering) 
applied to it.
- *Note* Currently images are only saved as either BMP or FRM, and
  which one it saves as is set by the original image's extension.
  ie: FRM's can only be saved as BMP, and all other compatible image
  types can only be saved as FRM.
  Plans are to add a function that switches the export type based
  on the extension the user picks in the save file dialogue window.
  (And hopefully also matching the extension in the filename itself).

### *Convert and Paint*
Finally, the "Convert and Paint" button will apply the post processing
(palettizing/color matching/dithering) to the image and creates a layer
to paint on.
You can change the color of the paint tool by clicking on one of the
predefined palette colors in the Fallout palette menu.
- Currently the only implemented paint tool is a single fill-rect tool
  with pre-defined values for the size
  Plans are to add some controls for the size of the tool, and hopefully
  other painting tools (specifically select and fill, gradient fill, 
  eraser, and maybe some shape fill/masking fill).
- Building this paint tool will probably take the longest, and is only
  useful for single frame images right now, though I am interested in
  ideas to make editing multiple frame animations more convenient.
  
### *Create Mask Tiles* (partially implemented)
The "Create Mask Tiles" button will open an editing layer on top of 
the original image (with partial transparency) which can only be painted
in black an white.
This editing window is used to make a special MSK file for each 
corresponding map tile that you draw on.
Mask tiles are used in the Fallout engine to block players from moving 
across some portion of the map.
The only real examples of this in either game is the west coast:
The ocean area is masked off using a mask tile, preventing the player
from swimming to the Oil Rig in Fallout 2 (and not much else in Fallout).
- Currently this editing window can only be used to draw in single pixels,
  and the export functions don't work yet.
  Plans are to add a fill function and maybe some line drawing functions
  to provide borders for the fill operation.

The "Export Mask Tiles" button is currently unimplemented.
- Currently working on this one, hoping to have it available for the next
  alpha release.
  Should work similar to the "Save as Map Tiles" button.
  
  
### *Other Features*
--Wide Character Support--
The first alpha release of this tool doesn't yet support wide characters
(languages that use an alphabet other than the English one).
The next alpha release should have wide character support, but will
need some testing.

--Multidirectional Animations in FRMs--
I'm not sure how much effort I should put into making this image
editor handle animation frames from all 6 directions for a standard
critter FRM, so this will probably be added later in developement.
I can't imagine it being much more difficult than what I've already 
made, but I haven't been able to come up with a layout to properly 
display all frames one way or the other.
Frame Animator 2 seems to handle this pretty well so far, and can
be used to easily put an animation together if the scripting 
language is understood.
Implementing something similar for this tool seems excessive, 
though I can see the value in having an open source tool that
would do the same job.

--Drag & Drop--
I plan on implementing a drag and drop feature, allowing users
to drag images onto the window and having the editor automatically
open them.
Eventually I'd like to implement this in such a way as to allow the
user to drop a set of map tile FRMs, or a set of mask tile MSKs,
onto the window and it will automatically organize them visually
into the same arrangement they have in the game.
This will probably include some sort of control to modify the
total number of tiles the map is wide, similar to how the engine
itself organizes the tiles using the "num_horizontal_tiles"
setting in the WORLDMAP.TXT file in the data/data folder.

--Automatic Registering of Map Tiles and Map Mask Tiles--
The Fallout engine uses WORLDMAP.TXT to register the map tiles
and mask tiles for the game to display.
There is currently no implementation to handle this file in this
editor, but I would like to add one at some point.
Actually physically registering the tiles isn't difficult, 
and you don't have to if they're the same number as the original
game, but it might be a neat feature and may cut down on some
of the learning curve for modding.

## *Final Thoughts*
Well, those are all the things I've got working so far, and 
a few things I'd like to get working in the future.
Hope the modding community finds this useful, and any support
(by that I mean financial) is always welcome!
Thanks for taking the time to read this, I don't know yet
how to do feedback here, so I'll just sign off with this:

It took me about a year to get this far, but I'm not done yet.
Keep an eye out for more modding tools in the future...even
if it does take a while to actually get them built :)

## *Build*
These packages are used in the build process:
GLAD
GLFW
SDL_image @ d3c6d59      ----  this is a git submodule, will have to "git submodule init", then "git submodule update"
imgui-docking
tinyfiledialogs
