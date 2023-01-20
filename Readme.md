# Genetic programming in C

### This is not an original project

Most of it is just me studying what Tsoding showed on his twitch, trying to make sense of it and learn something new.

Shoutout to Tsoding. 
* [his youtube](https://www.youtube.com/@TsodingDaily) and [this series](https://www.youtube.com/watch?v=iL--xqGgd0g&list=PLpM-Dvs8t0VZhPhStYD0aS30Y1awAv-DO)
* [twitch](https://www.twitch.tv/tsoding)
* [original repo for this project](https://github.com/tsoding/gp)

### What you can find in this project?

1. Simple implementation of primitive genetic programming based on a randomly generated turing-like state machine.
2. Separate executables for simulation and training.
3. Even though the starting state for agents is pseudo-random, after training they can 'evolve' into something that can eat almost all the food on board.
4. Unoptimized C code with no memory management (he-he, everything is on the stack).

### Dependencies

* SDL_2
* SDL2_gfx

### Building and running

```bash
cmake -S . -B ./build
cmake --build build
mkdir output
./build/simulation
```

Use ``./build/trainer`` if you want to train them for a predefined number of generations, but it requires ``./output/game_state.bin`` file.

### Controls

| Key                       | Action                                                                  |
|---------------------------|-------------------------------------------------------------------------|
| <kbd>r</kbd>              | Regenerate the game board                                               |
| <kbd>n</kbd>              | Make a next generation based on the best-performing agents              |
| <kbd>s</kbd>              | Step the state of the game                                              |
| <kbd>q</kbd>              | Quit                                                                    |
| <kbd>d</kbd>              | Dump the game state into ./output/game_state.bin                        |
| <kbd>l</kbd>              | Load the game state into ./output/game_state.bin                        |
| <kbd>mouseclick</kbd>     | If clicked on the entity, it will dump the information into the console |
