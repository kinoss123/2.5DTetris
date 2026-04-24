#ifndef _PLATFORM_
#define _PLATFORM_

class Platform {
  public:
    Platform();
    ~Platform();

    bool ShouldClose();
    int DeltaMs();
};

#endif
