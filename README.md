# Llamas
A small strategy game that I am working on during downtime 
 
##Controls

- Right Alt decreases zoom.

- Right Control increases zoom.

- Move the camera with the arrow keys.

- Space ends your turn.

- Backspace undos your last move.

- Minus and equals changes your currently equipped block.

- Left click places the selected block, right click deletes a block. Fortress blocks cannot be deleted.

##What is Llamas?

Llamas is a hybrid between a competitive turn based strategy game and Conway's game of life. On every turn you have the option to either place cells or place fortress blocks, akin to the decision to be defensive or offensive.

A Fortress block destroys all adjacent enemy cells in comes in contact with, and have a health of 20 blocks. This in turn means that a fortress can only defend you against at most 20 cells before it needs to be rebuilt. Fortress blocks are always slate grey.

A Cell, as explained by Conway's game of life, dies if it has less than 2 neighbours or if it has more than 3. An empty block will turn into a cell of the dominate team if it has no more or less than 3 adjacent cells. The color of your cell blocks is dependent on your team.

A Goal is the heart of your team. Build the fortress around your goal and try to tear down those of your enemies. Goals have 10 health. Goals are always green.

The obejctive is to build large cell structures in order to keep the battlefield under constant pressure and claim your victory.

> ![An Advanced Structure](http://www.conwaylife.com/w/images/9/9f/Gosperglidergun.png "Gosper Glider Gun") 

>Like a Gosper Glider Gun.

> ![An Advanced Structure](http://www.conwaylife.com/w/images/7/79/Glider.png "Gliders")

> Or glider generators.


5 cells and 5 fortress blocks can be created per turn per team member. No more than 8 players per team is recommended. Communication is done VIA TCP on port 69878. Make sure to properly port forward if you plan to play Llamas remotely.

The magenta counter is how many more cell blocks can be placed during this turn. The light brown counter (You need to use minus and equals to change the selected block) represents how many more fortress blocks can be made on this turn.

Games typically last roughly an hour or so when playing 2v2 and typically 15 minutes when playing 8v8. 

---------------
This is the fourth game I've made and by far the smallest as 90% of this was made in a single afternoon. Said being, I doubt I will continue this project for much longer. Don't expect often updates.
