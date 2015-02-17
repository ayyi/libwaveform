#ifndef __generator_h__
#define __generator_h__

class Generator
{
  public:
	virtual void init    () = 0;
	virtual void compute (int count, double** input, double** output) = 0;

	float on;
};


class Note
{
  public:
	int        length;
	Generator* generator;

	Note       (Generator* g, int _length);
	~Note      ();

	void compute (int count, double** input, double** output);

  private:
	int t;
};

#endif
