#include "Colors.h"

#include "consolewizard.h"

void Color::updateConsoleColor() {
	SetConsoleTextAttribute(getHandle(), consoleForecolor + 16 * consoleBackcolor);
}

Color::Color(COLOR forecolor, COLOR backcolor) : forecolor(forecolor), backcolor(backcolor) {}

void Color::setConsoleForecolor(COLOR forecolor) {
	if (consoleForecolor == forecolor) return;
	consoleForecolor = forecolor;
	updateConsoleColor();
}

void Color::setConsoleBackcolor(COLOR backcolor) {
	if (consoleBackcolor == backcolor) return;
	consoleBackcolor = backcolor;
	updateConsoleColor();
}

void Color::setConsoleColor(Color color) {
	if (consoleForecolor == color.forecolor && consoleBackcolor == color.backcolor) return;
	consoleForecolor = color.forecolor;
	consoleBackcolor = color.backcolor;
	updateConsoleColor();
}

COLOR Color::consoleForecolor = WHITE;
COLOR Color::consoleBackcolor = DARK;
