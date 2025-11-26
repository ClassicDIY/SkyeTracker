#pragma once

struct Position {
 public:
   float Azimuth;
   float Elevation;
};

struct SunsPosition : public Position {
 public:
   bool Dark;
};