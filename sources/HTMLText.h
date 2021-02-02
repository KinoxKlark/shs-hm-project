#ifndef SFML_HTML_TEXT
#define SFML_HTML_TEXT

#include <SFML/Graphics/Export.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/String.hpp>
#include <string>
#include <vector>

namespace sf {

	class HTMLText : public Drawable, public Transformable
	{
	public:
		enum Style
			{
				Regular       = 0,      
				Bold          = 1 << 0, 
				Italic        = 1 << 1, 
				Underlined    = 1 << 2, 
				StrikeThrough = 1 << 3  
			};

		HTMLText();

		HTMLText(const String& string, const Font& font, float textWidth, unsigned int characterSize = 30);
		
		void setString(const String& string);
 
		void setFont(const Font& font);

		void setTextWidth(float textWidth);
		
		void setCharacterSize(unsigned int size);
 
		void setLineSpacing(float spacingFactor);
 
		void setLetterSpacing(float spacingFactor);
 
		void setStyle(Uint32 style);
 
		SFML_DEPRECATED void setColor(const Color& color);
 
		void setFillColor(const Color& color);
 
		void setOutlineColor(const Color& color);
 
		void setOutlineThickness(float thickness);
 
		const String& getString() const;
 
		const Font* getFont() const;
 
		float getTextWidth();

		unsigned int getCharacterSize() const;

		float getLetterSpacing() const;
 
		float getLineSpacing() const;
 
		Uint32 getStyle() const;
 
		SFML_DEPRECATED const Color& getColor() const;
 
		const Color& getFillColor() const;
 
		const Color& getOutlineColor() const;
 
		float getOutlineThickness() const;
 
		Vector2f findCharacterPos(std::size_t index) const;
 
		FloatRect getLocalBounds() const;
 
		FloatRect getGlobalBounds() const;
		
	private:

		virtual void draw(RenderTarget& target, RenderStates states) const;
 
		void ensureGeometryUpdate() const;
 
		// Member data
		String              m_string;              
		const Font*         m_font;                
		float               m_textWidth;
		unsigned int        m_characterSize;       
		float               m_letterSpacingFactor; 
		float               m_lineSpacingFactor;   
		Uint32              m_style;               
		Color               m_fillColor;           
		Color               m_outlineColor;        
		float               m_outlineThickness;    
		mutable VertexArray m_vertices;            
		mutable VertexArray m_outlineVertices;     
		mutable FloatRect   m_bounds;              
		mutable bool        m_geometryNeedUpdate;  
		mutable Uint64      m_fontTextureId;
	};
	
} // namespace sf


#endif // SFML_HTML_TEXT
