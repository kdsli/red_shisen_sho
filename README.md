# red_shisen_sho
kdsli's version for shishen_sho

This project was created to teach my son programming and to support my professional skills, as well as due to the 
presence of a certain amount of free time.

It is written without viewing the source code of the KShisen program from KDE. The project uses only graphic images 
of this project, including tiles and background image.

The goal of the project was to find the optimal tiles removal algorithms.

In addition, the program has additional features in relation to the original game: 

- when a program reaches a deadlock, it demonstrates that the correct way to remove the tiles is possible.
- you can set your own background images by placing them in the UserBackhround directory.
- the program has a training mode, in which the hint, undo and redo functions are available, but the results do not fit 
  into the high score table. When the training mode is off, these functions are not available, but the results are recorded 
  in the table of records.

The main language of the program and comments is Russian, but there is an interface translation file in English

The program is not public, but quite workable.

The program was not going under Windows, but it will be checked in the near future and there should be no problems. 
Also, no work was done on installing the program into the Linux environment.

The program can be assembled according to the following algorithm:

- git clone https://github.com/kdsli/red_shisen_sho
- mkdir build
- cd build
- cmake ..
- make
- make install

The copy obtained in the output_Release directory is fully functional.

From the unfinished I can specify:

- the table of recorders is disabled
- no sounds are output

In the near future I plan to fix it.

Respectfully, Dmitry kdsli@kdsli.ru
