#pragma once

#include <cstdint>

#include "helpers/helpers.h"

#include "color.h"
#include "font.h"

namespace ui {

    class Attributes {
	public:

		Attributes():
		    raw_(0) {
		}

		bool empty() {
			return raw_ == 0;
		}

		/** Returns true if the attributes contain any of the visible ones.
		  
		    I.e. if rendering them is necessary.
		 */
		bool emptyVisible() {
			return raw_ & 0x7eff; // no end of line, no border above
		}

	    bool underline() const {
			return raw_ & UNDERLINE;
		}

		bool strikethrough() const {
			return raw_ & STRIKETHROUGH;
		}

		bool curlyUnderline() const {
			return raw_ & CURLY_UNDERLINE;
		}

		bool borderLeft() const {
			return raw_ & BORDER_LEFT;
		}

		bool borderTop() const {
			return raw_ & BORDER_TOP;
		}

		bool borderRight() const {
			return raw_ & BORDER_RIGHT;
		}

		bool borderBottom() const {
			return raw_ & BORDER_BOTTOM;
		}

		bool borderThick() const {
			return raw_ & BORDER_THICK;
		}

		bool borderAbove() const {
			return raw_ & BORDER_ABOVE;
		}

		bool endOfLine() const {
			return raw_ & END_OF_LINE;
		}

		bool operator == (Attributes other) const {
			return raw_ == other.raw_;
		}

		bool operator != (Attributes other) const {
			return raw_ != other.raw_;
		}

		Attributes operator + (Attributes other) const {
			return Attributes(raw_ | other.raw_);
		}

		Attributes operator - (Attributes other) const {
			return Attributes(raw_ & ~other.raw_);
		}

		static Attributes Underline() {
			return Attributes(UNDERLINE);
		}

		static Attributes Strikethrough() {
			return Attributes(STRIKETHROUGH);
		}

		static Attributes CurlyUnderline() {
			return Attributes(CURLY_UNDERLINE);
		}

		static Attributes BorderLeft() {
			return Attributes(BORDER_LEFT);
		}

		static Attributes BorderTop() {
			return Attributes(BORDER_TOP);
		}

		static Attributes BorderRight() {
			return Attributes(BORDER_RIGHT);
		}

		static Attributes BorderBottom() {
			return Attributes(BORDER_BOTTOM);
		}

		static Attributes BorderThick() {
			return Attributes(BORDER_THICK);
		}

		static Attributes BorderAbove() {
			return Attributes(BORDER_ABOVE);
		}

		static Attributes EndOfLine() {
			return Attributes(END_OF_LINE);
		}

	private:

        friend class Cell;

		static constexpr uint32_t UNDERLINE = 1 << 0;
		static constexpr uint32_t STRIKETHROUGH = 1 << 1;
		static constexpr uint32_t CURLY_UNDERLINE = 1 << 2;
		static constexpr uint32_t BORDER_LEFT = 1 << 3;
		static constexpr uint32_t BORDER_TOP = 1 << 4;
		static constexpr uint32_t BORDER_RIGHT = 1 << 5;
		static constexpr uint32_t BORDER_BOTTOM = 1 << 6;
		static constexpr uint32_t BORDER_THICK = 1 << 7;
		static constexpr uint32_t BORDER_ABOVE = 1 << 8;

		static constexpr uint32_t END_OF_LINE = 1 << 15;

		Attributes(uint16_t raw):
		    raw_(raw) {
		}

	    uint16_t raw_;
	};

    /** Base representation of a single UI cell.  

	    Contains packed information about a single cell, namely the codepoint (unencoded Unicode), the foreground (text), background, and decorations (underlines, strikethroughs, etc) attributes (underline, borders, etc.) and the font to use to render the cell. 

		21 bits codepoint
		24 bits text (RGB)
		24 bits background (RGB)
		32 bits decoration (RGBA)
		8 bits font (bold, italics, 8 sizes, 3bits reserved)
        16 bits decorations
		3 bits remanining (application specific, such as line end, modified, etc)

		TODO add font accessors, 
	 */
    class Cell {
	public:

		/** Default cell constructor is white space on black background. 
		 */
		Cell():
			big_{ 0xffffff0000000020, 0xffffffff00000000 } {
			static_assert(sizeof(Cell) == 16, "Invalid cell size, padding must be adjusted");
		}

		/** Returns the codepoint stored in the cell. 
		 */
	    char32_t codepoint() const {
			return small_[0] & 0x1fffff;
		}

		/** Returns the font of the cell. 
 		 */
		Font font() const {
			return Font(small_[0] >> 24);
		}

		/** Returns the foreground color of the cell. 
		 
		    Note that tghe foreground color's opacity is always 255. 
		 */
		Color foreground() const {
			return Color(small_[1] | 0xff);
		}

