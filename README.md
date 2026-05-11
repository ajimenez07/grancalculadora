# Gran Calculadora (Great Calculator)

> **Status**: Early development (~30% complete)  
> GUI is done, but in the future maybe more buttons are added (sin, cos, tan, etc.). Now the development is focused on the calculator display screen.

Graphic calculator built with **GTKmm4**, **Cairo** and **C++** that supports fractions and roots in a more visual mode.

![Calculator demo](screenshots/calculator.gif){: width="400px"}


## What have been done

The calculator is still in its initial development. The GUI is totally built, but some changes maybe will be done in the future.

The development is focused now on the display rendering system.

Button input functionality and left/right caret movement (referred to as "cursor" in the architecture) are now implemented, though further improvements are still pending.

## What's pending

- Finish the display system
- Expression evaluation
- Keyboard input
- Advanced functions as trigonometric functions or equation solver.
- Tests in Windows and Linux.

## Technologies

- **C++17** (smart pointers, modern features)
- **GTK4 / GTKmm** (GUI framework)
- **Cairo** (2D graphics)
- **Autoconf** (build system)

## License

MIT - see COPYING

## Author

Álex Jiménez <ajimenezba@edu.tecnocampus.cat>
