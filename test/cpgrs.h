
class Generator
{
  public:
	virtual void init    () = 0;
	virtual void compute (int count, double** input, double** output) = 0;

	float on;
};


class CPGRS : public Generator
{
  public:
	float     gain;          // gain
	float     attack;        // interface->addHorizontalSlider("attack",  &fslider4, 0.01f, 0.0f, 1.0f, 0.001f);
	float     sustain;       // interface->addHorizontalSlider("sustain", &fslider1, 0.5f, 0.0f, 1.0f, 0.01f);
	float     release;       // interface->addHorizontalSlider("release", &fslider2, 0.2f, 0.0f, 1.0f, 0.001f);
	float     decay;         // interface->addHorizontalSlider("decay",   &fslider3, 0.3f, 0.0f, 1.0f, 0.001f);
	float     freq;          // "freq", 4.4e+02f, 2e+01f, 2e+04f, 1.0f

  private:
	float     fSamplingFreq;
	float     filter_bandwidth; // 2-filter: addHorizontalSlider("bandwidth (Hz)", &fslider0, 1e+02f, 2e+01f, 2e+04f, 1e+01f);
	float     fConst0;
	float     fConst1;
	float     on_off_button; // gate (on/off)
	int       iRec1[2];
	float     fRec2[2];
	int       iRec3[2];
	float     fVec0[3];
	float     fRec0[3];

	int       t;

  public:

	void init    ();
	void compute (int count, double** input, double** output);
};