		/** Returns the background color of the cell. 
		 
		    Note that the background color's opacity is always 255.
		 */
		Color background() const {
			return Color(small_[2] | 0xff);
		}

		Color decorationColor() const {
			return Color(small_[3]);
		}

		Attributes attributes() const {
			return Attributes((small_[1] & 0xff << 8) + small_[2] & 0xff);
		}

		Cell & operator = (Cell const & other) {
			big_[0] = other.big_[0];
			big_[1] = other.big_[1];
			return *this;
		}

		bool operator == (Cell const & other) const {
			return big_[0] == other.big_[0] && big_[1] == other.big_[1];
		}

		bool operator != (Cell const & other) const {
			return big_[0] != other.big_[0] || big_[1] != other.big_[1];
		}

    protected:

	    unsigned padding() const {
			return small_[0] >> 21 & 0x7;
		}

		void setPadding(unsigned value) {
			ASSERT(value <= 7) << "Only three bits of padding are available";
			small_[0] &= 0xff1fffff;
			small_[0] |= (value << 21);
		}

	private:
	    static uint16_t AttributesToRaw(Attributes attrs) {
			return attrs.raw_;
		}

	    /** Sets the codepoint of the cell. 
 		 */
		template<typename CELL>
		friend typename std::enable_if<std::is_base_of<Cell, CELL>::value, CELL &>::type operator << (CELL & cell, char32_t codepoint) {
			cell.small_[0] &= 0xff700000;
			cell.small_[0] |= codepoint & 0x1fffff;
			return cell;
		}

		/** Sets the font of the cell. 
 		 */
		template<typename CELL>
		friend typename std::enable_if<std::is_base_of<Cell, CELL>::value, CELL &>::type operator << (CELL & cell, Font font) {
			cell.small_[0] &= 0xffffff;
			cell.small_[0] |= (*helpers::pointer_cast<unsigned char *>(&font)) << 24;
			return cell;
		}

		/** Foreground color */
		template<typename CELL>
		friend typename std::enable_if<std::is_base_of<Cell, CELL>::value, CELL &>::type operator << (CELL & cell, ForegroundColorHolder color) {
			cell.small_[1] &= 0xff;
			cell.small_[1] |= (color.value.toRGB() << 8);
			return cell;
		}

		/** Background color */
		template<typename CELL>
		friend typename std::enable_if<std::is_base_of<Cell, CELL>::value, CELL &>::type operator << (CELL & cell, BackgroundColorHolder color) {
			cell.small_[2] &= 0xff;
			cell.small_[2] |= (color.value.toRGB() << 8);
			return cell;
		}

		/** Decorations color */
		template<typename CELL>
		friend typename std::enable_if<std::is_base_of<Cell, CELL>::value, CELL &>::type operator << (CELL & cell, DecorationColorHolder color) {
			cell.small_[3] = color.value.toRGBA();
			return cell;
		}

		/** Sets the attributes of the cell. 
    	 */
		template<typename CELL>
		friend typename std::enable_if<std::is_base_of<Cell, CELL>::value, CELL &>::type operator << (CELL & cell, Attributes attr) {
			cell.small_[1] &= 0xffffff00;
			cell.small_[2] &= 0xffffff00;
			cell.small_[1] |= (Cell::AttributesToRaw(attr) >> 8);
			cell.small_[2] |= (Cell::AttributesToRaw(attr) & 0xff);
			return cell;
		}

		/** Sets the attributes of the cell. 
    	 */
		template<typename CELL>
		friend typename std::enable_if<std::is_base_of<Cell, CELL>::value, CELL &&>::type operator << (CELL && cell, Attributes attr) {
			cell.small_[1] &= 0xffffff00;
			cell.small_[2] &= 0xffffff00;
			cell.small_[1] |= (Cell::AttributesToRaw(attr) >> 8);
			cell.small_[2] |= (Cell::AttributesToRaw(attr) & 0xff);
			return std::move(cell);
		}

		/** Adds given attributes to the cell
    	 */
		template<typename CELL>
		friend typename std::enable_if<std::is_base_of<Cell, CELL>::value, CELL &>::type operator += (CELL & cell, Attributes attr) {
			cell << (cell.attributes() + attr);
			return cell;
		}

		/** Removes given attributes to the cell
    	 */
		template<typename CELL>
		friend typename std::enable_if<std::is_base_of<Cell, CELL>::value, CELL &>::type operator -= (CELL & cell, Attributes attr) {
			cell << (cell.attributes() - attr);
			return cell;
		}


	    union {
			uint64_t big_[2];
			uint32_t small_[4];
		};
	}; // ui::Cell

} // namespace ui