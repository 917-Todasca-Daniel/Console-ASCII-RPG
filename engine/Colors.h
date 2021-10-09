#ifndef COLORS_H
#define COLORS_H

enum COLOR : int { DARK = 0, DBLUE, DGREEN, DCYAN, DRED, DPURPLE, DYELLOW, DWHITE, GRAY, BLUE, GREEN, CYAN, RED, PURPLE, YELLOW, WHITE };

class Color
{
public:
	Color(COLOR forecolor = WHITE, COLOR backcolor = DARK);

	void setForecolor(COLOR forecolor) { this->forecolor = forecolor; }
	void setBackcolor(COLOR backcolor) { this->backcolor = backcolor; }

	void setColor(COLOR forecolor, COLOR backcolor) { setForecolor(forecolor), setBackcolor(backcolor); }
	void setColor(const Color& other) { setColor(other.forecolor, other.backcolor); }

	bool operator == (const Color& other) const { return (forecolor == other.forecolor && backcolor == other.backcolor); }
	bool operator != (const Color& other) const { return (forecolor != other.forecolor || backcolor != other.backcolor); }

protected:
	COLOR forecolor, backcolor;

private:
	static COLOR consoleForecolor, consoleBackcolor;
	static void setConsoleForecolor(COLOR forecolor);
	static void setConsoleBackcolor(COLOR backcolor);
	static void setConsoleColor(Color color);
	static void updateConsoleColor();

	friend class InmutableRenderer;
	friend class Debugger;
};

#endif // COLORS_H
