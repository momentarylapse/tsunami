
// types
struct rect
{
public:
	float x1, x2, y1, y2;
	rect(){};
	rect(float x1, float x2, float y1, float y2)
	{	this->x1 = x1;	this->x2 = x2;	this->y1 = y1;	this->y2 = y2;	}
	string str() const
	{	return format("(%f, %f, %f, %f)", x1, x2, y1, y2);	}
};

// rects
const rect r_id = rect(0, 1, 0, 1);
