class RoundedRectangleShape : public sf::Shape
{
	v2 size;
	std::vector<v2> points;
	v4 radius;

	void updateGeometry();
	
public:

	RoundedRectangleShape(v4 r = {}, u32 np = 10) : points(4*np), radius(r), size({r[0]+r[2],r[1]+r[3]}) 
	{ updateGeometry(); }

	v2 getSize() const { return size; }
	void setSize(v2 s) { size = s; updateGeometry(); }
	
	v4 getRadius() const { return radius; }
	void setRadius(v4 r) { radius = r; updateGeometry(); }
	
	std::size_t getPointCount() const override { return points.size(); }
	sf::Vector2f getPoint(std::size_t index) const override { return points[index]; }
};
