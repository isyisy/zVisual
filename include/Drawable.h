#ifndef DRAWABLE_H
#define DRAWABLE_H

class Drawable
{
public:
	Drawable():mIsActive(true){}

	virtual ~Drawable(){}

	virtual void initShader() = 0;

	virtual void updateData() = 0;

	virtual void render() = 0;

	void On(){ mIsActive = true; }

	void Off(){ mIsActive = false; }

protected:

	bool mIsActive;

};

#endif
