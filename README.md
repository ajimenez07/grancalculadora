# Gran Calculadora (Great Calculator)

**Status**: **Completed**. This project is finished and fully functional. No further updates are planned.

Graphic calculator built with **GTKmm4**, **Cairo** and **C++** that supports fractions and powers in a more visual mode.

<img src="screenshots/calculator.gif" alt="Calculator demo" width="400">

## Possible Improvements

* Full keyboard input support.
* Extended testing on Windows and macOS.
* A more comfortable UX (e. g: set the current number as the base of the power 
  on clicking the power button)
* Advanced functions as roots or trigonometrical functions.
* Show error messages. Now, if there is a eval error, there will be no change at the display.
* Fix segment fault on clicking "=" with a fraction that has no elements in numerator/denominator.

## Technologies

- **C++17** (smart pointers, modern features)
- **GTK4 / GTKmm** (GUI framework)
- **Cairo** (2D graphics)
- **Autoconf** (build system)

## License

MIT - see COPYING

## Author

Álex Jiménez <ajimenezba@edu.tecnocampus.cat>
